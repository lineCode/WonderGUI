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
#include <wg_receiver.h>

const char WgReceiver::CLASSNAME[] = {"Receiver"};



//____ IsInstanceOf() _________________________________________________________

bool WgReceiver::IsInstanceOf( const char * pClassName ) const
{
	if( pClassName==CLASSNAME )
		return true;

	return WgObject::IsInstanceOf(pClassName);
}

//____ ClassName() ____________________________________________________________

const char * WgReceiver::ClassName( void ) const
{
	return CLASSNAME;
}

//____ Cast() _________________________________________________________________

WgReceiverPtr WgReceiver::Cast( const WgObjectPtr& pObject )
{
	if( pObject && pObject->IsInstanceOf(CLASSNAME) )
		return WgReceiverPtr( static_cast<WgReceiver*>(pObject.RawPtr()) );

	return 0;
}

//____ _onRouteAdded() _________________________________________________________

void  WgReceiver::_onRouteAdded()
{	
}

//____ _onRouteRemoved() _______________________________________________________

void  WgReceiver::_onRouteRemoved()
{
}