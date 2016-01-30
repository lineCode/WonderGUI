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
#ifndef WG_GFXDEVICE_GL_DOT_H
#define WG_GFXDEVICE_GL_DOT_H

#ifndef WG_GEO_DOT_H
#	include <wg_geo.h>
#endif

#ifdef WIN32
#	include <windows.h>
#endif

#include <GL/gl.h>


#ifndef	WG_GFXDEVICE_DOT_H
#	include <wg_gfxdevice.h>
#endif


class WgSurfaceGL;


class WgGfxDeviceGL : public WgGfxDevice
{
public:
	WgGfxDeviceGL( WgSize canvas );
	~WgGfxDeviceGL();

	void	SetCanvas( WgSize canvas );

	//

	bool	BeginRender();
	bool	EndRender();

	void	SetTintColor( WgColor color );
	bool	SetBlendMode( WgBlendMode blendMode );
	void	Fill( const WgRect& rect, const WgColor& col );
	void	Blit( const WgSurface* src, const WgRect& srcrect, int dx, int dy  );

	void	StretchBlitSubPixel( const WgSurface * pSrc, float sx, float sy, float sw, float sh,
						   		 float dx, float dy, float dw, float dh, bool bTriLinear, float mipBias );
								 
								 
	void	FillSubPixel( const WgRectF& rect, const WgColor& col );
							 
								 
	void	ClipDrawHorrLine( const WgRect& clip, const WgCoord& start, int length, const WgColor& col );
	void	ClipDrawVertLine( const WgRect& clip, const WgCoord& start, int length, const WgColor& col );
	void	ClipPlotSoftPixels( const WgRect& clip, int nCoords, const WgCoord * pCoords, const WgColor& col, float thickness );
    void    ClipPlotPixels( const WgRect& clip, int nCoords, const WgCoord * pCoords, const WgColor& col, const WgColor * colors);

	void	DrawLine( WgCoord begin, WgCoord end, WgColor color, float thickness = 1.f );
	void	DrawArcNE( const WgRect& rect, WgColor color );
	void	DrawElipse( const WgRect& rect, WgColor color );
	void	DrawFilledElipse( const WgRect& rect, WgColor color );

	void	ClipDrawLine( const WgRect& clip, WgCoord begin, WgCoord end, WgColor color, float thickness = 1.f );
	void	ClipDrawArcNE( const WgRect& clip, const WgRect& rect, WgColor color );
	void	ClipDrawElipse( const WgRect& clip, const WgRect& rect, WgColor color );
	void	ClipDrawFilledElipse( const WgRect& clip, const WgRect& rect, WgColor color );
								 

protected:

	void	_setBlendMode( WgBlendMode blendMode );

	bool	m_bRendering;

	// GL states saved between BeginRender() and EndRender().

	GLboolean	m_glDepthTest;
	GLboolean	m_glTexture2D;
	GLboolean	m_glFog;
	GLboolean	m_glLighting;
	GLboolean	m_glBlendEnabled;
	GLint		m_glBlendSrc;
	GLint		m_glBlendDst;
	GLint		m_glMatrixMode;
	GLfloat		m_glColor[4];
	WgSize		m_size;

};

#endif //WG_GFXDEVICE_GL_DOT_H
