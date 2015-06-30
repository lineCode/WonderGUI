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

#include <wg_msgfunc.h>

const char WgMsgFunc::CLASSNAME[] = {"MsgFunc"};


//____ Constructors ____________________________________________________________

WgMsgFunc::WgMsgFunc( void(*fp)( const WgMsgPtr& pMsg) )
{
	m_callbackType = 0;
	m_pCallback = (void*) fp;	
}

WgMsgFunc::WgMsgFunc( void(*fp)( const WgMsgPtr& pMsg, int param), int param )
{
	m_callbackType = 1;
	m_pCallback = (void*) fp;
	m_param = param;
}

WgMsgFunc::WgMsgFunc( void(*fp)( const WgMsgPtr& pMsg, void * pParam), void * pParam )
{
	m_callbackType = 2;
	m_pCallback = (void*) fp;
	m_pParam = pParam;
}

WgMsgFunc::WgMsgFunc( void(*fp)( const WgMsgPtr& pMsg, const WgObjectPtr& pParam), const WgObjectPtr& pParam )
{
	m_callbackType = 3;
	m_pCallback = (void*) fp;
	m_pParamObj = pParam;
	
}


//____ IsInstanceOf() _________________________________________________________

bool WgMsgFunc::IsInstanceOf( const char * pClassName ) const
{
	if( pClassName==CLASSNAME )
		return true;

	return WgReceiver::IsInstanceOf(pClassName);
}

//____ ClassName() ____________________________________________________________

const char * WgMsgFunc::ClassName( void ) const
{
	return CLASSNAME;
}

//____ Cast() _________________________________________________________________

WgMsgFuncPtr WgMsgFunc::Cast( const WgObjectPtr& pObject )
{
	if( pObject && pObject->IsInstanceOf(CLASSNAME) )
		return WgMsgFuncPtr( static_cast<WgMsgFunc*>(pObject.RawPtr()) );

	return 0;
}

//____ OnMsg() _______________________________________________________________

void WgMsgFunc::OnMsg( const WgMsgPtr& pMsg )
{
	switch( m_callbackType )
	{
		case 0:
			((void(*)( const WgMsgPtr&))m_pCallback)(pMsg);
			break;
		case 1:
			((void(*)( const WgMsgPtr&, int ))m_pCallback)(pMsg,m_param);
			break;
		case 2:
			((void(*)( const WgMsgPtr&, void* ))m_pCallback)(pMsg,m_pParam);
			break;
		case 3:
			((void(*)( const WgMsgPtr&, const WgObjectPtr& ))m_pCallback)(pMsg,m_pParamObj);
			break;
		default:
			break;
	}
}

//____ _onRouteAdded() _________________________________________________________

void  WgMsgFunc::_onRouteAdded()
{	
	_incRefCount();
}

//____ _onRouteRemoved() _______________________________________________________

void  WgMsgFunc::_onRouteRemoved()
{
	_decRefCount();
}