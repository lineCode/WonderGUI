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

#ifndef	WG_FIELD_DOT_H
#define	WG_FIELD_DOT_H

#ifndef WG_ITEM_DOT_H
#	include <wg_item.h>
#endif

class WgField;
class WgObject;

//____ WgField ___________________________________________________________

class WgFieldHolder : public WgItemHolder
{
public:
	virtual void		_onFieldDirty( WgField * pField ) = 0;
	virtual void		_onFieldResize( WgField * pField ) = 0;
};

//____ WgField _________________________________________________________________

class WgField : public WgItem
{
public:
	WgField( WgFieldHolder * pHolder ) : WgItem(pHolder) {}

protected:

	inline void		_onDirty() { return static_cast<WgFieldHolder*>(m_pHolder)->_onFieldDirty(this); }
	inline void		_onResize() { return static_cast<WgFieldHolder*>(m_pHolder)->_onFieldResize(this); }
};



#endif //WG_FIELD_DOT_H
