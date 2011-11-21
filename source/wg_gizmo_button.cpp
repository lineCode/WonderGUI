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



#include <wg_gizmo_button.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>
#include <wg_key.h>
#include <wg_event.h>
#include <wg_eventhandler.h>

static const char	c_gizmoType[] = {"Button"};

//____ Constructor ____________________________________________________________

WgGizmoButton::WgGizmoButton()
{
	m_pText = &m_text;
	m_text.setAlignment( WgOrigo::midCenter() );
	m_text.setLineWidth(Size().w);					// We start with no textborders...

 	m_mode				= WG_MODE_NORMAL;

	m_bDownOutside		= false;
	m_aDisplace[0].x	= m_aDisplace[0].y = 0;
	m_aDisplace[1]		= m_aDisplace[2] = m_aDisplace[3] = m_aDisplace[0];

	for( int i = 0 ; i < WG_MAX_BUTTONS ; i++ )
	{
		m_bRenderDown[i] = 0;
		m_bPressedInside[i] = 0;
	}
	m_bRenderDown[0] = true;			// Default is first mouse button...

	m_bReturnPressed = false;
	m_bPointerInside = false;
}

//____ Destructor _____________________________________________________________

WgGizmoButton::~WgGizmoButton()
{
}

//____ Type() _________________________________________________________________

const char * WgGizmoButton::Type( void ) const
{
	return GetMyType();
}

//____ GetMyType() ____________________________________________________________

const char * WgGizmoButton::GetMyType()
{
	return c_gizmoType;
}

//____ SetSource() ____________________________________________________________

bool WgGizmoButton::SetSource( const WgBlockSetPtr& pGfx )
{
	m_pBgGfx = pGfx;

	if( pGfx && pGfx->IsOpaque() )
		m_bOpaque = true;
	else
		m_bOpaque = false;

	RequestRender();
	return true;
}

//____ SetIcon() ______________________________________________________________

void WgGizmoButton::SetIcon( const WgBlockSetPtr& pIconGfx )
{
	m_pIconGfx = pIconGfx;
	_iconModified();
}

bool WgGizmoButton::SetIcon( const WgBlockSetPtr& pIconGfx, WgOrientation orientation, WgBorders borders, float scale, bool bPushText )
{
	if( scale < 0 || scale > 1.f )
		return false;

	m_pIconGfx = pIconGfx;
	m_iconOrientation = orientation;
	m_iconBorders = borders;
	m_iconScale = scale;
	m_bIconPushText = bPushText;

	_iconModified();
	return true;
}


//____ GetTextAreaWidth() _____________________________________________________

Uint32 WgGizmoButton::GetTextAreaWidth()
{
	WgRect	contentRect(0,0,Size());

	if( m_pBgGfx )
		contentRect.Shrink(m_pBgGfx->ContentBorders());

	WgRect textRect = _getTextRect( contentRect, _getIconRect( contentRect, m_pIconGfx ) );

	return textRect.w;
}

//____ SetDisplacement() _______________________________________________________

bool WgGizmoButton::SetDisplacement( Sint8 _xUp, Sint8 _yUp, Sint8 _xOver, Sint8 _yOver, Sint8 _xDown, Sint8 _yDown )
{
  m_aDisplace[WG_MODE_NORMAL].x = _xUp;
  m_aDisplace[WG_MODE_NORMAL].y = _yUp;

  m_aDisplace[WG_MODE_MARKED].x = _xOver;
  m_aDisplace[WG_MODE_MARKED].y = _yOver;

  m_aDisplace[WG_MODE_SELECTED].x = _xDown;
  m_aDisplace[WG_MODE_SELECTED].y = _yDown;

  return  true;
}

void WgGizmoButton::GetDisplacement( Sint8& xUp, Sint8& yUp, Sint8& xOver, Sint8& yOver, Sint8& xDown, Sint8& yDown ) const
{
	xUp = m_aDisplace[WG_MODE_NORMAL].x;
	yUp = m_aDisplace[WG_MODE_NORMAL].y;

	xOver = m_aDisplace[WG_MODE_MARKED].x;
	yOver = m_aDisplace[WG_MODE_MARKED].y;

	xDown = m_aDisplace[WG_MODE_SELECTED].x;
	yDown = m_aDisplace[WG_MODE_SELECTED].y;
}


//____ HeightForWidth() _______________________________________________________

int WgGizmoButton::HeightForWidth( int width ) const
{
	int height = 0;

	if( m_pBgGfx )
		height = m_pBgGfx->Height();

	if( m_text.nbChars() != 0 )
	{
		WgBorders borders;

		if( m_pBgGfx )
			borders = m_pBgGfx->ContentBorders();

		int heightForText = m_text.heightForWidth(width-borders.Width()) + borders.Height();
		if( heightForText > height )
			height = heightForText;
	}

	//TODO: Take icon into account.

	return height;
}


//____ DefaultSize() _____________________________________________________________

WgSize WgGizmoButton::DefaultSize() const
{
	WgSize bestSize;

	if( m_pBgGfx )
		bestSize = m_pBgGfx->Size();

	if( m_text.nbChars() != 0 )
	{
		WgSize textSize = m_text.unwrappedSize();

		if( m_pBgGfx )
			textSize += m_pBgGfx->ContentBorders();

		if( textSize.w > bestSize.w )
			bestSize.w = textSize.w;

		if( textSize.h > bestSize.h )
			bestSize.h = textSize.h;
	}

	//TODO: Take icon into account.

	return bestSize;
}


//____ _onEnable() _____________________________________________________________

void WgGizmoButton::_onEnable()
{
	m_mode = WG_MODE_NORMAL;
	RequestRender();
}

//____ _onDisable() ____________________________________________________________

void WgGizmoButton::_onDisable()
{
	m_mode = WG_MODE_DISABLED;
	RequestRender();
}

//____ _onNewSize() ____________________________________________________________

void WgGizmoButton::_onNewSize( const WgSize& size )
{
	WgRect	contentRect(0,0,Size());

	if( m_pBgGfx )
		contentRect.Shrink(m_pBgGfx->ContentBorders());

	WgRect textRect = _getTextRect( contentRect, _getIconRect( contentRect, m_pIconGfx ) );

	m_text.setLineWidth(textRect.w);
}


//____ _onRender() _____________________________________________________________

void WgGizmoButton::_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip, Uint8 _layer )
{
	WgRect cli = _clip;
	WgRect can = _canvas;
	WgRect win = _window;

	// Render background

	if( m_pBgGfx )
		pDevice->ClipBlitBlock( _clip, m_pBgGfx->GetBlock(m_mode, _canvas), _canvas );

	// Get content rect with displacement.

	WgRect contentRect = _canvas;
	if( m_pBgGfx )
		contentRect -= m_pBgGfx->ContentBorders();

	contentRect.x += m_aDisplace[m_mode].x;
	contentRect.y += m_aDisplace[m_mode].y;

	// Get icon and text rect from content rect

	WgRect iconRect = _getIconRect( contentRect, m_pIconGfx );
	WgRect textRect = _getTextRect( contentRect, iconRect );

	// Render icon

	if( m_pIconGfx )
		pDevice->ClipBlitBlock( _clip, m_pIconGfx->GetBlock(m_mode, iconRect.Size()), iconRect );

	// Print text

 	if( !m_text.IsEmpty() )
	{
		m_text.setMode(m_mode);

		if( m_pBgGfx )
			m_text.SetBgBlockColors( m_pBgGfx->TextColors() );

		WgRect clip(textRect,_clip);
		pDevice->PrintText( clip, &m_text, textRect );
	}
}

//____ _onEvent() ______________________________________________________________

void WgGizmoButton::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	switch( pEvent->Type() )
	{
		case	WG_EVENT_KEY_PRESS:
		{
			if( static_cast<const WgEvent::KeyPress*>(pEvent)->TranslatedKeyCode() == WG_KEY_RETURN )
				m_bReturnPressed = true;
			break;
		}

		case	WG_EVENT_KEY_RELEASE:
		{
			if( static_cast< const WgEvent::KeyPress*>(pEvent)->TranslatedKeyCode() == WG_KEY_RETURN )
			{
				m_bReturnPressed = false;
				pHandler->QueueEvent( new WgEvent::ButtonPress(this) );
			}
			break;
		}

		case	WG_EVENT_MOUSE_ENTER:
			m_bPointerInside = true;
			break;

		case	WG_EVENT_MOUSE_LEAVE:
			m_bPointerInside = false;
			break;

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			int button = static_cast<const WgEvent::MouseButtonPress*>(pEvent)->Button();
			m_bPressedInside[button-1] = true;
			break;
		}
		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			const WgEvent::MouseButtonRelease* pEv = static_cast<const WgEvent::MouseButtonRelease*>(pEvent);
			int button = pEv->Button();
			m_bPressedInside[button-1] = false;
			break;
		}
		
		case WG_EVENT_MOUSEBUTTON_CLICK:
		{
			const WgEvent::MouseButtonClick* pEv = static_cast<const WgEvent::MouseButtonClick*>(pEvent);
			if( pEv->Button() == 1 )
				pHandler->QueueEvent( new WgEvent::ButtonPress(this) );			
			break;
		}
		
        default:
            break;

	}

	WgMode newMode = _getRenderMode();
	if( newMode != m_mode )
	{
		m_mode = newMode;
		RequestRender();
	}

}


//____ _onAction() _____________________________________________________________

void WgGizmoButton::_onAction( WgInput::UserAction action, int button, const WgActionDetails& info, const WgInput& inputObj )
{
//	if( !m_bEnabled )
//		return;

	switch( action )
	{
		case	WgInput::KEY_PRESS:
		{
			if( button == WG_KEY_RETURN )
				m_bReturnPressed = true;
			break;
		}

		case	WgInput::KEY_RELEASE:
		{
			if( button == WG_KEY_RETURN )
				m_bReturnPressed = false;
			break;

			//TODO: Send signal!
		}

		case	WgInput::POINTER_ENTER:
			m_bPointerInside = true;
			break;

		case	WgInput::POINTER_EXIT:
			m_bPointerInside = false;
			break;

		case WgInput::BUTTON_PRESS:
			m_bPressedInside[button-1] = true;
			break;

		case WgInput::BUTTON_RELEASE:
			//TODO: Send signal if right button was released!
		case WgInput::BUTTON_RELEASE_OUTSIDE:
			m_bPressedInside[button-1] = false;
			break;

        default:
            break;

	}

	WgMode newMode = _getRenderMode();
	if( newMode != m_mode )
	{
		m_mode = newMode;
		RequestRender();
	}
}

//_____ _getRenderMode() ________________________________________________________

WgMode WgGizmoButton::_getRenderMode()
{
	if( !IsEnabled() )
		return WG_MODE_DISABLED;

	if( m_bReturnPressed )
		return WG_MODE_SELECTED;

	if( m_bPointerInside || m_bDownOutside )
	{
		for( int i = 0 ; i < WG_MAX_BUTTONS ; i++ )
		{
			if( m_bRenderDown[i] && m_bPressedInside[i] )
				return WG_MODE_SELECTED;
		}
	}

	if( m_bPointerInside )
		return WG_MODE_MARKED;

	return WG_MODE_NORMAL;
}


//____ _onRefresh() ____________________________________________________________

void WgGizmoButton::_onRefresh( void )
{
	if( m_pBgGfx )
	{
		if( m_pBgGfx->IsOpaque() )
			m_bOpaque = true;
		else
			m_bOpaque = false;

		RequestRender();
	}
}


//____ SetPressAnim() __________________________________________________________

void	WgGizmoButton::SetPressAnim( bool _button1, bool _button2, bool _button3, bool _bDownOutside )
{
		m_bRenderDown[0]	= _button1;
		m_bRenderDown[1]	= _button2;
		m_bRenderDown[2]	= _button3;
		m_bDownOutside		= _bDownOutside;
}

//____ GetPressAnim() _________________________________________________________

void WgGizmoButton::GetPressAnim( bool& button1, bool& button2, bool& button3, bool& bDownWhenMouseOutside )
{
	button1 = m_bRenderDown[0];
	button2 = m_bRenderDown[1];
	button3 = m_bRenderDown[2];
	bDownWhenMouseOutside = m_bDownOutside;
}


//____ _onCloneContent() _______________________________________________________

void WgGizmoButton::_onCloneContent( const WgGizmo * _pOrg )
{

	WgGizmoButton * pOrg = (WgGizmoButton *) _pOrg;

	pOrg->Wg_Interface_TextHolder::_cloneInterface( this );

	m_text.setText(&pOrg->m_text);
	m_pText = &m_text;
	m_text.setHolder( this );

	m_pBgGfx		= pOrg->m_pBgGfx;
	m_pIconGfx		= pOrg->m_pIconGfx;
	m_mode			= pOrg->m_mode;

	for( int i = 0 ; i < WG_MAX_BUTTONS ; i++ )
		m_aDisplace[i]	= pOrg->m_aDisplace[i];

	for( int i = 0 ; i < WG_MAX_BUTTONS ; i++ )
	 	m_bRenderDown[i] = pOrg->m_bRenderDown[i];
}

//____ _onAlphaTest() ___________________________________________________________

bool WgGizmoButton::_onAlphaTest( const WgCoord& ofs )
{
	if( !m_pBgGfx )
		return false;

	WgSize	sz = Size();

	//TODO: Take icon into account.

	return	WgUtil::MarkTestBlock( ofs, m_pBgGfx->GetBlock(m_mode,sz), WgRect(0,0,sz) );
}

//____ _onGotInputFocus() ______________________________________________________

void WgGizmoButton::_onGotInputFocus()
{
	m_bFocused = true;
	RequestRender();
}

//____ _onLostInputFocus() _____________________________________________________

void WgGizmoButton::_onLostInputFocus()
{
	m_bFocused = false;
	m_bReturnPressed = false;
	RequestRender();
}


//____ _textModified() __________________________________________________________

void WgGizmoButton::_textModified()
{
	RequestRender();
}

//____ _iconModified() __________________________________________________________

void WgGizmoButton::_iconModified()
{
	//TODO: Should possibly refresh size too.
	RequestRender();
}
