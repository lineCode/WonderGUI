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

#include <wg_gizmo.h>
#include <wg_types.h>
#include <wg_skinmanager.h>

#	include <wg_rootpanel.h>
#	include <wg_eventhandler.h>

//____ Constructor ____________________________________________________________

WgGizmo::WgGizmo():m_id(0), m_pHook(0), m_pSkinNode(0), m_pointerStyle(WG_POINTER_DEFAULT),
					m_markPolicy( WG_MARKPOLICY_ALPHA ), m_bEnabled(true), m_bOpaque(false),
					m_bFocused(false), m_bTabLock(false), m_bReceiveTick(false),
					m_bRenderOne(false), m_bRendersAll(false)
{
}

//____ Destructor _____________________________________________________________

WgGizmo::~WgGizmo()
{
	if( m_pHook )
	{
		m_pHook->Parent()->ReleaseChild(this);
	}

	delete m_pSkinNode;
}


//____ GetPointerStyle() ________________________________________

WgPointerStyle WgGizmo::GetPointerStyle() const
{
	return m_pointerStyle;
}

//____ SetEnabled() _______________________________________________________________

void WgGizmo::SetEnabled( bool bEnabled )
{
	if( m_bEnabled != bEnabled || IsPanel() )
	{
		m_bEnabled = bEnabled;
		if( bEnabled )
			_onEnable();
		else
			_onDisable();
	}
}

//____ SetSkinManager() _______________________________________________________

void WgGizmo::SetSkinManager( WgSkinManager * pManager )
{
	if( m_pSkinNode )
		delete m_pSkinNode;

	if( pManager )
		m_pSkinNode = pManager->_newNode( this );
	else
		m_pSkinNode = 0;
}

//____ GetSkinManager() _______________________________________________________

WgSkinManager *	WgGizmo::GetSkinManager() const
{
	if( m_pSkinNode )
		return m_pSkinNode->GetManager();

	return 0;
}

//____ MarkTest() _____________________________________________________________

bool WgGizmo::MarkTest( const WgCoord& ofs )
{
	switch( m_markPolicy )
	{
	case WG_MARKPOLICY_ALPHA:
		return _onAlphaTest(ofs);
	case WG_MARKPOLICY_TRANSPARENT:
		return false;
	default:						//WG_MARKPOLICY_OPAQUE:
		return true;
	}
}

//____ CloneContent() _________________________________________________________

bool WgGizmo::CloneContent( const WgGizmo * _pOrg )
{
	if( _pOrg->Type() != Type() )
		return false;

	m_id			= _pOrg->m_id;

	m_pSkinNode		= _pOrg->m_pSkinNode;
	m_pointerStyle 	= _pOrg->m_pointerStyle;

	m_tooltip		= _pOrg->m_tooltip;
	m_markPolicy	= _pOrg->m_markPolicy;

	m_bEnabled		= _pOrg->m_bEnabled;
	m_bOpaque		= _pOrg->m_bOpaque;
	m_bTabLock		= _pOrg->m_bTabLock;

	m_bRenderOne	= _pOrg->m_bRenderOne;
	m_bRendersAll	= _pOrg->m_bRendersAll;

	_onCloneContent( _pOrg );
	return true;
}

//____ _onNewHook() ___________________________________________________________

void WgGizmo::_onNewHook( WgHook * pHook )
{
	m_pHook = pHook;
}

//____ _onNewRoot() ___________________________________________________________

void WgGizmo::_onNewRoot( WgRootPanel * pRoot )
{
	if( m_bReceiveTick && pRoot )
		pRoot->EventHandler()->_addTickReceiver(this);
}

//____ _startReceiveTicks() ___________________________________________________

void WgGizmo::_startReceiveTicks()
{
	if( !m_bReceiveTick )
	{
		m_bReceiveTick = true;

		if( m_pHook )
		{
			WgRootPanel * pRoot = m_pHook->Root();
			if( pRoot )
				pRoot->EventHandler()->_addTickReceiver(this);
		}
	}
}

//____ _stopReceiveTicks() ____________________________________________________

void WgGizmo::_stopReceiveTicks()
{
	m_bReceiveTick = false;
}

//____ _setSkinNode() __________________________________________________________

void WgGizmo::_setSkinNode( WgSkinNode * pNode )
{
	m_pSkinNode = pNode;
	_onRefresh();
}


//____ Local2abs() ____________________________________________________________

WgCoord WgGizmo::Local2abs( const WgCoord& cord ) const
{
	WgCoord c = ScreenPos();
	c.x += cord.x;
	c.y += cord.y;
	return c;
}

//____ Abs2local() ____________________________________________________________

WgCoord WgGizmo::Abs2local( const WgCoord& cord ) const
{
	WgCoord c = ScreenPos();
	return WgCoord( cord.x - c.x, cord.y - c.y );
}

//____ _eventHandler() __________________________________________________________

WgEventHandler * WgGizmo::_eventHandler() const
{
	if( m_pHook )
	{
		WgRootPanel * pRoot = m_pHook->Root();
		if( pRoot )
			return pRoot->EventHandler();
	}
	return 0;
}

//____ HeightForWidth() _______________________________________________________

int WgGizmo::HeightForWidth( int width ) const
{
	return DefaultSize().h;		// Default is to stick with best height no matter what width.
}

//____ WidthForHeight() _______________________________________________________

int WgGizmo::WidthForHeight( int height ) const
{
	return DefaultSize().w;		// Default is to stick with best width no matter what height.
}

//____ SetMarked() ____________________________________________________________

bool WgGizmo::SetMarked()
{
	return false;
}

//____ SetSelected() __________________________________________________________

bool WgGizmo::SetSelected()
{
	return false;
}

//____ SetNormal() ____________________________________________________________

bool WgGizmo::SetNormal()
{
	if( m_bEnabled )
		return true;
	else
		return false;
}

//____ Mode() _________________________________________________________________

WgMode WgGizmo::Mode() const
{
	if( m_bEnabled )
		return WG_MODE_NORMAL;
	else
		return WG_MODE_DISABLED;
}

//____ _getBlendMode() _________________________________________________________

WgBlendMode WgGizmo::_getBlendMode() const
{
	WgGizmoContainer * pParent = Parent();
	if( pParent && pParent->IsGizmo() )
		return pParent->CastToGizmo()->_getBlendMode();
	else
		return WG_BLENDMODE_BLEND;		// We always start out with WG_BLENDMODE_BLEND.
}

//____ _queueEvent() __________________________________________________________

void WgGizmo::_queueEvent( WgEvent::Event * pEvent )
{
	if( m_pHook )
	{
		WgRootPanel * pRoot = m_pHook->Root();
		if( pRoot )
		{
			pRoot->EventHandler()->QueueEvent(pEvent);
			return;
		}
	}

	delete pEvent;		// Can't queue event, silently delete it.
}


//____ _renderPatches() ________________________________________________________

void WgGizmo::_renderPatches( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches, Uint8 _layer )
{
	for( const WgRect * pRect = _pPatches->Begin() ; pRect != _pPatches->End() ; pRect++ )
	{
		WgRect clip( _window, *pRect );
		if( clip.w > 0 || clip.h > 0 )
			_onRender( pDevice, _canvas, _window, clip, _layer );
	}
}

//____ Fillers _______________________________________________________________

void WgGizmo::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
		container.Add( WgRect( geo, clip ) );
}

void WgGizmo::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
	if( (m_bOpaque && blendMode == WG_BLENDMODE_BLEND) || blendMode == WG_BLENDMODE_OPAQUE )
	{
		patches.Sub( WgRect( geo, clip ) );
	}
}

void WgGizmo::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip, Uint8 _layer )
{
}

void WgGizmo::_onNewSize( const WgSize& size )
{
	_requestRender();
}

void WgGizmo::_onRefresh()
{
	_requestRender();
}

void WgGizmo::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	pHandler->ForwardEvent( pEvent );
}

bool WgGizmo::_onAlphaTest( const WgCoord& ofs )
{
	return true;
}

void WgGizmo::_onEnable()
{
	_requestRender();
}

void WgGizmo::_onDisable()
{
	_requestRender();
}

void WgGizmo::_onGotInputFocus()
{
	m_bFocused = true;
}

void WgGizmo::_onLostInputFocus()
{
	m_bFocused = false;
}

bool WgGizmo::TempIsInputField() const
{
	return false;
}

Wg_Interface_TextHolder* WgGizmo::TempGetText()
{
	return 0;
}
