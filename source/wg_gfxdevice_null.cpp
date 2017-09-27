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

#include <wg_gfxdevice_null.h>

WgGfxDeviceNull::WgGfxDeviceNull( WgSize size ) : WgGfxDevice(size)
{
}

WgGfxDeviceNull::~WgGfxDeviceNull()
{
}

void WgGfxDeviceNull::Fill( const WgRect& rect, const WgColor& col )
{
}

void WgGfxDeviceNull::Blit( const WgSurface* src, const WgRect& srcrect, int dx, int dy  )
{
}

void WgGfxDeviceNull::TileBlit( const WgSurface* src, const WgRect& srcrect, const WgRect& dest )
{
}

void WgGfxDeviceNull::FillSubPixel( const WgRectF& rect, const WgColor& col )
{
}

void WgGfxDeviceNull::StretchBlitSubPixel( 	const WgSurface * pSrc, float sx, float sy, float sw, float sh,
						   		 			float dx, float dy, float dw, float dh, bool bTriLinear, float mipBias )
{
}

void WgGfxDeviceNull::DrawLine( WgCoord begin, WgCoord end, WgColor color, float thickness )
{
}

void WgGfxDeviceNull::DrawArcNE( const WgRect& rect, WgColor color )
{
}

void WgGfxDeviceNull::DrawElipse( const WgRect& rect, WgColor color )
{
}

void WgGfxDeviceNull::DrawFilledElipse( const WgRect& rect, WgColor color )
{
}

void WgGfxDeviceNull::ClipDrawLine( const WgRect& clip, WgCoord begin, WgCoord end, WgColor color, float thickness )
{
}

void WgGfxDeviceNull::ClipDrawArcNE( const WgRect& clip, const WgRect& rect, WgColor color )
{
}

void WgGfxDeviceNull::ClipDrawElipse( const WgRect& clip, const WgRect& rect, WgColor color )
{
}

void WgGfxDeviceNull::ClipDrawFilledElipse( const WgRect& clip, const WgRect& rect, WgColor color )
{
}

void WgGfxDeviceNull::ClipDrawHorrWave(const WgRect& clip, WgCoord begin, int length, const WgWaveLine& topLine, const WgWaveLine& bottomLine, WgColor front, WgColor back)
{
}