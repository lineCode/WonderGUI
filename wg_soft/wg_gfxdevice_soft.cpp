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

#include <algorithm>

#include <wg_gfxdevice_soft.h>
#include <wg_surface_soft.h>
#include <wg_base.h>

#include <assert.h>
#include <math.h>
#include <cstdlib>

#include <wg_userdefines.h>

#define NB_CURVETAB_ENTRIES	1024


#ifdef SOFTUBE_USE_PACE_FUSION
#include "PaceFusion.h"
PACE_FUSION_EXCLUDE_USER_CALLBACKS
#endif

//____ Constructor _____________________________________________________________

WgGfxDeviceSoft::WgGfxDeviceSoft() : WgGfxDevice(WgSize(0,0))
{
	m_pCanvas = 0;
	_initTables();
}
 
WgGfxDeviceSoft::WgGfxDeviceSoft( WgSurfaceSoft * pCanvas ) : WgGfxDevice( pCanvas?pCanvas->Size():WgSize() )
{
	m_pCanvas = pCanvas;
	WgGfxDevice::m_pCanvas = pCanvas;
	_initTables();
}

//____ Destructor ______________________________________________________________

WgGfxDeviceSoft::~WgGfxDeviceSoft()
{
	delete m_pCanvas;
	delete [] m_pDivTab;
}

//____ SetCanvas() _______________________________________________________________

bool WgGfxDeviceSoft::SetCanvas( WgSurface * pCanvas )
{
	if (pCanvas && pCanvas->Type() != WgSurfaceSoft::GetClass() )
		return false;

	m_pCanvas = static_cast<WgSurfaceSoft*>(pCanvas);
	WgGfxDevice::m_pCanvas = pCanvas;
	if( pCanvas )
		m_canvasSize = pCanvas->Size();
	else
		m_canvasSize = WgSize();
}

//____ Fill() ____________________________________________________________________

void WgGfxDeviceSoft::Fill( const WgRect& rect, const WgColor& col )
{
	if( !m_pCanvas || !m_pCanvas->m_pData )
		return;

	WgColor fillColor = col * m_tintColor;

	// Skip calls that won't affect destination

	if( fillColor.a == 0 && (m_blendMode == WG_BLENDMODE_BLEND || m_blendMode == WG_BLENDMODE_ADD) )
		return;

	// Optimize calls

	WgBlendMode blendMode = m_blendMode;
	if( blendMode == WG_BLENDMODE_BLEND && fillColor.a == 255 )
		blendMode = WG_BLENDMODE_OPAQUE;

	//

	int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	Uint8 * pDst = m_pCanvas->m_pData + rect.y * m_pCanvas->m_pitch + rect.x * pixelBytes;




	switch( blendMode )
	{
		case WG_BLENDMODE_OPAQUE:
		{
            int dstPixelBytes = m_pCanvas->m_pixelFormat.bits/8;
            
            if( dstPixelBytes == 4 )
            {
                for( int y = 0 ; y < rect.h ; y++ )
                {
                    Uint32 fillValue = ((int)fillColor.b) | (((int)fillColor.g) << 8) | (((int)fillColor.r) << 16);
                    
                    for( int x = 0 ; x < rect.w*pixelBytes ; x+=pixelBytes )
                    {
                        * ((Uint32*)(&pDst[x])) = fillValue;
                    }
                    pDst += m_pCanvas->m_pitch;
                }
            }
            else
            {
                for( int y = 0 ; y < rect.h ; y++ )
                {
                    for( int x = 0 ; x < rect.w*pixelBytes ; x+=pixelBytes )
                    {
                        pDst[x] = fillColor.b;
                        pDst[x+1] = fillColor.g;
                        pDst[x+2] = fillColor.r;
                    }
                    pDst += m_pCanvas->m_pitch;
                }
            }
		}
		break;
		case WG_BLENDMODE_BLEND:
		{
			int storedRed = ((int)fillColor.r) * fillColor.a;
			int storedGreen = ((int)fillColor.g) * fillColor.a;
			int storedBlue = ((int)fillColor.b) * fillColor.a;
			int invAlpha = 255-fillColor.a;

			for( int y = 0 ; y < rect.h ; y++ )
			{
				for( int x = 0 ; x < rect.w*pixelBytes ; x+= pixelBytes )
				{
					pDst[x] = m_pDivTab[pDst[x]*invAlpha + storedBlue];
					pDst[x+1] = m_pDivTab[pDst[x+1]*invAlpha + storedGreen];
					pDst[x+2] = m_pDivTab[pDst[x+2]*invAlpha + storedRed];
				}
				pDst += m_pCanvas->m_pitch;
			}
			break;
		}
		case WG_BLENDMODE_ADD:
		{
			int storedRed = (int) m_pDivTab[fillColor.r * fillColor.a];
			int storedGreen = (int) m_pDivTab[fillColor.g * fillColor.a];
			int storedBlue = (int) m_pDivTab[fillColor.b * fillColor.a];

			if( storedRed + storedGreen + storedBlue == 0 )
				return;

			for( int y = 0 ; y < rect.h ; y++ )
			{
				for( int x = 0 ; x < rect.w*pixelBytes ; x+= pixelBytes )
				{
					pDst[x] = m_limitTable[pDst[x] + storedBlue];
					pDst[x+1] = m_limitTable[pDst[x+1] + storedGreen];
					pDst[x+2] = m_limitTable[pDst[x+2] + storedRed];
				}
				pDst += m_pCanvas->m_pitch;
			}
			break;
		}
		case WG_BLENDMODE_MULTIPLY:
		{
			int storedRed = (int)fillColor.r;
			int storedGreen = (int)fillColor.g;
			int storedBlue = (int)fillColor.b;

			for( int y = 0 ; y < rect.h ; y++ )
			{
				for( int x = 0 ; x < rect.w*pixelBytes ; x+= pixelBytes )
				{
					pDst[x] = m_pDivTab[pDst[x] * storedBlue];
					pDst[x+1] = m_pDivTab[pDst[x+1] * storedGreen];
					pDst[x+2] = m_pDivTab[pDst[x+2] * storedRed];
				}
				pDst += m_pCanvas->m_pitch;
			}
			break;
		}
		case WG_BLENDMODE_INVERT:
		{
			int storedRed = (int)fillColor.r;
			int storedGreen = (int)fillColor.g;
			int storedBlue = (int)fillColor.b;

			int invertRed = 255 - (int)fillColor.r;
			int invertGreen = 255 - (int)fillColor.g;
			int invertBlue = 255 - (int)fillColor.b;


			for( int y = 0 ; y < rect.h ; y++ )
			{
				for( int x = 0 ; x < rect.w*pixelBytes ; x+= pixelBytes )
				{
					pDst[x] = m_pDivTab[(255-pDst[x]) * storedBlue + pDst[x] * invertBlue];
					pDst[x+1] = m_pDivTab[(255-pDst[x+1]) * storedGreen + pDst[x+1] * invertGreen];
					pDst[x+2] = m_pDivTab[(255-pDst[x+2]) * storedRed + pDst[x+2] * invertRed];
				}
				pDst += m_pCanvas->m_pitch;
			}
			break;
		}
		default:
			break;
	}
}

//____ FillSubPixel() ____________________________________________________________________

void WgGfxDeviceSoft::FillSubPixel( const WgRectF& rect, const WgColor& col )
{
	if( !m_pCanvas || !m_pCanvas->m_pData )
		return;
	
	WgColor fillColor = col * m_tintColor;
	
	// Skip calls that won't affect destination
	
	if( fillColor.a == 0 && (m_blendMode == WG_BLENDMODE_BLEND || m_blendMode == WG_BLENDMODE_ADD) )
		return;
	
	// Fill all but anti-aliased edges
	
	int x1 = (int) (rect.x + 0.999f);
	int y1 = (int) (rect.y + 0.999f);
	int x2 = (int) (rect.x + rect.w);
	int y2 = (int) (rect.y + rect.h);
		
	Fill( WgRect( x1,y1,x2-x1,y2-y1 ), col );	// Don't apply tint, done inside Fill().
	
	// Optimize calls
	
	WgBlendMode blendMode = m_blendMode;
	if( blendMode == WG_BLENDMODE_BLEND && fillColor.a == 255 )
		blendMode = WG_BLENDMODE_OPAQUE;
	
	// Draw the sides
		
	int aaLeft = (256 - (int)(rect.x * 256)) & 0xFF;
	int aaTop = (256 - (int)(rect.y * 256)) & 0xFF;
	int aaRight = ((int)((rect.x + rect.w) * 256)) & 0xFF;
	int aaBottom = ((int)((rect.y + rect.h) * 256)) & 0xFF;
	
	if( aaTop != 0 )
		_drawHorrVertLineAA( x1, (int) rect.y, x2-x1, fillColor, blendMode, aaTop, WG_HORIZONTAL );
	
	if( aaBottom != 0 )
		_drawHorrVertLineAA( x1, (int) y2, x2-x1, fillColor, blendMode, aaBottom, WG_HORIZONTAL );
	
	if( aaLeft != 0 )
		_drawHorrVertLineAA( (int) rect.x, y1, y2-y1, fillColor, blendMode, aaLeft, WG_VERTICAL );
	
	if( aaRight != 0 )
		_drawHorrVertLineAA( (int) x2, y1, y2-y1, fillColor, blendMode, aaRight, WG_VERTICAL );	
	
	// Draw corner pieces
	
	int aaTopLeft = aaTop * aaLeft / 256;
	int aaTopRight = aaTop * aaRight / 256;
	int aaBottomLeft = aaBottom * aaLeft / 256;
	int aaBottomRight = aaBottom * aaRight / 256;
	
	if( aaTopLeft != 0 )
		_plotAA( (int) rect.x, (int) rect.y, fillColor, blendMode, aaTopLeft );

	if( aaTopRight != 0 )
		_plotAA( x2, (int) rect.y, fillColor, blendMode, aaTopRight );

	if( aaBottomLeft != 0 )
		_plotAA( (int) rect.x, y2, fillColor, blendMode, aaBottomLeft );

	if( aaBottomRight != 0 )
		_plotAA( x2, y2, fillColor, blendMode, aaBottomRight );
}


//____ DrawLine() ______________________________________________________________

void WgGfxDeviceSoft::DrawLine( WgCoord beg, WgCoord end, WgColor color, float thickness )
{
	if( !m_pCanvas || !m_pCanvas->m_pData )
		return;

	WgColor fillColor = color * m_tintColor;

	// Skip calls that won't affect destination

	if( fillColor.a == 0 && (m_blendMode == WG_BLENDMODE_BLEND || m_blendMode == WG_BLENDMODE_ADD) )
		return;

	Uint8 *	pRow;
	int		rowInc, pixelInc;
	int 	length, width;
	int		pos, slope;

	if( abs(beg.x-end.x) > abs(beg.y-end.y) )
	{
		// Prepare mainly horizontal line segment
		
		if( beg.x > end.x )
			WgSwap( beg, end );
		
		length = end.x - beg.x;
		slope = ((end.y - beg.y) << 16) / length;

		width = (int) (thickness*m_lineThicknessTable[abs(slope>>12)]);
		pos = (beg.y << 16) - width/2;		
				
		rowInc = m_pCanvas->m_pixelFormat.bits/8;
		pixelInc = m_pCanvas->m_pitch;

		pRow = m_pCanvas->m_pData + beg.x * rowInc;
	}
	else
	{
		// Prepare mainly vertical line segment
		
		if( beg.y > end.y )
			WgSwap( beg, end );
		
		length = end.y - beg.y;
		if( length == 0 )
			return;											// TODO: Should stil draw the caps!

		slope = ((end.x - beg.x) << 16) / length;
		width = (int) (thickness*m_lineThicknessTable[abs(slope>>12)]);
		pos = (beg.x << 16) - width/2;		
				
		rowInc = m_pCanvas->m_pitch;
		pixelInc = m_pCanvas->m_pixelFormat.bits/8;

		pRow = m_pCanvas->m_pData + beg.y * rowInc;		
	}

	_drawLineSegment( pRow, rowInc, pixelInc, length, width, pos, slope, fillColor );
}

//____ ClipDrawLine() __________________________________________________________

void WgGfxDeviceSoft::ClipDrawLine( const WgRect& clip, WgCoord beg, WgCoord end, WgColor color, float thickness )
{
	if( !m_pCanvas || !m_pCanvas->m_pData )
		return;

	WgColor fillColor = color * m_tintColor;

	// Skip calls that won't affect destination

	if( fillColor.a == 0 && (m_blendMode == WG_BLENDMODE_BLEND || m_blendMode == WG_BLENDMODE_ADD) )
		return;

	Uint8 *	pRow;
	int		rowInc, pixelInc;
	int 	length, width;
	int		pos, slope;
	int		clipStart, clipEnd;

	if( abs(beg.x-end.x) > abs(beg.y-end.y) )
	{
		// Prepare mainly horizontal line segment
		
		if( beg.x > end.x )
			WgSwap( beg, end );
		
		length = end.x - beg.x;
		slope = ((end.y - beg.y) << 16) / length;

		width = (int) (thickness*m_lineThicknessTable[abs(slope>>12)]);
		pos = (beg.y << 16) - width/2;		
				
		rowInc = m_pCanvas->m_pixelFormat.bits/8;
		pixelInc = m_pCanvas->m_pitch;

		pRow = m_pCanvas->m_pData + beg.x * rowInc;

		// Do clipping for line segment
		
		if( beg.x > clip.x + clip.w || end.x < clip.x )
			return;										// Segement not visible.
			
		if( beg.x < clip.x )
		{
			int cut = clip.x - beg.x;
			length -= cut;
			pRow += rowInc*cut;
			pos += slope*cut;
		}

		if( end.x > clip.x + clip.w )
			length -= end.x - (clip.x+clip.w);

		clipStart = clip.y << 16;
		clipEnd = (clip.y + clip.h) <<16;
	}
	else
	{
		// Prepare mainly vertical line segment
		
		if( beg.y > end.y )
			WgSwap( beg, end );
		
		length = end.y - beg.y;
		if( length == 0 )
			return;											// TODO: Should stil draw the caps!

		slope = ((end.x - beg.x) << 16) / length;
		width = (int) (thickness*m_lineThicknessTable[abs(slope>>12)]);
		pos = (beg.x << 16) - width/2;		
				
		rowInc = m_pCanvas->m_pitch;
		pixelInc = m_pCanvas->m_pixelFormat.bits/8;

		pRow = m_pCanvas->m_pData + beg.y * rowInc;		

		// Do clipping for line segment
		
		if( beg.y > clip.y + clip.h || end.y < clip.y )
			return;										// Segement not visible.
			
		if( beg.y < clip.y )
		{
			int cut = clip.y - beg.y;
			length -= cut;
			pRow += rowInc*cut;
			pos += slope*cut;
		}

		if( end.y > clip.y + clip.h )
			length -= end.y - (clip.y+clip.h);
			
		clipStart = clip.x << 16;
		clipEnd = (clip.x + clip.w) <<16;
	}

	_clipDrawLineSegment( clipStart, clipEnd, pRow, rowInc, pixelInc, length, width, pos, slope, fillColor );
}

//____ _drawLineSegment() ______________________________________________________

void WgGfxDeviceSoft::_drawLineSegment( Uint8 * pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, WgColor color )
{
	//TODO: Translate to use m_pDivTab

	for( int i = 0 ; i < length ; i++ )
	{
		
		int beg = pos >> 16;
		int end = (pos + width) >> 16;
		int ofs = beg;
		
		if( beg == end )
		{
			// Special case, one pixel wide row
			
			int alpha = (color.a * width) >> 16;

			int invAlpha = 255 - alpha;
			Uint8 * pDst = pRow + ofs*pixelInc;
			
			pDst[0] = m_pDivTab[pDst[0]*invAlpha + color.b*alpha];
			pDst[1] = m_pDivTab[pDst[1]*invAlpha + color.g*alpha];
			pDst[2] = m_pDivTab[pDst[2]*invAlpha + color.r*alpha];			
		}
		else
		{
			// First anti-aliased pixel of column
			
			int alpha = (color.a * (65536 - (pos & 0xFFFF))) >> 16;
			
			int invAlpha = 255 - alpha;
			Uint8 * pDst = pRow + ofs*pixelInc;
			
			pDst[0] = m_pDivTab[pDst[0]*invAlpha + color.b*alpha];
			pDst[1] = m_pDivTab[pDst[1]*invAlpha + color.g*alpha];
			pDst[2] = m_pDivTab[pDst[2]*invAlpha + color.r*alpha];			
			ofs++;
			
			// All non-antialiased middle pixels of column
			
			
			if( ofs < end )
			{					
				alpha = color.a;	
				invAlpha = 255 - alpha;

				int storedRed = color.r * alpha;
				int storedGreen = color.g * alpha;
				int storedBlue = color.b * alpha;

				do 
				{
					pDst = pRow + ofs*pixelInc;						
					pDst[0] = m_pDivTab[pDst[0]*invAlpha + storedBlue];
					pDst[1] = m_pDivTab[pDst[1]*invAlpha + storedGreen];
					pDst[2] = m_pDivTab[pDst[2]*invAlpha + storedRed];			
					ofs++;
					
				} while( ofs < end );
			}

			// Last anti-aliased pixel of column

			int overflow = (pos+width) & 0xFFFF;
			if( overflow > 0 )
			{
				alpha = (color.a * overflow) >> 16;
				
				invAlpha = 255 - alpha;
				pDst = pRow + ofs*pixelInc;
				
				pDst[0] = m_pDivTab[pDst[0]*invAlpha + color.b*alpha];
				pDst[1] = m_pDivTab[pDst[1]*invAlpha + color.g*alpha];
				pDst[2] = m_pDivTab[pDst[2]*invAlpha + color.r*alpha];			
			}
		}
		
		pRow += rowInc;
		pos += slope;
	}
}

//____ _clipDrawLineSegment() ______________________________________________________

void WgGfxDeviceSoft::_clipDrawLineSegment( int clipStart, int clipEnd, Uint8 * pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, WgColor color )
{
	//TODO: Translate to use m_pDivTab

	for( int i = 0 ; i < length ; i++ )
	{
		
		if( pos >= clipEnd || pos + width <= clipStart )
		{
			pRow += rowInc;
			pos += slope;
			continue;
		}
		
		int clippedPos = pos;
		int clippedWidth = width;
		
		if( clippedPos < clipStart )
		{
			clippedWidth -= clipStart - clippedPos;
			clippedPos = clipStart;
		}
		
		if( clippedPos + clippedWidth > clipEnd )
			clippedWidth = clipEnd - clippedPos;
		
		
		int beg = clippedPos >> 16;
		int end = (clippedPos + clippedWidth) >> 16;
		int ofs = beg;
		
		if( beg == end )
		{
			// Special case, one pixel wide row
			
			int alpha = (color.a * clippedWidth) >> 16;

			int invAlpha = 255 - alpha;
			Uint8 * pDst = pRow + ofs*pixelInc;
			
			pDst[0] = m_pDivTab[pDst[0]*invAlpha + color.b*alpha];
			pDst[1] = m_pDivTab[pDst[1]*invAlpha + color.g*alpha];
			pDst[2] = m_pDivTab[pDst[2]*invAlpha + color.r*alpha];			
		}
		else
		{
			// First anti-aliased pixel of column
			
			int alpha = (color.a * (65536 - (clippedPos & 0xFFFF))) >> 16;
			
			int invAlpha = 255 - alpha;
			Uint8 * pDst = pRow + ofs*pixelInc;
			
			pDst[0] = m_pDivTab[pDst[0]*invAlpha + color.b*alpha];
			pDst[1] = m_pDivTab[pDst[1]*invAlpha + color.g*alpha];
			pDst[2] = m_pDivTab[pDst[2]*invAlpha + color.r*alpha];			
			ofs++;
			
			// All non-antialiased middle pixels of column
			
			
			if( ofs < end )
			{					
				alpha = color.a;	
				invAlpha = 255 - alpha;

				int storedRed = color.r * alpha;
				int storedGreen = color.g * alpha;
				int storedBlue = color.b * alpha;

				do 
				{
					pDst = pRow + ofs*pixelInc;						
					pDst[0] = m_pDivTab[pDst[0]*invAlpha + storedBlue];
					pDst[1] = m_pDivTab[pDst[1]*invAlpha + storedGreen];
					pDst[2] = m_pDivTab[pDst[2]*invAlpha + storedRed];			
					ofs++;
					
				} while( ofs < end );
			}

			// Last anti-aliased pixel of column

			int overflow = (clippedPos+clippedWidth) & 0xFFFF;
			if( overflow > 0 )
			{
				alpha = (color.a * overflow) >> 16;
				invAlpha = 255 - alpha;
				pDst = pRow + ofs*pixelInc;
			
				pDst[0] = m_pDivTab[pDst[0]*invAlpha + color.b*alpha];
				pDst[1] = m_pDivTab[pDst[1]*invAlpha + color.g*alpha];
				pDst[2] = m_pDivTab[pDst[2]*invAlpha + color.r*alpha];			
			}
		}
		
		pRow += rowInc;
		pos += slope;
	}
}

//____ ClipDrawHorrWave() _____________________________________________________

void WgGfxDeviceSoft::ClipDrawHorrWave(const WgRect& clip, WgCoord begin, int length, const WgWaveLine& topBorder, const WgWaveLine& bottomBorder, WgColor frontFill, WgColor backFill)
{
	if (!m_pCanvas || !m_pCanvas->m_pData)
		return;

	// Do early rough X-clipping with margin (need to trace lines with margin of thickest line).

	int ofs = 0;
	if (clip.x > begin.x || clip.x + clip.w < begin.x + length)
	{
		int margin = (int)(WgMax(topBorder.thickness, bottomBorder.thickness) / 2 + 0.99);

		if (clip.x > begin.x + margin)
		{
			ofs = clip.x - begin.x - margin;
			begin.x += ofs;
			length -= ofs;
		}

		if (begin.x + length - margin > clip.x + clip.w)
			length = clip.x + clip.w - begin.x + margin;

		if (length <= 0)
			return;
	}

	// Generate line traces

	int	bufferSize = (length + 1) * 2 * sizeof(int) * 2;	// length+1 * values per point * sizeof(int) * 2 separate traces.
	char * pBuffer = WgBase::MemStackAlloc(bufferSize);
	int * pTopBorderTrace = (int*)pBuffer;
	int * pBottomBorderTrace = (int*)(pBuffer + bufferSize / 2);

	_traceLine(pTopBorderTrace, length+1, topBorder, ofs);
	_traceLine(pBottomBorderTrace, length+1, bottomBorder, ofs);

	// Do proper X-clipping

	int startColumn = 0;
	if (begin.x < clip.x)
	{
		startColumn = clip.x - begin.x;
		length -= startColumn;
		begin.x += startColumn;
	}

	if (begin.x + length > clip.x + clip.w)
		length = clip.x + clip.w - begin.x;

	// Render columns

	uint8_t * pColumn = m_pCanvas->m_pData + begin.y * m_pCanvas->m_pitch + begin.x * (m_pCanvas->m_pixelFormat.bits / 8);
	int pos[2][4];						// Startpositions for the 4 fields of the column (topline, fill, bottomline, line end) for left and right edge of pixel column. 16 binals.

	int clipBeg = clip.y - begin.y;
	int clipLen = clip.h;

	WgColor	col[4];
	col[0] = topBorder.color;
	col[1] = frontFill;
	col[2] = bottomBorder.color;
	col[3] = backFill;


	for (int i = startColumn; i <= length + startColumn; i++)
	{
		// Old right pos becomes new left pos and old left pos will be reused for new right pos

		int * pLeftPos = pos[i % 2];
		int * pRightPos = pos[(i + 1) % 2];

		// Check if lines have intersected and in that case swap top and bottom lines and colors

		if (pTopBorderTrace[i * 2] > pBottomBorderTrace[i * 2])
		{
			std::swap(col[0], col[2]);
			std::swap(col[1], col[3]);
			std::swap(pTopBorderTrace, pBottomBorderTrace);

			// We need to regenerate leftpos since we now have swapped top and bottom line.

			if (i > startColumn)
			{
				int j = i - 1;
				pLeftPos[0] = pTopBorderTrace[j * 2] << 8;
				pLeftPos[1] = pTopBorderTrace[j * 2 + 1] << 8;

				pLeftPos[2] = pBottomBorderTrace[j * 2] << 8;
				pLeftPos[3] = pBottomBorderTrace[j * 2 + 1] << 8;

				if (pLeftPos[2] < pLeftPos[1])
				{
					pLeftPos[2] = pLeftPos[1];
					if (pLeftPos[3] < pLeftPos[2])
						pLeftPos[3] = pLeftPos[2];
				}
			}
		}

		// Generate new rightpos table

		pRightPos[0] = pTopBorderTrace[i * 2] << 8;
		pRightPos[1] = pTopBorderTrace[i * 2 + 1] << 8;

		pRightPos[2] = pBottomBorderTrace[i * 2] << 8;
		pRightPos[3] = pBottomBorderTrace[i * 2 + 1] << 8;


		if (pRightPos[2] < pRightPos[1])
		{
			pRightPos[2] = pRightPos[1];
			if (pRightPos[3] < pRightPos[2])
				pRightPos[3] = pRightPos[2];
		}

		// Render the column

		if (i > startColumn)
		{
			_clipDrawWaveColumn(clipBeg, clipLen, pColumn, pLeftPos, pRightPos, col, m_pCanvas->m_pitch);
			pColumn += m_pCanvas->m_pixelFormat.bits / 8;
		}
	}

	// Free temporary work memory

	WgBase::MemStackRelease(bufferSize);
}


//_____ _clipDrawWaveColumn() ________________________________________________

void WgGfxDeviceSoft::_clipDrawWaveColumn(int clipBeg, int clipLen, uint8_t * pColumn, int leftPos[4], int rightPos[4], WgColor col[3], int linePitch)
{
	// 16 binals on leftPos, rightPos and most calculations.

	int i = 0;

	int amount[4];
	int inc[4];

	int columnBeg = (WgMin(leftPos[0], rightPos[0]) & 0xFFFF0000) + 32768;		// Column starts in middle of first pixel

																				// Calculate start amount and increment for our 4 fields

	for (int i = 0; i < 4; i++)
	{
		int yBeg;
		int64_t xInc;

		if (leftPos[i] < rightPos[i])
		{
			yBeg = leftPos[i];
			xInc = (int64_t)65536 * 65536 / (rightPos[i] - leftPos[i] + 1);
		}
		else
		{
			yBeg = rightPos[i];
			xInc = (int64_t)65536 * 65536 / (leftPos[i] - rightPos[i] + 1);
		}

		WG_LIMIT(xInc, (int64_t)0, (int64_t)65536);

		inc[i] = (int)xInc;

		int64_t startAmount = -((xInc * (yBeg - columnBeg)) >> 16);
		amount[i] = (int)startAmount;
	}

	// Do clipping

	if (columnBeg < (clipBeg << 16))
	{
		int64_t forwardAmount = (clipBeg << 16) - columnBeg;

		for (int i = 0; i < 4; i++)
			amount[i] += (inc[i] * forwardAmount) >> 16;

		columnBeg = (clipBeg << 16);
	}

	uint8_t * pDstClip = pColumn + (clipBeg + clipLen) * linePitch;

	// Render the column

	uint8_t * pDst = pColumn + linePitch * (columnBeg >> 16);

	switch (m_blendMode)
	{
	case WG_BLENDMODE_BLEND:
	{
		// First render loop, run until we are fully into fill (or later section).
		// This needs to cover all possibilities since topLine, fill and bottomLine might be less than 1 pixel combined
		// in which case they should all be rendered.

		while (amount[1] < 65536 && pDst < pDstClip)
		{
			int aFrac = amount[0];
			int bFrac = amount[1];
			int cFrac = amount[2];
			int dFrac = amount[3];
			WG_LIMIT(aFrac, 0, 65536);
			WG_LIMIT(bFrac, 0, 65536);
			WG_LIMIT(cFrac, 0, 65536);
			WG_LIMIT(dFrac, 0, 65536);

			aFrac = ((aFrac - bFrac)*col[0].a) / 255;
			bFrac = ((bFrac - cFrac)*col[1].a) / 255;
			cFrac = ((cFrac - dFrac)*col[2].a) / 255;

			int backFraction = 65536 - aFrac - bFrac - cFrac;

			pDst[0] = (pDst[0] * backFraction + col[0].b * aFrac + col[1].b * bFrac + col[2].b * cFrac) >> 16;
			pDst[1] = (pDst[1] * backFraction + col[0].g * aFrac + col[1].g * bFrac + col[2].g * cFrac) >> 16;
			pDst[2] = (pDst[2] * backFraction + col[0].r * aFrac + col[1].r * bFrac + col[2].r * cFrac) >> 16;
			pDst += linePitch;

			for (int i = 0; i < 4; i++)
				amount[i] += inc[i];
		}

		// Second render loop, optimzed fill-section loop until bottomLine starts to fade in.

		if (amount[2] <= 0 && pDst < pDstClip)
		{
			if (col[1].a == 255)
			{
				while (amount[2] <= 0 && pDst < pDstClip)
				{
					pDst[0] = col[1].b;
					pDst[1] = col[1].g;
					pDst[2] = col[1].r;
					pDst += linePitch;

					amount[2] += inc[2];
					amount[3] += inc[3];
				}
			}
			else
			{
				int fillFrac = (65536 * col[1].a) / 255;

				int fillB = col[1].b * fillFrac;
				int fillG = col[1].g * fillFrac;
				int fillR = col[1].r * fillFrac;
				int backFraction = 65536 - fillFrac;

				while (amount[2] <= 0 && pDst < pDstClip)
				{
					pDst[0] = (pDst[0] * backFraction + fillB) >> 16;
					pDst[1] = (pDst[1] * backFraction + fillG) >> 16;
					pDst[2] = (pDst[2] * backFraction + fillR) >> 16;
					pDst += linePitch;

					amount[2] += inc[2];
					amount[3] += inc[3];
				}
			}
		}


		// Third render loop, from when bottom line has started to fade in.
		// We can safely ignore topLine (not visible anymore) and amount[2] is guaranteed to have reached 65536.

		while (amount[3] < 65536 && pDst < pDstClip)
		{
			int cFrac = amount[2];
			int dFrac = amount[3];
			WG_LIMIT(cFrac, 0, 65536);
			WG_LIMIT(dFrac, 0, 65536);

			int bFrac = ((65536 - cFrac)*col[1].a) / 255;
			cFrac = ((cFrac - dFrac)*col[2].a) / 255;

			int backFraction = 65536 - bFrac - cFrac;

			pDst[0] = (pDst[0] * backFraction + col[1].b * bFrac + col[2].b * cFrac) >> 16;
			pDst[1] = (pDst[1] * backFraction + col[1].g * bFrac + col[2].g * cFrac) >> 16;
			pDst[2] = (pDst[2] * backFraction + col[1].r * bFrac + col[2].r * cFrac) >> 16;
			pDst += linePitch;

			amount[2] += inc[2];
			amount[3] += inc[3];
		}
		break;
	}

	case WG_BLENDMODE_ADD:
		break;

	case WG_BLENDMODE_INVERT:
		break;

	case WG_BLENDMODE_MULTIPLY:
		break;

	case WG_BLENDMODE_OPAQUE:
		break;

	default:
		break;

	}
}



//____ ClipDrawHorrLine() _____________________________________________________

void WgGfxDeviceSoft::ClipDrawHorrLine( const WgRect& clip, const WgCoord& start, int length, const WgColor& col )
{
	if( start.y < clip.y || start.y >= clip.y + clip.h || start.x >= clip.x + clip.w || start.x + length <= clip.x )
		return;

	int x = start.x;

	if( x < clip.x )
	{
		length = start.x + length - clip.x;
		x = clip.x;
	}

	if( x + length > clip.x + clip.w )
		length = clip.x + clip.w - x;

	_drawHorrVertLine( x, start.y, length, col, WG_HORIZONTAL );
}

//____ ClipDrawVertLine() _____________________________________________________

void WgGfxDeviceSoft::ClipDrawVertLine( const WgRect& clip, const WgCoord& start, int length, const WgColor& col )
{
	if( start.x < clip.x || start.x >= clip.x + clip.w || start.y >= clip.y + clip.h || start.y + length <= clip.y )
		return;

	int y = start.y;

	if( y < clip.y )
	{
		length = start.y + length - clip.y;
		y = clip.y;
	}

	if( y + length > clip.y + clip.h )
		length = clip.y + clip.h - y;

	_drawHorrVertLine( start.x, y, length, col, WG_VERTICAL );
}

//____ ClipPlotSoftPixels() _______________________________________________________

void WgGfxDeviceSoft::ClipPlotSoftPixels( const WgRect& clip, int nCoords, const WgCoord * pCoords, const WgColor& col, float thickness )
{
	int pitch = m_pCanvas->m_pitch;
	int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;

	int offset[4];

	offset[0] = -pixelBytes;
	offset[1] = -pitch;
	offset[2] = pixelBytes;
	offset[3] = pitch;

	int alpha = (int) (256*(thickness - 1.f)/2);

	int storedRed = ((int)col.r) * alpha;
	int storedGreen = ((int)col.g) * alpha;
	int storedBlue = ((int)col.b) * alpha;
	int invAlpha = 255-alpha;

	int yp = pCoords[0].y;

	for( int i = 0 ; i < nCoords ; i++ )
	{
		int x = pCoords[i].x;
		int begY;
		int endY;

		if( yp > pCoords[i].y )
		{
			begY = pCoords[i].y;
			endY = yp-1;
		}
		else if( pCoords[i].y > yp )
		{
			begY = yp+1;
			endY = pCoords[i].y;
		}
		else
		{
			begY = endY = yp;
		}

		for( int y = begY ; y <= endY ; y++ )
		{
			Uint8 * pDst = m_pCanvas->m_pData + y * m_pCanvas->m_pitch + pCoords[i].x * pixelBytes;

			if( y >= clip.y && y <= clip.y + clip.h -1 && x >= clip.x && x <= clip.x + clip.w -1 )
			{
				pDst[0] = col.b;
				pDst[1] = col.g;
				pDst[2] = col.r;

				for( int x = 0 ; x < 4 ; x++ )
				{
					int ofs = offset[x];
					pDst[ofs] = m_pDivTab[pDst[ofs]*invAlpha + storedBlue];
					pDst[ofs+1] = m_pDivTab[pDst[ofs+1]*invAlpha + storedGreen];
					pDst[ofs+2] = m_pDivTab[pDst[ofs+2]*invAlpha + storedRed];
				}
			}
		}

		yp = pCoords[i].y;
	}
}
//____ ClipPlotPixels() _______________________________________________________

void WgGfxDeviceSoft::ClipPlotPixels( const WgRect& clip, int nCoords, const WgCoord * pCoords, const WgColor * colors)
{
	const int pitch = m_pCanvas->m_pitch;
	const int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;

	for( int i = 0 ; i < nCoords ; i++ )
	{
		const int x = pCoords[i].x;
		const int y = pCoords[i].y;

		Uint8 * pDst = m_pCanvas->m_pData + y * pitch + x * pixelBytes;

		if( y >= clip.y && y <= clip.y + clip.h -1 && x >= clip.x && x <= clip.x + clip.w -1 )
		{
		  const int alpha = colors[i].a;
		  const int invAlpha = 255-alpha;

		  pDst[0] = (Uint8) ((pDst[0]*invAlpha + (int)colors[i].b*alpha) >> 8);
		  pDst[1] = (Uint8) ((pDst[1]*invAlpha + (int)colors[i].g*alpha) >> 8);
		  pDst[2] = (Uint8) ((pDst[2]*invAlpha + (int)colors[i].r*alpha) >> 8);
		}
	}
}


//____ _drawHorrVertLine() ________________________________________________

void WgGfxDeviceSoft::_drawHorrVertLine( int _x, int _y, int _length, const WgColor& _col, WgOrientation orientation  )
{
	if( !m_pCanvas || !m_pCanvas->m_pData || _length <= 0  )
		return;
	
	WgColor fillColor = _col * m_tintColor;

	// Skip calls that won't affect destination

	if( fillColor.a == 0 && (m_blendMode == WG_BLENDMODE_BLEND || m_blendMode == WG_BLENDMODE_ADD) )
		return;

	// Optimize calls

	WgBlendMode blendMode = m_blendMode;
	if( blendMode == WG_BLENDMODE_BLEND && fillColor.a == 255 )
		blendMode = WG_BLENDMODE_OPAQUE;

	//

	int pitch = m_pCanvas->m_pitch;
	int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	Uint8 * pDst = m_pCanvas->m_pData + _y * m_pCanvas->m_pitch + _x * pixelBytes;

	int inc;

	if( orientation == WG_HORIZONTAL )
		inc = pixelBytes;
	else
		inc = pitch;

	//

	switch( blendMode )
	{
		case WG_BLENDMODE_OPAQUE:
		{
			for( int x = 0 ; x < _length*inc ; x+=inc )
			{
				pDst[x] = fillColor.b;
				pDst[x+1] = fillColor.g;
				pDst[x+2] = fillColor.r;
			}
		}
		break;
		case WG_BLENDMODE_BLEND:
		{
			int storedRed = ((int)fillColor.r) * fillColor.a;
			int storedGreen = ((int)fillColor.g) * fillColor.a;
			int storedBlue = ((int)fillColor.b) * fillColor.a;
			int invAlpha = 255-fillColor.a;

			for( int x = 0 ; x < _length*inc ; x+=inc )
			{
				pDst[x] = m_pDivTab[pDst[x]*invAlpha + storedBlue];
				pDst[x+1] = m_pDivTab[pDst[x+1]*invAlpha + storedGreen];
				pDst[x+2] = m_pDivTab[pDst[x+2]*invAlpha + storedRed];
			}

			break;
		}
		case WG_BLENDMODE_ADD:
		{
			int storedRed = m_pDivTab[fillColor.r * (int) fillColor.a];
			int storedGreen = m_pDivTab[fillColor.g * (int) fillColor.a];
			int storedBlue = m_pDivTab[fillColor.b * (int) fillColor.a];

			if( storedRed + storedGreen + storedBlue == 0 )
				return;

			for( int x = 0 ; x < _length*inc ; x+= inc )
			{
				pDst[x] = m_limitTable[pDst[x] + storedBlue];
				pDst[x+1] = m_limitTable[pDst[x+1] + storedGreen];
				pDst[x+2] = m_limitTable[pDst[x+2] + storedRed];
			}
			break;
		}
		case WG_BLENDMODE_MULTIPLY:
		{
			int storedRed = (int)fillColor.r;
			int storedGreen = (int)fillColor.g;
			int storedBlue = (int)fillColor.b;

			for( int x = 0 ; x < _length*inc ; x+= inc )
			{
				pDst[x] = m_pDivTab[pDst[x] * storedBlue];
				pDst[x+1] = m_pDivTab[pDst[x+1] * storedGreen];
				pDst[x+2] = m_pDivTab[pDst[x+2] * storedRed];
			}
			break;
		}
		case WG_BLENDMODE_INVERT:
		{
			int storedRed = (int)fillColor.r;
			int storedGreen = (int)fillColor.g;
			int storedBlue = (int)fillColor.b;

			int invertRed = 255 - (int)fillColor.r;
			int invertGreen = 255 - (int)fillColor.g;
			int invertBlue = 255 - (int)fillColor.b;


			for( int x = 0 ; x < _length*inc ; x+= inc )
			{
				pDst[x] = m_pDivTab[(255-pDst[x]) * storedBlue + pDst[x] * invertBlue];
				pDst[x+1] = m_pDivTab[(255-pDst[x+1]) * storedGreen + pDst[x+1] * invertGreen];
				pDst[x+2] = m_pDivTab[(255-pDst[x+2]) * storedRed + pDst[x+2] * invertRed];
			}
			break;
		}
		default:
			break;
	}
}

//____ _drawHorrVertLineAA() ________________________________________________

void WgGfxDeviceSoft::_drawHorrVertLineAA( int _x, int _y, int _length, const WgColor& _col, WgBlendMode blendMode, int _aa, WgOrientation orientation )
{
	int pitch = m_pCanvas->m_pitch;
	int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	Uint8 * pDst = m_pCanvas->m_pData + _y * m_pCanvas->m_pitch + _x * pixelBytes;
	
	int inc;
	if( orientation == WG_HORIZONTAL )
		inc = pixelBytes;
	else
		inc = pitch;

	switch( blendMode )
	{
		case WG_BLENDMODE_OPAQUE:
		{
			int storedRed = ((int)_col.r) * _aa;
			int storedGreen = ((int)_col.g) * _aa;
			int storedBlue = ((int)_col.b) * _aa;
			int invAlpha = 255- _aa;
			
			for( int x = 0 ; x < _length*inc ; x+= inc )
			{
				pDst[x] = m_pDivTab[pDst[x]*invAlpha + storedBlue];
				pDst[x+1] = m_pDivTab[pDst[x+1]*invAlpha + storedGreen];
				pDst[x+2] = m_pDivTab[pDst[x+2]*invAlpha + storedRed];
			}
			break;
		}
		case WG_BLENDMODE_BLEND:
		{
			int aa = m_pDivTab[_col.a * _aa];
			
			int storedRed = _col.r * aa;
			int storedGreen = _col.g * aa;
			int storedBlue = _col.b * aa;
			int invAlpha = 255-aa;
			
			for( int x = 0 ; x < _length*inc ; x+= inc )
			{
				pDst[x] = m_pDivTab[pDst[x]*invAlpha + storedBlue];
				pDst[x+1] = m_pDivTab[pDst[x+1]*invAlpha + storedGreen];
				pDst[x+2] = m_pDivTab[pDst[x+2]*invAlpha + storedRed];
			}
			break;
		}
		case WG_BLENDMODE_ADD:
		{
			int aa = m_pDivTab[_col.a * _aa];
			
			int storedRed = m_pDivTab[_col.r * aa];
			int storedGreen = m_pDivTab[_col.g * aa];
			int storedBlue = m_pDivTab[_col.b * aa];
			
			if( storedRed + storedGreen + storedBlue == 0 )
				return;
			
			for( int x = 0 ; x < _length*inc ; x+= inc )
			{
				pDst[x] = m_limitTable[pDst[x] + storedBlue];
				pDst[x+1] = m_limitTable[pDst[x+1] + storedGreen];
				pDst[x+2] = m_limitTable[pDst[x+2] + storedRed];
			}
			break;
		}
		case WG_BLENDMODE_MULTIPLY:
		{
			int storedRed = (int) m_pDivTab[_col.r*_aa];
			int storedGreen = (int) m_pDivTab[_col.g*_aa];
			int storedBlue = (int) m_pDivTab[_col.b*_aa];
			
			int invAlpha = 255 - _aa;
			
			for( int x = 0 ; x < _length*inc ; x+= inc )
			{
				pDst[x] = m_pDivTab[(pDst[x]*invAlpha) + (pDst[x] * storedBlue)];
				pDst[x+1] = m_pDivTab[(pDst[x+1]*invAlpha) + (pDst[x+1] * storedGreen)];
				pDst[x+2] = m_pDivTab[(pDst[x+2]*invAlpha) + (pDst[x+2] * storedRed)];
			}
			break;
		}
		case WG_BLENDMODE_INVERT:
		{
			//TODO: Translate to use m_pDivTab

			int storedRed = (int)_col.r;
			int storedGreen = (int)_col.g;
			int storedBlue = (int)_col.b;
			
			int invertRed = 255 - (int)_col.r;
			int invertGreen = 255 - (int)_col.g;
			int invertBlue = 255 - (int)_col.b;
			
			int invAlpha = (255 - _aa) << 8;
			
			for( int x = 0 ; x < _length*inc ; x+= inc )
			{
				pDst[x] = ( (pDst[x]*invAlpha) + _aa * ((255-pDst[x]) * storedBlue + pDst[x] * invertBlue) ) >> 16;
				pDst[x+1] = ( (pDst[x+1]*invAlpha) + _aa * ((255-pDst[x+1]) * storedGreen + pDst[x+1] * invertGreen) )  >> 16;
				pDst[x+2] = ( (pDst[x+2]*invAlpha) + _aa * ((255-pDst[x+2]) * storedRed + pDst[x+2] * invertRed) ) >> 16;
			}
			break;
		}
		default:
			break;
	}	
}

//____ _plotAA() ________________________________________________

void WgGfxDeviceSoft::_plotAA( int _x, int _y, const WgColor& _col, WgBlendMode blendMode, int _aa )
{
	//TODO: Translate to use m_pDivTab

	int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	Uint8 * pDst = m_pCanvas->m_pData + _y * m_pCanvas->m_pitch + _x * pixelBytes;
	
	switch( blendMode )
	{
		case WG_BLENDMODE_OPAQUE:
		{
			int storedRed = ((int)_col.r) * _aa;
			int storedGreen = ((int)_col.g) * _aa;
			int storedBlue = ((int)_col.b) * _aa;
			int invAlpha = 255- _aa;
			
			pDst[0] = (Uint8) ((pDst[0]*invAlpha + storedBlue) >> 8);
			pDst[1] = (Uint8) ((pDst[1]*invAlpha + storedGreen) >> 8);
			pDst[2] = (Uint8) ((pDst[2]*invAlpha + storedRed) >> 8);
			break;
		}
		case WG_BLENDMODE_BLEND:
		{
			int aa = _col.a * _aa;
			
			int storedRed = (((int)_col.r) * aa) >> 8;
			int storedGreen = (((int)_col.g) * aa) >> 8;
			int storedBlue = (((int)_col.b) * aa) >> 8;
			int invAlpha = 255-(aa>>8);
			
			pDst[0] = (Uint8) ((pDst[0]*invAlpha + storedBlue) >> 8);
			pDst[1] = (Uint8) ((pDst[1]*invAlpha + storedGreen) >> 8);
			pDst[2] = (Uint8) ((pDst[2]*invAlpha + storedRed) >> 8);
			break;
		}
		case WG_BLENDMODE_ADD:
		{
			int aa = _col.a * _aa;
			
			int storedRed = (((int)_col.r) * aa) >> 16;
			int storedGreen = (((int)_col.g) * aa) >> 16;
			int storedBlue = (((int)_col.b) * aa) >> 16;
			
			if( storedRed + storedGreen + storedBlue == 0 )
				return;
			
			pDst[0] = m_limitTable[pDst[0] + storedBlue];
			pDst[1] = m_limitTable[pDst[1] + storedGreen];
			pDst[2] = m_limitTable[pDst[2] + storedRed];
			break;
		}
		case WG_BLENDMODE_MULTIPLY:
		{
			int storedRed = (int)_col.r;
			int storedGreen = (int)_col.g;
			int storedBlue = (int)_col.b;
			
			int invAlpha = (255 - _aa) << 8;
			
			pDst[0] = ( (pDst[0]*invAlpha) + (_aa * pDst[0] * storedBlue) ) >> 16;
			pDst[1] = ( (pDst[1]*invAlpha) + (_aa * pDst[1] * storedGreen) ) >> 16;
			pDst[2] = ( (pDst[2]*invAlpha) + (_aa * pDst[2] * storedRed) ) >> 16;
			break;
		}
		case WG_BLENDMODE_INVERT:
		{
			int storedRed = (int)_col.r;
			int storedGreen = (int)_col.g;
			int storedBlue = (int)_col.b;
			
			int invertRed = 255 - (int)_col.r;
			int invertGreen = 255 - (int)_col.g;
			int invertBlue = 255 - (int)_col.b;
			
			int invAlpha = (255 - _aa) << 8;
			
			pDst[0] = ( (pDst[0]*invAlpha) + _aa * ((255-pDst[0]) * storedBlue + pDst[0] * invertBlue) ) >> 16;
			pDst[1] = ( (pDst[1]*invAlpha) + _aa * ((255-pDst[1]) * storedGreen + pDst[1] * invertGreen) )  >> 16;
			pDst[2] = ( (pDst[2]*invAlpha) + _aa * ((255-pDst[2]) * storedRed + pDst[2] * invertRed) ) >> 16;
			break;
		}
		default:
			break;
	}	
}



//____ DrawElipse() _______________________________________________________________

void WgGfxDeviceSoft::DrawElipse( const WgRect& rect, WgColor color )
{
	WgColor fillColor = color * m_tintColor;

	// Skip calls that won't affect destination

	if( fillColor.a == 0 && (m_blendMode == WG_BLENDMODE_BLEND || m_blendMode == WG_BLENDMODE_ADD) )
		return;

	if( !m_pCanvas || !m_pCanvas->m_pData || rect.h < 2 || rect.w < 1 )
		return;

	int sectionHeight = rect.h/2;
	int maxWidth = rect.w/2;

	Uint8 * pLineBeg = m_pCanvas->m_pData + rect.y * m_pCanvas->m_pitch;
	int pitch = m_pCanvas->m_pitch;

	int center = (rect.x + rect.w/2) << 8;

	int sinOfsInc = (NB_CURVETAB_ENTRIES << 16) / sectionHeight;
	int sinOfs = 0;

	int begOfs = 0;
	int peakOfs = 0;
	int endOfs = 0;

	for( int i = 0 ; i < sectionHeight ; i++ )
	{
		peakOfs = ((s_pCurveTab[sinOfs>>16] * maxWidth) >> 8);
		endOfs = (s_pCurveTab[(sinOfs+(sinOfsInc-1))>>16] * maxWidth) >> 8;

		_drawHorrFadeLine( pLineBeg + i*pitch, center + begOfs -256, center + peakOfs -256, center + endOfs, fillColor );
		_drawHorrFadeLine( pLineBeg + i*pitch, center - endOfs, center - peakOfs, center - begOfs +256, fillColor );

		_drawHorrFadeLine( pLineBeg + (sectionHeight*2-i-1)*pitch, center + begOfs -256, center + peakOfs -256, center + endOfs, fillColor );
		_drawHorrFadeLine( pLineBeg + (sectionHeight*2-i-1)*pitch, center - endOfs, center - peakOfs, center - begOfs +256, fillColor );

		begOfs = peakOfs;
		sinOfs += sinOfsInc;
	}
}

//____ _clipDrawHorrFadeLine() _______________________________________________________________

void WgGfxDeviceSoft::_clipDrawHorrFadeLine( int clipX1, int clipX2, Uint8 * pLineStart, int begOfs, int peakOfs, int endOfs, WgColor color )
{
	//TODO: Translate to use m_pDivTab

	int pitch = m_pCanvas->m_pitch;
	int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	Uint8 * p = pLineStart + (begOfs>>8) * pixelBytes;
	Uint8 * pClip1 = pLineStart + clipX1*pixelBytes;
	Uint8 * pClip2 = pLineStart + clipX2*pixelBytes;

	int alphaInc, alpha, len;

	if( (peakOfs>>8) == (begOfs>>8) )
	{
		alphaInc = 0;
		alpha = (256-(peakOfs&0xff) + (peakOfs-begOfs)/2) << 14;
		len = 1;
	}
	else
	{
		alphaInc = (255 << 22) / (peakOfs - begOfs);			// alpha inc per pixel with 14 binals.
		alpha = ((256 - (begOfs&0xff)) * alphaInc) >> 9;		// alpha for ramp up start pixel with 14 binals.
		len = ((peakOfs+256) >> 8) - (begOfs >> 8);
	}

	for( int i = 0 ; i < len ; i++ )
	{
		if( p >= pClip1 && p < pClip2 )
		{
			int invAlpha = (255 << 14) - alpha;

			p[0] = ((color.b * alpha) + (p[0]*invAlpha)) >> 22;
			p[1] = ((color.g * alpha) + (p[1]*invAlpha)) >> 22;
			p[2] = ((color.r * alpha) + (p[2]*invAlpha)) >> 22;
		}
		alpha += alphaInc;
		if( alpha > 255 << 14 )
			alpha = 255 << 14;

		p += pixelBytes;
	}

	if( (endOfs>>8) == ((peakOfs + 256)>>8) )
	{
		alphaInc = 0;
		alpha = ((peakOfs&0xff)+(endOfs-peakOfs-256)/2) << 14;
		len = 1;
	}
	else
	{
		alphaInc = (255 << 22) / (endOfs - (peakOfs+256));						// alpha dec per pixel with 14 binals.
		alpha = (255 << 14) - (((256 - (peakOfs&0xff)) * alphaInc) >> 9);	// alpha for ramp down start pixel with 14 binals.
		len = (endOfs >> 8) - ((peakOfs+256) >> 8);
		alphaInc = -alphaInc;
	}

	for( int i = 0 ; i < len ; i++ )
	{
		if( p >= pClip1 && p < pClip2 )
		{
			int invAlpha = (255 << 14) - alpha;

			p[0] = ((color.b * alpha) + (p[0]*invAlpha)) >> 22;
			p[1] = ((color.g * alpha) + (p[1]*invAlpha)) >> 22;
			p[2] = ((color.r * alpha) + (p[2]*invAlpha)) >> 22;
		}
		alpha += alphaInc;
		p += pixelBytes;
	}
}


//____ _drawHorrFadeLine() _______________________________________________________________

void WgGfxDeviceSoft::_drawHorrFadeLine( Uint8 * pLineStart, int begOfs, int peakOfs, int endOfs, WgColor color )
{
	//TODO: Translate to use m_pDivTab
    
    float colorAlpha = (float)color.a*(1.0f/255.0f);

	int pitch = m_pCanvas->m_pitch;
	int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;
	Uint8 * p = pLineStart + (begOfs>>8) * pixelBytes;

	int alphaInc, alpha, len;

	if( (peakOfs>>8) == (begOfs>>8) )
	{
		alphaInc = 0;
		alpha = (256-(peakOfs&0xff) + (peakOfs-begOfs)/2) << 14;
		len = 1;
	}
	else
	{
		alphaInc = (255 << 22) / (peakOfs - begOfs);			// alpha inc per pixel with 14 binals.
		alpha = ((256 - (begOfs&0xff)) * alphaInc) >> 9;		// alpha for ramp up start pixel with 14 binals.

		
        len = ((peakOfs+256) >> 8) - (begOfs >> 8);
	}

	for( int i = 0 ; i < len ; i++ )
	{
        // Add alpha from color
        int alpha2 = (int)wg_round((float)alpha * colorAlpha);

		int invAlpha = (255 << 14) - alpha2;

        if(alpha2 >= 0)
        {
            p[0] = ((color.b * alpha2) + (p[0]*invAlpha)) >> 22;
            p[1] = ((color.g * alpha2) + (p[1]*invAlpha)) >> 22;
            p[2] = ((color.r * alpha2) + (p[2]*invAlpha)) >> 22;
        }
        
        
		alpha += alphaInc;
		if( alpha > 255 << 14 )
			alpha = 255 << 14;

		p += pixelBytes;
	}

	if( (endOfs>>8) == ((peakOfs + 256)>>8) )
	{
		alphaInc = 0;
		alpha = ((peakOfs&0xff)+(endOfs-peakOfs-256)/2) << 14;
		len = 1;
	}
	else
	{
		alphaInc = (255 << 22) / (endOfs - (peakOfs+256));						// alpha dec per pixel with 14 binals.
		alpha = (255 << 14) - (((256 - (peakOfs&0xff)) * alphaInc) >> 9);	// alpha for ramp down start pixel with 14 binals.
		len = (endOfs >> 8) - ((peakOfs+256) >> 8);
		alphaInc = -alphaInc;
	}

	for( int i = 0 ; i < len ; i++ )
	{
        // Add alpha from color
//        alpha = (alpha * color.a) >> 8;
        int alpha2 = (int)wg_round((float)alpha * colorAlpha);

		int invAlpha = (255 << 14) - alpha2;

        if(alpha2 >= 0)
        {
            p[0] = ((color.b * alpha2) + (p[0]*invAlpha)) >> 22;
            p[1] = ((color.g * alpha2) + (p[1]*invAlpha)) >> 22;
            p[2] = ((color.r * alpha2) + (p[2]*invAlpha)) >> 22;
		}
        
        alpha += alphaInc;
		p += pixelBytes;
	}
	
}

//____ ClipDrawElipse() _______________________________________________________________

void WgGfxDeviceSoft::ClipDrawElipse( const WgRect& clip, const WgRect& rect, WgColor color )
{
	WgColor fillColor = color * m_tintColor;

	// Skip calls that won't affect destination

	if( fillColor.a == 0 && (m_blendMode == WG_BLENDMODE_BLEND || m_blendMode == WG_BLENDMODE_ADD) )
		return;

	if( !m_pCanvas || !m_pCanvas->m_pData || rect.h < 2 || rect.w < 1 )
		return;

	if( !rect.IntersectsWith(clip) )
		return;

	if( clip.Contains(rect) )
		return DrawElipse(rect,color);

	int sectionHeight = rect.h/2;
	int maxWidth = rect.w/2;

	Uint8 * pLineBeg = m_pCanvas->m_pData + rect.y*m_pCanvas->m_pitch;
	int pitch = m_pCanvas->m_pitch;

	int center = (rect.x + rect.w/2) << 8;

	int sinOfsInc = (NB_CURVETAB_ENTRIES << 16) / sectionHeight;
	int sinOfs = 0;

	int begOfs = 0;
	int peakOfs = 0;
	int endOfs = 0;

	for( int i = 0 ; i < sectionHeight ; i++ )
	{
		peakOfs = ((s_pCurveTab[sinOfs>>16] * maxWidth) >> 8);
		endOfs = (s_pCurveTab[(sinOfs+(sinOfsInc-1))>>16] * maxWidth) >> 8;

		if( rect.y + i >= clip.y && rect.y + i < clip.y + clip.h ) 
		{		
			_clipDrawHorrFadeLine( clip.x, clip.x+clip.w, pLineBeg + i*pitch, center + begOfs -256, center + peakOfs -256, center + endOfs, fillColor );
			_clipDrawHorrFadeLine( clip.x, clip.x+clip.w, pLineBeg + i*pitch, center - endOfs, center - peakOfs, center - begOfs +256, fillColor );
		}

		int y2 = sectionHeight*2-i-1;
		if( rect.y + y2 >= clip.y && rect.y + y2 < clip.y + clip.h ) 
		{
			_clipDrawHorrFadeLine( clip.x, clip.x+clip.w, pLineBeg + y2*pitch, center + begOfs -256, center + peakOfs -256, center + endOfs, fillColor );
			_clipDrawHorrFadeLine( clip.x, clip.x+clip.w, pLineBeg + y2*pitch, center - endOfs, center - peakOfs, center - begOfs +256, fillColor );
		}

		begOfs = peakOfs;
		sinOfs += sinOfsInc;
	}
}

//____ DrawFilledElipse() _____________________________________________________

void WgGfxDeviceSoft::DrawFilledElipse( const WgRect& rect, WgColor color )
{
	int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;

	Uint8 * pLineCenter = m_pCanvas->m_pData + rect.y * m_pCanvas->m_pitch + (rect.x+rect.w/2) * pixelBytes;

	int sinOfsInc = (NB_CURVETAB_ENTRIES << 16) / (rect.h/2);
	int sinOfs = sinOfsInc >> 1;

	for( int j = 0 ; j < 2 ; j++ )
	{
		for( int i = 0 ; i < rect.h/2 ; i++ )
		{
			int lineLen = ((s_pCurveTab[sinOfs>>16] * rect.w/2 + 32768)>>16)*pixelBytes;
			Uint8 * pLineBeg = pLineCenter - lineLen;
			Uint8 * pLineEnd = pLineCenter + lineLen;

			for( Uint8 * p = pLineBeg ; p < pLineEnd ; p += pixelBytes )
			{
				p[0] = color.b;
				p[1] = color.g;
				p[2] = color.r;
			}

			sinOfs += sinOfsInc;
			pLineCenter += m_pCanvas->m_pitch;
		}
		sinOfsInc = -sinOfsInc;
		sinOfs = (NB_CURVETAB_ENTRIES << 16) + (sinOfsInc >> 1);
	}
}

//____ ClipDrawFilledElipse() _____________________________________________________

void WgGfxDeviceSoft::ClipDrawFilledElipse( const WgRect& clip, const WgRect& rect, WgColor color )
{
	if( !rect.IntersectsWith(clip) )
		return;

	if( clip.Contains(rect) )
		return DrawFilledElipse(rect,color);

	int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;

	Uint8 * pLine = m_pCanvas->m_pData + rect.y * m_pCanvas->m_pitch;

	int sinOfsInc = (NB_CURVETAB_ENTRIES << 16) / (rect.h/2);
	int sinOfs = sinOfsInc >> 1;

	for( int j = 0 ; j < 2 ; j++ )
	{
		for( int i = 0 ; i < rect.h/2 ; i++ )
		{
			if( rect.y + j*(rect.h/2) + i >= clip.y && rect.y + j*(rect.h/2) + i < clip.y + clip.h )
			{
				int lineLen = ((s_pCurveTab[sinOfs>>16] * rect.w/2 + 32768)>>16);
	
				int beg = rect.x + rect.w/2 - lineLen;
				int end = rect.x + rect.w/2 + lineLen;

				if( beg < clip.x )
					beg = clip.x;

				if( end > clip.x + clip.w )
					end = clip.x + clip.w;

				if( beg < end )
				{
					Uint8 * pLineBeg = pLine + beg * pixelBytes;	
					Uint8 * pLineEnd = pLine + end * pixelBytes;			

					for( Uint8 * p = pLineBeg ; p < pLineEnd ; p += pixelBytes )
					{
						p[0] = color.b;
						p[1] = color.g;
						p[2] = color.r;
					}
				}
			}

			sinOfs += sinOfsInc;
			pLine += m_pCanvas->m_pitch;
		}

		sinOfsInc = -sinOfsInc;
		sinOfs = (NB_CURVETAB_ENTRIES << 16) + (sinOfsInc >> 1);
	}
}


//____ DrawArcNE() ____________________________________________________________

void WgGfxDeviceSoft::DrawArcNE( const WgRect& rect, WgColor color )
{
	int pixelBytes = m_pCanvas->m_pixelFormat.bits/8;

	Uint8 * pLineBeg = m_pCanvas->m_pData + rect.y * m_pCanvas->m_pitch + rect.x * pixelBytes;

	int sinOfsInc = (NB_CURVETAB_ENTRIES << 16) / rect.h;
	int sinOfs = sinOfsInc >> 1;

	for( int i = 0 ; i < rect.h ; i++ )
	{
		Uint8 * pLineEnd = pLineBeg + ((s_pCurveTab[sinOfs>>16] * rect.w + 32768)>>16)*pixelBytes;

		for( Uint8 * p = pLineBeg ; p < pLineEnd ; p += pixelBytes )
		{
			p[0] = color.b;
			p[1] = color.g;
			p[2] = color.r;
		}

		sinOfs += sinOfsInc;
		pLineBeg += m_pCanvas->m_pitch;
	}

}

//____ ClipDrawArcNE() _________________________________________________________

void WgGfxDeviceSoft::ClipDrawArcNE( const WgRect& clip, const WgRect& rect, WgColor color )
{
	//TODO: Implement!!!
}


//____ Blit() __________________________________________________________________

void WgGfxDeviceSoft::Blit( const WgSurface* pSrcSurf, const WgRect& srcrect, int dx, int dy  )
{
	if( m_tintColor.argb == 0xFFFFFFFF )
		_blit( pSrcSurf, srcrect, dx, dy );
	else
		_tintBlit( pSrcSurf, srcrect, dx, dy );
}

//____ _blit() _____________________________________________________________

void WgGfxDeviceSoft::_blit( const WgSurface* _pSrcSurf, const WgRect& srcrect, int dx, int dy  )
{
	if( !_pSrcSurf || !m_pCanvas || _pSrcSurf->Type() != WgSurfaceSoft::GetClass() )
		return;

	WgSurfaceSoft * pSrcSurf = (WgSurfaceSoft*) _pSrcSurf;

	if( !m_pCanvas->m_pData || !pSrcSurf->m_pData )
		return;

	int srcPixelBytes = pSrcSurf->m_pixelFormat.bits/8;
	int dstPixelBytes = m_pCanvas->m_pixelFormat.bits/8;

	int	srcPitchAdd = pSrcSurf->m_pitch - srcrect.w*srcPixelBytes;
	int	dstPitchAdd = m_pCanvas->m_pitch - srcrect.w*dstPixelBytes;

	Uint8 * pDst = m_pCanvas->m_pData + dy * m_pCanvas->m_pitch + dx * dstPixelBytes;
	Uint8 * pSrc = pSrcSurf->m_pData + srcrect.y * pSrcSurf->m_pitch + srcrect.x * srcPixelBytes;

	WgBlendMode		blendMode = m_blendMode;
	if( srcPixelBytes == 3 && blendMode == WG_BLENDMODE_BLEND )
		blendMode = WG_BLENDMODE_OPAQUE;

	switch( blendMode )
	{
		case WG_BLENDMODE_OPAQUE:
		{
			if( srcPixelBytes == 4 && dstPixelBytes == 4 )
			{
				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						* ((Uint32*)pDst) = * ((Uint32*)pSrc) & 0x00FFFFFF;
						pSrc += 4;
						pDst += 4;
					}

					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
			}
			else
			{
				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						pDst[0] = pSrc[0];
						pDst[1] = pSrc[1];
						pDst[2] = pSrc[2];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
			}

			break;
		}
		case WG_BLENDMODE_BLEND:
		{
			if( srcPixelBytes == 4 )
			{
				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						int alpha = pSrc[3];
						int invAlpha = 255-alpha;

						pDst[0] = m_pDivTab[pDst[0]*invAlpha + pSrc[0]*alpha];
						pDst[1] = m_pDivTab[pDst[1]*invAlpha + pSrc[1]*alpha];
						pDst[2] = m_pDivTab[pDst[2]*invAlpha + pSrc[2]*alpha];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
			}
			else
			{
				// Should never get here, skips to blendmode opaque instead.
			}
			break;
		}
		case WG_BLENDMODE_ADD:
		{
			if( srcPixelBytes == 4 )
			{
				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						int alpha = pSrc[3];

						pDst[0] = m_limitTable[pDst[0] + (int) m_pDivTab[pSrc[0]*alpha] ];
						pDst[1] = m_limitTable[pDst[1] + (int) m_pDivTab[pSrc[1]*alpha] ];
						pDst[2] = m_limitTable[pDst[2] + (int) m_pDivTab[pSrc[2]*alpha] ];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
			}
			else
			{
				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						pDst[0] = m_limitTable[pDst[0] + pSrc[0]];
						pDst[1] = m_limitTable[pDst[1] + pSrc[1]];
						pDst[2] = m_limitTable[pDst[2] + pSrc[2]];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
			}
			break;
		}
		case WG_BLENDMODE_MULTIPLY:
		{
			for( int y = 0 ; y < srcrect.h ; y++ )
			{
				for( int x = 0 ; x < srcrect.w ; x++ )
				{
					pDst[0] = m_pDivTab[ pDst[0]*pSrc[0] ];
					pDst[1] = m_pDivTab[ pDst[1]*pSrc[1] ];
					pDst[2] = m_pDivTab[ pDst[2]*pSrc[2] ];
					pSrc += srcPixelBytes;
					pDst += dstPixelBytes;
				}
				pSrc += srcPitchAdd;
				pDst += dstPitchAdd;
			}
			break;
		}
		case WG_BLENDMODE_INVERT:
		{
			for( int y = 0 ; y < srcrect.h ; y++ )
			{
				for( int x = 0 ; x < srcrect.w ; x++ )
				{
					pDst[0] = m_pDivTab[pSrc[0]*(255-pDst[0]) + pDst[0]*(255-pSrc[0])];
					pDst[1] = m_pDivTab[pSrc[1]*(255-pDst[1]) + pDst[1]*(255-pSrc[1])];
					pDst[2] = m_pDivTab[pSrc[2]*(255-pDst[2]) + pDst[2]*(255-pSrc[2])];
					pSrc += srcPixelBytes;
					pDst += dstPixelBytes;
				}
				pSrc += srcPitchAdd;
				pDst += dstPitchAdd;
			}
			break;
		}
		default:
			break;
	}
}



//____ _tintBlit() _____________________________________________________________

void WgGfxDeviceSoft::_tintBlit( const WgSurface* _pSrcSurf, const WgRect& srcrect, int dx, int dy  )
{
	if( !_pSrcSurf || !m_pCanvas || _pSrcSurf->Type() != WgSurfaceSoft::GetClass() )
		return;

	WgSurfaceSoft * pSrcSurf = (WgSurfaceSoft*) _pSrcSurf;

	if( !m_pCanvas->m_pData || !pSrcSurf->m_pData )
		return;

	int srcPixelBytes = pSrcSurf->m_pixelFormat.bits/8;
	int dstPixelBytes = m_pCanvas->m_pixelFormat.bits/8;

	int	srcPitchAdd = pSrcSurf->m_pitch - srcrect.w*srcPixelBytes;
	int	dstPitchAdd = m_pCanvas->m_pitch - srcrect.w*dstPixelBytes;

	Uint8 * pDst = m_pCanvas->m_pData + dy * m_pCanvas->m_pitch + dx * dstPixelBytes;
	Uint8 * pSrc = pSrcSurf->m_pData + srcrect.y * pSrcSurf->m_pitch + srcrect.x * srcPixelBytes;

	WgBlendMode		blendMode = m_blendMode;
	if( srcPixelBytes == 3 && blendMode == WG_BLENDMODE_BLEND && m_tintColor.a == 255 )
		blendMode = WG_BLENDMODE_OPAQUE;

	switch( blendMode )
	{
		case WG_BLENDMODE_OPAQUE:
		{
			int tintRed = (int) m_tintColor.r;
			int tintGreen = (int) m_tintColor.g;
			int tintBlue = (int) m_tintColor.b;

			for( int y = 0 ; y < srcrect.h ; y++ )
			{
				for( int x = 0 ; x < srcrect.w ; x++ )
				{
					pDst[0] = m_pDivTab[pSrc[0]*tintBlue];
					pDst[1] = m_pDivTab[pSrc[1]*tintGreen];
					pDst[2] = m_pDivTab[pSrc[2]*tintRed];
					pSrc += srcPixelBytes;
					pDst += dstPixelBytes;
				}
				pSrc += srcPitchAdd;
				pDst += dstPitchAdd;
			}
			break;
		}
		case WG_BLENDMODE_BLEND:
		{
            if( m_tintColor.a == 0 )
                break;
            
			if( srcPixelBytes == 4 )
			{
				int tintAlpha = (int) m_tintColor.a;
				int tintRed = (int) m_tintColor.r;
				int tintGreen = (int) m_tintColor.g;
				int tintBlue = (int) m_tintColor.b;

				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						int alpha = m_pDivTab[pSrc[3]*tintAlpha];
						int invAlpha = 255-alpha;

						int srcBlue		= m_pDivTab[pSrc[0] * tintBlue];
						int srcGreen	= m_pDivTab[pSrc[1] * tintGreen];
						int srcRed		= m_pDivTab[pSrc[2] * tintRed];
						

						pDst[0] = m_pDivTab[ pDst[0]*invAlpha + srcBlue*alpha ];
						pDst[1] = m_pDivTab[ pDst[1]*invAlpha + srcGreen*alpha ];
						pDst[2] = m_pDivTab[ pDst[2]*invAlpha + srcRed*alpha ];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
			}
			else
			{
				int tintAlpha = (int) m_tintColor.a;
				int tintRed = (int) m_pDivTab[ m_tintColor.r * tintAlpha ];
				int tintGreen = (int) m_pDivTab[ m_tintColor.g * tintAlpha ];
				int tintBlue = (int) m_pDivTab[ m_tintColor.b * tintAlpha ];
				int invAlpha = 255-tintAlpha;

				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						pDst[0] = m_pDivTab[ pDst[0]*invAlpha + pSrc[0]*tintBlue ];
						pDst[1] = m_pDivTab[ pDst[1]*invAlpha + pSrc[1]*tintGreen ];
						pDst[2] = m_pDivTab[ pDst[2]*invAlpha + pSrc[2]*tintRed ];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
			}
			break;
		}
		case WG_BLENDMODE_ADD:
		{
            if( m_tintColor.a == 0 )
                break;

            if( srcPixelBytes == 4 )
			{
				int tintAlpha = (int) m_tintColor.a;
				int tintRed = (int) m_tintColor.r;
				int tintGreen = (int) m_tintColor.g;
				int tintBlue = (int) m_tintColor.b;

				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						int alpha = m_pDivTab[ pSrc[3]*tintAlpha ];

						int srcBlue		= m_pDivTab[pSrc[0] * tintBlue];
						int srcGreen	= m_pDivTab[pSrc[1] * tintGreen];
						int srcRed		= m_pDivTab[pSrc[2] * tintRed];

						pDst[0] = m_limitTable[pDst[0] + (int) m_pDivTab[srcBlue*alpha] ];
						pDst[1] = m_limitTable[pDst[1] + (int) m_pDivTab[srcGreen*alpha] ];
						pDst[2] = m_limitTable[pDst[2] + (int) m_pDivTab[ srcRed*alpha] ];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
			}
			else
			{
				int tintAlpha = (int) m_tintColor.a;
				int tintRed = (int) m_pDivTab[m_tintColor.r * tintAlpha];
				int tintGreen = (int) m_pDivTab[m_tintColor.g * tintAlpha];
				int tintBlue = (int) m_pDivTab[m_tintColor.b * tintAlpha];

				for( int y = 0 ; y < srcrect.h ; y++ )
				{
					for( int x = 0 ; x < srcrect.w ; x++ )
					{
						pDst[0] = m_limitTable[pDst[0] + (int) m_pDivTab[pSrc[0]*tintBlue] ];
						pDst[1] = m_limitTable[pDst[1] + (int) m_pDivTab[pSrc[1]*tintGreen] ];
						pDst[2] = m_limitTable[pDst[2] + (int) m_pDivTab[pSrc[2]*tintRed] ];
						pSrc += srcPixelBytes;
						pDst += dstPixelBytes;
					}
					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
			}
			break;
		}
		case WG_BLENDMODE_MULTIPLY:
		{
			int tintRed = (int) m_tintColor.r;
			int tintGreen = (int) m_tintColor.g;
			int tintBlue = (int) m_tintColor.b;

			for( int y = 0 ; y < srcrect.h ; y++ )
			{
				for( int x = 0 ; x < srcrect.w ; x++ )
				{
					int srcBlue		= m_pDivTab[pSrc[0] * tintBlue];
					int srcGreen	= m_pDivTab[pSrc[1] * tintGreen];
					int srcRed		= m_pDivTab[pSrc[2] * tintRed];


					pDst[0] = m_pDivTab[srcBlue*pDst[0]];
					pDst[1] = m_pDivTab[srcGreen*pDst[1]];
					pDst[2] = m_pDivTab[srcRed*pDst[2]];
					pSrc += srcPixelBytes;
					pDst += dstPixelBytes;
				}
				pSrc += srcPitchAdd;
				pDst += dstPitchAdd;
			}
			break;
		}
		case WG_BLENDMODE_INVERT:
		{
			int tintRed = (int) m_tintColor.r;
			int tintGreen = (int) m_tintColor.g;
			int tintBlue = (int) m_tintColor.b;

			for( int y = 0 ; y < srcrect.h ; y++ )
			{
				for( int x = 0 ; x < srcrect.w ; x++ )
				{
					int srcBlue = m_pDivTab[tintBlue*pSrc[0]];
					int srcGreen = m_pDivTab[tintGreen*pSrc[1]];
					int srcRed = m_pDivTab[tintRed*pSrc[2]];

					pDst[0] = m_pDivTab[srcBlue*(255-pDst[0]) + pDst[0]*(255-srcBlue)];
					pDst[1] = m_pDivTab[srcGreen*(255-pDst[1]) + pDst[1]*(255-srcGreen)];
					pDst[2] = m_pDivTab[srcRed*(255-pDst[2]) + pDst[2]*(255-srcRed)];
					pSrc += srcPixelBytes;
					pDst += dstPixelBytes;
				}
				pSrc += srcPitchAdd;
				pDst += dstPitchAdd;
			}
			break;
		}
		default:
			break;
	}
}


//____ StretchBlitSubPixel() ___________________________________________________

void WgGfxDeviceSoft::StretchBlitSubPixel( const WgSurface * _pSrcSurf, float sx, float sy, float sw, float sh,
						   		 float _dx, float _dy, float _dw, float _dh, bool bTriLinear, float mipBias )
{
	if( !_pSrcSurf || !m_pCanvas || _pSrcSurf->Type() != WgSurfaceSoft::GetClass() )
		return;

	WgSurfaceSoft * pSrcSurf = (WgSurfaceSoft*) _pSrcSurf;

	if( !m_pCanvas->m_pData || !pSrcSurf->m_pData )
		return;

	int dx = (int) _dx;
	int dy = (int) _dy;
	int dw = (int) _dw;
	int dh = (int) _dh;

	WgBlendMode		blendMode = m_blendMode;
	if( pSrcSurf->m_pixelFormat.bits == 24 && blendMode == WG_BLENDMODE_BLEND && m_tintColor.a == 255 )
		blendMode = WG_BLENDMODE_OPAQUE;

	if( m_tintColor == 0xFFFFFFFF )
	{
		switch( blendMode )
		{
			case WG_BLENDMODE_OPAQUE:
				_stretchBlitOpaque( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				break;
			case WG_BLENDMODE_BLEND:
				if( pSrcSurf->m_pixelFormat.bits == 24 )
					assert(0);							// SHOULD NEVER GET HERE!
				else
					_stretchBlitBlend32( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				break;
			case WG_BLENDMODE_ADD:
				if( pSrcSurf->m_pixelFormat.bits == 24 )
					_stretchBlitAdd24( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				else
					_stretchBlitAdd32( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				break;
			case WG_BLENDMODE_MULTIPLY:
				_stretchBlitMultiply( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				break;
			case WG_BLENDMODE_INVERT:
				_stretchBlitInvert( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				break;
		}
	}
	else
	{
		switch( blendMode )
		{
			case WG_BLENDMODE_OPAQUE:
				_stretchBlitTintedOpaque( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				break;
			case WG_BLENDMODE_BLEND:
				if( pSrcSurf->m_pixelFormat.bits == 24 )
					_stretchBlitTintedBlend24( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				else
					_stretchBlitTintedBlend32( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				break;
			case WG_BLENDMODE_ADD:
				if( pSrcSurf->m_pixelFormat.bits == 24 )
					_stretchBlitTintedAdd24( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				else
					_stretchBlitTintedAdd32( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				break;
			case WG_BLENDMODE_MULTIPLY:
				_stretchBlitTintedMultiply( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				break;
			case WG_BLENDMODE_INVERT:
				_stretchBlitTintedInvert( pSrcSurf, sx, sy, sw, sh, dx, dy, dw, dh );
				break;
		}
	}
}


#define STRETCHBLIT( _bReadAlpha_, _init_, _loop_ )										\
{																						\
	int srcPixelBytes = pSrcSurf->m_pixelFormat.bits/8;									\
	int dstPixelBytes = m_pCanvas->m_pixelFormat.bits/8;								\
																						\
	int	srcPitch = pSrcSurf->m_pitch;													\
	int	dstPitch = m_pCanvas->m_pitch;													\
																						\
	_init_																				\
																						\
	if( pSrcSurf->scaleMode() == WG_SCALEMODE_INTERPOLATE )															\
	{																					\
		int ofsY = (int) (sy*32768);		/* We use 15 binals for all calculations */	\
		int incY = (int) (sh*32768/dh);													\
																						\
		for( int y = 0 ; y < dh ; y++ )													\
		{																				\
			int fracY2 = ofsY & 0x7FFF;													\
			int fracY1 = 32768 - fracY2;												\
																						\
			int ofsX = (int) (sx*32768);												\
			int incX = (int) (sw*32768/dw);												\
																						\
			Uint8 * pDst = m_pCanvas->m_pData + (dy+y) * m_pCanvas->m_pitch + dx * dstPixelBytes;	\
			Uint8 * pSrc = pSrcSurf->m_pData + (ofsY>>15) * pSrcSurf->m_pitch;						\
																						\
			for( int x = 0 ; x < dw ; x++ )												\
			{																			\
				int fracX2 = ofsX & 0x7FFF;												\
				int fracX1 = 32768 - fracX2;											\
																						\
				Uint8 * p = pSrc + (ofsX >> 15)*srcPixelBytes;							\
																						\
				int mul11 = fracX1*fracY1 >> 15;										\
				int mul12 = fracX2*fracY1 >> 15;										\
				int mul21 = fracX1*fracY2 >> 15;										\
				int mul22 = fracX2*fracY2 >> 15;										\
																						\
				int srcBlue = (p[0]*mul11 + p[srcPixelBytes]*mul12 + p[srcPitch]*mul21 + p[srcPitch+srcPixelBytes]*mul22) >> 15; 	\
				p++;																												\
				int srcGreen = (p[0]*mul11 + p[srcPixelBytes]*mul12 + p[srcPitch]*mul21 + p[srcPitch+srcPixelBytes]*mul22) >> 15;	\
				p++;																												\
				int srcRed = (p[0]*mul11 + p[srcPixelBytes]*mul12 + p[srcPitch]*mul21 + p[srcPitch+srcPixelBytes]*mul22) >> 15;		\
				int srcAlpha;																										\
				if( _bReadAlpha_ )																									\
				{																													\
					p++;																											\
					srcAlpha = (p[0]*mul11 + p[srcPixelBytes]*mul12 + p[srcPitch]*mul21 + p[srcPitch+srcPixelBytes]*mul22) >> 15;	\
				}																		\
																						\
				_loop_																	\
																						\
				ofsX += incX;															\
				pDst += dstPixelBytes;													\
			}																			\
			ofsY += incY;																\
		}																				\
	}																					\
	else	/* UNFILTERED */															\
	{																					\
		int ofsY = (int) (sy*32768);		/* We use 15 binals for all calculations */	\
		int incY = (int) (sh*32768/dh);													\
																						\
		for( int y = 0 ; y < dh ; y++ )													\
		{																				\
			int ofsX = (int) (sx*32768);												\
			int incX = (int) (sw*32768/dw);												\
																						\
			Uint8 * pDst = m_pCanvas->m_pData + (dy+y) * m_pCanvas->m_pitch + dx * dstPixelBytes;	\
			Uint8 * pSrc = pSrcSurf->m_pData + (ofsY>>15) * pSrcSurf->m_pitch;						\
																						\
			for( int x = 0 ; x < dw ; x++ )												\
			{																			\
				Uint8 * p = pSrc + (ofsX >> 15)*srcPixelBytes;							\
																						\
				int srcBlue = p[0]; 													\
				int srcGreen = p[1];													\
				int srcRed = p[2];														\
				int srcAlpha;															\
				if( _bReadAlpha_ )														\
					srcAlpha = p[3];													\
																						\
				_loop_																	\
																						\
				ofsX += incX;															\
				pDst += dstPixelBytes;													\
			}																			\
			ofsY += incY;																\
		}																				\
	}																					\
}																						\



//____ _stretchBlitTintedOpaque() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitTintedOpaque( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
														int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( false,

	int tintRed = (int) m_tintColor.r;
	int tintGreen = (int) m_tintColor.g;
	int tintBlue = (int) m_tintColor.b;

	,

	pDst[0] = m_pDivTab[srcBlue*tintBlue];
	pDst[1] = m_pDivTab[srcGreen*tintGreen];
	pDst[2] = m_pDivTab[srcRed*tintRed];

	)
}

//____ _stretchBlitTintedBlend32() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitTintedBlend32( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
														 int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( true,

	int tintAlpha = (int) m_tintColor.a;
	int tintRed = (int) m_tintColor.r;
	int tintGreen = (int) m_tintColor.g;
	int tintBlue = (int) m_tintColor.b;

	,

	int alpha = m_pDivTab[srcAlpha*tintAlpha];
	int invAlpha = 255-alpha;

	srcBlue = m_pDivTab[srcBlue * tintBlue];
	srcGreen = m_pDivTab[srcGreen * tintGreen];
	srcRed = m_pDivTab[srcRed * tintRed];


	pDst[0] = m_pDivTab[pDst[0]*invAlpha + srcBlue*alpha];
	pDst[1] = m_pDivTab[pDst[1]*invAlpha + srcGreen*alpha];
	pDst[2] = m_pDivTab[pDst[2]*invAlpha + srcRed*alpha];

	)
}

//____ _stretchBlitTintedBlend24() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitTintedBlend24( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
														 int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( false,

	int tintAlpha = (int) m_tintColor.a;
	int tintRed = (int) m_pDivTab[m_tintColor.r * tintAlpha];
	int tintGreen = (int) m_pDivTab[m_tintColor.g * tintAlpha];
	int tintBlue = (int) m_pDivTab[m_tintColor.b * tintAlpha];
	int invAlpha = 255-tintAlpha;

	,

	pDst[0] = m_pDivTab[pDst[0]*invAlpha + srcBlue*tintBlue];
	pDst[1] = m_pDivTab[pDst[1]*invAlpha + srcGreen*tintGreen];
	pDst[2] = m_pDivTab[pDst[2]*invAlpha + srcRed*tintRed];

	)
}


//____ _stretchBlitTintedAdd32() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitTintedAdd32( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
														int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( true,

	int tintAlpha = (int) m_tintColor.a;
	int tintRed = (int) m_tintColor.r;
	int tintGreen = (int) m_tintColor.g;
	int tintBlue = (int) m_tintColor.b;

	,

	int alpha = m_pDivTab[srcAlpha*tintAlpha];

	srcBlue = m_pDivTab[srcBlue * tintBlue];
	srcGreen = m_pDivTab[srcGreen * tintGreen];
	srcRed = m_pDivTab[srcRed * tintRed];

	pDst[0] = m_limitTable[pDst[0] + (int) m_pDivTab[srcBlue*alpha] ];
	pDst[1] = m_limitTable[pDst[1] + (int) m_pDivTab[srcGreen*alpha] ];
	pDst[2] = m_limitTable[pDst[2] + (int) m_pDivTab[srcRed*alpha] ];

	)
}


//____ _stretchBlitTintedAdd24() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitTintedAdd24( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
														int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( false,

	int tintAlpha = (int) m_tintColor.a;
	int tintRed = (int) m_pDivTab[m_tintColor.r * tintAlpha];
	int tintGreen = (int) m_pDivTab[m_tintColor.g * tintAlpha];
	int tintBlue = (int) m_pDivTab[m_tintColor.b * tintAlpha];

	,

	pDst[0] = m_limitTable[pDst[0] + (int) m_pDivTab[srcBlue*tintBlue]];
	pDst[1] = m_limitTable[pDst[1] + (int) m_pDivTab[srcGreen*tintGreen]];
	pDst[2] = m_limitTable[pDst[2] + (int) m_pDivTab[srcRed*tintRed]];

	)
}


//____ _stretchBlitTintedMultiply() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitTintedMultiply( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
														  int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( false,

	int tintRed = (int) m_tintColor.r;
	int tintGreen = (int) m_tintColor.g;
	int tintBlue = (int) m_tintColor.b;

	,

	srcBlue = m_pDivTab[srcBlue * tintBlue];
	srcGreen = m_pDivTab[srcGreen * tintGreen];
	srcRed = m_pDivTab[srcRed * tintRed];

	pDst[0] = m_pDivTab[pDst[0]*srcBlue];
	pDst[1] = m_pDivTab[pDst[1]*srcGreen];
	pDst[2] = m_pDivTab[pDst[2]*srcRed];

	)
}

//____ _stretchBlitTintedInvert() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitTintedInvert( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
											      int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( false,

	int tintRed = (int) m_tintColor.r;
	int tintGreen = (int) m_tintColor.g;
	int tintBlue = (int) m_tintColor.b;

	,

	srcBlue = m_pDivTab[srcBlue * tintBlue];
	srcGreen = m_pDivTab[srcGreen * tintGreen];
	srcRed = m_pDivTab[srcRed * tintRed];

	pDst[0] = m_pDivTab[srcBlue*(255-pDst[0]) + pDst[0]*(255-srcBlue)];
	pDst[1] = m_pDivTab[srcGreen*(255-pDst[1]) + pDst[1]*(255-srcGreen)];
	pDst[2] = m_pDivTab[srcRed*(255-pDst[2]) + pDst[2]*(255-srcRed)];

	)
}


//____ _stretchBlitOpaque() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitOpaque( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
												  int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( false,

	,

	pDst[0] = srcBlue;
	pDst[1] = srcGreen;
	pDst[2] = srcRed;

	)
}

//____ _stretchBlitBlend32() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitBlend32( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
												   int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( true,

	,

	int invAlpha = 255-srcAlpha;

	pDst[0] = m_pDivTab[pDst[0]*invAlpha + srcBlue*srcAlpha];
	pDst[1] = m_pDivTab[pDst[1]*invAlpha + srcGreen*srcAlpha];
	pDst[2] = m_pDivTab[pDst[2]*invAlpha + srcRed*srcAlpha];

	)
}

//____ _stretchBlitAdd32() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitAdd32( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
												 int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( true,

	,

	pDst[0] = m_limitTable[pDst[0] + (int) m_pDivTab[srcBlue*srcAlpha] ];
	pDst[1] = m_limitTable[pDst[1] + (int) m_pDivTab[srcGreen*srcAlpha] ];
	pDst[2] = m_limitTable[pDst[2] + (int) m_pDivTab[srcRed*srcAlpha] ];

	)
}


//____ _stretchBlitAdd24() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitAdd24( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
												 int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( false,

	,

	pDst[0] = m_limitTable[pDst[0] + srcBlue];
	pDst[1] = m_limitTable[pDst[1] + srcGreen];
	pDst[2] = m_limitTable[pDst[2] + srcRed];

	)
}


//____ _stretchBlitMultiply() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitMultiply( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
												    int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( false,

	,

	pDst[0] = m_pDivTab[pDst[0]*srcBlue];
	pDst[1] = m_pDivTab[pDst[1]*srcGreen];
	pDst[2] = m_pDivTab[pDst[2]*srcRed];

	)
}

//____ _stretchBlitInvert() ____________________________________________

void WgGfxDeviceSoft::_stretchBlitInvert( const WgSurfaceSoft * pSrcSurf, float sx, float sy, float sw, float sh,
											      int dx, int dy, int dw, int dh )
{
	STRETCHBLIT( false,

	,

	pDst[0] = m_pDivTab[(srcBlue*(255-pDst[0]) + pDst[0]*(255-srcBlue))];
	pDst[1] = m_pDivTab[(srcGreen*(255-pDst[1]) + pDst[1]*(255-srcGreen))];
	pDst[2] = m_pDivTab[(srcRed*(255-pDst[2]) + pDst[2]*(255-srcRed))];

	)
}



//____ _initTables() ___________________________________________________________

void WgGfxDeviceSoft::_initTables()
{
	// Init limitTable

	for( int i = 0 ; i < 256 ; i++ )
		m_limitTable[i] = i;

	for( int i = 256 ; i < 512 ; i++ )
		m_limitTable[i] = 255;

	// Init divTable

	m_pDivTab = new Uint8[65536];

	for( int i = 0 ; i < 65536 ; i++ )
		m_pDivTab[i] = i / 255;

	// Init lineThicknessTable
	
	for( int i = 0 ; i < 17 ; i++ )
	{
		double b = i/16.0;
		m_lineThicknessTable[i] = (int) (sqrt( 1.0 + b*b ) * 65536);
	}

}
