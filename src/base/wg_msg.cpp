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

#include <wg_msg.h>
#include <wg_key.h>
#include <wg_widget.h>
#include <wg_button.h>
#include <wg_animplayer.h>
#include <wg_valuedisplay.h>
#include <wg_valueeditor.h>
#include <wg_scrollbar.h>
#include <wg_textdisplay.h>
#include <wg_lineeditor.h>
#include <wg_menu.h>
#include <wg_legacytextfield.h>

namespace wg 
{
	//#include <wg_tablepanel.h>
	
	
	//____ Msg ______________________________________________________________
	
	const char Msg::CLASSNAME[] = {"Msg"};
	
	bool Msg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Object::isInstanceOf(pClassName);
	}
	
	const char * Msg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	Msg_p Msg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return Msg_p( static_cast<Msg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	void Msg::setCopyTo( const Receiver_p& pReceiver )
	{
		m_pCopyTo = pReceiver;
	}
	
	void Msg::setRepost( const Object_p& pSource, const Receiver_p& pCopyTo )
	{
		m_pRepostSource = pSource;	
		m_pRepostCopyTo = pCopyTo;
	}
	
	void Msg::swallow()
	{
		m_pRepostSource = 0;
		m_pRepostCopyTo = 0;
	}
	
	bool Msg::doRepost()
	{
		if( m_pRepostSource )
		{
			m_pSource = m_pRepostSource;
			m_pCopyTo = m_pRepostCopyTo;
			
			m_pRepostSource = 0;
			m_pRepostCopyTo = 0;
			
			return true;
		}
		return false;
	}
	
	
	
	bool Msg::isMouseMsg() const
	{
		if( m_type == WG_MSG_WHEEL_ROLL ||
			m_type == WG_MSG_MOUSE_ENTER ||
			m_type == WG_MSG_MOUSE_LEAVE ||	
			m_type == WG_MSG_MOUSE_MOVE ||	
			m_type == WG_MSG_MOUSE_POSITION ||
			isMouseButtonMsg() )
			return true;
		else
			return false;
				
	}
		
	bool Msg::isMouseButtonMsg() const
	{
		if( m_type == WG_MSG_MOUSE_CLICK ||
			m_type == WG_MSG_MOUSE_DOUBLE_CLICK ||
			m_type == WG_MSG_MOUSE_DRAG ||
			m_type == WG_MSG_MOUSE_PRESS ||
			m_type == WG_MSG_MOUSE_RELEASE ||
			m_type == WG_MSG_MOUSE_REPEAT )
			return true;
		else
			return false;
	}
		
	bool Msg::isKeyMsg() const
	{
		if( m_type == WG_MSG_KEY_PRESS ||
			m_type == WG_MSG_KEY_RELEASE ||
			m_type == WG_MSG_KEY_REPEAT )
			return true;
		else
			return false;
	}
	
	//____ KeyMsg ____________________________________________________________
	
	const char KeyMsg::CLASSNAME[] = {"KeyMsg"};
	
	bool KeyMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * KeyMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	KeyMsg_p KeyMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return KeyMsg_p( static_cast<KeyMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	bool KeyMsg::isCursorKey() const
	{
		if( m_translatedKeyCode == WG_KEY_UP || m_translatedKeyCode == WG_KEY_DOWN ||
			m_translatedKeyCode == WG_KEY_LEFT || m_translatedKeyCode == WG_KEY_RIGHT )
			return true;
		else
			return false;
	}
		
	bool KeyMsg::isMovementKey() const
	{
		if( m_translatedKeyCode == WG_KEY_PAGE_UP || m_translatedKeyCode == WG_KEY_PAGE_DOWN ||
			m_translatedKeyCode == WG_KEY_HOME || m_translatedKeyCode == WG_KEY_END ||
			isCursorKey() )
			return true;
		else
			return false;
	}
		
	//____ MouseButtonMsg _____________________________________________________
	
	const char MouseButtonMsg::CLASSNAME[] = {"MouseButtonMsg"};
	
	bool MouseButtonMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * MouseButtonMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	MouseButtonMsg_p MouseButtonMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return MouseButtonMsg_p( static_cast<MouseButtonMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	
	//____ FocusGainedMsg ________________________________________________________
	
	const char FocusGainedMsg::CLASSNAME[] = {"FocusGainedMsg"};
	
	FocusGainedMsg::FocusGainedMsg()
	{
		m_type = WG_MSG_FOCUS_GAINED;
	}
	
	bool FocusGainedMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * FocusGainedMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	FocusGainedMsg_p FocusGainedMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return FocusGainedMsg_p( static_cast<FocusGainedMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	
	//____ FocusLostMsg __________________________________________________________
	
	const char FocusLostMsg::CLASSNAME[] = {"FocusLostMsg"};
	
	FocusLostMsg::FocusLostMsg()
	{
		m_type = WG_MSG_FOCUS_LOST;
	}
	
	bool FocusLostMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * FocusLostMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	FocusLostMsg_p FocusLostMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return FocusLostMsg_p( static_cast<FocusLostMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	
	//____ MouseEnterMsg _______________________________________________________
	
	const char MouseEnterMsg::CLASSNAME[] = {"MouseEnterMsg"};
	
	MouseEnterMsg::MouseEnterMsg( const Coord& pos )
	{
		m_type = WG_MSG_MOUSE_ENTER;
	
		m_pointerPos = pos;
	}
	
	MouseEnterMsg::MouseEnterMsg( Widget * pWidget )
	{
		m_type = WG_MSG_MOUSE_ENTER;
		m_pSource	= pWidget;
		m_pCopyTo 	= pWidget;
	}
	
	bool MouseEnterMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * MouseEnterMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	MouseEnterMsg_p MouseEnterMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return MouseEnterMsg_p( static_cast<MouseEnterMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	
	//____ MouseLeaveMsg ________________________________________________________
	
	const char MouseLeaveMsg::CLASSNAME[] = {"MouseLeaveMsg"};
	
	MouseLeaveMsg::MouseLeaveMsg()
	{
		m_type = WG_MSG_MOUSE_LEAVE;
	}
	
	MouseLeaveMsg::MouseLeaveMsg( Widget * pWidget )
	{
		m_type = WG_MSG_MOUSE_LEAVE;
		m_pSource		= pWidget;
		m_pCopyTo 		= pWidget;
	}
	
	bool MouseLeaveMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * MouseLeaveMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	MouseLeaveMsg_p MouseLeaveMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return MouseLeaveMsg_p( static_cast<MouseLeaveMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ MouseMoveMsg __________________________________________________________
	
	const char MouseMoveMsg::CLASSNAME[] = {"MouseMoveMsg"};
	
	MouseMoveMsg::MouseMoveMsg( const Coord& pos )
	{
		m_type = WG_MSG_MOUSE_MOVE;
	
		m_pointerPos = pos;
	}
	
	MouseMoveMsg::MouseMoveMsg( Widget * pWidget )
	{
		m_type = WG_MSG_MOUSE_MOVE;
		m_pSource		= pWidget;
		m_pCopyTo 		= pWidget;
	}
	
	bool MouseMoveMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * MouseMoveMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	MouseMoveMsg_p MouseMoveMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return MouseMoveMsg_p( static_cast<MouseMoveMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	
	//____ MousePressMsg ______________________________________________________
	
	const char MousePressMsg::CLASSNAME[] = {"MousePressMsg"};
	
	MousePressMsg::MousePressMsg( WgMouseButton button ) : MouseButtonMsg(button)
	{
		m_type = WG_MSG_MOUSE_PRESS;
	}
	
	MousePressMsg::MousePressMsg( WgMouseButton button, Widget * pWidget ) : MouseButtonMsg(button)
	{
		m_type			= WG_MSG_MOUSE_PRESS;
		m_pSource		= pWidget;
		m_pCopyTo 		= pWidget;
	}
	
	bool MousePressMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return MouseButtonMsg::isInstanceOf(pClassName);
	}
	
	const char * MousePressMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	MousePressMsg_p MousePressMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return MousePressMsg_p( static_cast<MousePressMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ MouseRepeatMsg ______________________________________________________
	
	const char MouseRepeatMsg::CLASSNAME[] = {"MouseRepeatMsg"};
	
	MouseRepeatMsg::MouseRepeatMsg( WgMouseButton button ) : MouseButtonMsg(button)
	{
		m_type = WG_MSG_MOUSE_REPEAT;
	}
	
	MouseRepeatMsg::MouseRepeatMsg( WgMouseButton button, Widget * pWidget ) : MouseButtonMsg(button)
	{
		m_type			= WG_MSG_MOUSE_REPEAT;
		m_pSource		= pWidget;
		m_pCopyTo 		= pWidget;
	}
	
	bool MouseRepeatMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return MouseButtonMsg::isInstanceOf(pClassName);
	}
	
	const char * MouseRepeatMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	MouseRepeatMsg_p MouseRepeatMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return MouseRepeatMsg_p( static_cast<MouseRepeatMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	
	//____ MouseReleaseMsg _____________________________________________________
	
	const char MouseReleaseMsg::CLASSNAME[] = {"MouseReleaseMsg"};
	
	MouseReleaseMsg::MouseReleaseMsg( WgMouseButton button ) : MouseButtonMsg(button)
	{
		m_type = WG_MSG_MOUSE_RELEASE;
	
		m_bReleaseInside = true;			// Always assumed to have had the press inside our window.
		m_bReleaseInside = true;			// Always assumed to be inside our window now.
	}
	
	MouseReleaseMsg::MouseReleaseMsg( WgMouseButton button, Widget * pWidget, bool bPressInside, bool bReleaseInside ) : MouseButtonMsg(button)
	{
		m_type			= WG_MSG_MOUSE_RELEASE;
		m_pSource		= pWidget;
		m_pCopyTo 		= pWidget;
	
		m_bPressInside = bPressInside;
		m_bReleaseInside = bReleaseInside;
	}
	
	bool MouseReleaseMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return MouseButtonMsg::isInstanceOf(pClassName);
	}
	
	const char * MouseReleaseMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	MouseReleaseMsg_p MouseReleaseMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return MouseReleaseMsg_p( static_cast<MouseReleaseMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	bool MouseReleaseMsg::pressInside() const
	{
		return m_bPressInside;
	}
	
	bool MouseReleaseMsg::releaseInside() const
	{
		return m_bReleaseInside;
	}
	
	
	
	//____ MouseClickMsg _____________________________________________________
	
	const char MouseClickMsg::CLASSNAME[] = {"MouseClickMsg"};
	
	MouseClickMsg::MouseClickMsg( WgMouseButton button ) : MouseButtonMsg(button)
	{
		m_type = WG_MSG_MOUSE_CLICK;
	}
	
	MouseClickMsg::MouseClickMsg( WgMouseButton button, Widget * pWidget ) : MouseButtonMsg(button)
	{
		m_type = WG_MSG_MOUSE_CLICK;
		m_pSource		= pWidget;
		m_pCopyTo 		= pWidget;
	}
	
	bool MouseClickMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return MouseButtonMsg::isInstanceOf(pClassName);
	}
	
	const char * MouseClickMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	MouseClickMsg_p MouseClickMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return MouseClickMsg_p( static_cast<MouseClickMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ MouseDoubleClickMsg _____________________________________________________
	
	const char MouseDoubleClickMsg::CLASSNAME[] = {"MouseDoubleClickMsg"};
	
	MouseDoubleClickMsg::MouseDoubleClickMsg( WgMouseButton button ) : MouseButtonMsg(button)
	{
		m_type = WG_MSG_MOUSE_DOUBLE_CLICK;
	}
	
	MouseDoubleClickMsg::MouseDoubleClickMsg( WgMouseButton button, Widget * pWidget ) : MouseButtonMsg(button)
	{
		m_type = WG_MSG_MOUSE_DOUBLE_CLICK;
		m_pSource		= pWidget;
		m_pCopyTo		= pWidget;
	}
	
	bool MouseDoubleClickMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return MouseButtonMsg::isInstanceOf(pClassName);
	}
	
	const char * MouseDoubleClickMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	MouseDoubleClickMsg_p MouseDoubleClickMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return MouseDoubleClickMsg_p( static_cast<MouseDoubleClickMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ KeyPressMsg ___________________________________________________________
	
	const char KeyPressMsg::CLASSNAME[] = {"KeyPressMsg"};
	
	KeyPressMsg::KeyPressMsg( int native_keycode ) : KeyMsg(native_keycode)
	{
		m_type 			= WG_MSG_KEY_PRESS;
	}
	
	KeyPressMsg::KeyPressMsg( int native_keycode, Widget * pWidget ) : KeyMsg(native_keycode)
	{
		m_type 			= WG_MSG_KEY_PRESS;
		m_pSource		= pWidget;
		m_pCopyTo		= pWidget;
	}
	
	bool KeyPressMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return KeyMsg::isInstanceOf(pClassName);
	}
	
	const char * KeyPressMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	KeyPressMsg_p KeyPressMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return KeyPressMsg_p( static_cast<KeyPressMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ KeyRepeatMsg ___________________________________________________________
	
	const char KeyRepeatMsg::CLASSNAME[] = {"KeyRepeatMsg"};
	
	KeyRepeatMsg::KeyRepeatMsg( int native_keycode ) : KeyMsg(native_keycode)
	{
		m_type 			= WG_MSG_KEY_REPEAT;
	}
	
	KeyRepeatMsg::KeyRepeatMsg( int native_keycode, Widget * pWidget ) : KeyMsg(native_keycode)
	{
		m_type 			= WG_MSG_KEY_REPEAT;
		m_pSource		= pWidget;
		m_pCopyTo 		= pWidget;
	}
	
	bool KeyRepeatMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return KeyMsg::isInstanceOf(pClassName);
	}
	
	const char * KeyRepeatMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	KeyRepeatMsg_p KeyRepeatMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return KeyRepeatMsg_p( static_cast<KeyRepeatMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ KeyReleaseMsg ________________________________________________________
	
	const char KeyReleaseMsg::CLASSNAME[] = {"KeyReleaseMsg"};
	
	KeyReleaseMsg::KeyReleaseMsg( int native_keycode ) : KeyMsg(native_keycode)
	{
		m_type 		= WG_MSG_KEY_RELEASE;
	}
	
	KeyReleaseMsg::KeyReleaseMsg( int native_keycode, Widget * pWidget ) : KeyMsg(native_keycode)
	{
		m_type 			= WG_MSG_KEY_RELEASE;
		m_pSource		= pWidget;
		m_pCopyTo		= pWidget;
	}
	
	bool KeyReleaseMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return KeyMsg::isInstanceOf(pClassName);
	}
	
	const char * KeyReleaseMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	KeyReleaseMsg_p KeyReleaseMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return KeyReleaseMsg_p( static_cast<KeyReleaseMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ CharacterMsg __________________________________________________________
	
	const char CharacterMsg::CLASSNAME[] = {"CharacterMsg"};
	
	CharacterMsg::CharacterMsg( unsigned short character )
	{
		m_type = WG_MSG_CHARACTER;
		m_char = character;
	}
	
	CharacterMsg::CharacterMsg( unsigned short character, Widget * pWidget )
	{
		m_type			= WG_MSG_CHARACTER;
		m_char			= character;
		m_pSource		= pWidget;
		m_pCopyTo		= pWidget;
	}
	
	unsigned short CharacterMsg::character() const
	{
		return m_char;
	}
	
	bool CharacterMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * CharacterMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	CharacterMsg_p CharacterMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return CharacterMsg_p( static_cast<CharacterMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	
	//____ WheelRollMsg __________________________________________________________
	
	const char WheelRollMsg::CLASSNAME[] = {"WheelRollMsg"};
	
	WheelRollMsg::WheelRollMsg( int wheel, Coord distance )
	{
		m_type = WG_MSG_WHEEL_ROLL;
		m_wheel = wheel;
		m_distance = distance;
	}
	
	WheelRollMsg::WheelRollMsg( int wheel, Coord distance, Widget * pWidget )
	{
		m_type			= WG_MSG_WHEEL_ROLL;
		m_wheel			= wheel;
		m_distance		= distance;
		m_pSource		= pWidget;
		m_pCopyTo		= pWidget;
	}
	
	int WheelRollMsg::wheel() const
	{
		return m_wheel;
	}
	
	Coord WheelRollMsg::distance() const
	{
		return m_distance;
	}
	
	bool WheelRollMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * WheelRollMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	WheelRollMsg_p WheelRollMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return WheelRollMsg_p( static_cast<WheelRollMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ TickMsg _______________________________________________________________
	
	const char TickMsg::CLASSNAME[] = {"TickMsg"};
	
	TickMsg::TickMsg( int ms )
	{
		m_type = WG_MSG_TICK;
		m_millisec = ms;
	}
	
	TickMsg::TickMsg( int ms, Receiver * pReceiver )
	{
		m_type 			= WG_MSG_TICK;
		m_millisec 		= ms;
		m_pCopyTo 		= pReceiver;
	}
		
	int TickMsg::millisec() const
	{
		return m_millisec;
	}
	
	bool TickMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * TickMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	TickMsg_p TickMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return TickMsg_p( static_cast<TickMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ PointerChangeMsg _______________________________________________________________
	
	const char PointerChangeMsg::CLASSNAME[] = {"PointerChangeMsg"};
	
	PointerChangeMsg::PointerChangeMsg( WgPointerStyle style )
	{
		m_type = WG_MSG_POINTER_CHANGE;
		m_style = style;
	}
		
	WgPointerStyle PointerChangeMsg::style() const
	{
		return m_style;
	}
	
	bool PointerChangeMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * PointerChangeMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	PointerChangeMsg_p PointerChangeMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return PointerChangeMsg_p( static_cast<PointerChangeMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ SelectMsg _________________________________________________________
	
	const char SelectMsg::CLASSNAME[] = {"SelectMsg"};
	
	SelectMsg::SelectMsg( Object * pSource )
	{
		m_type 			= WG_MSG_SELECT;
		m_pSource 		= pSource;
	}
	
	bool SelectMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * SelectMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	SelectMsg_p SelectMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return SelectMsg_p( static_cast<SelectMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ ToggleMsg ______________________________________________
	
	const char ToggleMsg::CLASSNAME[] = {"ToggleMsg"};
	
	ToggleMsg::ToggleMsg( Object * pSource, bool bSet )
	{
		m_type = WG_MSG_TOGGLE;
		m_pSource = pSource;
		m_bSet = bSet;
	}
	
	bool ToggleMsg::isSet() const
	{
		return m_bSet;
	}
	
	bool ToggleMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * ToggleMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	ToggleMsg_p ToggleMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return ToggleMsg_p( static_cast<ToggleMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ ValueChangeMsg ___________________________________________
	
	const char ValueUpdateMsg::CLASSNAME[] = {"ValueUpdateMsg"};
	
	ValueUpdateMsg::ValueUpdateMsg( Object * pSource, int64_t value, double fraction, bool bFinal )
	{
		m_type = WG_MSG_VALUE_UPDATE;
		m_pSource = pSource;
		m_value = value;
		m_fraction = fraction;
		m_bFinal = bFinal;
	}
	
	int64_t ValueUpdateMsg::value() const
	{
		return m_value;
	}
	
	double ValueUpdateMsg::fraction() const
	{
		return m_fraction;
	}
	
	bool ValueUpdateMsg::isFinal() const
	{
		return m_bFinal;
	}
	
	bool ValueUpdateMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * ValueUpdateMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	ValueUpdateMsg_p ValueUpdateMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return ValueUpdateMsg_p( static_cast<ValueUpdateMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ RangeUpdateMsg ___________________________________________
	
	const char RangeUpdateMsg::CLASSNAME[] = {"RangeUpdateMsg"};
	
	RangeUpdateMsg::RangeUpdateMsg( Object * pSource, int intOfs, int intLength, double fracOfs, double fracLength, bool bFinal )
	{
		m_type = WG_MSG_RANGE_UPDATE;
		m_pSource 	= pSource;
		m_ofs		= intOfs;
		m_length	= intLength;
		m_fracOfs	= fracOfs;
		m_fracLength= fracLength;
		m_bFinal	= bFinal;
	}
	
	bool RangeUpdateMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * RangeUpdateMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	RangeUpdateMsg_p RangeUpdateMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return RangeUpdateMsg_p( static_cast<RangeUpdateMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	int RangeUpdateMsg::offset() const
	{
		return m_ofs;
	}
	
	int RangeUpdateMsg::length() const
	{
		return m_length;
	}
	
	double RangeUpdateMsg::fracOffset() const
	{
		return m_fracOfs;
	}
	
	double RangeUpdateMsg::fracLength() const
	{
		return m_fracLength;
	}
	
	bool RangeUpdateMsg::isFinal() const
	{
		return m_bFinal;
	}
	
	
	//____ TextEditMsg ________________________________________________________
	
	const char TextEditMsg::CLASSNAME[] = {"TextEditMsg"};
	
	EditText_p TextEditMsg::text() const
	{
		return m_pText;
	}
	
	TextEditMsg::TextEditMsg( const EditText_p& pText, bool bFinal )
	{
		m_type 		= WG_MSG_TEXT_EDIT;
		m_pSource 	= pText->holder();
		m_pText 	= pText;
		m_bFinal	= bFinal;
	}
	
	bool TextEditMsg::isFinal() const
	{
		return m_bFinal;
	}
	
	bool TextEditMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * TextEditMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	TextEditMsg_p TextEditMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return TextEditMsg_p( static_cast<TextEditMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ ItemMsg ________________________________________________________
	
	const char ItemMsg::CLASSNAME[] = {"ItemMsg"};
	
	ItemMsg::ItemMsg( Object * pSource, int itemId, const Object_p& pItemObject ) : m_item(itemId,pItemObject)
	{
		m_pSource 	= pSource;
	}
	
	bool ItemMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * ItemMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	ItemMsg_p ItemMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return ItemMsg_p( static_cast<ItemMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	int ItemMsg::itemId() const 
	{ 
		return m_item.id; 
	}
	
	Object_p ItemMsg::itemObject() const 
	{ 
		return m_item.pObject; 
	}
	
	//____ ItemToggleMsg ________________________________________________________
	
	const char ItemToggleMsg::CLASSNAME[] = {"ItemToggleMsg"};
	
	ItemToggleMsg::ItemToggleMsg( Object * pSource, int itemId, const Object_p& pItemObject, bool bSet ) : ItemMsg(pSource,itemId,pItemObject)
	{
		m_type = WG_MSG_ITEM_TOGGLE;
		m_bSet = bSet;
	}
	
	bool ItemToggleMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return ItemMsg::isInstanceOf(pClassName);
	}
	
	const char * ItemToggleMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	ItemToggleMsg_p ItemToggleMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return ItemToggleMsg_p( static_cast<ItemToggleMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	bool ItemToggleMsg::isSet() const
	{
		return m_bSet;
	}
	
	//____ ItemMousePressMsg ________________________________________________________
	
	const char ItemMousePressMsg::CLASSNAME[] = {"ItemMousePressMsg"};
	
	ItemMousePressMsg::ItemMousePressMsg( Object * pSource, int itemId, const Object_p& pItemObject, WgMouseButton button ) : ItemMsg(pSource,itemId,pItemObject)
	{
		m_type = WG_MSG_ITEM_MOUSE_PRESS;
		m_button = button;
	}
	
	bool ItemMousePressMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return ItemMsg::isInstanceOf(pClassName);
	}
	
	const char * ItemMousePressMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	ItemMousePressMsg_p ItemMousePressMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return ItemMousePressMsg_p( static_cast<ItemMousePressMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	WgMouseButton ItemMousePressMsg::button() const
	{
		return m_button;
	}
	
	//____ ItemListMsg ________________________________________________________
	
	const char ItemListMsg::CLASSNAME[] = {"ItemListMsg"};
	
	ItemListMsg::ItemListMsg( Object * pSource, int nbItems, ItemInfo * pItems )
	{
		m_pSource 	= pSource;
		m_nbItems	= nbItems;
		m_pItems	= pItems;
	}
	
	ItemListMsg::~ItemListMsg()
	{
		delete [] m_pItems;
	}
	
	bool ItemListMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * ItemListMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	ItemListMsg_p ItemListMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return ItemListMsg_p( static_cast<ItemListMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	int ItemListMsg::nbItems() const
	{
		return m_nbItems;
	}
	
	const ItemInfo * ItemListMsg::items() const
	{
		return m_pItems;
	}
	
	//____ ItemsSelectMsg ________________________________________________________
	
	const char ItemsSelectMsg::CLASSNAME[] = {"ItemsSelectMsg"};
	
	ItemsSelectMsg::ItemsSelectMsg( Object * pSource, int nbItems, ItemInfo * pItems ) : ItemListMsg(pSource,nbItems,pItems)
	{
		m_type = WG_MSG_ITEMS_SELECT;
	}
	
	bool ItemsSelectMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return ItemListMsg::isInstanceOf(pClassName);
	}
	
	const char * ItemsSelectMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	ItemsSelectMsg_p ItemsSelectMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return ItemsSelectMsg_p( static_cast<ItemsSelectMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ ItemsUnselectMsg ________________________________________________________
	
	const char ItemsUnselectMsg::CLASSNAME[] = {"ItemsUnselectMsg"};
	
	ItemsUnselectMsg::ItemsUnselectMsg( Object * pSource, int nbItems, ItemInfo * pItems ) : ItemListMsg(pSource,nbItems,pItems)
	{
		m_type = WG_MSG_ITEMS_UNSELECT;
	}
	
	bool ItemsUnselectMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return ItemListMsg::isInstanceOf(pClassName);
	}
	
	const char * ItemsUnselectMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	ItemsUnselectMsg_p ItemsUnselectMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return ItemsUnselectMsg_p( static_cast<ItemsUnselectMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ PopupClosedMsg ____________________________________________
	
	const char PopupClosedMsg::CLASSNAME[] = {"PopupClosedMsg"};
	
	PopupClosedMsg::PopupClosedMsg( Widget * pPopup, const Widget_wp& pCaller )
	{
		m_type			= WG_MSG_POPUP_CLOSED;
		m_pCopyTo		= pCaller.rawPtr();
		m_pSource		= pPopup;
	}
	
	bool PopupClosedMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * PopupClosedMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	PopupClosedMsg_p PopupClosedMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return PopupClosedMsg_p( static_cast<PopupClosedMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ ModalMoveOutsideMsg ___________________________________________________
	
	const char ModalMoveOutsideMsg::CLASSNAME[] = {"ModalMoveOutsideMsg"};
	
	ModalMoveOutsideMsg::ModalMoveOutsideMsg( Widget * pSource )
	{
		m_type = WG_MSG_MODAL_MOVE_OUTSIDE;
		m_pSource		= pSource;
	}
	
	bool ModalMoveOutsideMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * ModalMoveOutsideMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	ModalMoveOutsideMsg_p ModalMoveOutsideMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return ModalMoveOutsideMsg_p( static_cast<ModalMoveOutsideMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	
	//____ ModalBlockedPressMsg _______________________________________________
	
	const char ModalBlockedPressMsg::CLASSNAME[] = {"ModalBlockedPressMsg"};
	
	ModalBlockedPressMsg::ModalBlockedPressMsg( WgMouseButton button, Widget * pSource ) : MouseButtonMsg(button)
	{
		m_type			= WG_MSG_MODAL_BLOCKED_PRESS;
		m_pSource 		= pSource;
	}
	
	bool ModalBlockedPressMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * ModalBlockedPressMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	ModalBlockedPressMsg_p ModalBlockedPressMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return ModalBlockedPressMsg_p( static_cast<ModalBlockedPressMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ ModalBlockedReleaseMsg _____________________________________________
	
	const char ModalBlockedReleaseMsg::CLASSNAME[] = {"ModalBlockedReleaseMsg"};
	
	ModalBlockedReleaseMsg::ModalBlockedReleaseMsg( WgMouseButton button, Widget * pSource ) : MouseButtonMsg(button)
	{
		m_type			= WG_MSG_MODAL_BLOCKED_RELEASE;
		m_pSource 		= pSource;
	}
	
	bool ModalBlockedReleaseMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * ModalBlockedReleaseMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	ModalBlockedReleaseMsg_p ModalBlockedReleaseMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return ModalBlockedReleaseMsg_p( static_cast<ModalBlockedReleaseMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ LinkMsg __________________________________________________
	
	const char LinkMsg::CLASSNAME[] = {"LinkMsg"};
	
	bool LinkMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * LinkMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	LinkMsg_p LinkMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return LinkMsg_p( static_cast<LinkMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ LinkSelectMsg __________________________________________________
	
	const char LinkSelectMsg::CLASSNAME[] = {"LinkSelectMsg"};
	
	LinkSelectMsg::LinkSelectMsg( const TextLink_p& pLink )
	{
		m_type			= WG_MSG_LINK_SELECT;
		m_pSource		= pLink;
	}
	
	bool LinkSelectMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return LinkMsg::isInstanceOf(pClassName);
	}
	
	const char * LinkSelectMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	LinkSelectMsg_p LinkSelectMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return LinkSelectMsg_p( static_cast<LinkSelectMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ LinkMouseEnterMsg __________________________________________________
	
	const char LinkMouseEnterMsg::CLASSNAME[] = {"LinkMouseEnterMsg"};
	
	LinkMouseEnterMsg::LinkMouseEnterMsg( const TextLink_p& pLink )
	{
		m_type			= WG_MSG_LINK_MOUSE_ENTER;
		m_pSource		= pLink;
	}
	
	bool LinkMouseEnterMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return LinkMsg::isInstanceOf(pClassName);
	}
	
	const char * LinkMouseEnterMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	LinkMouseEnterMsg_p LinkMouseEnterMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return LinkMouseEnterMsg_p( static_cast<LinkMouseEnterMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ LinkMouseLeaveMsg __________________________________________________
	
	const char LinkMouseLeaveMsg::CLASSNAME[] = {"LinkMouseLeaveMsg"};
	
	LinkMouseLeaveMsg::LinkMouseLeaveMsg( const TextLink_p& pLink )
	{
		m_type			= WG_MSG_LINK_MOUSE_LEAVE;
		m_pSource		= pLink;
	}
	
	bool LinkMouseLeaveMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return LinkMsg::isInstanceOf(pClassName);
	}
	
	const char * LinkMouseLeaveMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	LinkMouseLeaveMsg_p LinkMouseLeaveMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return LinkMouseLeaveMsg_p( static_cast<LinkMouseLeaveMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ LinkMouseButtonMsg __________________________________________________
	
	const char LinkMouseButtonMsg::CLASSNAME[] = {"LinkMouseButtonMsg"};
	
	WgMouseButton LinkMouseButtonMsg::button() const
	{
		return m_button;
	}
	
	bool LinkMouseButtonMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return LinkMsg::isInstanceOf(pClassName);
	}
	
	const char * LinkMouseButtonMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	LinkMouseButtonMsg_p LinkMouseButtonMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return LinkMouseButtonMsg_p( static_cast<LinkMouseButtonMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ LinkMousePressMsg __________________________________________________
	
	const char LinkMousePressMsg::CLASSNAME[] = {"LinkMousePressMsg"};
	
	LinkMousePressMsg::LinkMousePressMsg( const TextLink_p& pLink, WgMouseButton button )
	{
		m_type			= WG_MSG_LINK_MOUSE_PRESS;
		m_pSource		= pLink;
		m_button		= button;
	}
	
	bool LinkMousePressMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return LinkMouseButtonMsg::isInstanceOf(pClassName);
	}
	
	const char * LinkMousePressMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	LinkMousePressMsg_p LinkMousePressMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return LinkMousePressMsg_p( static_cast<LinkMousePressMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ LinkMouseRepeatMsg __________________________________________________
	
	const char LinkMouseRepeatMsg::CLASSNAME[] = {"LinkMouseRepeatMsg"};
	
	LinkMouseRepeatMsg::LinkMouseRepeatMsg( const TextLink_p& pLink, WgMouseButton button )
	{
		m_type			= WG_MSG_LINK_MOUSE_REPEAT;
		m_pSource		= pLink;
		m_button		= button;
	}
	
	bool LinkMouseRepeatMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return LinkMouseButtonMsg::isInstanceOf(pClassName);
	}
	
	const char * LinkMouseRepeatMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	LinkMouseRepeatMsg_p LinkMouseRepeatMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return LinkMouseRepeatMsg_p( static_cast<LinkMouseRepeatMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ LinkMouseReleaseMsg __________________________________________________
	
	const char LinkMouseReleaseMsg::CLASSNAME[] = {"LinkMouseReleaseMsg"};
	
	LinkMouseReleaseMsg::LinkMouseReleaseMsg( const TextLink_p& pLink, WgMouseButton button )
	{
		m_type			= WG_MSG_LINK_MOUSE_RELEASE;
		m_pSource		= pLink;
		m_button		= button;
	}
	
	bool LinkMouseReleaseMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return LinkMouseButtonMsg::isInstanceOf(pClassName);
	}
	
	const char * LinkMouseReleaseMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	LinkMouseReleaseMsg_p LinkMouseReleaseMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return LinkMouseReleaseMsg_p( static_cast<LinkMouseReleaseMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ LinkMouseClickMsg __________________________________________________
	
	const char LinkMouseClickMsg::CLASSNAME[] = {"LinkMouseClickMsg"};
	
	LinkMouseClickMsg::LinkMouseClickMsg( const TextLink_p& pLink, WgMouseButton button )
	{
		m_type			= WG_MSG_LINK_MOUSE_CLICK;
		m_pSource		= pLink;
		m_button		= button;
	}
	
	bool LinkMouseClickMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return LinkMouseButtonMsg::isInstanceOf(pClassName);
	}
	
	const char * LinkMouseClickMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	LinkMouseClickMsg_p LinkMouseClickMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return LinkMouseClickMsg_p( static_cast<LinkMouseClickMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ LinkMouseDoubleClickMsg __________________________________________________
	
	const char LinkMouseDoubleClickMsg::CLASSNAME[] = {"LinkMouseDoubleClickMsg"};
	
	LinkMouseDoubleClickMsg::LinkMouseDoubleClickMsg( const TextLink_p& pLink, WgMouseButton button )
	{
		m_type			= WG_MSG_LINK_MOUSE_DOUBLE_CLICK;
		m_pSource		= pLink;
		m_button		= button;
	}
	
	bool LinkMouseDoubleClickMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return LinkMouseButtonMsg::isInstanceOf(pClassName);
	}
	
	const char * LinkMouseDoubleClickMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	LinkMouseDoubleClickMsg_p LinkMouseDoubleClickMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return LinkMouseDoubleClickMsg_p( static_cast<LinkMouseDoubleClickMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	
	//____ MousePositionMsg _______________________________________________________
	
	const char MousePositionMsg::CLASSNAME[] = {"MousePositionMsg"};
	
	MousePositionMsg::MousePositionMsg()
	{
		m_type = WG_MSG_MOUSE_POSITION;
	}
	
	bool MousePositionMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Msg::isInstanceOf(pClassName);
	}
	
	const char * MousePositionMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	MousePositionMsg_p MousePositionMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return MousePositionMsg_p( static_cast<MousePositionMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ MouseDragMsg _________________________________________________________
	
	const char MouseDragMsg::CLASSNAME[] = {"MouseDragMsg"};
	
	MouseDragMsg::MouseDragMsg( WgMouseButton button, const Coord& startPos, const Coord& prevPos, const Coord& currPos ) : MouseButtonMsg(button)
	{
		m_type = WG_MSG_MOUSE_DRAG;
	
		m_startPos = startPos;
		m_prevPos = prevPos;
		m_currPos = currPos;
	}
	
	MouseDragMsg::MouseDragMsg( WgMouseButton button, Widget * pWidget, const Coord& startPos, const Coord& prevPos, const Coord& currPos ) : MouseButtonMsg(button)
	{
		m_type 			= WG_MSG_MOUSE_DRAG;
		m_pSource		= pWidget;
		m_pCopyTo	= pWidget;
	
		m_startPos = startPos;
		m_prevPos = prevPos;
		m_currPos = currPos;
	}
	
	bool MouseDragMsg::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return MouseButtonMsg::isInstanceOf(pClassName);
	}
	
	const char * MouseDragMsg::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	MouseDragMsg_p MouseDragMsg::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return MouseDragMsg_p( static_cast<MouseDragMsg*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	Coord MouseDragMsg::draggedTotal() const
	{
		return m_currPos - m_startPos;
	}
	
	Coord MouseDragMsg::draggedNow() const
	{
		return m_currPos - m_prevPos;
	}
	
	Coord MouseDragMsg::currPos() const
	{
		return m_currPos;
	}
	
	Coord MouseDragMsg::startPos() const
	{
		return m_startPos;
	}
	
	Coord MouseDragMsg::prevPos() const
	{
		return m_prevPos;
	}
	

} // namespace wg
