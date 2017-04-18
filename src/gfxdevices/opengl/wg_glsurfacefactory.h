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

#ifndef	WG_GLSURFACEFACTORY_DOT_H
#define	WG_GLSURFACEFACTORY_DOT_H
#pragma once

#include <wg_surfacefactory.h>

namespace wg
{

	class GlSurfaceFactory;
	typedef	StrongPtr<GlSurfaceFactory,SurfaceFactory_p>		GlSurfaceFactory_p;
	typedef	WeakPtr<GlSurfaceFactory,SurfaceFactory_wp>	GlSurfaceFactory_wp;

	//____ GlSurfaceFactory _____________________________________________________

	class GlSurfaceFactory : public SurfaceFactory
	{
	public:
		static GlSurfaceFactory_p	create() { return GlSurfaceFactory_p(new GlSurfaceFactory()); }

		bool						isInstanceOf( const char * pClassName ) const;
		const char *				className( void ) const;
		static const char			CLASSNAME[];
		static GlSurfaceFactory_p	cast( const Object_p& pObject );

		Size		maxSize() const;
	
		Surface_p	createSurface( Size size, PixelType type = PixelType::BGRA_8, SurfaceHint hint = SurfaceHint::Static ) const;
        Surface_p	createSurface( Size size, PixelType type, const Blob_p& pBlob, int pitch, SurfaceHint hint = SurfaceHint::Static ) const;
        Surface_p	createSurface( Size size, PixelType type, uint8_t * pPixels, int pitch, const PixelFormat * pPixelFormat = 0, SurfaceHint hint = SurfaceHint::Static ) const ;
        Surface_p	createSurface( const Surface_p& pOther, SurfaceHint hint = SurfaceHint::Static ) const;
	};
}


#endif //WG_GLSURFACEFACTORY_DOT_H

