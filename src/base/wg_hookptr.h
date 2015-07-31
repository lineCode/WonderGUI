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

#ifndef WG_HOOKPTR_DOT_H

namespace wg 
{
	#define WG_HOOKPTR_DOT_H
	
	class WgHook;
	
	class WgHook_pHub
	{
	public:
		int				refCnt;
		WgHook *		pObj;
	};
	
	class WgHook_p
	{
	public:
		WgHook_p() : m_pHub(0) {}
		WgHook_p( WgHook * pObj );
		WgHook_p(const WgHook_p& r)
		{
			m_pHub = r.m_pHub;
			if( m_pHub )
				m_pHub->refCnt++;
		}
	
		~WgHook_p();
	
	    inline WgHook_p& operator=( WgHook_p const & r)
		{
			copy( r );
			return *this;
		}
	
		inline WgHook& operator*() const { return * rawPtr(); }
		inline WgHook * operator->() const { return rawPtr(); }
	
		//TODO: Fix so that we get right value if both are null-pointers, but have different hubs.
		inline bool operator==(const WgHook_p& other) const { return m_pHub == other.m_pHub; }
		inline bool operator!=(const WgHook_p& other) const { return m_pHub != other.m_pHub; }
		inline bool operator<(const WgHook_p& other) const { return m_pHub < other.m_pHub ? true : false; }
		inline bool operator>(const WgHook_p& other) const { return m_pHub > other.m_pHub ? true : false; }
		inline bool operator<=(const WgHook_p& other) const { return m_pHub <= other.m_pHub ? true : false; }
		inline bool operator>=(const WgHook_p& other) const { return m_pHub >= other.m_pHub ? true : false; }
	
		inline operator bool() const { return (m_pHub != 0 && m_pHub->pObj != 0); }
	
		inline WgHook * rawPtr() const
		{
			if( m_pHub )
				return m_pHub->pObj;
			else
				return 0;
		}
	
		void copy( WgHook_p const & r );
	
	
	
	protected:
		WgHook_pHub *		m_pHub;
	
	};
	
	//____ WgHookTypePtr _________________________________________________________
	
	template<class T,class P> class WgHookTypePtr : public P
	{
	public:
		WgHookTypePtr(T* p=0) : P( p ) {};
		WgHookTypePtr(const WgHookTypePtr<T,P>& r) : P( r.rawPtr() ) {};
	//	WgWeakPtr(const WgStrongPtr<T,P>& r) : P( r.rawPtr() ) {};
		~WgHookTypePtr() {};
	
		inline T & operator*() const { return * rawPtr(); }
		inline T * operator->() const{ return rawPtr(); }
	
		inline bool operator==(const WgHookTypePtr<T,P>& other) const { return this->m_pHub == other.m_pHub; }
		inline bool operator!=(const WgHookTypePtr<T,P>& other) const { return this->m_pHub != other.m_pHub; }
	
	//	inline operator bool() const { return (this->m_pObj != 0); }
	
		inline T * rawPtr() const
		{
			if( this->m_pHub && this->m_pHub->pObj )
				return static_cast<T*>(this->m_pHub->pObj);
			else
				return reinterpret_cast<T*>(0);
		}
	};
	
	
	

} // namespace wg
#endif //WG_HOOKPTR_DOT_H
