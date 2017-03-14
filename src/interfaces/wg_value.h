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

#ifndef WG_VALUE_DOT_H
#define WG_VALUE_DOT_H

#ifndef WG_INTERFACE_DOT_H
#	include <wg_interface.h>
#endif

#ifndef WG_TEXTSTYLE_DOT_H
#	include <wg_textstyle.h>
#endif

#ifndef WG_TEXTMAPPER_DOT_H
#	include <wg_textmapper.h>
#endif


//#ifndef WG_VALUEFORMAT_DOT_H
//#	include <wg_valueformat.h>
//#endif

#ifndef WG_VALUEITEM_DOT_H
#	include <wg_valueitem.h>
#endif

namespace wg 
{
	
	
	class Value;
	typedef	StrongInterfacePtr<Value,Interface_p>	Value_p;
	typedef	WeakInterfacePtr<Value,Interface_wp>	Value_wp;
	
	/**
	 * @brief Interface to a basic value display item
	 * 
	 * The value in a basic value item is set by the widget itself and can
	 * not be modified directly either through the API or UI. Only the formatting
	 * and appearance of the value can be modified through this API.
	 * 
	 */
	
	class Value : public Interface
	{
	public:
		Value(ValueItem* pItem) : m_pItem(pItem) {}
	
		inline Value_p			ptr() { return Value_p(_object(),this); }
	
		inline void				setFormatter( const ValueFormatter_p& pFormatter ) { m_pItem->setFormatter(pFormatter); }
		inline void				clearFormatter() { m_pItem->clearFormatter(); }
		inline ValueFormatter_p	formatter() const { return m_pItem->formatter(); }
	
		inline void				setStyle( const TextStyle_p& pStyle ) { m_pItem->setStyle(pStyle); }
		inline void				clearStyle() { m_pItem->clearStyle(); }
		inline TextStyle_p		style() const { return m_pItem->style(); }
	
		inline void				setTextMapper( const TextMapper_p& pTextMapper ) { m_pItem->setTextMapper(pTextMapper); }
		inline void				clearTextMapper() { m_pItem->clearTextMapper(); }
		inline TextMapper_p		textMapper() const { return m_pItem->textMapper(); }
	
		inline State			state() const { return m_pItem->state(); }
	protected:
		Object * 		_object() const;

		ValueItem *		m_pItem;
	};
	
	

} // namespace wg
#endif //WG_VALUE_DOT_H
