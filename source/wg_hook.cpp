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

#include <wg_hook.h>
#include <wg_panel.h>
#include <wg_gizmo.h>
#include <wg_rootpanel.h>
#include <assert.h>


//____ Destructor _____________________________________________________________

WgHook::~WgHook()
{
	if( m_pGizmo )
	{
		m_pGizmo->m_pHook = 0;
		delete m_pGizmo;
	}
}

//____ _attachGizmo() __________________________________________________________

void WgHook::_attachGizmo( WgGizmo * pGizmo )
{
	assert( pGizmo->Parent() == 0 );

	if( m_pGizmo )
		m_pGizmo->m_pHook = 0;

	m_pGizmo = pGizmo;

	if( pGizmo )
		pGizmo->m_pHook = this;
}

//____ _relinkGizmo() __________________________________________________________

void WgHook::_relinkGizmo()
{
	if( m_pGizmo )
		m_pGizmo->m_pHook = this;
}

//____ _releaseGizmo() _________________________________________________________

WgGizmo* WgHook::_releaseGizmo()
{
	WgGizmo * p = m_pGizmo;
	m_pGizmo = 0;

	if( p )
		p->m_pHook = 0;

	return p;
}

//____ _requestFocus() _________________________________________________________

bool WgHook::_requestFocus()
{
	return Parent()->_focusRequested(this, m_pGizmo);
}

//____ _releaseFocus() _________________________________________________________

bool WgHook::_releaseFocus()
{
	return Parent()->_focusReleased(this, m_pGizmo);
}

//____ Root() _________________________________________________________________

WgRootPanel * WgHook::Root() const
{
	WgGizmoContainer * pParent = _parent();

	if( pParent->IsGizmo() )
	{
		WgHook * pHook = pParent->CastToGizmo()->Hook();
		if( pHook )
			return pHook->Root();
	}
	else if( pParent->IsRoot() )
		return pParent->CastToRoot();

	return 0;
}

//____ EventHandler() __________________________________________________________

WgEventHandler * WgHook::EventHandler() const
{
	WgRootPanel * pRoot = Root();
	if( pRoot )
		return pRoot->EventHandler();

	return 0;
}


//____ SetVisible() _____________________________________________________________

bool WgHook::SetVisible( bool bVisible )
{
	if( bVisible != m_bVisible )
	{
		if( bVisible )
		{
			m_bVisible = true;
			_requestRender();
		}
		else
		{
			_requestRender();
			m_bVisible = false;
		}		
	}
	return true;
}

