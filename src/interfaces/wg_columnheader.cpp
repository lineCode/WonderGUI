
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

#include <wg_columnheader.h>
#include <wg_columnheaderitem.h>

namespace wg 
{
	//____ Constructor() ___________________________________________________________

	ColumnHeader::ColumnHeader(ColumnHeaderItem* pItem) : m_pItem(pItem), icon(&pItem->icon), arrow(&pItem->arrow), label(&pItem->label)
	{
	}
		
	//____ _object() _______________________________________________________________

	Object * ColumnHeader::_object() const
	{
		return m_pItem->_object();
	}

} // namespace wg
