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

#include <wg_textstyle.h>

namespace wg 
{
	
	const char WgTextStyle::CLASSNAME[] = {"TextStyle"};
	
	
	
	//____ Constructor _____________________________________________________________
	
	WgTextStyle::WgTextStyle()
	{
		m_pFirstChild = 0;
		m_pNextSibling = 0;
		m_pPrevSibling = 0;
	
		_clearSet( &m_specAttr );
		_clearSet( &m_combAttr );
	}
	
	
	//____ Destructor ______________________________________________________________
	
	WgTextStyle::~WgTextStyle()
	{
		if( m_pNextSibling )
			m_pNextSibling->m_pPrevSibling = m_pPrevSibling;
	
		if( m_pPrevSibling )
			m_pPrevSibling->m_pNextSibling = m_pNextSibling;
	}
	
	//____ isInstanceOf() _________________________________________________________
	
	bool WgTextStyle::isInstanceOf( const char * pClassName ) const
	{
		if( pClassName==CLASSNAME )
			return true;
	
		return WgObject::isInstanceOf(pClassName);
	}
	
	//____ className() ____________________________________________________________
	
	const char * WgTextStyle::className( void ) const
	{
		return CLASSNAME;
	}
	
	//____ cast() _________________________________________________________________
	
	WgTextStyle_p WgTextStyle::cast( const WgObject_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return WgTextStyle_p( static_cast<WgTextStyle*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ setParent() _____________________________________________________________
	
	bool WgTextStyle::setParent( const WgTextStyle_p& pParent )
	{
		// Check so we don't get circular references.
	
		if( pParent )
		{
			WgTextStyle * p = pParent.rawPtr();
			while( p != 0 && p != this )
				p = p->m_pParent.rawPtr();
	
			if( p == this )
				return false;
		}
	
		//
	
		if( m_pParent )
		{
			if( m_pNextSibling )
				m_pNextSibling->m_pPrevSibling = m_pPrevSibling;
	
			if( m_pPrevSibling )
				m_pPrevSibling->m_pNextSibling = m_pNextSibling;
			else
				m_pParent->m_pFirstChild = m_pNextSibling;
		}
	
		m_pParent = pParent;
		if( pParent )
		{
			m_pNextSibling = m_pParent->m_pFirstChild;
			m_pPrevSibling = 0;
			m_pParent->m_pFirstChild = this;
		}
	
		// Update combined values
	
		_refreshComb();
		return true;
	}
	
	
	//____ cascade() _______________________________________________________________
	
	void WgTextStyle::cascade()
	{
		WgTextStyle * pChild = m_pFirstChild;
		while( pChild )
		{
			if( pChild->_refreshComb() )
				pChild->cascade();
	
			pChild = pChild->m_pNextSibling;
		}
	}
	
	//____ setFont() _______________________________________________________________
	
	void WgTextStyle::setFont( const WgFont_p& pFont )
	{
		if( pFont != m_specAttr.pFont )
		{
			m_specAttr.pFont = pFont;
			if( !pFont && m_pParent )
				m_combAttr.pFont = m_pParent->m_combAttr.pFont;
			else
				m_combAttr.pFont = pFont;
		}
	}
	
	//____ setLink() _______________________________________________________________
	
	void WgTextStyle::setLink( const WgTextLink_p& pLink )
	{
		if( pLink != m_specAttr.pLink )
		{
			m_specAttr.pLink = pLink;
			if( !pLink && m_pParent )
				m_combAttr.pLink = m_pParent->m_combAttr.pLink;
			else
				m_combAttr.pLink = pLink;
		}
	}
	
	//____ setColor() ______________________________________________________________
	
	void WgTextStyle::setColor( WgColor color )
	{
		if( color == WgColor::none )
			clearColor();
		else
		{
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				m_specAttr.color[i] = color;
				m_combAttr.color[i] = color;
			}
		}}
	
	void WgTextStyle::setColor( WgColor color, WgState state )
	{
		if( color == WgColor::none )
			clearColor(state);
		else
		{
			int idx = WgUtil::_stateToIndex(state);
			m_specAttr.color[idx] = color;
			m_combAttr.color[idx] = color;
		}
	}
	
	//____ setBgColor() ______________________________________________________________
	
	void WgTextStyle::setBgColor( WgColor color )
	{
		if( color == WgColor::none )
			clearBgColor();
		else
		{
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				m_specAttr.bgColor[i] = color;
				m_combAttr.bgColor[i] = color;
			}
		}
	}
	
	void WgTextStyle::setBgColor( WgColor color, WgState state )
	{
		if( color == WgColor::none )
			clearBgColor(state);
		else
		{
			int idx = WgUtil::_stateToIndex(state);
			m_specAttr.bgColor[idx] = color;
			m_combAttr.bgColor[idx] = color;
		}
	}
	
	//____ setSize() _______________________________________________________________
	
	void WgTextStyle::setSize( int size )
	{
		if( size == WG_FONTSIZE_INHERIT )
			clearSize();
		else
		{
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				m_specAttr.size[i] = size;
				m_combAttr.size[i] = size;
			}
		}
	}
	
	void WgTextStyle::setSize( int size, WgState state )
	{
		if( size == WG_FONTSIZE_INHERIT )
			clearSize(state);
		else
		{
			int idx = WgUtil::_stateToIndex(state);
			m_specAttr.size[idx] = size;
			m_combAttr.size[idx] = size;
		}
	}
	
	
	//____ setDecoration() _________________________________________________________
	
	void WgTextStyle::setDecoration( WgTextDecoration decoration )
	{
		if( decoration == WG_DECORATION_INHERIT )
			clearDecoration();
		else
		{
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				m_specAttr.decoration[i] = decoration;
				m_combAttr.decoration[i] = decoration;
			}
		}
	}
	
	void WgTextStyle::setDecoration( WgTextDecoration decoration, WgState state )
	{
		if( decoration == WG_DECORATION_INHERIT )
			clearDecoration(state);
		else
		{
			int idx = WgUtil::_stateToIndex(state);
			m_specAttr.decoration[idx] = decoration;
			m_combAttr.decoration[idx] = decoration;
		}
	}
	
	
	//____ clearFont() _____________________________________________________________
	
	void WgTextStyle::clearFont()
	{
		m_specAttr.pFont = 0;
		if( m_pParent )
			m_combAttr.pFont = m_pParent->m_combAttr.pFont;
		else
			m_combAttr.pFont = 0;
	}
	
	//____ clearLink() _____________________________________________________________
	
	void WgTextStyle::clearLink()
	{
		m_specAttr.pLink = 0;
		if( m_pParent )
			m_combAttr.pLink = m_pParent->m_combAttr.pLink;
		else
			m_combAttr.pLink = 0;
	}
	
	//____ clearColor() ____________________________________________________________
	
	void WgTextStyle::clearColor()
	{
		if( m_pParent )
		{
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				m_specAttr.color[i] = WgColor::none;
				m_combAttr.color[i] = m_pParent->m_combAttr.color[i];
			}
		}
		else
		{		for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				m_specAttr.color[i] = WgColor::none;
				m_combAttr.color[i] = WgColor::none;
			}
		}
	}
	
	void WgTextStyle::clearColor( WgState state )
	{
		int idx = WgUtil::_stateToIndex(state);
	
		m_specAttr.color[idx] = WgColor::none;
		m_combAttr.color[idx] = m_pParent ? m_pParent->m_combAttr.color[idx] : WgColor::none;
	}
	
	
	//____ clearBgColor() ____________________________________________________________
	
	void WgTextStyle::clearBgColor()
	{
		if( m_pParent )
		{
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				m_specAttr.bgColor[i] = WgColor::none;
				m_combAttr.bgColor[i] = m_pParent->m_combAttr.bgColor[i];
			}
		}
		else
		{
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				m_specAttr.bgColor[i] = WgColor::none;
				m_combAttr.bgColor[i] = WgColor::none;
			}
		}
	}
	
	void WgTextStyle::clearBgColor( WgState state )
	{
		int idx = WgUtil::_stateToIndex(state);
	
		m_specAttr.bgColor[idx] = WgColor::none;
		m_combAttr.bgColor[idx] = m_pParent ? m_pParent->m_combAttr.bgColor[idx] : WgColor::none;
	}
	
	//____ clearSize() ____________________________________________________________
	
	void WgTextStyle::clearSize()
	{
		if( m_pParent )
		{
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				m_specAttr.size[i] = WG_FONTSIZE_INHERIT;
				m_combAttr.size[i] = m_pParent->m_combAttr.size[i];
			}
		}
		else
		{
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				m_specAttr.size[i] = WG_FONTSIZE_INHERIT;
				m_combAttr.size[i] = WG_FONTSIZE_INHERIT;
			}
		}
	}
	
	void WgTextStyle::clearSize( WgState state )
	{
		int idx = WgUtil::_stateToIndex(state);
	
		m_specAttr.size[idx] = WG_FONTSIZE_INHERIT;
		m_combAttr.size[idx] = m_pParent ? m_pParent->m_combAttr.size[idx] : WG_FONTSIZE_INHERIT;
	}
	
	//____ clearDecoration() ____________________________________________________________
	
	void WgTextStyle::clearDecoration()
	{
		if( m_pParent )
		{
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				m_specAttr.decoration[i] = WG_DECORATION_INHERIT;
				m_combAttr.decoration[i] = m_pParent->m_combAttr.decoration[i];
			}
		}
		else
		{
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				m_specAttr.decoration[i] = WG_DECORATION_INHERIT;
				m_combAttr.decoration[i] = WG_DECORATION_INHERIT;
			}
		}
	}
	
	void WgTextStyle::clearDecoration( WgState state )
	{
		int idx = WgUtil::_stateToIndex(state);
	
		m_specAttr.decoration[idx] = WG_DECORATION_INHERIT;
		m_combAttr.decoration[idx] = m_pParent ? m_pParent->m_combAttr.decoration[idx] : WG_DECORATION_INHERIT;
	}
	
	//____ exportAttr() ____________________________________________________________
	
	void WgTextStyle::exportAttr( WgState state, WgTextAttr2 * pDest ) const
	{
		int idx = WgUtil::_stateToIndex(state);
	
		pDest->pFont 		= m_combAttr.pFont;
		pDest->pLink 		= m_combAttr.pLink;
		pDest->size 		= m_combAttr.size[idx];
		pDest->color		= m_combAttr.color[idx];
		pDest->bgColor		= m_combAttr.bgColor[idx];
		pDest->decoration	= m_combAttr.decoration[idx];
				
		if( pDest->size == WG_FONTSIZE_INHERIT )
			pDest->size = 12;								// Default to size 12.
	}
	
	//____ addToAttr() _____________________________________________________________
	
	void WgTextStyle::addToAttr( WgState state, WgTextAttr2 * pDest ) const 
	{
		int idx = WgUtil::_stateToIndex(state);
	
		if( m_combAttr.pFont )
			pDest->pFont = m_combAttr.pFont;
		if( m_combAttr.pLink )
			pDest->pLink = m_combAttr.pLink;
		if( m_combAttr.size[idx] != WG_FONTSIZE_INHERIT )
			pDest->size	= m_combAttr.size[idx];
		if( m_combAttr.color[idx] != WgColor::none )
			pDest->color = m_combAttr.color[idx];
		if( m_combAttr.bgColor[idx] != WgColor::none )
			pDest->bgColor = m_combAttr.bgColor[idx];
		if( m_combAttr.decoration[idx] != WG_DECORATION_INHERIT )
			pDest->decoration = m_combAttr.decoration[idx];
	}
	
	
	//____ refreshComb() ___________________________________________________________
	
	bool WgTextStyle::_refreshComb()
	{
		if( m_pParent )
		{
			AttrSet		newComb;
	
			newComb.pFont = m_specAttr.pFont ? m_specAttr.pFont : m_pParent->m_combAttr.pFont;
			newComb.pLink = m_specAttr.pLink ? m_specAttr.pLink : m_pParent->m_combAttr.pLink;
	
	
			for( int i = 0 ; i < WG_NB_STATES ; i++ )
			{
				newComb.size[i] = m_specAttr.size[i] != WG_FONTSIZE_INHERIT ? m_specAttr.size[i] : m_pParent->m_combAttr.size[i];
				newComb.color[i] = m_specAttr.color[i] != WgColor::none ? m_specAttr.color[i] : m_pParent->m_combAttr.color[i];
				newComb.bgColor[i] = m_specAttr.bgColor[i] != WgColor::none ? m_specAttr.bgColor[i] : m_pParent->m_combAttr.bgColor[i];
				newComb.decoration[i] = m_specAttr.decoration[i] != WG_DECORATION_INHERIT ? m_specAttr.decoration[i] : m_pParent->m_combAttr.decoration[i];
			}
	
			if( _compareSets( &newComb, &m_combAttr ) == false )
			{
				m_combAttr = newComb;
				return true;
			}
			return false;
		}
		else
		{
			bool retVal = !_compareSets( &m_specAttr, &m_combAttr );
			m_combAttr = m_specAttr;
			return retVal;
		}
	}
	
	//____ _clearSet() _____________________________________________________________
	
	void WgTextStyle::_clearSet( WgTextStyle::AttrSet * pSet )
	{
		pSet->pFont = 0;
		pSet->pLink = 0;
	
		for( int i = 0 ; i < WG_NB_STATES ; i++ )
		{
			pSet->size[i] 		= WG_FONTSIZE_INHERIT;
			pSet->color[i] 		= WgColor::none;
			pSet->bgColor[i] 	= WgColor::none;
			pSet->decoration[i] = WG_DECORATION_INHERIT;
		}
	}
	
	//____ _compareSets() __________________________________________________________
	
	bool WgTextStyle::_compareSets( WgTextStyle::AttrSet * p1, WgTextStyle::AttrSet * p2 )
	{
		if( p1->pFont != p2->pFont || p1->pLink != p2->pLink )
			return false;
	
		for( int i = 0 ; i < WG_NB_STATES ; i++ )
		{
			if( p1->size[i] 		!= p2->size[i] ||
				p1->color[i] 		!= p2->color[i] ||
				p1->bgColor[i] 		!= p2->bgColor[i] ||
				p1->decoration[i] 	!= p2->decoration[i] )
				return false;
		}
	
		return true;
	}

} // namespace wg
