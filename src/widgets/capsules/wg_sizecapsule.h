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

#include <vector>

#ifndef WG_SIZECAPSULE_DOT_H
#define WG_SIZECAPSULE_DOT_H

#ifndef WG_CAPSULE_DOT_H
#	include <wg_capsule.h>
#endif

#ifndef WG_SIZESCALER_DOT_H
#	include <wg_sizescaler.h>
#endif

namespace wg 
{
	
	class WgSizeCapsule;
	typedef	WgStrongPtr<WgSizeCapsule,WgCapsule_p>		WgSizeCapsule_p;
	typedef	WgWeakPtr<WgSizeCapsule,WgCapsule_p>		WgSizeCapsule_wp;
	
	class WgSizeCapsule : public WgCapsule, public WgScalable
	{
	public:
		static WgSizeCapsule_p	create() { return WgSizeCapsule_p(new WgSizeCapsule()); }
	
		bool		isInstanceOf( const char * pClassName ) const;
		const char *className( void ) const;
		static const char	CLASSNAME[];
		static WgSizeCapsule_p	cast( const WgObject_p& pObject );
	
		void	setSizes( WgSize min, WgSize preferred, WgSize max );
	
		void	setPreferredSize( WgSize size );
		void	setMinSize( WgSize size );
		void	setMaxSize( WgSize size );
	
		WgSize	preferredSize() const;
		WgSize	minSize() const;
		WgSize	maxSize() const;
	
		int		matchingHeight( int width ) const;
		int		matchingWidth( int height ) const;
	
	protected:
		WgSizeCapsule();
		virtual ~WgSizeCapsule();
		virtual WgWidget* _newOfMyType() const { return new WgSizeCapsule(); };
	
	private:
		void	_onScaleChanged();
	
		WgSize			m_min;
		WgSize			m_max;
		WgSize			m_preferred;
	};
	

} // namespace wg
#endif //WG_SIZECAPSULE_DOT_H
