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

#ifndef	WG_ICON_DOT_H
#define	WG_ICON_DOT_H
#pragma once

#include <wg_types.h>
#include <wg_geo.h>
#include <wg_interface.h>
#include <wg_pointers.h>
#include <wg_skin.h>
#include <wg_iconitem.h>

namespace wg 
{
	
	class Icon;
	typedef	StrongInterfacePtr<Icon>	Icon_p;
	typedef	WeakInterfacePtr<Icon>		Icon_wp;
	
	class Icon : public Interface
	{
	public:
		/** @private */

		Icon(IconItem* pItem) : m_pItem(pItem) {}
	
		//.____ Content _____________________________________________

		inline bool			set( Skin * pIconGfx, Origo origo = Origo::West, Border padding = Border(0),
								 float scale = 0.f, bool bOverlap = false ) { return m_pItem->set(pIconGfx,origo,padding,scale,bOverlap); }
		inline void			clear() { m_pItem->clear(); }

		//.____ Appearance _____________________________________________

		inline bool			setScale( float scaleFactor ) { return m_pItem->setScale(scaleFactor); }
		inline void			setOrigo( Origo origo ) { m_pItem->setOrigo(origo); }
		inline void			setPadding( Border padding ) { m_pItem->setPadding(padding); }
		inline void			setOverlap( bool bOverlap ) { m_pItem->setOverlap(bOverlap); }
		inline void			setSkin( Skin * pSkin ) { m_pItem->setSkin(pSkin); }
	
		inline float		scale() const { return m_pItem->scale(); }
		inline Origo		origo() const { return m_pItem->origo(); }
		inline Border		padding() const { return m_pItem->padding(); }
		inline bool			overlap() const { return m_pItem->overlap(); }
		inline Skin_p		skin() const { return m_pItem->skin(); }

		//.____ Misc __________________________________________________

		inline Icon_p		ptr() { return Icon_p(this); }


	protected:
		Object * 			_object() const { return m_pItem->_object(); };
	
		IconItem *			m_pItem;
	};
	

} // namespace wg
#endif	// WG_ICON_DOT_H
