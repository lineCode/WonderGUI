
#ifdef WIN32
#	include <SDL.h>
#	include <SDL_image.h>
#else
#	include <SDL2/SDL.h>
#	include <SDL2/SDL_image.h>
#endif

#include <list>

#include <wondergui.h>
#include <fonts/freetype/wg_freetypefont.h>
#include <gfxdevices/software/wg_softgfxdevice.h>
#include <gfxdevices/software/wg_softsurfacefactory.h>

#include <gfxdevices/opengl/wg_glgfxdevice.h>
#include <gfxdevices/opengl/wg_glsurface.h>
#include <gfxdevices/opengl/wg_glsurfacefactory.h>

#include <wg_fileutil.h>

#include <testunit.h>

using namespace wg;
using namespace std;

enum class DisplayMode
{
	Testee,
	Reference,
	Both,
	Diff,
	Time
};

enum Device
{
	TESTEE = 0,
	REFERENCE = 1
};


struct TestDevice
{
	TestUnit * pUnit = nullptr;
	bool	bWorking = false;
	double	render_time = 0;						// Seconds to render number of rounds
	double	stalling_time = 0;						// Seconds for endRender() call afterwards. OpenGL stalls here. 
};

struct Test
{
	bool	bSelected = false;

	TestDevice devices[2];							// Testee, followed by Reference
};

const Size	g_canvasSize(512, 512);
const Size	g_windowSize(1400, 900);



bool		init_system( Rect windowGeo );
void		exit_system();

bool		process_system_events(const RootPanel_p& pRoot);
int64_t		getSystemTicks();
void		update_window_rects( const Rect * pRects, int nRects );

bool		init_wondergui();
void		exit_wondergui();

bool		setup_chrome();
void		teardown_chrome();

void		setup_testdevices();
void		destroy_testdevices();

void		setup_tests();
bool		add_test(TestUnit * pTesteeTest, TestUnit * pReferenceTest);
void		run_tests( GfxDevice * pDevice, Device device );
void		clock_test(TestDevice * pTestDevice, int rounds, GfxDevice * pDevice);
void		destroy_tests();

void		update_displaymode();
void		display_test_results();


MouseButton translateSDLMouseButton(Uint8 button);
Canvas_p create_canvas(GfxDevice_p pDevice);
void refresh_performance_display();
void refresh_performance_measurements();


SDL_Window *		g_pSDLWindow = nullptr;

Surface_p			g_pWindowSurface = nullptr;				// Set by init_system()
GfxDevice_p			g_pBaseGfxDevice = nullptr;
SurfaceFactory_p	g_pBaseSurfaceFactory = nullptr;
RootPanel_p			g_pRoot = nullptr;
ScrollPanel_p		g_pViewPanel = nullptr;

GfxDevice_p			g_pTesteeDevice = nullptr;
GfxDevice_p			g_pReferenceDevice = nullptr;

Canvas_p			g_pTesteeCanvas = nullptr;
Canvas_p			g_pReferenceCanvas = nullptr;

Widget_p			g_pPerformanceDisplay = nullptr;
PackList_p			g_pPerformanceList = nullptr;
TextMapper_p		g_pPerformanceValueMapper = nullptr;
SizeBroker_p		g_pPerformanceEntryBroker = nullptr;

vector<GfxDevice_p>	g_testdevices;
vector<Test>		g_tests;

TextStyle_p			g_pButtonLabelStyle = nullptr;
TextMapper_p		g_pButtonLabelMapper = nullptr;

DisplayMode			g_displayMode = DisplayMode::Testee;
float				g_zoomFactor = 1.f;

bool				g_bRedrawTestee = true;
bool				g_bRedrawReference = true;
bool				g_bRefreshPerformance = false;
//____ main() _________________________________________________________________

int main(int argc, char *argv[] )
{

	if (!init_system({ 20,20, g_windowSize }))
		return -1;
	
	if (!init_wondergui() )
		return -1;

	setup_testdevices();
	setup_tests();

	if (!setup_chrome())
		return -1;

	update_displaymode();

	bool bContinue = true;

	int64_t prevTicks = getSystemTicks();

	while (bContinue)
	{
		// Handle system events

		bContinue = process_system_events(g_pRoot);
		Base::msgRouter()->dispatch();

		// Run tests

		if (g_bRefreshPerformance)
		{
			refresh_performance_measurements();
			refresh_performance_display();
			g_bRefreshPerformance = false;
		}

		if (g_displayMode != DisplayMode::Time)
		{
			if (g_bRedrawTestee)
			{
				run_tests(g_pTesteeDevice, TESTEE);
				g_bRedrawTestee = false;
			}

			if (g_bRedrawReference)
			{
				run_tests(g_pReferenceDevice, REFERENCE);
				g_bRedrawReference = false;
			}

			display_test_results();
		}

		// Render

		g_pRoot->render();

		//

		SDL_Delay(10);

		// Update time

		int64_t ticks = getSystemTicks();
		Base::msgRouter()->post(TickMsg::create(ticks, (int)(ticks-prevTicks)));
		prevTicks = ticks;
	}

	teardown_chrome();

	destroy_tests();
	destroy_testdevices();

	exit_wondergui();
	exit_system();
	return 0;
}

//____ update_displaymode() ______________________________________________________

void update_displaymode()
{
	Widget_p viewChild;

	// Setup view panel


	switch (g_displayMode)
	{
		case DisplayMode::Testee:
			g_pTesteeCanvas = create_canvas(g_pTesteeDevice);
			g_pReferenceCanvas = nullptr;
			viewChild = g_pTesteeCanvas;
			break;

		case DisplayMode::Reference:
			g_pTesteeCanvas = nullptr;
			g_pReferenceCanvas = create_canvas(g_pReferenceDevice);
			viewChild = g_pReferenceCanvas;
			break;

		case DisplayMode::Both:
		{
			g_pTesteeCanvas = create_canvas(g_pTesteeDevice);
			g_pReferenceCanvas = create_canvas(g_pReferenceDevice);

			auto pPack = PackPanel::create();
			pPack->setOrientation(Orientation::Horizontal);
			pPack->children << g_pTesteeCanvas;
			pPack->children << g_pReferenceCanvas;
			viewChild = pPack;

			break;
		}
		case DisplayMode::Diff:
			break;
		case DisplayMode::Time:
			refresh_performance_display();
			viewChild = g_pPerformanceDisplay;
			break;
	}

	// Set zoom factor

	if (g_zoomFactor != 1.f)
	{
		Size size{ (int) (512 * g_zoomFactor), (int) (512 * g_zoomFactor) };

		if (g_pTesteeCanvas)
			g_pTesteeCanvas->canvas.setPresentationScaling(SizePolicy2D::Scale);

		if (g_pReferenceCanvas)
			g_pReferenceCanvas->canvas.setPresentationScaling(SizePolicy2D::Scale);


		auto pZoomer = SizeCapsule::create();
		pZoomer->setSizes(size, size, size);
		pZoomer->child = viewChild;
		viewChild = pZoomer;

	}

	//

	g_pViewPanel->view = viewChild;

}

//____ refresh_performance_display() __________________________________________

void refresh_performance_display()
{
	g_pPerformanceList->children.clear();

	for (Test t : g_tests)
	{
		if (t.bSelected)
		{
			auto pEntry = PackPanel::create();
			pEntry->setOrientation(Orientation::Horizontal);
			pEntry->setSizeBroker(g_pPerformanceEntryBroker);

			auto pLabel = TextDisplay::create();
			pLabel->text.set(t.devices[TESTEE].pUnit->name());

//			auto pValue = ValueDisplay::create();
//			pValue->value.set(t.devices[TESTEE].render_time*1000);

			auto pValuePacker = PackPanel::create();
			pValuePacker->setOrientation(Orientation::Vertical);

			char value[128];

			auto pValueTestee = TextDisplay::create();
			sprintf_s(value, 128, " %.1f ms", t.devices[TESTEE].render_time * 1000);
			pValueTestee->text.set(value);
			pValueTestee->text.setTextMapper(g_pPerformanceValueMapper);

			auto pValueRef = TextDisplay::create();
			sprintf_s(value, 128, " %.1f ms", t.devices[REFERENCE].render_time * 1000);
			pValueRef->text.set(value);
			pValueRef->text.setTextMapper(g_pPerformanceValueMapper);

			pValuePacker->children << pValueTestee;
			pValuePacker->children << pValueRef;

			pEntry->children << pLabel;
			pEntry->children << pValuePacker;

//			std::map<int,Widget_p> v;
//			v[1] = Button::create();

			std::vector<Widget_p> v;
			v.push_back(Button::create());

			Widget_p * pBegin = nullptr;
			Widget_p * pEnd = nullptr;

//			pEntry->children.add(1, 2);
			pEntry->children.add(v.begin(), v.end());

			pEntry->children.setWeight(0, 2, { 0.f,1.f });
//			pEntry->children.setPadding(1, { 0,0,0,10 });

			g_pPerformanceList->children << pEntry;
		}
	}
}


//____ create_canvas() _______________________________________________________

Canvas_p create_canvas(GfxDevice_p pDevice)
{
	auto pCanvas = Canvas::create();
	pCanvas->canvas.setSize(g_canvasSize);
	pCanvas->canvas.setDevice(GlGfxDevice::create({ 0,0,10,10 }));
	pCanvas->canvas.setBackColor(Color::Black);
	return pCanvas;
}

//____ display_test_results() _________________________________________________

void display_test_results()
{
	switch (g_displayMode)
	{
		case DisplayMode::Testee:
			g_pTesteeCanvas->canvas.surface()->copyFrom(g_pTesteeDevice->canvas(), { 0,0 } );
			g_pTesteeCanvas->canvas.present();
			break;

		case DisplayMode::Reference:
			g_pReferenceCanvas->canvas.surface()->copyFrom(g_pReferenceDevice->canvas(), { 0,0 });
			g_pReferenceCanvas->canvas.present();
			break;

		case DisplayMode::Both:
			g_pTesteeCanvas->canvas.surface()->copyFrom(g_pTesteeDevice->canvas(), { 0,0 });
			g_pTesteeCanvas->canvas.present();

			g_pReferenceCanvas->canvas.surface()->copyFrom(g_pReferenceDevice->canvas(), { 0,0 });
			g_pReferenceCanvas->canvas.present();
			break;

		case DisplayMode::Diff:
			break;

		case DisplayMode::Time:
			break;
	}
}


//____ run_tests() ____________________________________________________________

void run_tests(GfxDevice * pDevice, Device device)
{
	if (!pDevice)
		return;

	pDevice->beginRender();
	pDevice->fill(g_canvasSize, Color::Black);

	for (auto& test : g_tests)
	{
		if (test.bSelected)
			test.devices[device].pUnit->run(pDevice, g_canvasSize);
	}

	pDevice->endRender();
}


//____ refresh_performance_measurements() ___________________________________________________

void refresh_performance_measurements()
{
	for (auto& test : g_tests)
	{
		if (test.bSelected)
		{
			clock_test(&test.devices[TESTEE], 1000, g_pTesteeDevice);
			clock_test(&test.devices[REFERENCE], 1000, g_pReferenceDevice);
		}
	}
}


//____ clock_test() ___________________________________________________________

void clock_test(TestDevice * pTestDevice, int rounds, GfxDevice * pDevice )
{
	Uint64 first = SDL_GetPerformanceCounter();
	Uint64 start;
	Uint64 end;
	Uint64 stall;

	pDevice->beginRender();

	do { start = SDL_GetPerformanceCounter(); } while (start == first);

	for( int i = 0 ; i < rounds ; i++ )
		pTestDevice->pUnit->run(pDevice, g_canvasSize);

	end = SDL_GetPerformanceCounter();

	pDevice->endRender();

	stall = SDL_GetPerformanceCounter();

	Uint64 freq = SDL_GetPerformanceFrequency();

	pTestDevice->render_time = (end - start) / (double)freq;
	pTestDevice->stalling_time = (stall - end) / (double)freq;
}



//____ setup_testdevices() ____________________________________________________

void setup_testdevices()
{
	auto pSoftCanvas = SoftSurface::create(g_canvasSize);
	auto pSoft = SoftGfxDevice::create(pSoftCanvas);

	auto pOpenGLCanvas = GlSurface::create(g_canvasSize);
	auto pOpenGL = GlGfxDevice::create(pOpenGLCanvas);

//	auto pSoft2 = SoftGfxDevice::create();
//	auto pOpenGL = GlGfxDevice::create( Size(256,256) );
//	auto pOpenGL2 = GlGfxDevice::create(Size(256, 256));

	g_testdevices.push_back(pSoft);
//	g_testdevices.push_back(pSoft2);

	g_testdevices.push_back(pOpenGL);
//	g_testdevices.push_back(pOpenGL2);

	g_pReferenceDevice = pSoft;
	g_pTesteeDevice = pOpenGL;

}

//____ destroy_testdevices() ____________________________________________________

void destroy_testdevices()
{
	g_testdevices.clear();

	g_pTesteeDevice = nullptr;
	g_pReferenceDevice = nullptr;
}

//____ setup_tests() ____________________________________________________

void setup_tests()
{
	destroy_tests();

	add_test( new test::StraightFill(), new test::StraightFill());
	add_test( new test::BlendFill(), new test::BlendFill());
	add_test( new test::OffscreenBGRACanvas(), new test::OffscreenBGRACanvas());
	add_test(new test::StretchBlitBlends(), new test::StretchBlitBlends());
	add_test(new test::DrawToBGR_8(), new test::DrawToBGR_8());
	add_test(new test::DrawToBGRA_8(), new test::DrawToBGRA_8());
	add_test(new test::DrawToBGRX_8(), new test::DrawToBGRX_8());
	add_test(new test::DrawToBGRA_4(), new test::DrawToBGRA_4());
	add_test(new test::DrawToBGR_565(), new test::DrawToBGR_565());
}

//____ add_test() _____________________________________________________________

bool add_test(TestUnit * pTesteeTest, TestUnit * pReferenceTest)
{
	Test t;
	t.devices[TESTEE].pUnit = pTesteeTest;
	t.devices[TESTEE].bWorking = pTesteeTest->init(g_pTesteeDevice, g_canvasSize);

	t.devices[REFERENCE].pUnit = pReferenceTest;
	t.devices[REFERENCE].bWorking = pReferenceTest->init(g_pReferenceDevice, g_canvasSize);

	g_tests.push_back(t);
	return (t.devices[TESTEE].bWorking && t.devices[REFERENCE].bWorking );
}


//____ destroy_tests() ____________________________________________________

void destroy_tests()
{
	for (auto& test : g_tests)
	{
		delete test.devices[TESTEE].pUnit;
		delete test.devices[REFERENCE].pUnit;
	}

	g_tests.clear();
}

//____ update_window_rects() __________________________________________________

void update_window_rects(const Rect * pRects, int nRects)
{
	if (nRects == 0)
		return;

	std::vector<SDL_Rect>	rects;

	for (int i = 0; i < nRects; i++)
	{
		SDL_Rect r = { pRects[i].x, pRects[i].y, pRects[i].w, pRects[i].h };
		rects.push_back(r);
	}

	SDL_UpdateWindowSurfaceRects(g_pSDLWindow, &rects.front(), nRects);
}


//____ setup_chrome() _________________________________________________________

bool setup_chrome()
{
	// Load resources

	auto pPlateSurface = FileUtil::loadSurface("../resources/grey_plate.bmp", g_pBaseSurfaceFactory );
	BlockSkin_p pPlateSkin = BlockSkin::createStaticFromSurface(pPlateSurface, Border(3));
	pPlateSkin->setContentPadding(Border(5));

	auto pPressablePlateSurface = FileUtil::loadSurface("../resources/grey_pressable_plate.bmp", g_pBaseSurfaceFactory );
	BlockSkin_p pPressablePlateSkin = BlockSkin::createClickableFromSurface(pPressablePlateSurface, 0, Border(3));
	pPressablePlateSkin->setContentPadding(Border(3));

	auto pButtonSurface = FileUtil::loadSurface("../resources/simple_button.bmp", g_pBaseSurfaceFactory );
	BlockSkin_p pSimpleButtonSkin = BlockSkin::createClickableFromSurface(pButtonSurface, 0, Border(3));
	pSimpleButtonSkin->setContentPadding(Border(5));

	// Setup base layers

	auto pLayerStack = StackPanel::create();
	pLayerStack->setSkin(ColorSkin::create(Color::AntiqueWhite));
	g_pRoot->child = pLayerStack;

	// Divid screen into sidebar and canvaspanel with top section

	auto pMidSection = PackPanel::create();
	pMidSection->setOrientation(Orientation::Horizontal);
	pMidSection->setSizeBroker(UniformSizeBroker::create());
	auto it = pLayerStack->children << pMidSection;
	pLayerStack->children.setSizePolicy(it, SizePolicy2D::Stretch);

	auto pSidebar = PackPanel::create();
	pSidebar->setOrientation(Orientation::Vertical);
	pSidebar->setSkin(pPlateSkin);

	auto pCanvasPanel = PackPanel::create();
	pCanvasPanel->setOrientation(Orientation::Vertical);
	pCanvasPanel->setSizeBroker(UniformSizeBroker::create());

	auto pViewNav = PackPanel::create();
	pViewNav->setOrientation(Orientation::Horizontal);
	pViewNav->setSizeBroker(UniformSizeBroker::create());
	pViewNav->setSkin( pPlateSkin );

	auto pViewPanel = ScrollPanel::create();
	pViewPanel->setSkin( ColorSkin::create(Color::SlateGrey) );
	g_pViewPanel = pViewPanel;

	pMidSection->children << pCanvasPanel;
	pMidSection->children << pSidebar;

	pMidSection->children.setWeight(0, 1.f);
	pMidSection->children.setWeight(1, 0.f);


	pCanvasPanel->children << pViewNav;
	pCanvasPanel->children << pViewPanel;

	pCanvasPanel->children.setWeight(0, 0.f);
	pCanvasPanel->children.setWeight(1, 1.f);


	// Setup view navigator


	auto pLeftFiller = Filler::create();
	auto pDispModeSection = PackPanel::create();
	auto pRightFiller = Filler::create();
	auto pDispZoomSection = PackPanel::create();

	pViewNav->children << pLeftFiller;
	pViewNav->children << pDispModeSection;
	pViewNav->children << pRightFiller;
	pViewNav->children << pDispZoomSection;

	pViewNav->children.setWeight(0, 1.f);
	pViewNav->children.setWeight(1, 0.f);
	pViewNav->children.setWeight(2, 1.f);
	pViewNav->children.setWeight(3, 0.f);

	// Setup display mode section

	pDispModeSection->setOrientation(Orientation::Horizontal);
	pDispModeSection->setSizeBroker(UniformSizeBroker::create());


	auto pTesteeButton = Button::create();
	pTesteeButton->setSkin(pSimpleButtonSkin);
	pTesteeButton->label.set("Testee");
	pTesteeButton->label.setStyle(g_pButtonLabelStyle);
	pTesteeButton->label.setTextMapper(g_pButtonLabelMapper);

	auto pRefButton = Button::create();
	pRefButton->setSkin(pSimpleButtonSkin);
	pRefButton->label.set("Reference");
	pRefButton->label.setStyle(g_pButtonLabelStyle);
	pRefButton->label.setTextMapper(g_pButtonLabelMapper);

	auto pBothButton = Button::create();
	pBothButton->setSkin(pSimpleButtonSkin);
	pBothButton->label.set("Both");
	pBothButton->label.setStyle(g_pButtonLabelStyle);
	pBothButton->label.setTextMapper(g_pButtonLabelMapper);

	auto pDiffButton = Button::create();
	pDiffButton->setSkin(pSimpleButtonSkin);
	pDiffButton->label.set("Diff");
	pDiffButton->label.setStyle(g_pButtonLabelStyle);
	pDiffButton->label.setTextMapper(g_pButtonLabelMapper);

	auto pTimeButton = Button::create();
	pTimeButton->setSkin(pSimpleButtonSkin);
	pTimeButton->label.set("Time");
	pTimeButton->label.setStyle(g_pButtonLabelStyle);
	pTimeButton->label.setTextMapper(g_pButtonLabelMapper);

	pDispModeSection->children	<< pTesteeButton;
	pDispModeSection->children	<< pRefButton;
	pDispModeSection->children	<< pBothButton;
	pDispModeSection->children	<< pDiffButton;
	pDispModeSection->children  << pTimeButton;

	Base::msgRouter()->addRoute(pTesteeButton, MsgType::Select, [](Msg* pMsg) {
		g_displayMode = DisplayMode::Testee;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pRefButton, MsgType::Select, [](Msg* pMsg) {
		g_displayMode = DisplayMode::Reference;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pBothButton, MsgType::Select, [](Msg* pMsg) {
		g_displayMode = DisplayMode::Both;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pDiffButton, MsgType::Select, [](Msg* pMsg) {
		g_displayMode = DisplayMode::Diff;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pTimeButton, MsgType::Select, [](Msg* pMsg) {
		g_displayMode = DisplayMode::Time;
		update_displaymode();
	});

	// Setup display zoom section

	pDispModeSection->setOrientation(Orientation::Horizontal);
	pDispModeSection->setSizeBroker(UniformSizeBroker::create());

	auto pX1Button = Button::create();
	pX1Button->setSkin(pSimpleButtonSkin);
	pX1Button->label.set(" X1 ");
	pX1Button->label.setStyle(g_pButtonLabelStyle);
	pX1Button->label.setTextMapper(g_pButtonLabelMapper);

	auto pX2Button = Button::create();
	pX2Button->setSkin(pSimpleButtonSkin);
	pX2Button->label.set(" X2 ");
	pX2Button->label.setStyle(g_pButtonLabelStyle);
	pX2Button->label.setTextMapper(g_pButtonLabelMapper);

	auto pX4Button = Button::create();
	pX4Button->setSkin(pSimpleButtonSkin);
	pX4Button->label.set(" X4 ");
	pX4Button->label.setStyle(g_pButtonLabelStyle);
	pX4Button->label.setTextMapper(g_pButtonLabelMapper);

	auto pX8Button = Button::create();
	pX8Button->setSkin(pSimpleButtonSkin);
	pX8Button->label.set(" X8 ");
	pX8Button->label.setStyle(g_pButtonLabelStyle);
	pX8Button->label.setTextMapper(g_pButtonLabelMapper);

	pDispZoomSection->children << pX1Button;
	pDispZoomSection->children << pX2Button;
	pDispZoomSection->children << pX4Button;
	pDispZoomSection->children << pX8Button;

	Base::msgRouter()->addRoute(pX1Button, MsgType::Select, [](Msg* pMsg) {
		g_zoomFactor = 1.f;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pX2Button, MsgType::Select, [](Msg* pMsg) {
		g_zoomFactor = 2.f;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pX4Button, MsgType::Select, [](Msg* pMsg) {
		g_zoomFactor = 4.f;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pX8Button, MsgType::Select, [](Msg* pMsg) {
		g_zoomFactor = 8.f;
		update_displaymode();
	});

	// Setup sidebar

	auto pTestList = PackList::create();
	pTestList->setSelectMode(SelectMode::FlipOnSelect);

	Base::msgRouter()->addRoute(pTestList, MsgType::ItemsSelect, [&](Msg* _pMsg) {

		auto pMsg = ItemsSelectMsg::cast(_pMsg);
		auto p = pMsg->items();
		for (int x = 0; x < pMsg->nbItems(); x++)
			g_tests[p->id].bSelected = true;

		g_bRedrawTestee = true;
		g_bRedrawReference = true;
	});

	Base::msgRouter()->addRoute(pTestList, MsgType::ItemsUnselect, [&](Msg* _pMsg) {

		auto pMsg = ItemsUnselectMsg::cast(_pMsg);
		auto p = pMsg->items();
		for (int x = 0; x < pMsg->nbItems(); x++)
			g_tests[p->id].bSelected = false;

		g_bRedrawTestee = true;
		g_bRedrawReference = true;
	});




	auto pSkin = BoxSkin::create(Color::White, { 1 }, Color::Black);
	pSkin->setContentPadding(8);

	auto pEntrySkin = BoxSkin::create(Color::White, { 1 }, Color::Black);
	pEntrySkin->setContentPadding(8);
	pEntrySkin->setStateColor(StateEnum::Hovered, Color::AntiqueWhite);
	pEntrySkin->setStateColor(StateEnum::HoveredSelected, Color::Aquamarine);
	pEntrySkin->setStateColor(StateEnum::Selected, Color::Aquamarine);

	pTestList->setSkin(pSkin);
	pTestList->setEntrySkin(pEntrySkin);

	int id = 0;
	for (Test&  test : g_tests)
	{
		string name = test.devices[TESTEE].pUnit->name();

		auto pEntry = TextDisplay::create();
		pEntry->setId(id++);
		pEntry->text.set(name);
		pTestList->children.add(pEntry);
	}
	
	pSidebar->children << pTestList;


	// Setup performance display

	{
		auto pMapper = StdTextMapper::create();
		pMapper->setAlignment(Origo::East);
		g_pPerformanceValueMapper = pMapper;

		auto pBroker = UniformSizeBroker::create();
		g_pPerformanceEntryBroker = pBroker;


		auto pBase = PackPanel::create();
		pBase->setOrientation(Orientation::Vertical);

		auto pList = PackList::create();
		pList->setSkin(ColorSkin::create(Color::White));

		auto pOddEntrySkin = BoxSkin::create(Color::White, Border(0), Color::White);
		pOddEntrySkin->setContentPadding(Border(0));

		auto pEvenEntrySkin = BoxSkin::create(Color::PaleGreen, Border(0), Color::PaleGreen);
		pEvenEntrySkin->setContentPadding(Border(0));

		pList->setEntrySkin(pOddEntrySkin, pEvenEntrySkin);


		// Create the bottom section

		auto pBottom = PackPanel::create();
		pBottom->setSkin(pPlateSkin);
		pBottom->setOrientation(Orientation::Horizontal);
		pBottom->setSizeBroker(UniformSizeBroker::create());

		auto pRefresh = Button::create();
		pRefresh->setSkin(pSimpleButtonSkin);
		pRefresh->label.set("REFRESH");
		pRefresh->label.setStyle(g_pButtonLabelStyle);
		pRefresh->label.setTextMapper(g_pButtonLabelMapper);

		Base::msgRouter()->addRoute(pRefresh, MsgType::Select, [](Msg * pMsg) {
			g_bRefreshPerformance = true;
		});


		auto pFiller = Filler::create();

		auto it = pBottom->children << pFiller;
		pBottom->children << pRefresh;

		//

		pBase->children << pList;
		pBase->children << pBottom;

		g_pPerformanceDisplay = pBase;
		g_pPerformanceList = pList;
	}

	return true;
}

//____ teardown_chrome() ______________________________________________________

void teardown_chrome()
{
}


//____ init_wondergui() _______________________________________________________

bool init_wondergui()
{
	Base::init();

	g_pBaseSurfaceFactory = GlSurfaceFactory::create();
//	g_pBaseSurfaceFactory = SoftSurfaceFactory::create();

	g_pBaseGfxDevice = GlGfxDevice::create(g_windowSize);
//	g_pBaseGfxDevice = SoftGfxDevice::create( SoftSurface::cast(g_pWindowSurface));
	g_pRoot = RootPanel::create(g_pBaseGfxDevice);

	FreeTypeFont::init(g_pBaseSurfaceFactory);

	// Init textmappers

	auto pMapper = StdTextMapper::create();
	pMapper->setAlignment(Origo::Center);
	g_pButtonLabelMapper = pMapper;

	// Init fonts

	Blob_p pFontFile = FileUtil::loadBlob("../resources/DroidSans.ttf");
	FreeTypeFont_p pFont = FreeTypeFont::create(pFontFile, 1);

	TextStyle_p pStyle = TextStyle::create();
	pStyle->setFont(pFont);
	pStyle->setSize(16);
	pStyle->setColor(Color::Black);
	Base::setDefaultStyle(pStyle);

	g_pButtonLabelStyle = TextStyle::create();
	g_pButtonLabelStyle->setFont(pFont);
	g_pButtonLabelStyle->setSize(16);
	g_pButtonLabelStyle->setColor(Color::Black);
	Base::setDefaultStyle(pStyle);


	return true;
}

//____ exit_wondergui() _______________________________________________________

void exit_wondergui()
{
	g_pButtonLabelStyle = nullptr;
	g_pRoot = nullptr;
	g_pBaseGfxDevice = nullptr;
	g_pBaseSurfaceFactory = nullptr;

	g_pViewPanel = nullptr;
	g_pTesteeCanvas = nullptr;
	g_pReferenceCanvas = nullptr;

	FreeTypeFont::exit();
	Base::exit();
}


//____ init_system() _______________________________________________________

bool init_system( Rect windowGeo )
{
	// initialize SDL video
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Unable to init SDL: %s\n", SDL_GetError());
		return false;
	}

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	SDL_Window * pWin = SDL_CreateWindow("GfxDevice TestApp", windowGeo.x, windowGeo.y, windowGeo.w, windowGeo.h, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	if( pWin == nullptr )
	{
		printf("Unable to create SDL window: %s\n", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);

	SDL_GLContext context = SDL_GL_CreateContext(pWin);

	SDL_GL_SetSwapInterval(1);

	//	SDL_Renderer * pRenderer = SDL_CreateRenderer(pWin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

#ifdef WIN32  
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
#endif

	glDrawBuffer(GL_FRONT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();




	SDL_Surface * pWinSurf = SDL_GetWindowSurface(pWin);
	if (pWinSurf == nullptr)
	{
		printf("Unable to get window SDL Surface: %s\n", SDL_GetError());
		return false;
	}

	PixelFormat format = PixelFormat::Unknown;

	switch (pWinSurf->format->BitsPerPixel)
	{
		case 32:
			format = PixelFormat::BGRA_8;
			break;
		case 24:
			format = PixelFormat::BGR_8;
			break;
		default:
		{
			printf("Unsupported pixelformat of SDL Surface!\n");
			return false;
		}
	}

	g_pSDLWindow = pWin;

	Blob_p pCanvasBlob = Blob::create(pWinSurf->pixels, 0);
	g_pWindowSurface = SoftSurface::create(Size(pWinSurf->w, pWinSurf->h), format, pCanvasBlob, pWinSurf->pitch);

	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

	return true;
}

//____ exit_system() __________________________________________________________

void exit_system()
{
	IMG_Quit();
}

//____ getSystemTicks() ________________________________________________________

int64_t getSystemTicks()
{
	return SDL_GetPerformanceCounter() * 1000 / SDL_GetPerformanceFrequency();
}


//____ process_system_events() ________________________________________________

bool process_system_events(const RootPanel_p& pRoot)
{
	// Process all the SDL events in a loop

	InputHandler_p pInput = Base::inputHandler();

	SDL_Event e;
	while (SDL_PollEvent(&e)) {

		switch (e.type)
		{
		case SDL_QUIT:
			return false;

		case SDL_MOUSEMOTION:
			pInput->setPointer(pRoot, Coord(e.motion.x, e.motion.y));
			break;

		case SDL_MOUSEBUTTONDOWN:
			pInput->setButton(translateSDLMouseButton(e.button.button), true);
			break;

		case SDL_MOUSEBUTTONUP:
			pInput->setButton(translateSDLMouseButton(e.button.button), false);
			break;

		case SDL_MOUSEWHEEL:
		{
			Coord distance(e.wheel.x, e.wheel.y);
			if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
				distance *= -1;

			pInput->setWheelRoll(1, distance);
			break;
		}

		case SDL_KEYDOWN:
		{
			pInput->setKey(e.key.keysym.sym, true);
			break;
		}

		case SDL_KEYUP:
		{
			pInput->setKey(e.key.keysym.sym, false);
			break;
		}

		case SDL_TEXTINPUT:
			pInput->putText(e.text.text);
			break;


		default:
			break;
		}
	}

	return true;
}

//____ translateSDLMouseButton() __________________________________________________
//
// Translate SDL mouse button enums to WonderGUI equivalents.
//
MouseButton translateSDLMouseButton(Uint8 button)
{
	switch (button)
	{
	default:
	case SDL_BUTTON_LEFT:
		return MouseButton::Left;
	case SDL_BUTTON_MIDDLE:
		return MouseButton::Middle;
	case SDL_BUTTON_RIGHT:
		return MouseButton::Right;
	case SDL_BUTTON_X1:
		return MouseButton::X1;
	case SDL_BUTTON_X2:
		return MouseButton::X2;
	}
}



