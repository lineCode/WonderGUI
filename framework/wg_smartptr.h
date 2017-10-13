#ifndef	WG_SMARTPTR_DOT_H
#define	WG_SMARTPTR_DOT_H

#ifndef WG_MEMPOOL_DOT_H
#	include <wg_mempool.h>
#endif

#ifndef	WG_REFCOUNTED_DOT_H
#	include <wg_refcounted.h>
#endif


//___ WgSmartPtrImpl __________________________________________________________

class WgSmartPtrImpl
{
public:

	WgSmartPtrImpl(WgRefCounted * p)
	{
		m_pObj = p;
		if( p )
			p->m_ref++;
	}

	~WgSmartPtrImpl()
	{
		if( m_pObj )
		{
			m_pObj->m_ref--;
			if( m_pObj->m_ref == 0 )
				delete m_pObj;
		}
	}

	void copy( WgSmartPtrImpl const & r )
	{
		if( m_pObj != r.m_pObj )
		{
			if( m_pObj )
			{
				m_pObj->m_ref--;
				if( m_pObj->m_ref == 0 )
					delete m_pObj;
			}

			m_pObj = r.m_pObj;
			if( m_pObj )
				m_pObj->m_ref++;
		}
	}

	WgRefCounted * m_pObj;
protected:
};


//____ WgSmartPtr _____________________________________________________________

template<class T> class WgSmartPtr : public WgSmartPtrImpl
{
public:
	WgSmartPtr(T* p=0) : WgSmartPtrImpl( p ) {};
	WgSmartPtr(const WgSmartPtr<T>& r) : WgSmartPtrImpl( r.m_pObj ) {};
	~WgSmartPtr() {};
    
    
    inline WgSmartPtr<T> & operator=( WgSmartPtr<T> const & r)
	{
		copy( r );
		return *this;
	}
    
	inline T & operator*() const { return * (T*) m_pObj; }
	inline T * operator->() const{ return (T*) m_pObj; }
    
	inline bool operator==(const WgSmartPtr<T>& other) const { return m_pObj == other.m_pObj; }
	inline bool operator!=(const WgSmartPtr<T>& other) const { return m_pObj != other.m_pObj; }
    
	inline operator bool() const { return (m_pObj != 0); }
    
	inline T * GetRealPtr() const { return (T*) m_pObj; }
};


template<class T,class P> class WgSmartChildPtr : public P
{
public:
	WgSmartChildPtr(T* p=0) : P( p ) {};
	WgSmartChildPtr(const WgSmartChildPtr<T,P>& r) : P( (T*) r.m_pObj ) {};
	~WgSmartChildPtr() {};
    
    /*
     inline WgSmartChildPtr<T,P> & operator=( WgSmartChildPtr<T,P> const & r)
     {
     copy( r );
     return *this;
     }
     */
	inline T & operator*() const { return * (T*) P::m_pObj; }
	inline T * operator->() const{ return (T*) P::m_pObj; }
    
	inline bool operator==(const WgSmartChildPtr<T,P>& other) const { return P::m_pObj == other.m_pObj; }
	inline bool operator!=(const WgSmartChildPtr<T,P>& other) const { return P::m_pObj != other.m_pObj; }
    
	inline operator bool() const { return (P::m_pObj != 0); }
    
	inline T * GetRealPtr() const { return (T*) P::m_pObj; }
};






//____ WgSmartPtrPooled _______________________________________________________

template<class T> class WgSmartPtrPooled
{
public:
	WgSmartPtrPooled(T* p=0)
	{
		m_pObj = p;
		if( p )
			((WgRefCountedPooled*)p)->m_ref++;
	};

	WgSmartPtrPooled(const WgSmartPtrPooled<T>& r)
	{
		m_pObj = r.m_pObj;
		if( m_pObj )
			((WgRefCountedPooled*)m_pObj)->m_ref++;
	}

	~WgSmartPtrPooled()
	{
		if( m_pObj )
		{
			((WgRefCountedPooled*)m_pObj)->m_ref--;
			if( ((WgRefCountedPooled*)m_pObj)->m_ref == 0 )
			{
				m_pObj->~T();
				((WgRefCountedPooled*)m_pObj)->m_pPool->FreeEntry(m_pObj);
			}
		}
	};


    WgSmartPtrPooled<T> & operator=( WgSmartPtrPooled<T> const & r)
	{
		if( m_pObj != r.m_pObj )
		{
			if( m_pObj )
			{
				((WgRefCountedPooled*)m_pObj)->m_ref--;

				if( ((WgRefCountedPooled*)m_pObj)->m_ref == 0 )
				{
					m_pObj->~T();
					((WgRefCountedPooled*)m_pObj)->m_pPool->FreeEntry(m_pObj);
				}
			}

			m_pObj = r.m_pObj;
			if( m_pObj )
				((WgRefCountedPooled*)m_pObj)->m_ref++;
		}
		return *this;
	}

	inline T & operator*() const { return * m_pObj; }
	inline T * operator->() const{ return m_pObj; }

	inline bool operator==(const WgSmartPtrPooled<T>& other) const { return m_pObj == other.m_pObj; }
	inline bool operator!=(const WgSmartPtrPooled<T>& other) const { return m_pObj != other.m_pObj; }

	inline operator bool() const { return (m_pObj != 0); }

	inline T * GetRealPtr() const { return (T*) m_pObj; }

private:
	T * m_pObj;
};

//____ WgRefCountPtr __________________________________________________________

template<class T> class WgRefCountPtr
{
public:
	WgRefCountPtr(T* p=0)
	{
		m_pObj = p;
		if( p )
			p->m_ref++;
	}

	WgRefCountPtr(const WgRefCountPtr<T>& r)
	{
		m_pObj = r.m_pObj;
		if( m_pObj )
			m_pObj->m_ref++;
	}

	~WgRefCountPtr()
	{
		if( m_pObj )
			m_pObj->m_ref--;
	}


    inline WgRefCountPtr<T> & operator=( WgRefCountPtr<T> const & r)
	{
		if( m_pObj )
			m_pObj->m_ref--;

		m_pObj = r.m_pObj;
		if( m_pObj )
			m_pObj->m_ref++;

		return *this;
	}

	inline T & operator*() const { return * (T*) m_pObj; }
	inline T * operator->() const{ return (T*) m_pObj; }

	inline bool operator==(const WgRefCountPtr<T>& other) const { return m_pObj == other.m_pObj; }
	inline bool operator!=(const WgRefCountPtr<T>& other) const { return m_pObj != other.m_pObj; }

	inline operator bool() const { return (m_pObj != 0); }

	inline T * GetRealPtr() const { return (T*) m_pObj; }

private:
	T * m_pObj;
};


//____ WgWeakPtrImpl __________________________________________________________

class WgWeakPtrImpl
{
public:
	WgWeakPtrImpl() { m_pHub = 0; }
	WgWeakPtrImpl( WgWeakPtrTarget * pObj );
	~WgWeakPtrImpl();

	void copy( WgWeakPtrImpl const & r );

	WgWeakPtrHub * m_pHub;
};


//____ WgWeakPtr ______________________________________________________________

template<class T> class WgWeakPtr : private WgWeakPtrImpl
{
public:
	WgWeakPtr() {}
	WgWeakPtr( T * pObj ) : WgWeakPtrImpl( pObj ) {}

	WgWeakPtr(const WgWeakPtr<T>& r)
	{

		m_pHub = r.m_pHub;
		if( m_pHub )
			m_pHub->refCnt++;
	}

	~WgWeakPtr() {}


    inline WgWeakPtr<T> & operator=( WgWeakPtr<T> const & r)
	{
		copy( r );
		return *this;
	}

	inline T & operator*() const { return * GetRealPtr(); }
	inline T * operator->() const { return GetRealPtr(); }

	//TODO: Fix so that we get right value if both are null-pointers, but have different hubs.
	inline bool operator==(const WgWeakPtr<T>& other) const { return m_pHub == other.m_pHub; }
	inline bool operator!=(const WgWeakPtr<T>& other) const { return m_pHub != other.m_pHub; }
	inline bool operator<(const WgWeakPtr<T>& other) const { return m_pHub < other.m_pHub ? true : false; }
	inline bool operator>(const WgWeakPtr<T>& other) const { return m_pHub > other.m_pHub ? true : false; }
	inline bool operator<=(const WgWeakPtr<T>& other) const { return m_pHub <= other.m_pHub ? true : false; }
	inline bool operator>=(const WgWeakPtr<T>& other) const { return m_pHub >= other.m_pHub ? true : false; }

	inline operator bool() const { return (m_pHub != 0 && m_pHub->pObj != 0); }

	inline T * GetRealPtr() const
	{
		if( m_pHub && m_pHub->pObj )
			return static_cast<T*>(m_pHub->pObj);
		else
			return reinterpret_cast<T*>(0);
	}
};



#endif //WG_SMARTPTR_DOT_H

