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

#include <new>

#include <memory.h>

#include <wg_surface.h>
#include <wg_geo.h>

#include <wg_blockset.h>
#include <wg_mempool.h>

WgMemPool * WgSurface::g_pBlockSetMemPool = 0;

//____ WgSurface() ____________________________________________________________

WgSurface::WgSurface()
{
	m_accessMode	= WG_NO_ACCESS;
	m_pPixels		= 0;

	if( !g_pBlockSetMemPool )
		g_pBlockSetMemPool = new WgMemPool( 16, sizeof(WgBlockSet) );


	memset( &m_pixelFormat, 0, sizeof(WgPixelFormat) );
}

//____ ~WgSurface() ____________________________________________________________

WgSurface::~WgSurface()
{
}

//____ Width() ________________________________________________________________

int WgSurface::Width() const
{
	return Size().w;
}

//____ Height() _______________________________________________________________

int WgSurface::Height() const
{
	return Size().h;
}


//____ Col2Pixel() ____________________________________________________________

Uint32 WgSurface::Col2Pixel( const WgColor& col ) const
{
	Uint32 pix = ((col.r << m_pixelFormat.R_shift) & m_pixelFormat.R_mask) |
				 ((col.g << m_pixelFormat.G_shift) & m_pixelFormat.G_mask) |
				 ((col.b << m_pixelFormat.B_shift) & m_pixelFormat.B_mask) |
				 ((col.a << m_pixelFormat.A_shift) & m_pixelFormat.A_mask);

	return pix;
}

//____ Pixel2Col() ____________________________________________________________

WgColor WgSurface::Pixel2Col( Uint32 pixel ) const
{
	WgColor col( (pixel & m_pixelFormat.R_mask) >> m_pixelFormat.R_shift,
				 (pixel & m_pixelFormat.G_mask) >> m_pixelFormat.G_shift,
				 (pixel & m_pixelFormat.B_mask) >> m_pixelFormat.B_shift,
				 (pixel & m_pixelFormat.A_mask) >> m_pixelFormat.A_shift );

	return col;
}


//____ Fill() _________________________________________________________________

bool WgSurface::Fill( WgColor col )
{

	WgAccessMode oldMode = m_accessMode;

	if( oldMode != WG_READ_WRITE && oldMode != WG_WRITE_ONLY )
	{
		Lock( WG_WRITE_ONLY );
		if( m_pPixels == 0 )
			return false;
	}	

	//


	Uint32 pixel = Col2Pixel( col );
	int width = Width();
	int height = Height();
	int pitch = Pitch();
	Uint8 * pDest = m_pPixels;

	bool ret = true;
	switch( m_pixelFormat.bits )
	{
		case 8:
			for( int y = 0 ; y < height ; y++ )
			{
				for( int x = 0 ; x < width ; x++ )
					pDest[x] = (Uint8) pixel;
				pDest += pitch;
			}
			break;
		case 16:
			for( int y = 0 ; y < height ; y++ )
			{
				for( int x = 0 ; x < width ; x++ )
					((Uint16*)pDest)[x] = (Uint16) pixel;
				pDest += pitch;
			}
			break;
		case 24:
		{
			Uint8 one = (Uint8) pixel;
			Uint8 two = (Uint8) (pixel>>8);
			Uint8 three = (Uint8) (pixel>>16);

			for( int y = 0 ; y < height ; y++ )
			{
				for( int x = 0 ; x < width ; x++ )
				{
					pDest[x++] = one;
					pDest[x++] = two;
					pDest[x++] = three;
				}
				pDest += pitch - width*3;
			}
			break;
		}
		case 32:
			for( int y = 0 ; y < height ; y++ )
			{
				for( int x = 0 ; x < width ; x++ )
					((Uint32*)pDest)[x] = pixel;
				pDest += pitch;
			}
			break;
		default:
			ret = false;
	}

	//

	if( oldMode == WG_NO_ACCESS )
		Unlock();
	else if( oldMode == WG_READ_ONLY )
		Lock( oldMode );

	return ret;
}


//____ defineBlockSet() ________________________________________________________

WgBlockSetPtr WgSurface::defineBlockSet(	const WgRect& normal, const WgRect& marked,
											const WgRect& selected, const WgRect& disabled,
											const WgRect& special, const WgBorders& gfxBorders, 
											const WgBorders& contentBorders, const WgColorSetPtr& pTextColors, Uint32 flags ) const
{

	WgBlockSet * p =new(g_pBlockSetMemPool->allocEntry())
							WgBlockSet(g_pBlockSetMemPool, this, normal, marked, selected, disabled, special, gfxBorders, contentBorders, pTextColors, flags );

//	m_blockSets.push_back( p );
	return WgBlockSetPtr(p);
}

WgBlockSetPtr WgSurface::defineBlockSet(	const WgRect& normal, const WgRect& marked,
											const WgRect& selected, const WgRect& disabled,
											const WgBorders& gfxBorders, const WgBorders& contentBorders, 
											const WgColorSetPtr& pTextColors, Uint32 flags ) const
{

	WgBlockSet * p =new(g_pBlockSetMemPool->allocEntry())
							WgBlockSet(g_pBlockSetMemPool, this, normal, marked, selected, disabled, normal, gfxBorders, contentBorders, pTextColors, flags );

//	m_blockSets.push_back( p );
	return WgBlockSetPtr(p);
}

WgBlockSetPtr WgSurface::defineBlockSet(	const WgRect& normal, const WgRect& marked,
										const WgRect& selected, const WgBorders& gfxBorders,
										const WgBorders& contentBorders, const WgColorSetPtr& pTextColors, 
										Uint32 flags ) const
{
	WgBlockSet * p = new( g_pBlockSetMemPool->allocEntry())
							WgBlockSet(g_pBlockSetMemPool, this, normal, marked, selected, normal, normal, gfxBorders, contentBorders, pTextColors, flags );

//	m_blockSets.push_back( p );
	return WgBlockSetPtr(p);
}

WgBlockSetPtr WgSurface::defineBlockSet( const WgRect& normal, const WgRect& disabled,
										const WgBorders& gfxBorders, const WgBorders& contentBorders, 
										const WgColorSetPtr& pTextColors, Uint32 flags ) const
{
	WgBlockSet * p = new( g_pBlockSetMemPool->allocEntry())
							WgBlockSet(g_pBlockSetMemPool, this, normal, normal, normal, disabled, normal, gfxBorders, contentBorders, pTextColors, flags );

//	m_blockSets.push_back( p );
	return WgBlockSetPtr(p);
}

WgBlockSetPtr WgSurface::defineBlockSet( const WgRect& normal, const WgBorders& gfxBorders, const WgBorders& contentBorders, 
										 const WgColorSetPtr& pTextColors, Uint32 flags ) const
{
	WgBlockSet * p = new( g_pBlockSetMemPool->allocEntry())
							WgBlockSet(g_pBlockSetMemPool, this, normal, normal, normal, normal, normal, gfxBorders, contentBorders, pTextColors, flags );

//	m_blockSets.push_back( p );
	return WgBlockSetPtr(p);
}

WgBlockSetPtr WgSurface::defineBlockSet( const WgHorrTile5& tile, const WgBorders& gfxBorders, const WgBorders& contentBorders, 
										 const WgColorSetPtr& pTextColors, Uint32 flags ) const
{
	int	w = (tile.w - tile.skip*4) / 5;
	int h = tile.h;
	int	skip = w + tile.skip;

	return defineBlockSet(	WgRect(tile.x, tile.y, w, h),
							WgRect(tile.x+skip, tile.y, w, h),
							WgRect(tile.x+skip*2, tile.y, w, h),
							WgRect(tile.x+skip*3, tile.y, w, h),
							WgRect(tile.x+skip*4, tile.y, w, h),
							gfxBorders, contentBorders, pTextColors, flags );
}

WgBlockSetPtr WgSurface::defineBlockSet( const WgHorrTile4& tile, const WgBorders& gfxBorders, const WgBorders& contentBorders, 
										 const WgColorSetPtr& pTextColors, Uint32 flags ) const
{
	int	w = (tile.w - tile.skip*3) / 4;
	int h = tile.h;
	int	skip = w + tile.skip;

	return defineBlockSet(	WgRect(tile.x, tile.y, w, h),
							WgRect(tile.x+skip, tile.y, w, h),
							WgRect(tile.x+skip*2, tile.y, w, h),
							WgRect(tile.x+skip*3, tile.y, w, h),
							gfxBorders, contentBorders, pTextColors, flags );
}

WgBlockSetPtr WgSurface::defineBlockSet( const WgHorrTile3& tile, const WgBorders& gfxBorders, const WgBorders& contentBorders, 
										 const WgColorSetPtr& pTextColors, Uint32 flags ) const
{
	int	w = (tile.w - tile.skip*2) / 3;
	int h = tile.h;
	int	skip = w + tile.skip;

	return defineBlockSet(	WgRect(tile.x, tile.y, w, h),
							WgRect(tile.x+skip, tile.y, w, h),
							WgRect(tile.x+skip*2, tile.y, w, h),
							gfxBorders, contentBorders, pTextColors, flags );
}

WgBlockSetPtr WgSurface::defineBlockSet( const WgHorrTile2& tile, const WgBorders& gfxBorders, const WgBorders& contentBorders, 
										 const WgColorSetPtr& pTextColors, Uint32 flags ) const
{
	int	w = (tile.w - tile.skip) / 2;
	int h = tile.h;
	int	skip = w + tile.skip;

	return defineBlockSet(	WgRect(tile.x, tile.y, w, h),
							WgRect(tile.x+skip, tile.y, w, h),
							gfxBorders, contentBorders, pTextColors, flags );
}

WgBlockSetPtr WgSurface::defineBlockSet( const WgVertTile5& tile, const WgBorders& gfxBorders, const WgBorders& contentBorders, 
										 const WgColorSetPtr& pTextColors, Uint32 flags ) const
{
	int	w = tile.w;
	int h = (tile.h - tile.skip*4) / 5;
	int	skip = h + tile.skip;

	return defineBlockSet(	WgRect(tile.x, tile.y, w, h),
							WgRect(tile.x, tile.y+skip, w, h),
							WgRect(tile.x, tile.y+skip*2, w, h),
							WgRect(tile.x, tile.y+skip*3, w, h),
							WgRect(tile.x, tile.y+skip*4, w, h),
							gfxBorders, contentBorders, pTextColors, flags );
}

WgBlockSetPtr WgSurface::defineBlockSet( const WgVertTile4& tile, const WgBorders& gfxBorders, const WgBorders& contentBorders, 
										 const WgColorSetPtr& pTextColors, Uint32 flags ) const
{
	int	w = tile.w;
	int h = (tile.h - tile.skip*3) / 4;
	int	skip = h + tile.skip;

	return defineBlockSet(	WgRect(tile.x, tile.y, w, h),
							WgRect(tile.x, tile.y+skip, w, h),
							WgRect(tile.x, tile.y+skip*2, w, h),
							WgRect(tile.x, tile.y+skip*3, w, h),
							gfxBorders, contentBorders, pTextColors, flags );
}

WgBlockSetPtr WgSurface::defineBlockSet( const WgVertTile3& tile, const WgBorders& gfxBorders, const WgBorders& contentBorders, 
										 const WgColorSetPtr& pTextColors, Uint32 flags ) const
{
	int	w = tile.w;
	int h = (tile.h - tile.skip*2) / 3;
	int	skip = h + tile.skip;

	return defineBlockSet(	WgRect(tile.x, tile.y, w, h),
							WgRect(tile.x, tile.y+skip, w, h),
							WgRect(tile.x, tile.y+skip*2, w, h),
							gfxBorders, contentBorders, pTextColors, flags );
}

WgBlockSetPtr WgSurface::defineBlockSet( const WgVertTile2& tile, const WgBorders& gfxBorders, const WgBorders& contentBorders, 
										 const WgColorSetPtr& pTextColors, Uint32 flags ) const
{
	int	w = tile.w;
	int h = tile.h / 2;
	int	skip = h + tile.skip;

	return defineBlockSet(	WgRect(tile.x, tile.y, w, h),
							WgRect(tile.x, tile.y+skip, w, h),
							gfxBorders, contentBorders, pTextColors, flags );
}

