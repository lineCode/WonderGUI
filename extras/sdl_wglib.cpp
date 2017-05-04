#include <sdl_wglib.h>
//#include <wg_surface_sdl.h>
#include <wg_surface_soft.h>
#include <wondergui.h>


namespace sdl_wglib
{
	static WgEventHandler * g_pHandler;
	static int				g_ticks = 0;

	//____ MapKeys() ___________________________________________________________

	void MapKeys()
	{
		WgBase::MapKey( WG_KEY_SHIFT, SDLK_LSHIFT );
		WgBase::MapKey( WG_KEY_SHIFT, SDLK_RSHIFT );
		WgBase::MapKey( WG_KEY_CONTROL, SDLK_LCTRL );
		WgBase::MapKey( WG_KEY_CONTROL, SDLK_RCTRL );
		WgBase::MapKey( WG_KEY_ALT, SDLK_LALT );
		WgBase::MapKey( WG_KEY_ALT, SDLK_RALT );

		WgBase::MapKey( WG_KEY_LEFT, SDLK_LEFT );
		WgBase::MapKey( WG_KEY_RIGHT, SDLK_RIGHT );
		WgBase::MapKey( WG_KEY_UP, SDLK_UP );
		WgBase::MapKey( WG_KEY_DOWN, SDLK_DOWN );

		WgBase::MapKey( WG_KEY_HOME, SDLK_HOME );
		WgBase::MapKey( WG_KEY_END, SDLK_END );
		WgBase::MapKey( WG_KEY_PAGE_UP, SDLK_PAGEUP );
		WgBase::MapKey( WG_KEY_PAGE_DOWN, SDLK_PAGEDOWN );

		WgBase::MapKey( WG_KEY_RETURN, SDLK_RETURN );
		WgBase::MapKey( WG_KEY_BACKSPACE, SDLK_BACKSPACE );
		WgBase::MapKey( WG_KEY_DELETE, SDLK_DELETE );
		WgBase::MapKey( WG_KEY_TAB, SDLK_TAB );
		WgBase::MapKey( WG_KEY_ESCAPE, SDLK_ESCAPE );
	}

	//____ BeginEvents() _______________________________________________________

	void BeginEvents( WgEventHandler * pHandler )
	{
		g_pHandler = pHandler;

		// Add a tick event first as the first.

		int ticks = SDL_GetTicks();
		pHandler->QueueEvent( new WgEvent::Tick( ticks - g_ticks ) );
		g_ticks = ticks;
	}

	//____ TranslateEvent() ____________________________________________________

	void TranslateEvent( SDL_Event& event )
	{
		switch (event.type)
		{
			// check for keypresses
			case SDL_KEYDOWN:
			{
				g_pHandler->QueueEvent( new WgEvent::KeyPress( event.key.keysym.sym ) );
//				if( event.key.keysym.unicode != 0 )
//					g_pHandler->QueueEvent( new WgEvent::Character( event.key.keysym.unicode ) );
				break;
			}

			case SDL_KEYUP:
			{
				g_pHandler->QueueEvent( new WgEvent::KeyRelease( event.key.keysym.sym ) );
				break;
			}

			case	SDL_MOUSEMOTION:
			{
				g_pHandler->QueueEvent( new WgEvent::MouseMove( WgCoord( event.motion.x, event.motion.y ) ) );
				break;
			}

			case	SDL_MOUSEBUTTONDOWN:
				if(event.button.button == 4 )
					g_pHandler->QueueEvent( new WgEvent::MouseWheelRoll( 1, 1 ) );
				else if(event.button.button == 5)
					g_pHandler->QueueEvent( new WgEvent::MouseWheelRoll( 1, -1 ) );
				else
				{
					g_pHandler->QueueEvent( new WgEvent::MouseButtonPress( event.button.button ) );
				}
				break;

			case	SDL_MOUSEBUTTONUP:
				if( event.button.button != 4 && event.button.button != 5 )
					g_pHandler->QueueEvent( new WgEvent::MouseButtonRelease( event.button.button ) );
				break;
		}
	}

	//_____ EndEvents() ________________________________________________________

	void EndEvents()
	{
		g_pHandler->ProcessEvents();
	}

	//____ LoadSurface() __________________________________________________________

	WgSurface * LoadSurface( const char * path, const WgSurfaceFactory& factory )
	{
		SDL_Surface* bmp = IMG_Load(path);
		if (!bmp)
		{
			printf("Unable to load bitmap: %s\n", IMG_GetError());
			return 0;
		}

//		WgSurfaceSDL	wrapper( bmp );

		WgPixelType type;
		
		if( bmp->format->Amask == 0 )
			type = WG_PIXEL_BGR_8;
		else
			type = WG_PIXEL_BGRA_8;

		WgSize dimensions( bmp->w, bmp->h );

		WgSurface * pSurf = factory.CreateSurface( dimensions, type );

		if( !pSurf )
		{
			printf("Unable to create surface for loaded bitmap: %s\n", path);
			return 0;
		}

		
		WgPixelFormat	pixelFormat;
		ConvertPixelFormat( &pixelFormat, bmp->format );

		if( !pSurf->CopyFrom( &pixelFormat, (uint8_t*) bmp->pixels, bmp->pitch, dimensions, dimensions) )
		{
			printf("Unable to copy loaded bitmap '%s' to surface.\n", path);
			delete pSurf;
			return 0;
		};

		return pSurf;
	}

	//____ LoadStdWidgets() _____________________________________________________

	WgResDB * LoadStdWidgets( const char * pImagePath, const WgSurfaceFactory& factory )
	{
		const int HSLIDER_BTN_OFS 		= 1;
		const int VSLIDER_BTN_OFS 		= HSLIDER_BTN_OFS + 19;
		const int SLIDER_OFS 			= VSLIDER_BTN_OFS + 19;
		const int SLIDER_BACK_OFS		= SLIDER_OFS + 10;
		const int RESIZE_BUTTON_OFS 	= SLIDER_BACK_OFS + 13;
		const int CHECKBOX_OFS 			= RESIZE_BUTTON_OFS + 22;
		const int RADIOBUTTON_OFS 		= CHECKBOX_OFS + 13;
		const int BUTTON_OFS			= RADIOBUTTON_OFS + 13;
		const int PLATE_OFS				= BUTTON_OFS + 10;
		const int SPLITS_AND_FRAME_OFS	= PLATE_OFS + 10;
		const int COMBOBOX_OFS			= SPLITS_AND_FRAME_OFS + 10;
		const int TILES_OFS				= 192;

		WgSurface * pSurface = LoadSurface( pImagePath, factory );
		if( !pSurface )
			return 0;

		WgBlocksetPtr pHSliderBtnBwdBlocks	= WgBlockset::CreateFromRow(pSurface, WgRect(1,HSLIDER_BTN_OFS,74,17), 4, 2, WG_OPAQUE);
		pHSliderBtnBwdBlocks->SetFrame(WgBorders(3));
		pHSliderBtnBwdBlocks->SetPadding(WgBorders(4));

		WgBlocksetPtr pHSliderBtnFwdBlocks	= WgBlockset::CreateFromRow(pSurface, WgRect(77,HSLIDER_BTN_OFS,74,17), 4, 2, WG_OPAQUE);
		pHSliderBtnFwdBlocks->SetFrame(WgBorders(3));
		pHSliderBtnFwdBlocks->SetPadding(WgBorders(4));

		WgBlocksetPtr pVSliderBtnBwdBlocks	= WgBlockset::CreateFromRow(pSurface, WgRect(1,VSLIDER_BTN_OFS,74,17), 4, 2, WG_OPAQUE);
		pVSliderBtnBwdBlocks->SetFrame(WgBorders(3));
		pVSliderBtnBwdBlocks->SetPadding(WgBorders(4));

		WgBlocksetPtr pVSliderBtnFwdBlocks	= WgBlockset::CreateFromRow(pSurface, WgRect(77,VSLIDER_BTN_OFS,74,17), 4, 2, WG_OPAQUE);
		pVSliderBtnFwdBlocks->SetFrame(WgBorders(3));
		pVSliderBtnFwdBlocks->SetPadding(WgBorders(4));

		WgBlocksetPtr pSliderBlocks	= WgBlockset::CreateFromRow(pSurface, WgRect(1,SLIDER_OFS,38,8), 4, 2, WG_OPAQUE);
		pSliderBlocks->SetFrame(WgBorders(2));
		pSliderBlocks->SetPadding(WgBorders(3));

		WgBlocksetPtr pSliderBackBlocks	= WgBlockset::CreateFromRect(pSurface, WgRect(1,SLIDER_BACK_OFS,5,5), WG_OPAQUE );
		pSliderBackBlocks->SetFrame(WgBorders(2));
		pSliderBackBlocks->SetPadding(WgBorders(2));

		WgBlocksetPtr pResizeButtonBlocks = WgBlockset::CreateFromRow(pSurface, WgRect(1,RESIZE_BUTTON_OFS,86,20), 4, 2, WG_OPAQUE);
		pResizeButtonBlocks->SetFrame(WgBorders(3));

		WgBlocksetPtr pCheckboxUncheckedBlocks = WgBlockset::CreateFromRow(pSurface, WgRect(1,CHECKBOX_OFS,50,11), 4, 2, WG_OPAQUE);
		pCheckboxUncheckedBlocks->SetFrame(WgBorders(2));
		pCheckboxUncheckedBlocks->SetPadding(WgBorders(3));

		WgBlocksetPtr pCheckboxCheckedBlocks = WgBlockset::CreateFromRow(pSurface, WgRect(53,CHECKBOX_OFS,50,11), 4, 2, WG_OPAQUE);
		pCheckboxCheckedBlocks->SetFrame(WgBorders(2));
		pCheckboxCheckedBlocks->SetPadding(WgBorders(3));

		WgBlocksetPtr pRadiobuttonUncheckedBlocks = WgBlockset::CreateFromRow(pSurface, WgRect(1,RADIOBUTTON_OFS,50,11), 4, 2, WG_OPAQUE);
		pRadiobuttonUncheckedBlocks->SetPadding(WgBorders(3));

		WgBlocksetPtr pRadiobuttonCheckedBlocks = WgBlockset::CreateFromRow(pSurface, WgRect(53,RADIOBUTTON_OFS,50,11), 4, 2, WG_OPAQUE);
		pRadiobuttonCheckedBlocks->SetPadding(WgBorders(3));

		WgBlocksetPtr pButtonBlocks = WgBlockset::CreateFromRow(pSurface, WgRect(1,BUTTON_OFS,38,8), 4, 2, WG_OPAQUE);
		pButtonBlocks->SetFrame(WgBorders(3));
		pButtonBlocks->SetPadding(WgBorders(4));

		WgBlocksetPtr pPlateBlocks = WgBlockset::CreateFromRow(pSurface, WgRect(1,PLATE_OFS,38,8), 4, 2, WG_OPAQUE);
		pPlateBlocks->SetFrame(WgBorders(3));
		pPlateBlocks->SetPadding(WgBorders(4));

		WgBlocksetPtr pHSplitBlocks = WgBlockset::CreateFromRect(pSurface, WgRect(1,SPLITS_AND_FRAME_OFS,8,2), WG_OPAQUE);

		WgBlocksetPtr pVSplitBlocks = WgBlockset::CreateFromRect(pSurface, WgRect(11,SPLITS_AND_FRAME_OFS,2,8), WG_OPAQUE);

		WgBlocksetPtr pFrameBlocks = WgBlockset::CreateFromRect(pSurface, WgRect(1,SPLITS_AND_FRAME_OFS,8,8), WG_OPAQUE);
		pFrameBlocks->SetFrame(WgBorders(2));
		pFrameBlocks->SetPadding(WgBorders(3));

		WgBlocksetPtr pComboboxBlocks = WgBlockset::CreateFromRow(pSurface, WgRect(1,COMBOBOX_OFS,98,20), 4, 2, WG_OPAQUE);
		pComboboxBlocks->SetFrame(WgBorders(1,1,20,1));
		pComboboxBlocks->SetPadding(WgBorders(2,2,21,2));

		WgBlocksetPtr pBgCheckeredGreyBlocks = WgBlockset::CreateFromRect( pSurface, WgRect(0,TILES_OFS,64,64), WG_OPAQUE | WG_TILE_ALL );
		WgBlocksetPtr pBgBlueGradientBlocks = WgBlockset::CreateFromRect( pSurface, WgRect(1*64,TILES_OFS,64,64), WG_OPAQUE );

		WgColorsetPtr pSelectionColors = WgColorset::Create( WgColor(0x0), WgColor(0x40FFFFFF), WgColor(0x80FFFFFF), WgColor(0x40000000), WgColor(0x0) );


		WgResDB * pDB = new WgResDB();

		// Create standard button

		WgButton * pButton = new WgButton();
		pButton->SetSource( pButtonBlocks );
		pDB->AddWidget( "button", pButton );

		// Create standard plate

		WgImage * pPlate = new WgImage();
		pPlate->SetSource( pPlateBlocks );
		pDB->AddWidget( "plate", pPlate );

		// Create standard checkbox

		WgCheckBox * pCheckbox = new WgCheckBox();
		pCheckbox->SetIcons( pCheckboxUncheckedBlocks, pCheckboxCheckedBlocks );
		pDB->AddWidget( "checkbox", pCheckbox );

		// Create standard radiobutton

		WgRadioButton * pRadiobutton = new WgRadioButton();
		pRadiobutton->SetIcons( pRadiobuttonUncheckedBlocks, pRadiobuttonCheckedBlocks );
		pDB->AddWidget( "radiobutton", pRadiobutton );

		// Create standard horizontal slider

		WgHSlider * pHSlider = new WgHSlider();
		pHSlider->SetSource( pSliderBackBlocks, pSliderBlocks, pHSliderBtnBwdBlocks, pHSliderBtnFwdBlocks );
		pDB->AddWidget( "hslider", pHSlider );

		// Create standard vertical slider

		WgVSlider * pVSlider = new WgVSlider();
		pVSlider->SetSource( pSliderBackBlocks, pSliderBlocks, pVSliderBtnBwdBlocks, pVSliderBtnFwdBlocks );
		pDB->AddWidget( "vslider", pVSlider );

		// Create standard menubar

		WgMenubar * pMenubar = new WgMenubar();
		pMenubar->SetBgSource( pPlateBlocks );
		pDB->AddWidget( "menubar", pMenubar );

		// Create Background bitmaps

		WgImage * pBgCheckeredGrey = new WgImage();
		pBgCheckeredGrey->SetSource( pBgCheckeredGreyBlocks );
		pDB->AddWidget( "bg_checkered_grey", pBgCheckeredGrey );

		WgImage * pBgBlueGradient = new WgImage();
		pBgBlueGradient->SetSource( pBgBlueGradientBlocks );
		pDB->AddWidget( "bg_blue_gradient", pBgBlueGradient );

		// Create standard menu

		WgMenu * pMenu = new WgMenu();
		pMenu->SetBgSource( pPlateBlocks, 16, 16 );
		pMenu->SetSeparatorSource( pHSplitBlocks, WgBorders(1) );
		pMenu->SetCheckBoxSource( pCheckboxUncheckedBlocks, pCheckboxCheckedBlocks);
		pMenu->SetRadioButtonSource( pRadiobuttonUncheckedBlocks, pRadiobuttonCheckedBlocks);
		pMenu->SetSliderSource( pSliderBackBlocks, pSliderBlocks, pVSliderBtnBwdBlocks, pVSliderBtnFwdBlocks );
		pMenu->SetTileColors( pSelectionColors );
		pDB->AddWidget( "menu", pMenu );

		// Create standard combobox

		WgCombobox * pCombobox = new WgCombobox();
		pCombobox->SetSource( pComboboxBlocks );
		pDB->AddWidget( "combobox", pCombobox );

		// Create standard view
		
		{
			WgScrollPanel * pView = new WgScrollPanel();
			
			WgWidget * pHSlider = pDB->CloneWidget( "hslider" );
			WgWidget * pVSlider = pDB->CloneWidget( "vslider" );

			if( pHSlider )
				pView->SetHSlider( static_cast<WgHSlider*>(pHSlider) );
			if( pVSlider )
				pView->SetVSlider( static_cast<WgVSlider*>(pVSlider) );

			pView->SetFillerBlocks( pPlateBlocks );
			pDB->AddWidget( "view", pView );
		}

		return pDB;
	}

	//____ LoadBitmapFont() ____________________________________________________

	WgFont * LoadBitmapFont( const char * pImgPath, const char * pSpecPath, const WgSurfaceFactory& factory )
	{
		//TODO: This leaks memory until we have ref-counted

		WgSurface * pFontImg = sdl_wglib::LoadSurface( pImgPath, factory );

		char * pFontSpec = (char*) LoadFile( pSpecPath );

		WgBitmapGlyphs * pGlyphs = new WgBitmapGlyphs( pFontImg, pFontSpec );

		WgFont * pFont = new WgFont();
		pFont->SetBitmapGlyphs( pGlyphs, WG_STYLE_NORMAL, 0 );

		free( pFontSpec );
		return pFont;
	}

	//____ FileSize() _____________________________________________________________

	int FileSize( const char * pPath )
	{
		FILE * fp = fopen( pPath, "rb" );
		if( !fp )
			return 0;

		fseek( fp, 0, SEEK_END );
		int size = ftell(fp);
		fseek( fp, 0, SEEK_SET );
		fclose( fp );

		return size;
	}

	//____ LoadFile() _____________________________________________________________

	void * LoadFile( const char * pPath )
	{
		FILE * fp = fopen( pPath, "rb" );
		if( !fp )
			return 0;

		fseek( fp, 0, SEEK_END );
		int size = ftell(fp);
		fseek( fp, 0, SEEK_SET );

		char * pMem = (char*) malloc( size+1 );
		pMem[size] = 0;
		int nRead = fread( pMem, 1, size, fp );
		fclose( fp );

		if( nRead < size )
		{
			free( pMem );
			return 0;
		}

		return pMem;
	}
	
	//____ ConvertPixelFormat() ______________________________________________________

	void ConvertPixelFormat( WgPixelFormat * pWGFormat, const SDL_PixelFormat * pSDLFormat )
	{
		pWGFormat->type = WG_PIXEL_CUSTOM;
		pWGFormat->bits = pSDLFormat->BitsPerPixel;

		pWGFormat->R_mask = pSDLFormat->Rmask;
		pWGFormat->G_mask = pSDLFormat->Gmask;
		pWGFormat->B_mask = pSDLFormat->Bmask;
		pWGFormat->A_mask = pSDLFormat->Amask;

		pWGFormat->R_shift = pSDLFormat->Rshift;
		pWGFormat->G_shift = pSDLFormat->Gshift;
		pWGFormat->B_shift = pSDLFormat->Bshift;
		pWGFormat->A_shift = pSDLFormat->Ashift;

		pWGFormat->R_bits = 8 - pSDLFormat->Rloss;
		pWGFormat->G_bits = 8 - pSDLFormat->Gloss;
		pWGFormat->B_bits = 8 - pSDLFormat->Bloss;
		pWGFormat->A_bits = 8 - pSDLFormat->Aloss;

	}
	
	
};
