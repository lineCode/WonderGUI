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
#ifndef WG_OBJECT_DOT_H

namespace wg 
{
	#define WG_OBJECT_DOT_H
	
	class Object;
	class Object_p;
	
	class WeakPtrHub
	{
	public:
		int					refCnt;
		Object *			pObj;
	};
	
	
	/**
	 * @brief Base class for all reference counted objects in WonderGUI.
	 *
	 * Base class for all reference counted objects in WonderGUI.
	 *
	 * Object provides the datastructures needed for smart pointers, weak pointers and
	 * destruction notifiers as well as methods for identifying object types and 
	 * dynamic cast of smart pointers.
	 * 
	 * Objects that are based on Object are implicitly destroyed when their last
	 * reference disappears and should never be explicitly destroyed.
	 *
	 */
	
	class Object
	{
		friend class Object_p;
		friend class Object_wp;
		template<class T, class P> friend class StrongPtr;
		template<class T, class P> friend class WeakPtr;
	
		friend class Interface_p;
		friend class Interface_wp;
	
	public:

		//.____ Identification _________________________________________________

		virtual bool		isInstanceOf( const char * pClassName ) const;
		static Object_p		cast( const Object_p& pObject );				// Provided just for completeness sake.
		virtual const char *className( void ) const;
		static const char	CLASSNAME[];
	
	protected:
		Object() : m_pWeakPtrHub(0), m_refCount(0) {}
		virtual ~Object() { if( m_pWeakPtrHub ) m_pWeakPtrHub->pObj = 0; }
	
		inline void _incRefCount() { m_refCount++; }
		inline void _decRefCount() { m_refCount--; if( m_refCount == 0 ) _destroy(); }

		inline void _incRefCount(int amount) { m_refCount += amount; }
		inline void _decRefCount(int amount) { m_refCount -= amount; if( m_refCount == 0 ) _destroy(); }
	
		WeakPtrHub *	m_pWeakPtrHub;
	
	private:
		virtual void 	_destroy();			// Pointers should call destroy instead of destructor.
		int				m_refCount;
	};
	
	

} // namespace wg
#endif //WG_OBJECT_DOT_H
