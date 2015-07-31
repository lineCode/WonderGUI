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

#ifndef WG_SURFACEFACTORY_DOT_H
#define	WG_SURFACEFACTORY_DOT_H
//==============================================================================

#ifndef	WG_SURFACE_DOT_H
#	include <wg_surface.h>
#endif

namespace wg 
{
	
	
	class WgSurfaceFactory;
	typedef	WgStrongPtr<WgSurfaceFactory,WgObject_p>	WgSurfaceFactory_p;
	typedef	WgWeakPtr<WgSurfaceFactory,WgObject_wp>	WgSurfaceFactory_wp;
	
	
	//____ WgSurfaceFactory _______________________________________________________
	/**
	 * @brief Factory class for creating surfaces.
	 *
	 * WgSurfaceFactory is the base class for all surface factories. The surface factories
	 * are used by WonderGUI components that needs to dynamically create surfaces as
	 * part of their operation, like WgVectorFont.
	 *
	 **/
	class WgSurfaceFactory : public WgObject
	{
	public:
		bool						isInstanceOf( const char * pClassName ) const;
		const char *				className( void ) const;
		static const char			CLASSNAME[];
		static WgSurfaceFactory_p	cast( const WgObject_p& pObject );
	
		virtual WgSurface_p createSurface( const WgSize& size, WgPixelType type = WG_PIXEL_ARGB_8 ) const = 0;
	protected:
		virtual ~WgSurfaceFactory() {}
	};
	
	
	//==============================================================================

} // namespace wg
#endif // WG_SURFACE_DOT_H
