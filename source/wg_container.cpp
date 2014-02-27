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

#include <vector>
#include <wg_container.h>
#include <wg_panel.h>

#include <wg_patches.h>

#ifndef WG_GFXDEVICE_DOT_H
#	include <wg_gfxdevice.h>
#endif

const char WgContainer::CLASSNAME[] = {"Container"};

//____ Constructor _____________________________________________________________

WgContainer::WgContainer() : m_bSiblingsOverlap(true)
{
}

//____ IsInstanceOf() _________________________________________________________

bool WgContainer::IsInstanceOf( const char * pClassName ) const
{ 
	if( pClassName==CLASSNAME )
		return true;

	return WgWidget::IsInstanceOf(pClassName);
}

//____ ClassName() ____________________________________________________________

const char * WgContainer::ClassName( void ) const
{ 
	return CLASSNAME; 
}

//____ Cast() _________________________________________________________________

WgContainerPtr WgContainer::Cast( const WgObjectPtr& pObject )
{
	if( pObject && pObject->IsInstanceOf(CLASSNAME) )
		return WgContainerPtr( static_cast<WgContainer*>(pObject.GetRealPtr()) );

	return 0;
}

//____ IsContainer() ______________________________________________________________

bool WgContainer::IsContainer() const
{
	return true;
}

//____ _isPanel() ______________________________________________________________

bool WgContainer::_isPanel() const
{
	return false;
}


//____ _findWidget() ____________________________________________________________

WgWidget * WgContainer::_findWidget( const WgCoord& ofs, WgSearchMode mode )
{
	WgRect childGeo;
	WgHook * pHook = _lastHookWithGeo( childGeo );
	WgWidget * pResult = 0;

	while( pHook && !pResult )
	{
		bool bVisibleHook = _isPanel()?static_cast<WgPanelHook*>(pHook)->IsVisible():true;

		if( bVisibleHook && childGeo.Contains( ofs ) )
		{
			if( pHook->_widget()->IsContainer() )
			{
				pResult = static_cast<WgContainer*>(pHook->_widget())->_findWidget( ofs - childGeo.Pos(), mode );
			}
			else
			{
				switch( mode )
				{
					case WG_SEARCH_ACTION_TARGET:
					case WG_SEARCH_MARKPOLICY:
						if( pHook->_widget()->MarkTest( ofs - childGeo.Pos() ) )
							pResult = pHook->_widget();
						break;
					case WG_SEARCH_GEOMETRY:
						pResult = pHook->_widget();
						break;
				}
			}
		}
		pHook = _prevHookWithGeo( childGeo, pHook );
	}

	// Return us if search mode is GEOMETRY

	if( !pResult && mode == WG_SEARCH_GEOMETRY )
		pResult = this;

	return pResult;
}

//____ _focusRequested() _______________________________________________________

bool WgContainer::_focusRequested( WgHook * pBranch, WgWidget * pWidgetRequesting )
{
	WgHook * p = Hook();
	if( p )
		return p->Holder()->_focusRequested( p, pWidgetRequesting );
	else
		return false;
}

//____ _focusReleased() ________________________________________________________

bool WgContainer::_focusReleased( WgHook * pBranch, WgWidget * pWidgetReleasing )
{
	WgHook * p = Hook();
	if( p )
		return p->Holder()->_focusReleased( p, pWidgetReleasing );
	else
		return false;
}


WgModalLayer *  WgContainer::_getModalLayer() const
{
	const WgContainer * p = Parent();

	if( p )
		return p->_getModalLayer();
	else
		return 0;
}

WgMenuLayer * WgContainer::_getMenuLayer() const
{
	const WgContainer * p = Parent();

	if( p )
		return p->_getMenuLayer();
	else
		return 0;
}

//____ _onStateChanged() ______________________________________________________

void WgContainer::_onStateChanged( WgState oldState, WgState newState )
{
	WgWidget::_onStateChanged(oldState,newState);

	if( oldState.IsEnabled() != newState.IsEnabled() )
	{
		bool bEnabled = newState.IsEnabled();
		WgWidget * p = _firstWidget();
		while( p )
		{
			p->SetEnabled(bEnabled);
			p = p->NextSibling();
		}
	}
}

//____ _renderPatches() _____________________________________________________
// Default implementation for panel rendering patches.
class WidgetRenderContext
{
public:
	WidgetRenderContext() : pWidget(0) {}
	WidgetRenderContext( WgWidget * pWidget, const WgRect& geo ) : pWidget(pWidget), geo(geo) {}

	WgWidget *	pWidget;
	WgRect		geo;
	WgPatches	patches;
};

void WgContainer::_renderPatches( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{

	// We start by eliminating dirt outside our geometry

	WgPatches 	patches( _pPatches->Size() );								// TODO: Optimize by pre-allocating?

	for( const WgRect * pRect = _pPatches->Begin() ; pRect != _pPatches->End() ; pRect++ )
	{
		if( _canvas.IntersectsWith( *pRect ) )
			patches.Push( WgRect(*pRect,_canvas) );
	}


	// Render container itself
	
	for( const WgRect * pRect = patches.Begin() ; pRect != patches.End() ; pRect++ )
		_onRender(pDevice, _canvas, _window, *pRect );
		
	
	// Render children

	WgRect	dirtBounds = patches.Union();
	
	if( m_bSiblingsOverlap )
	{

		// Create WidgetRenderContext's for siblings that might get dirty patches

		std::vector<WidgetRenderContext> renderList;

		WgRect childGeo;
		WgHook * p = _firstHookWithGeo( childGeo );
		while(p)
		{
			WgRect geo = childGeo + _canvas.Pos();

			bool bVisibleHook = _isPanel()?static_cast<WgPanelHook*>(p)->IsVisible():true;

			if( bVisibleHook && geo.IntersectsWith( dirtBounds ) )
				renderList.push_back( WidgetRenderContext(p->_widget(), geo ) );

			p = _nextHookWithGeo( childGeo, p );
		}

		// Go through WidgetRenderContexts in reverse order (topmost first), push and mask dirt

		for( int i = renderList.size()-1 ; i >= 0 ; i-- )
		{
			WidgetRenderContext * p = &renderList[i];

			p->patches.Push( &patches );

			p->pWidget->_onMaskPatches( patches, p->geo, p->geo, pDevice->GetBlendMode() );		//TODO: Need some optimizations here, grandchildren can be called repeatedly! Expensive!

			if( patches.IsEmpty() )
				break;
		}

		// Go through WidgetRenderContexts and render the patches

		for( int i = 0 ; i < (int) renderList.size() ; i++ )
		{
			WidgetRenderContext * p = &renderList[i];
			p->pWidget->_renderPatches( pDevice, p->geo, p->geo, &p->patches );
		}

	}
	else
	{
		WgRect childGeo;
		WgHook * p = _firstHookWithGeo( childGeo );

		while(p)
		{
			WgRect canvas = childGeo + _canvas.Pos();
			bool bVisibleHook = _isPanel()?static_cast<WgPanelHook*>(p)->IsVisible():true;
			if( bVisibleHook && canvas.IntersectsWith( dirtBounds ) )
				p->_widget()->_renderPatches( pDevice, canvas, canvas, &patches );
			p = _nextHookWithGeo( childGeo, p );
		}

	}
}


//____ _onAlphaTest() _________________________________________________________

bool WgContainer::_onAlphaTest( const WgCoord& ofs )
{
	return false;		// By default cointainers have nothing to display themselves.
}

//____ _onCloneContent() _______________________________________________________

void WgContainer::_onCloneContent( const WgContainer * _pOrg )
{
	m_bSiblingsOverlap 	= _pOrg->m_bSiblingsOverlap;
}

//____ _onCollectPatches() _______________________________________________________

void WgContainer::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
	WgRect childGeo;
	WgHook * p = _firstHookWithGeo( childGeo );

	while(p)
	{
		bool bVisibleHook = _isPanel()?static_cast<WgPanelHook*>(p)->IsVisible():true;
		if( bVisibleHook )
			p->_widget()->_onCollectPatches( container, childGeo + geo.Pos(), clip );
		p = _nextHookWithGeo( childGeo, p );
	}
}

//____ _onMaskPatches() __________________________________________________________

void WgContainer::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
	// Default implementation, should probably be made redundant over time...

	WgRect childGeo;
	WgHook * p = _firstHookWithGeo( childGeo );

	while(p)
	{
		bool bVisibleHook = _isPanel()?static_cast<WgPanelHook*>(p)->IsVisible():true;
		if( bVisibleHook )
			p->_widget()->_onMaskPatches( patches, childGeo + geo.Pos(), clip, blendMode );
		p = _nextHookWithGeo( childGeo, p );
	}
}