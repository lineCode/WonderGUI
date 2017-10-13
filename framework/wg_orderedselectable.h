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

#ifndef WG_ORDERED_SELECTABLE_DOT_H
#define WG_ORDERED_SELECTABLE_DOT_H

#ifndef WG_VECTORPANEL_DOT_H
#	include <wg_vectorpanel.h>
#endif

#ifndef WG_BLOCKSET_DOT_H
#	include <wg_blockset.h>
#endif

#ifndef WG_COLORSET_DOT_H
#	include <wg_colorset.h>
#endif


class WgOrdSelLayout;


class WgOrdSelHook : public WgVectorHook
{
	friend class WgOrdSelLayout;

public:
	inline bool				Select() { return SetSelected(true); }
	inline bool				Unselect() { return SetSelected(false); }
	bool					SetSelected( bool bSelected );
	inline bool				Selected() const { return m_bSelected; }

	void					SetSelectable( bool bSelectable );
	inline bool				Selectable() const { return m_bSelectable; }


	inline WgOrdSelHook*	Prev() const { return _prev(); }
	inline WgOrdSelHook*	Next() const { return _next(); }

	inline WgOrdSelHook*	PrevSelected() const { return _prevSelectedHook(); }
	inline WgOrdSelHook*	NextSelected() const { return _nextSelectedHook(); }

	inline WgOrdSelLayout*	Parent() const { return (WgOrdSelLayout*) _parent(); }

protected:
	PROTECTED_LINK_METHODS( WgOrdSelHook );

	WgOrdSelHook();
	~WgOrdSelHook();

	WgOrdSelHook *	_prevSelectedHook() const;
	WgOrdSelHook *	_nextSelectedHook() const;

	bool			m_bSelectable;
	bool			m_bSelected;
};

class WgOrdSelLayout : public WgVectorPanel
{
	friend class WgOrdSelHook;
public:
	int				SelectAll();
	void			UnselectAll();

	inline int		GetNbSelected() const { return m_nbSelected; }

	void			SetSelectMode( WgSelectMode mode );
	inline WgSelectMode	SelectMode() const					{ return m_selectMode; }

	inline void		SetAutoScrollOnSelect( bool bScroll )	{ m_bScrollOnSelect = bScroll; }
	inline bool		AutoScrollOnSelect() const				{ return m_bScrollOnSelect; }

	inline WgOrdSelHook *	FirstHook() const { return static_cast<WgOrdSelHook*>( _firstHook());}
	inline WgOrdSelHook *	LastHook() const { return static_cast<WgOrdSelHook*>( _lastHook());}

	inline WgOrdSelHook *	FirstSelectedHook() const { return _firstSelectedHook(); }
	inline WgOrdSelHook *	LastSelectedHook() const { return _lastSelectedHook(); }


	inline void		SetBgBlocks( const WgBlocksetPtr& pBg ) { SetBgBlocks( pBg, pBg ); }
	void			SetBgBlocks( const WgBlocksetPtr& pOddBg, const WgBlocksetPtr& pEvenBg );
	void			SetFgBlocks( const WgBlocksetPtr& pFg );

	inline void		SetBgColors( const WgColorsetPtr& pBg ) { SetBgColors( pBg, pBg ); }
	void			SetBgColors( const WgColorsetPtr& pOddBg, const WgColorsetPtr& pEvenBg );

	inline WgBlocksetPtr	BgBlocksEven() const { return m_pEvenBgBlocks; }
	inline WgBlocksetPtr	BgBlocksOdd() const { return m_pOddBgBlocks; }


protected:
	WgOrdSelLayout();
	~WgOrdSelLayout();

	void			_onCloneContent( const WgWidget * _pOrg );

	WgOrdSelHook *	_firstSelectedHook() const;
	WgOrdSelHook *	_lastSelectedHook() const;

	virtual void	_onWidgetSelected( WgOrdSelHook * pSelected );			// so parent can do what it needs to.
	virtual void	_onWidgetUnselected( WgOrdSelHook * pUnselected );		// so parent can do what it needs to.


	WgColorsetPtr	m_pOddBgColors;
	WgColorsetPtr	m_pEvenBgColors;
	WgBlocksetPtr	m_pOddBgBlocks;
	WgBlocksetPtr	m_pEvenBgBlocks;
	WgBlocksetPtr	m_pFgBlocks;

	bool			m_bScrollOnSelect;
	WgSelectMode	m_selectMode;
	int				m_nbSelected;
//	WgOrdSelHook *	m_pLatestSelected;				// Pointer at latest selected hook if any is selected.

};


#endif //WG_ORDERED_SELECTABLE_DOT_H
