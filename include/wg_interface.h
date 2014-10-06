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
#ifndef WG_INTERFACE_DOT_H
#define WG_INTERFACE_DOT_H

class WgInterfacePtr;
class WgInterfaceWeakPtr;

class WgObject;

class WgInterface
{
	friend class WgInterfacePtr;
	friend class WgInterfaceWeakPtr;
public:
	virtual bool			IsInstanceOf( const char * pClassName ) const;
	virtual const char *	ClassName( void ) const;
	static const char		CLASSNAME[];
	static WgInterfacePtr	Cast( const WgInterfacePtr& pInterface );				// Provided just for completeness sake.
	WgInterfacePtr			Ptr();

protected:
	virtual WgObject * 		_object() const = 0;
};


#endif //WG_INTERFACE_DOT_H
