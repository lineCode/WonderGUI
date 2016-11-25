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

#ifndef	WG_TABLIST_DOT_H
#define	WG_TABLIST_DOT_H

#ifndef	WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif

#ifndef WG_CHAIN_DOT_H
#	include <wg_chain.h>
#endif

#ifndef WG_TEXT_DOT_H
#	include <wg_text.h>
#endif

#ifndef WG_BLOCKSET_DOT_H
#	include <wg_blockset.h>
#endif

class WgFont;
class WgSurface;
class WgCharSeq;

class WgTab : public WgLink
{
	friend class WgTablist;
public:
	WgTab( int id );
	virtual ~WgTab();

	LINK_METHODS( WgTab );

	int				GetId() const			{ return m_id; }
	void			SetAlert(bool bAlertOn) { m_bAlert = bAlertOn; }
	bool			GetAlert() const		{ return m_bAlert; }

	const WgChar*	getText() const				{ return m_text.getText(); }
	void			setText(const char* pText)	{ return m_text.setText(pText); }

	void			SetSource( const WgBlocksetPtr& pGfx ) { m_pGfx = pGfx; }
	WgBlocksetPtr	GetSource() const			{ return m_pGfx; }

	void			SetId( int id ) { m_id = id; }

private:

	WgText			m_text;
	int				m_width;		// Width of this tab.
	int				m_advance;		// Distance from beginning of this tab to the next one.
	int				m_id;
	bool			m_bAlert;
	WgBlocksetPtr	m_pGfx;
	bool			m_bVisible;
};

class WgTablist : public WgWidget
{
public:
	// It's possible to specify different sources for leftmost, rightmost, and middle tabs
	enum SourceType
	{
		SourceTypeLeft,
		SourceTypeMiddle,
		SourceTypeRight,
        SourceTypeAll,
	};
	static const int m_nSourceTypes = 3;
/*
	enum TabWidthMode
	{
		TabWidthModeNormal,	// All tabs have required width
		TabWidthModeUnified,// All tabs have same width
		TabWidthModeExpand,	// Tabs are stretched out to cover the entire width of the tab list
		TabWidthModeExpand2,// Tabs are overlapped to cover the entire width of the tab list
	};
*/

	enum TabExpandMode
	{
		NO_EXPAND,
		GROW_TABS,
		SPREAD_TABS,
		UNIFY_TABS,
	};

	enum TabCompressMode
	{
		NO_COMPRESS,
		SHRINK_TABS,
		OVERLAP_TABS,
	};

	enum TabWidthMode
	{
		INDIVIDUAL_WIDTH,
		UNIFIED_WIDTH
	};


	WgTablist();
	~WgTablist();
	virtual const char * Type() const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgTablist(); };


	//____ Methods __________________________________________

	void			SetTextProperties( const WgTextpropPtr& pProp );
	WgTextpropPtr	GetTextProperties() const { return m_pProp; }

	void			SetSource( WgBlocksetPtr pBlockSet, SourceType type = SourceTypeAll);
	WgBlocksetPtr	GetSource(SourceType type = SourceTypeMiddle) const			{ return m_sources[type]; }

	void	SetOverlap( int overlap );
	int		GetOverlap() const { return m_overlap; }

	void	SetMaxOverlap( int maxOverlap );
	int		GetMaxOverlap() const { return m_maxOverlap; }

	void 	SetMinTabWidth( int minWidth );
	int 	GetMinTabWidth() const { return m_minTabWidth; }

	void 	SetAlertRate( int millisec );
	int		GetAlertRate() const { return m_alertRate; }

	void 	SetTextAlignment( WgOrigo alignment );
	WgOrigo GetTextAlignment() const { return m_textAlignment; }

	void			SetTabExpandMode( TabExpandMode mode );
	TabExpandMode	GetTabExpandMode() const { return m_tabExpandMode; }

	void			SetTabCompressMode( TabCompressMode mode );
	TabCompressMode	GetTabCompressMode() const { return m_tabCompressMode; }

	void 			SetTabWidthMode( TabWidthMode mode );
	TabWidthMode	GetTabWidthMode() const { return m_tabWidthMode; }

	bool AddTab( int id, const WgCharSeq& text, int position = -1, const WgBlocksetPtr& pGfx = 0 );
	bool RemoveTab( int id );
	void RemoveAllTabs( );

	bool MoveTab( int id, int position );
	bool SetTabText( int id, const WgCharSeq& text );

	bool SetTabId( int id, int newId );
	int GetTabId(int position) const;

	void	setTabMouseOpacity( bool bOpaque ) { m_bTabOpaqueForMouse = bOpaque; };
	bool	getTabMouseOpacity() const { return m_bTabOpaqueForMouse; }


	bool SetTabTextColor( int id, WgColor col );

	Uint32	GetTabWidth( int id ) const;

	bool SelectTab( int id );
	int GetSelectedTabId() const;
	int GetSelectedTabPos() const;
	bool SetAlert( int id, bool bAlertOn );
	bool GetAlert( int id ) const;
	bool ShowTab( int id, bool bVisible );
	int GetTabCount( ) const;

	bool HasTab( int id ) const;

	WgTab*			GetSelectedTab() const;
	WgTab*			GetFirstTab() const;
	WgTab*			GetLastTab() const;
	WgTab* 			GetFirstVisibleTab() const;
	WgTab* 			GetLastVisibleTab() const;

	WgSize			PreferredSize() const;

protected:
	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip );
	void	_onRefresh();
	void	_onNewSize( const WgSize& size );
	void 	_onEvent( const WgEvent::Event * _pEvent, WgEventHandler * pHandler );
	bool	_onAlphaTest( const WgCoord& ofs );

private:

	WgBlocksetPtr	_getTabSource( WgTab * pTab ) const;
	WgMode			_getTabMode(const WgTab& tab) const;
	WgTab*			_findTab( int id ) const;
	WgTab*	 		_pos2Tab( int x, int y ) const;

	void			_resizeTabs();
	int				_calcTabsWantedWidth( WgTab * pTab ) const;

	void			_renderTab( WgGfxDevice * pDevice, WgTab& tab, WgRect dest, const WgRect& clip );

	WgChain<WgTab>	m_tabs;

	WgTextpropPtr	m_pProp;

	WgOrigo	m_textAlignment;
	WgBlocksetPtr	m_sources[m_nSourceTypes];

	WgTab *		m_pTabSelected;
	WgTab *		m_pTabMarked;

	bool		m_bTabOpaqueForMouse;
	bool		m_bAlertOn;
	int			m_alertModeCnt;
	int			m_alertRate;				// Milliseconds between switching graphics.

	int			m_minTabWidth;
	int			m_overlap;					// As set by user.
	int			m_maxOverlap;				// Maximum allowed overlap in TabCompressMode::OVERLAP_TABS. set by user.

	TabWidthMode	m_tabWidthMode;
	TabExpandMode	m_tabExpandMode;
	TabCompressMode m_tabCompressMode;

	int			m_widestTab;
};





#endif //WG_TABLIST_DOT_H
