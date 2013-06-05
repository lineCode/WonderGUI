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
#ifndef WG_IMAGE_DOT_H
#define WG_IMAGE_DOT_H


#ifndef WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif

#ifndef WG_SKIN_DOT_H
#	include <wg_skin.h>
#endif


//____ WgImage __________________________________________________________

class WgImage : public WgWidget
{
public:
	WgImage();
	virtual ~WgImage();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgImage(); };
	

	void			SetImage( WgSurface * pSurface, const WgRect& rect );
	void			SetImage( WgSurface * pSurface );

	WgSurface *		ImageSurface() const { return m_pSurface; }
	WgRect			ImageRect() const { return m_rect; }

	void			SetSkin( const WgSkinPtr& pSkin );
	WgSkinPtr		Skin() const { return m_pSkin; }

	WgSize			PreferredSize() const;

protected:

	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip );
	bool	_onAlphaTest( const WgCoord& ofs );
	void	_onEnable();
	void	_onDisable();

private:

	WgSkinPtr	m_pSkin;
	WgSurface *	m_pSurface;
	WgRect		m_rect;

};


#endif //WG_FILLER_DOT_H
