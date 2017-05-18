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
#ifndef WG_PEN_DOT_H
#define WG_PEN_DOT_H

#ifndef WG_TYPES_DOT_H
#	include <wg_types.h>
#endif

#ifndef WG_COLOR_DOT_H
#	include <wg_color.h>
#endif

#ifndef WG_GEO_DOT_H
#	include <wg_geo.h>
#endif

#ifndef WG_TEXTPROP_DOT_H
#	include <wg_textprop.h>
#endif

#ifndef WG_GLYPHSET_DOT_H
#	include <wg_glyphset.h>
#endif

#ifndef	WG_GFXDEVICE_DOT_H
#	include <wg_gfxdevice.h>
#endif

class WgTextNode;

//____ WgPen _____________________________________________________________

class WgPen
{
friend class WgFont;

public:
	WgPen();
	WgPen( WgGfxDevice * pDevice, const WgCoord& origo, const WgRect& clip = WgRect() );
//	WgPen( const WgTextpropPtr& pTextprop, const WgTextpropPtr& pCharProp = 0, WgMode mode = WG_MODE_NORMAL ) { SetTextprop( pTextprop, pCharProp, mode ); }
//	WgPen( Uint16 hTextprop, Uint16 hCharProp = 0, WgMode mode = WG_MODE_NORMAL ) { SetTextprop( hTextprop, hCharProp, mode ); }
	~WgPen() {}

	void					SetClipRect( const WgRect& clip );
	inline void				SetDevice( WgGfxDevice * pDevice ) { m_pDevice = pDevice; }
	void					SetTextNode( WgTextNode * pNode ) { m_pTextNode = pNode; _onAttrChanged(); }
	void					SetScale( int scale );

	void					SetOrigo( const WgCoord& pos ) { m_origo = pos; }

	bool					SetAttributes( const WgTextAttr& attr );
	bool					SetSize( int size );
	void					SetFont( WgFont * pFont );
	void					SetStyle( WgFontStyle style );
	void					SetColor( WgColor color );
//	void					SetCharVisibility( int visibilityFlags );		// We need something better here...


	inline void				SetPos( const WgCoord& pos ) { m_pos = pos; }
	inline void				SetPosX( int x ) { m_pos.x = x; }
	inline void				SetPosY( int y ) { m_pos.y = y; }

	inline void				Move( const WgCoord& pos ) { m_pos += pos; }
	inline void				MoveX( int x ) { m_pos.x += x; }
	inline void				MoveY( int y ) { m_pos.y += y; }


	void					SetTab( int width ) { m_tabWidth = width; }
	bool					SetChar( Uint32 chr );
	void					FlushChar() { m_pGlyph = &m_dummyGlyph; m_dummyGlyph.SetAdvance(0); }
	void					ApplyKerning() { if( m_pPrevGlyph != &m_dummyGlyph && m_pGlyph != &m_dummyGlyph ) m_pos.x += m_pGlyphs->GetKerning( m_pPrevGlyph, m_pGlyph, m_size ); }

	inline void				AdvancePos() { m_pos.x += m_pGlyph->Advance(); }							///< Advances position past current character.
	inline void				AdvancePosMonospaced() { m_pos.x += m_pGlyphs->GetMaxGlyphAdvance(m_size); }	///< Advances position past current character using monospace spacing.
	void					AdvancePosCursor( const WgCursorInstance& instance );

	inline WgGlyphPtr		GetGlyph() const { return m_pGlyph; }
	inline WgCoord			GetPos() const { return m_pos; }
	inline int				GetPosX() const { return m_pos.x; }
	inline int				GetPosY() const { return m_pos.y; }

//	inline WgCoord			GetBlitPos() const { return WgCoord( m_pos.x + m_pGlyph->BearingX(), m_pos.y + m_pGlyph->BearingY() ); }
//	inline int				GetBlitPosX() const { return m_pos.x + m_pGlyph->BearingX(); }
//	inline int				GetBlitPosY() const { return m_pos.y + m_pGlyph->BearingY(); }

	inline const WgRect&	GetClipRect() const { return m_clipRect; }
	inline bool				HasClipRect() const { return m_bClip; }

	inline WgFont *			GetFont() const { return m_pFont; }
	inline int				GetSize() const { return m_size; }
	inline WgFontStyle		GetStyle() const { return m_style; }
	inline WgColor			GetColor() const { return m_color; }
	inline WgGlyphset *		GetGlyphset() const { return m_pGlyphs; }


	inline int				GetLineSpacing() const { return m_pGlyphs->GetLineSpacing(m_size); }
	inline int				GetLineHeight() const { return m_pGlyphs->GetHeight(m_size); }
	inline int				GetBaseline() const { return m_pGlyphs->GetBaseline(m_size); }

	void					BlitChar() const;
	bool					BlitCursor( const WgCursorInstance& instance ) const;

private:
	void _init();

	void _onAttrChanged();

	class DummyGlyph : public WgGlyph
	{
	public:
		DummyGlyph() : WgGlyph( 0, 0, 0 ) {}
		const WgGlyphBitmap * GetBitmap() { return 0; }

		void SetAdvance( int advance ) { m_advance = advance; }
	};

	//

	//

	WgGlyphset *m_pGlyphs;			// Pointer at our glyphs.

	WgGlyphPtr		m_pPrevGlyph;	// Previous glyph, saved to allow for kerning.
	WgGlyphPtr		m_pGlyph;		// Current glyph.

	WgFont *		m_pFont;		// Pointer back to the font.
	int				m_wantedSize;	// Size we requested.
	int				m_size;			// Fontsize we got a glyphset for, which might be smaller than what we requested.
	WgFontStyle		m_style;		// Style of glyphset we requested.
	WgColor			m_color;		// Color this pen draws in.

	bool			m_bShowSpace;	// Set if space control character should be shown (usually a dot in the middle of the cell).
	bool			m_bShowCRLF;	// Set if the CR/LF control character should be shown.

	WgCoord			m_origo;		// Origo position, from where we start printing and count tab-positions.
	WgCoord			m_pos;			// Position of this pen in screen pixels.

	DummyGlyph		m_dummyGlyph;	// Dummy glyph used for whitespace, tab etc

	int				m_tabWidth;		// Tab width in pixels.

	WgGfxDevice *	m_pDevice;		// Device used for blitting.
	WgTextNode *	m_pTextNode;	// TextManager used for scaling/layouting the text.

	bool			m_bClip;		// Set if we have a clipping rectangle.
	WgRect			m_clipRect;		// Clipping rectangle used for ClipBlit().

	int				m_scale;		// Widget scale, for text sizes.

};

#endif //WG_PEN_DOT_H
