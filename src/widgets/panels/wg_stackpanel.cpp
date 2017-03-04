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

#include <wg_stackpanel.h>
#include <wg_util.h>
#include <wg_patches.h>

namespace wg 
{
	
	const char StackPanel::CLASSNAME[] = {"StackPanel"};
		
	
	
	void StackPanelChildren::add( const Widget_p& pWidget )
	{
		
	}
	
	void StackPanelChildren::insert( const Widget_p& pWidget )
	{
		
	}

	void StackPanelChildren::setSizePolicy( int index, SizePolicy policy )
	{
		if( index < 0 || index >= m_pSlotArray->size() )
			return;

		auto pSlot = m_pSlotArray->slot(index);
			
		if( policy != pSlot->sizePolicy )
		{
			pSlot->pWidget->_requestRender();
			pSlot->sizePolicy = policy;
			pSlot->pWidget->_requestRender();
		};		
	}
	
	SizePolicy StackPanelChildren::sizePolicy( int index ) const
	{
		if( index < 0 || index >= m_pSlotArray->size() )
			return SizePolicy::Default;

		return m_pSlotArray->slot(index)->sizePolicy;
		
	}

	void StackPanelChildren::setOrigo( int index, Origo origo )
	{
		if( index < 0 || index >= m_pSlotArray->size() )
			return;

		auto pSlot = m_pSlotArray->slot(index);
			
		if( origo != pSlot->origo )
		{
			pSlot->pWidget->_requestRender();
			pSlot->origo = origo;
			pSlot->pWidget->_requestRender();
		};
		
	}
	
	Origo StackPanelChildren::origo( int index ) const
	{
		if( index < 0 || index >= m_pSlotArray->size() )
			return Origo::Center;

		return m_pSlotArray->slot(index)->origo;
		
	}

	
	
	//____ Constructor ____________________________________________________________
	
	StackPanel::StackPanel()
	{
		m_bSiblingsOverlap = true;
	}
	
	//____ Destructor _____________________________________________________________
	
	StackPanel::~StackPanel()
	{
	}
	
	//____ isInstanceOf() _________________________________________________________
	
	bool StackPanel::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Panel::isInstanceOf(pClassName);
	}
	
	//____ className() ____________________________________________________________
	
	const char * StackPanel::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	//____ cast() _________________________________________________________________
	
	StackPanel_p StackPanel::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return StackPanel_p( static_cast<StackPanel*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ matchingHeight() _______________________________________________________
	
	int StackPanel::matchingHeight( int width ) const
	{
		int height = 0;

		StackPanelSlot * pSlot = m_children.begin();
		StackPanelSlot * pEnd = m_children.end();
	
		while( pSlot != pEnd )
		{
			int h = pSlot->pWidget->matchingHeight(width);
			if( h > height )
				height = h;
			pSlot++
		}
	
		return height;
	}
	
	//____ matchingWidth() _______________________________________________________
	
	int StackPanel::matchingWidth( int height ) const
	{
		int width = 0;
	
		StackPanelSlot * pSlot = m_children.begin();
		StackPanelSlot * pEnd = m_children.end();
	
		while( pSlot != pEnd )
		{
			int h = pSlot->pWidget->matchingWidth(height);
			if( h > height )
				height = h;
			pSlot++
		}
	
		return width;
	}
	
	
	//____ preferredSize() _____________________________________________________________
	
	Size StackPanel::preferredSize() const
	{
		return m_preferredSize;
	}
	
	//____ _cloneContent() ______________________________________________________
	
	void StackPanel::_cloneContent( const Widget * _pOrg )
	{
		Panel::_cloneContent( _pOrg );

		//TODO: Implement		
	}

	//____ _setSize() ___________________________________________________________
	
	void StackPanel::_setSize( const Size& size )
	{
		Panel::_setSize(size);
		_adaptChildrenToSize();
	}

	//____ _firstChild() _______________________________________________________

	Widget * StackPanel::_firstChild() const
	{
		if( m_children.isEmpty() )
			return nullptr;
			
		return m_children.first()->pWidget;
	}

	//____ _lastChild() ________________________________________________________
	
	Widget * StackPanel::_lastChild() const
	{
		if( m_children.isEmpty() )
			return nullptr;
			
		return m_children.last()->pWidget;
		
	}
	
	
	//____ _firstChildWithGeo() _____________________________________________________
	
	void StackPanel::_firstChildWithGeo( WidgetWithGeo& package ) const
	{
		if( m_children.isEmpty() )
			package.pWidget = nullptr;
		else
		{
			StackPanelSlot * pSlot = m_children.first();
			package.pMagic = pSlot;
			package.pWidget = pSlot->pWidget;
			package.geo = _childGeo(pSlot);
			
		}			
	}

	//____ _nextChildWithGeo() ______________________________________________________
	
	void StackPanel::_nextChildWithGeo( WidgetWithGeo& package ) const
	{
		StackPanelSlot * pSlot = (StackPanelSlot*) package.pMagic;
		
		if( pSlot == m_children.last() )
			package.pWidget = nullptr;
		else
		{
			pSlot++;
			package.pMagic = pSlot;
			package.pWidget = pSlot->pWidget;
			package.geo = _childGeo(pSlot);			
		}
	}

	//____ _object() ___________________________________________________________

	Object * StackPanel::_object()
	{
		return this;
	}
	
	const Object *  StackPanel::_object() const
	{
		 return this;
	}

	void StackPanel::_didAddSlots( Slot * pSlot, int nb )
	{
		
	}
	
	void StackPanel::_willRemoveSlots( Slot * pSlot, int nb )
	{
		
	}

	Coord StackPanel::_childPos( void * pChildRef ) const
	{
		
	}
	
	Size StackPanel::_childSize( void * pChildRef ) const
	{
		
	}

	void StackPanel::_childRequestRender( void * pChildRef )
	{
		_childRequestRender( pChildRef, _childGeo((StackPanelSlot*) pChildRef) );
	}
	
	void StackPanel::_childRequestRender( void * pChildRef, const Rect& rect )
	{
		
	}
	
	void StackPanel::_childRequestResize( void * pChildRef )
	{
		_refreshPreferredSize();		
	}

	Widget * StackPanel::_prevChild( void * pChildRef ) const
	{
		StackPanelSlot * p = (StockPanelSlot *) pChildRef;
		
		if( p > m_children.begin() )
			return p[-1]->pWidget;
	}
	
	Widget * StackPanel::_nextChild( void * pChildRef ) const
	{
		StackPanelSlot * p = (StockPanelSlot *) pChildRef;
		
		if( p < m_children.last() )
			return p[1]->pWidget;		
	}



	Rect StackPanel::_childGeo( StackPanelSlot * pSlot )
	{
		
	}
	
	
	void StackPanel::_renderRequested( VectorHook * _pHook, const Rect& _rect )
	{
		StackHook * pHook = static_cast<StackHook*>(_pHook);
	
		if( !pHook->isVisible() )
			return;
	
		// Put our rectangle into patches
	
		Rect rect = _rect + pHook->_getGeo(Rect(0,0,m_size)).pos();
	
	
		Patches patches;
		patches.add( rect );
	
		// Remove portions of patches that are covered by opaque upper siblings
	
		StackHook * pCover = ((StackHook*)pHook)->_next();
		while( pCover )
		{
			Rect geo = pCover->_getGeo(m_size);
			if( pCover->isVisible() && geo.intersectsWith( rect ) )
				pCover->_widget()->_maskPatches( patches, geo, Rect(0,0,65536,65536 ), _getBlendMode() );
	
			pCover = pCover->_next();
		}
	
		// Make request render calls
	
		for( const Rect * pRect = patches.begin() ; pRect < patches.end() ; pRect++ )
			_requestRender( * pRect );
	}
	
	//____ _onWidgetAppeared() _____________________________________________________
	
	void StackPanel::_onWidgetAppeared( VectorHook * _pInserted )
	{
		StackHook * pInserted = (StackHook*) _pInserted;
	
		bool	bRequestResize = false;
	
		// Check if we need to resize to fit Widget in current width
	
		int height = pInserted->_widget()->matchingHeight(m_size.w);
		if( height > m_size.h )
			bRequestResize = true;
	
		// Update bestSize
	
		Size preferred = pInserted->_widget()->preferredSize();
	
		if( preferred.w > m_preferredSize.w )
		{
			m_preferredSize.w = preferred.w;
			bRequestResize = true;
		}
		if( preferred.h > m_preferredSize.h )
		{
			m_preferredSize.h = preferred.h;
			bRequestResize = true;
		}
	
		if( bRequestResize )
			_requestResize();
	
		// Adapt inserted Widget to our size
	
		pInserted->_widget()->_setSize(m_size);
	
		// Force a render.
	
		_renderRequested( pInserted );
	}
	
	//____ _onWidgetDisappeared() __________________________________________________
	
	void StackPanel::_onWidgetDisappeared( VectorHook * _pToBeRemoved )
	{
		bool	bRequestResize = false;
		StackHook * pToBeRemoved = (StackHook*) _pToBeRemoved;
	
		// Get dirty rectangles for all visible sections of pToBeRemoved.
	
		_renderRequested( pToBeRemoved );
	
		// Update m_preferredSize, skiping pToBeRemoved
	
		Size	preferredSize;
		StackHook * pHook = static_cast<StackHook*>(m_hooks.first());
		while( pHook )
		{
			if( pHook != pToBeRemoved )
			{
				Size sz = pHook->_widget()->preferredSize();
				if( sz.w > preferredSize.w )
					preferredSize.w = sz.w;
				if( sz.h > preferredSize.h )
					preferredSize.h = sz.h;
			}
			pHook = pHook->_next();
		}
	
		if( preferredSize != m_preferredSize )
			bRequestResize = true;
	
		m_preferredSize = preferredSize;
	
		// Check if removal might affect height for current width
	
		int height = pToBeRemoved->_widget()->matchingHeight(m_size.w);
		if( height >= m_size.h )
			bRequestResize = true;
	
		//
	
		if( bRequestResize )
			_requestResize();
	}
	
	
	//____ _refreshAllWidgets() ____________________________________________________
	
	void StackPanel::_refreshAllWidgets()
	{
		_refreshPreferredSize();
		_adaptChildrenToSize();
		_requestRender();
	}
	
	
	//____ _refreshPreferredSize() _____________________________________________________
	
	void StackPanel::_refreshPreferredSize()
	{
		Size	preferredSize;

		auto * pSlot = m_children.begin();
		auto * pEnd = m_children.end();
		
		while( pSlot != pEnd )
		{
			Size sz = pSlot->_paddedPreferredSize();
			if( sz.w > preferredSize.w )
				preferredSize.w = sz.w;
			if( sz.h > preferredSize.h )
				preferredSize.h = sz.h;

			pSlot++;
		}

		if( m_preferredSize != preferredSize)
		{
			m_preferredSize = preferredSize;
			_requestResize();
		}
	}
	
	//____ _adaptChildrenToSize() ___________________________________________________________
	
	void StackPanel::_adaptChildrenToSize()
	{
		auto * pSlot = m_children.begin();
		auto * pEnd = m_children.end();
		
		while( pSlot != pEnd )
		{
			pSlot->pWidget->_setSize( _childGeo(pSlot) );
			pSlot++;
		}
	}

	//____ _childGeo() ___________________________________________________________

	Rect StackPanel::_childGeo( const StackPanelSlot * pSlot ) const
	{
		Rect base = Rect( m_size ) - m_padding;
	
		if( base.w <= 0 || base.h <= 0 )
			return Rect(0,0,0,0);
	
		switch( m_sizePolicy )
		{
			default:
			case DEFAULT:
			{
				Size	size = pSlot->pWidget->preferredSize();
				Rect geo = Util::origoToRect( pSlot->origo, base, size );
	
				if( geo.w > base.w )
				{
					geo.x = 0;
					geo.w = base.w;
				}
	
				if( geo.h > base.h )
				{
					geo.y = 0;
					geo.h = base.h;
				}
				return geo;
			}
			case STRETCH:
			{
				return base;
			}
			case SCALE:
			{
				Size	orgSize = pSlot->pWidget->preferredSize();
				Size	size;
	
				float	fracX = orgSize.w / (float) base.w;
				float	fracY = orgSize.h / (float) base.h;
	
				if( fracX > fracY )
				{
					size.w = base.w;
					size.h = int (orgSize.h / fracX);
				}
				else
				{
					size.h = base.h;
					size.w = (int) (orgSize.w / fracY);
				}
	
				return Util::origoToRect( pSlot->origo, base, size );
			}
		}
	}




} // namespace wg
