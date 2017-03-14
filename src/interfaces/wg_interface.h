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

#include <wg_pointers.h>

namespace wg 
{
		
	
	/**
	 * @brief Provides access to items embedded into a Widget or Object.
	 *
	 * Interfaces are provided by Widgets and other reference counted Objects
	 * to provide API access to their embedded items such as labels and
	 * icons.
	 *
	 * The interface concept of WonderGUI serves two purposes:
	 *
	 * First it
	 * provides a nice API-level abstraction to keep methods for accessing
	 * different components of a widget logically separated while providing
	 * an identical way to access embedded items in all widgets that
	 * contains them.
	 *
	 * Secondly it provides a safe way to pass around pointers to the embedded
	 * items since interface pointers do reference counting on the object
	 * providing the interface, while pointing at the interface directly.
	 *
	 **/
	
	class Interface
	{
		friend class Interface_p;
		friend class Interface_wp;
	public:
		inline Interface_p		ptr() { return Interface_p(_object(),this);}		///< @brief Get a pointer to this interface.
		inline Object_p			holder() { return Object_p( _object() ); };			///< @breif Get a pointer to the object providing this interface.
	
	protected:
		virtual Object * 		_object() const = 0;
	};
	
	

} // namespace wg
#endif //WG_INTERFACE_DOT_H
