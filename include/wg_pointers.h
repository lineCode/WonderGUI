#ifndef	WG_POINTERS_DOT_H
#define	WG_POINTERS_DOT_H

#ifndef WG_MEMPOOL_DOT_H
#	include <wg_mempool.h>
#endif

#ifndef	WG_OBJECT_DOT_H
#	include <wg_object.h>
#endif

#ifndef WG_COMPONENT_DOT_H
#	include <wg_component.h>
#endif

class WgObjectWeakPtr;

//____ WgObjectPtr _____________________________________________________________

class WgObjectPtr
{
public:
	WgObjectPtr(WgObject* p=0)
	{
		m_pObj = p;
		if( p )
			p->m_refCount++;
	}

	WgObjectPtr(const WgObjectPtr& r)
	{
		m_pObj = r.m_pObj;
		if( m_pObj )
			m_pObj->m_refCount++;
	}

	WgObjectPtr( const WgObjectWeakPtr& r );

	~WgObjectPtr()
	{
		if( m_pObj )
		{
			m_pObj->m_refCount--;
			if( m_pObj->m_refCount == 0 )
				delete m_pObj;
		}
	}


    inline WgObjectPtr & operator=( WgObjectPtr const & r)
	{
		copy( r );
		return *this;
	}

	inline WgObject& operator*() const { return * m_pObj; }
	inline WgObject* operator->() const{ return m_pObj; }

	inline bool operator==(const WgObjectPtr& other) const { return m_pObj == other.m_pObj; }
	inline bool operator!=(const WgObjectPtr& other) const { return m_pObj != other.m_pObj; }

	inline operator bool() const { return (m_pObj != 0); }

	inline WgObject * GetRealPtr() const { return m_pObj; }

protected:
	void copy( WgObjectPtr const & r )
	{
		if( m_pObj != r.m_pObj )
		{
			if( m_pObj )
			{
				m_pObj->m_refCount--;
				if( m_pObj->m_refCount == 0 )
					m_pObj->_destroy();
			}

			m_pObj = r.m_pObj;
			if( m_pObj )
				m_pObj->m_refCount++;
		}
	}


	WgObject * m_pObj;
};


//____ WgStrongPtr ________________________________________________________

template<class T,class P> class WgStrongPtr : public P
{
public:
	WgStrongPtr(T* p=0) : P( p ) {};
	WgStrongPtr(const WgStrongPtr<T,P>& r) : P( (T*) r.m_pObj ) {};
//	WgStrongPtr(const WgWeakPtr<T,P>& r) : P( (T*) r.GetRealPtr() ) {};
	~WgStrongPtr() {};

/*
    inline WgStrongPtr<T,P> & operator=( WgStrongPtr<T,P> const & r)
	{
		copy( r );
		return *this;
	}
*/
	inline T & operator*() const { return * (T*) this->m_pObj; }
	inline T * operator->() const{ return (T*) this->m_pObj; }

	inline bool operator==(const WgStrongPtr<T,P>& other) const { return this->m_pObj == other.m_pObj; }
	inline bool operator!=(const WgStrongPtr<T,P>& other) const { return this->m_pObj != other.m_pObj; }

//	inline operator bool() const { return (this->m_pObj != 0); }

	inline T * GetRealPtr() const { return (T*) this->m_pObj; }
};


//____ WgObjectWeakPtr ______________________________________________________________

class WgObjectWeakPtr
{
public:
	WgObjectWeakPtr() : m_pHub(0) {}
	WgObjectWeakPtr( WgObject * pObj );
	WgObjectWeakPtr(const WgObjectPtr& r);
	WgObjectWeakPtr(const WgObjectWeakPtr& r)
	{

		m_pHub = r.m_pHub;
		if( m_pHub )
			m_pHub->refCnt++;
	}

	~WgObjectWeakPtr();

    inline WgObjectWeakPtr& operator=( WgObjectWeakPtr const & r)
	{
		copy( r );
		return *this;
	}

	inline WgObject& operator*() const { return * GetRealPtr(); }
	inline WgObject * operator->() const { return GetRealPtr(); }

	//TODO: Fix so that we get right value if both are null-pointers, but have different hubs.
	inline bool operator==(const WgObjectWeakPtr& other) const { return m_pHub == other.m_pHub; }
	inline bool operator!=(const WgObjectWeakPtr& other) const { return m_pHub != other.m_pHub; }
	inline bool operator<(const WgObjectWeakPtr& other) const { return m_pHub < other.m_pHub ? true : false; }
	inline bool operator>(const WgObjectWeakPtr& other) const { return m_pHub > other.m_pHub ? true : false; }
	inline bool operator<=(const WgObjectWeakPtr& other) const { return m_pHub <= other.m_pHub ? true : false; }
	inline bool operator>=(const WgObjectWeakPtr& other) const { return m_pHub >= other.m_pHub ? true : false; }

	inline operator bool() const { return (m_pHub != 0 && m_pHub->pObj != 0); }

	inline WgObject * GetRealPtr() const
	{
		if( m_pHub )
			return m_pHub->pObj;
		else
			return 0;
	}

	void copy( WgObjectWeakPtr const & r );

	WgWeakPtrHub * m_pHub;

};

//____ WgWeakPtr _________________________________________________________

template<class T,class P> class WgWeakPtr : public P
{
public:
	WgWeakPtr(T* p=0) : P( p ) {};
	WgWeakPtr(const WgWeakPtr<T,P>& r) : P( r.GetRealPtr() ) {};
//	WgWeakPtr(const WgStrongPtr<T,P>& r) : P( r.GetRealPtr() ) {};
	~WgWeakPtr() {};

	inline T & operator*() const { return * GetRealPtr(); }
	inline T * operator->() const{ return GetRealPtr(); }

	inline bool operator==(const WgWeakPtr<T,P>& other) const { return this->m_pHub == other.m_pHub; }
	inline bool operator!=(const WgWeakPtr<T,P>& other) const { return this->m_pHub != other.m_pHub; }

//	inline operator bool() const { return (this->m_pObj != 0); }

	inline T * GetRealPtr() const
	{
		if( this->m_pHub && this->m_pHub->pObj )
			return static_cast<T*>(this->m_pHub->pObj);
		else
			return reinterpret_cast<T*>(0);
	}
};



//____ WgComponentPtr _____________________________________________________________

// m_pObj and m_pInterface must both be valid or null.

class WgComponentPtr
{
public:

	WgComponentPtr()
	{
		m_pComponent = 0;
	}

	WgComponentPtr(WgComponent* pComponent )
	{
		m_pComponent = pComponent;
		if( pComponent )
			pComponent->_object()->m_refCount++;
	}

	WgComponentPtr(const WgComponentPtr& r)
	{
		m_pComponent = r.m_pComponent;
		if( m_pComponent )
			m_pComponent->_object()->m_refCount++;
	}

	~WgComponentPtr()
	{
		if( m_pComponent )
		{
			WgObject * pObj = m_pComponent->_object();
			pObj->m_refCount--;
			if( pObj->m_refCount == 0 )
				delete pObj;
		}
	}


    inline WgComponentPtr & operator=( WgComponentPtr const & r)
	{
		copy( r );
		return *this;
	}

	inline WgComponent& operator*() const { return * m_pComponent; }
	inline WgComponent* operator->() const{ return m_pComponent; }

	inline bool operator==(const WgComponentPtr& other) const { return m_pComponent == other.m_pComponent; }
	inline bool operator!=(const WgComponentPtr& other) const { return m_pComponent != other.m_pComponent; }

	inline operator bool() const { return (m_pComponent != 0); }

	inline WgComponent * GetRealPtr() const { return m_pComponent; }

protected:
	void copy( WgComponentPtr const & r )
	{
		if( m_pComponent != r.m_pComponent )
		{
			if( m_pComponent )
			{
				WgObject * pObj = m_pComponent->_object();
				pObj->m_refCount--;
				if( pObj->m_refCount == 0 )
					pObj->_destroy();
			}

			m_pComponent = r.m_pComponent;
			if( m_pComponent )
				m_pComponent->_object()->m_refCount++;
		}
	}

	WgComponent *	m_pComponent;
};

//____ WgCompStrongPtr ________________________________________________________

template<class T,class P> class WgCompStrongPtr : public P
{
public:
	WgCompStrongPtr( int dummy = 0 ) : P( 0 ) {};
	WgCompStrongPtr(T* pComponent) : P( pComponent ) {};
	WgCompStrongPtr(const WgCompStrongPtr<T,P>& r) : P( (T*) r.m_pComponent ) {};
	~WgCompStrongPtr() {};

	inline T & operator*() const { return * (T*) this->m_pComponent; }
	inline T * operator->() const{ return (T*) this->m_pComponent; }

	inline bool operator==(const WgCompStrongPtr<T,P>& other) const { return this->m_pComponent == other.m_pComponent; }
	inline bool operator!=(const WgCompStrongPtr<T,P>& other) const { return this->m_pComponent != other.m_pComponent; }

	inline T * GetRealPtr() const { return (T*) this->m_pComponent; }
};


//____ WgComponentWeakPtr ______________________________________________________________

class WgComponentWeakPtr
{
public:
	WgComponentWeakPtr() { m_pHub = 0; m_pComponent = 0; }
	WgComponentWeakPtr( WgComponent * pComponent );

	WgComponentWeakPtr(const WgComponentWeakPtr& r)
	{
		m_pComponent = r.m_pComponent;
		m_pHub = r.m_pHub;
		if( m_pHub )
			m_pHub->refCnt++;
	}

	~WgComponentWeakPtr();


    inline WgComponentWeakPtr& operator=( WgComponentWeakPtr const & r)
	{
		copy( r );
		return *this;
	}

	inline WgComponent& operator*() const { return * GetRealPtr(); }
	inline WgComponent * operator->() const { return GetRealPtr(); }

	inline bool operator==(const WgComponentWeakPtr& other) const { return m_pComponent == other.m_pComponent; }
	inline bool operator!=(const WgComponentWeakPtr& other) const { return m_pComponent != other.m_pComponent; }
	inline bool operator<(const WgComponentWeakPtr& other) const { return m_pComponent < other.m_pComponent ? true : false; }
	inline bool operator>(const WgComponentWeakPtr& other) const { return m_pComponent > other.m_pComponent ? true : false; }
	inline bool operator<=(const WgComponentWeakPtr& other) const { return m_pComponent <= other.m_pComponent ? true : false; }
	inline bool operator>=(const WgComponentWeakPtr& other) const { return m_pComponent >= other.m_pComponent ? true : false; }

	inline operator bool() const { return (m_pHub != 0 && m_pHub->pObj != 0); }

	inline WgComponent * GetRealPtr() const
	{
		if( m_pHub && m_pHub->pObj )
			return m_pComponent;
		else
			return 0;
	}

	void copy( WgComponentWeakPtr const & r );

	WgWeakPtrHub *	m_pHub;
	WgComponent *	m_pComponent;

};

//____ WgCompWeakPtr _________________________________________________________

template<class T,class P> class WgCompWeakPtr : public P
{
public:
	WgCompWeakPtr(T* pComponent=0) : P( pComponent ) {};
	WgCompWeakPtr(const WgCompWeakPtr<T,P>& r) : P( r.GetRealPtr() ) {};
	~WgCompWeakPtr() {};

	inline T & operator*() const { return * GetRealPtr(); }
	inline T * operator->() const{ return GetRealPtr(); }

	inline bool operator==(const WgCompWeakPtr<T,P>& other) const { return this->m_pComponent == other.m_pComponent; }
	inline bool operator!=(const WgCompWeakPtr<T,P>& other) const { return this->m_pComponent != other.m_pComponent; }

	inline T * GetRealPtr() const
	{
		if( this->m_pHub && this->m_pHub->pObj )
			return static_cast<T*>(this->m_pComponent);
		else
			return reinterpret_cast<T*>(0);
	}
};



#endif //WG_POINTERS_DOT_H

