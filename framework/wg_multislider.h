/*=========================================================================

                         >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

                            -----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                            -----------
	
  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/
#ifndef WG_MULTISLIDERS_DOT_H
#define WG_MULTISLIDERS_DOT_H


#ifndef WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif






//____ WgMultiSlider ____________________________________________________________

class WgMultiSlider : public WgWidget
{
protected:
	struct Slider;

public:
	WgMultiSlider();
	virtual ~WgMultiSlider();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgMultiSlider(); };

	WgSize	PreferredPixelSize() const;

	struct Bounds
	{
		float	min;
		float	max;
		int		steps;
	};
	
	class Visitor
	{
	public:
		Visitor(WgMultiSlider * pWidget, Slider * pSlider) : m_pWidget(pWidget), m_pSlider(pSlider) {}
	protected:
		WgMultiSlider * m_pWidget;
		Slider *		m_pSlider;
	};

	class SetValueVisitorBase : public Visitor
	{
	public:
		SetValueVisitorBase(WgMultiSlider * pWidget, Slider * pSlider);

		float		handleFactor(int sliderId);
		float		setHandleFactor(int sliderId, float value);

		WgCoordF	handleFactor2D(int sliderId);
		WgCoordF	setHandleFactor2D(int sliderId, WgCoordF value);
	};

	class SetValueVisitor : public SetValueVisitorBase
	{
	public:
		using SetValueVisitorBase::handleFactor;
		SetValueVisitor(WgMultiSlider * pWidget, Slider * pSlider);

		float		handleFactor();
		Bounds		valueBounds();
	};

	class SetValueVisitor2D : public SetValueVisitorBase
	{
	public:
		using SetValueVisitorBase::handleFactor2D;
		SetValueVisitor2D(WgMultiSlider * pWidget, Slider * pSlider);

		WgCoordF	handleFactor2D();
		Bounds		valueBoundsX();
		Bounds		valueBoundsY();
	};

	class SetHandleVisitor : public Visitor
	{
	public:
		SetHandleVisitor(WgMultiSlider * pWidget, Slider * pSlider);

		float		value();
		Bounds		valueBounds();
	};

	class SetHandleVisitor2D : public Visitor
	{
	public:
		SetHandleVisitor2D(WgMultiSlider * pWidget, Slider * pSlider);

		float		valueX();
		float		valueY();
		Bounds		valueBoundsX();
		Bounds		valueBoundsY();
	};



	class SetGeoVisitor : public Visitor
	{
	public:
		SetGeoVisitor(WgMultiSlider * pWidget, Slider * pSlider);

		WgCoordF	handlePos(int sliderId);
		WgRectF		geo(int sliderId);
	}; 

	typedef std::function<float(SetHandleVisitor& visitor)>		SetHandleFunc;
	typedef std::function<WgCoordF(SetHandleVisitor2D& visitor)>	SetHandleFunc2D;

	typedef std::function<float(SetValueVisitor& visitor)>			SetValueFunc;
	typedef std::function<WgCoordF(SetValueVisitor2D& visitor)>		SetValueFunc2D;

	typedef std::function<WgRectF(SetGeoVisitor& visitor)>			SetGeoFunc;
	

	void	SetDefaults(const WgSkinPtr& pBgSkin, const WgSkinPtr& pHandleSkin, WgCoordF handleHotspot = { 0.5f,0.5f }, WgBorders markExtension = WgBorders(0) );
	void	SetCallback(std::function<void(int sliderId, float value, float value2)>& callback);


	int		AddSlider(	int id, WgDirection dir, SetGeoFunc pSetGeoFunc, float startValue = 0.f, float minValue = 0.f, float maxValue = 1.f, int steps = 0,
						 SetHandleFunc pSetHandleFunc = nullptr, SetValueFunc pSetValueFunc = nullptr, const WgSkinPtr& pBgSkin = nullptr, 
						const WgSkinPtr& pHandleSkin = nullptr, WgCoordF handleHotspot = { -1.f,-1.f }, WgBorders markExtension = WgBorders(0));

	int		AddSlider2D( int id, WgOrigo origo, SetGeoFunc pSetGeoFunc, float startValueX = 0.f, float startValueY = 0.f, 
						float minValueX = 0.f,  float maxValueX = 1.f, int stepsX = 0, float minValueY = 0.f, float maxValueY = 1.f, int stepsY = 0,
						SetHandleFunc2D pSetHandleFunc = nullptr, SetValueFunc2D pSetValueFunc = nullptr,
						const WgSkinPtr& pBgSkin = nullptr, const WgSkinPtr& pHandleSkin = nullptr, WgCoordF handleHotspot = { -1.f, -1.f }, WgBorders markExtension = WgBorders(0) );

	float	SetSliderValue(int id, float value, float value2 = NAN);


	bool	MarkTest(const WgCoord& ofs) override;

	void	SetSkin(const WgSkinPtr& pSkin);



protected:

	struct Slider
	{
		int				id;

		bool			is2D;

		float			value[2];
		Bounds			bounds[2];

		WgOrigo			origo;
		WgRectF			geo;				// 0.f -> 1.f, within widgets content rect
		WgCoordF		handlePos;			// 0.f -> 1.f, within geo
		WgCoordF		handleHotspot;		// 0.f -> 1.f, within its own size
		WgState			handleState;
		int				geoState;			// 0 = needs refresh, 1 = refresh in progress, 2 = refreshed.

		WgSkinPtr		pBgSkin;
		WgSkinPtr		pHandleSkin;
		WgBorders		markExtension;		// Frame surrounding slider that also marks the slider. Measured in points, not pixels.

		SetHandleFunc	pSetHandleFunc;
		SetHandleFunc2D	pSetHandleFunc2D;

		SetValueFunc	pSetValueFunc;
		SetValueFunc2D	pSetValueFunc2D;

		SetGeoFunc		pSetGeoFunc;
	};

	void	_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler) override;
	void	_onCloneContent( const WgWidget * _pOrg ) override;
	void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip ) override;
	bool	_onAlphaTest( const WgCoord& ofs ) override;
	void	_setScale(int scale) override;

	void	_updateHandlePos(Slider& slider);									// Updates handlePos from its parameter values.
	void	_updateGeo(Slider& slider);

	WgRect	_sliderGeo(Slider& slider, const WgRect& canvas);
	WgRect	_sliderSkinGeo(Slider& slider, const WgRect& sliderGeo );
	WgRect	_sliderHandleGeo(Slider& slider, const WgRect& sliderGeo );

	Slider * _markedSlider(WgCoord ofs, WgCoord * pOfsOutput = nullptr );
	void	_markSlider(Slider * pSlider);
	void	_selectSlider(Slider * pSlider);
	void	_requestRenderHandle(Slider * pSlider);


	WgCoordF	_setHandlePosition(Slider& slider, WgCoordF pos);					// Set handlePos and update value(s).
	WgCoordF	_setHandleFactor(Slider& slider, WgCoordF factor);					// Convert handleFactor to handlePos and update handlePos and value(s).

	float		_setValue(Slider& slider, float valueX, float valueY = NAN);			// Set value(s) and update handlePos.

	WgCoordF	_handleFactor(Slider& slider);										// Get the factor (0.f-1.f) for current handle position & origo.

	WgCoordF	_convertFactorPos(WgCoordF in, WgOrigo origo);


	void		_refreshSliders();
	void		_refreshSliderGeo();

	Slider *	_findSlider(int sliderId);

private:


	WgSkinPtr			m_pDefaultBgSkin;
	WgSkinPtr			m_pDefaultHandleSkin;
	WgCoordF			m_defaultHandleHotspot = { 0.5f, 0.5f };
	WgBorders			m_defaultMarkExtension;

	std::vector<Slider>	m_sliders;

	int					m_selectedSlider = -1;
	WgCoord				m_selectPressOfs;

	std::function<void(int sliderId, float value, float value2 )>	m_callback = nullptr;

};

#endif //WG_MULTISLIDERS_DOT_H