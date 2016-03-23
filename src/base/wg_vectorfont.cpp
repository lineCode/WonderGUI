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

/* TODO: See if we can be better off using multiple size-objects instead of changing size all the time.
   TODO: Separate glyph bitmap from glyph struct, not render or prio cache slot until bitmap is accessed (probably has more to do with Pen).
*/

#include <wg_userdefines.h>

#ifdef	USE_FREETYPE
#include <wg_base.h>
#include <wg_vectorfont.h>
#include <wg_surface.h>
#include <wg_surfacefactory.h>
#include <assert.h>


#include <ft2build.h>
#include FT_FREETYPE_H

namespace wg 
{
	
	const char VectorFont::CLASSNAME[] = {"VectorFont"};
	
	Chain<VectorFont::CacheSlot>	VectorFont::s_cacheSlots[c_glyphSlotSizes];
	Chain<VectorFont::CacheSurf>	VectorFont::s_cacheSurfaces;
	SurfaceFactory_p					VectorFont::s_pSurfaceFactory = 0;
	
	
	//____ Constructor ____________________________________________________________
	
	VectorFont::VectorFont( char* pTTF_File, int bytes, int faceIndex )
	{
		m_pData = pTTF_File;
		m_ftCharSize	= 0;
		m_accessCounter = 0;
		m_sizeOffset	= 0;
		m_size 			= 0;
	
		for( int i = 0 ; i <= MaxFontSize ; i++ )
		{
			m_cachedGlyphsIndex[i] = 0;
			m_whitespaceAdvance[i] = 0;
		}
	
		FT_Error err = FT_New_Memory_Face(	Base::getFreeTypeLibrary(),
											(const FT_Byte *)pTTF_File,
											bytes,
											0,
											&m_ftFace );
	
		if( err )
		{
	//		int x = 0;
			//TODO: Error handling...
		}
	
	
		setRenderMode( RenderMode::CrispEdges );
		setSize( 10 );
	}
	
	//____ Destructor _____________________________________________________________
	
	VectorFont::~VectorFont()
	{
		for( int size = 0 ; size <= MaxFontSize ; size++ )
		{
			if( m_cachedGlyphsIndex[size] != 0 )
			{
				for( int page = 0 ; page < 256 ; page++ )
				{
					if( m_cachedGlyphsIndex[size][page] != 0 )
						delete [] m_cachedGlyphsIndex[size][page];
				}
	
				delete [] m_cachedGlyphsIndex[size];
			}
		}
	
		FT_Done_Face( m_ftFace );
		delete[] m_pData;
	}
	
	//____ isInstanceOf() _________________________________________________________
	
	bool VectorFont::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Font::isInstanceOf(pClassName);
	}
	
	//____ className() ____________________________________________________________
	
	const char * VectorFont::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	//____ cast() _________________________________________________________________
	
	VectorFont_p VectorFont::cast( const Object_p& pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return VectorFont_p( static_cast<VectorFont*>(pObject.rawPtr()) );
	
		return 0;
	}
	
	//____ setSize() __________________________________________________________
	
	bool VectorFont::setSize( int size )
	{
			if( size == m_ftCharSize )
				return true;

			int ftSize = size + m_sizeOffset;
		
			// Sanity check
		
			if( ftSize > MaxFontSize || ftSize < 0 )
				return 0;

		
		
			FT_Error err = FT_Set_Char_Size( m_ftFace, ftSize*64, 0, 0,0 );
	//		FT_Error err = FT_Set_Pixel_Sizes( m_ftFace, 0, size );
			if( err )
			{
				m_size = 0;
				m_ftCharSize = 0;
				return false;
			}
	
	
			m_size = size;
			m_ftCharSize = ftSize;
			_refreshRenderFlags();
			return true;
	}
	

	//____ _refreshRenderFlags() _______________________________________________
	
	void VectorFont::_refreshRenderFlags()
	{
		switch( m_renderMode[m_ftCharSize] )
		{
			case RenderMode::Monochrome:
				m_renderFlags = FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO;
				break;
			case RenderMode::CrispEdges:
				m_renderFlags = FT_LOAD_TARGET_NORMAL;
				break;
			case RenderMode::BestShapes:
				m_renderFlags = FT_LOAD_TARGET_LIGHT;
				break;

			default:
				break;
		}
	}
	
	
	//____ setRenderMode() ________________________________________________________
	
	bool VectorFont::setRenderMode( RenderMode mode, int startSize, int endSize )
	{
		if( startSize < 0 || startSize > endSize || startSize > MaxFontSize )
			return false;
	
		if( endSize > MaxFontSize )
			endSize = MaxFontSize;
	
		for( int i = startSize ; i <= endSize ; i++ )
			m_renderMode[i] =mode;
	
		// Force update of m_renderFlags since current size might be affected
	
		_refreshRenderFlags();
	
		return true;
	}
	
	//____ kerning() ___________________________________________________________
	
	int VectorFont::kerning( Glyph_p pLeftGlyph, Glyph_p pRightGlyph )
	{
		if( !pLeftGlyph || !pRightGlyph )
			return 0;
		
		// Get kerning info
	
		FT_Vector	delta;
		FT_Get_Kerning( m_ftFace, pLeftGlyph->kerningIndex(), pRightGlyph->kerningIndex(), FT_KERNING_DEFAULT, &delta );
	
		return delta.x >> 6;
	}
	
	//____ whitespaceAdvance() _________________________________________________
	
	int VectorFont::whitespaceAdvance()
	{
		if( !m_whitespaceAdvance[m_ftCharSize] )
		{
			FT_Error err;
		
			// Load whitespace glyph
	
			err = FT_Load_Char( m_ftFace, ' ', FT_LOAD_RENDER );
			if( err )
				return 0;
	
			// Get and return advance
			m_whitespaceAdvance[m_ftCharSize] = m_ftFace->glyph->advance.x >> 6;
		}
	
		return m_whitespaceAdvance[m_ftCharSize];
	}
	
	//____ height() ____________________________________________________________
	
	int VectorFont::height()
	{	
		return (m_ftFace->size->metrics.ascender - m_ftFace->size->metrics.descender) >> 6;
	}
	
	//____ lineSpacing() ____________________________________________________________
	
	int VectorFont::lineSpacing()
	{
		return (m_ftFace->size->metrics.height) >> 6;
	}
	
	
	//____ baseline() ____________________________________________________________
	
	int VectorFont::baseline()
	{	
		return (m_ftFace->size->metrics.ascender) >> 6;
	}
	
	
	//____ nbGlyphs() __________________________________________________________
	
	int VectorFont::nbGlyphs()
	{
		return m_ftFace->num_glyphs;
	}
	
	//____ hasGlyphs() ____________________________________________________________
	
	bool VectorFont::hasGlyphs()
	{
		return m_ftFace->num_glyphs?true:false;
	}
	
	//____ isMonospace() __________________________________________________________
	
	bool VectorFont::isMonospace()
	{
		return FT_IS_FIXED_WIDTH( m_ftFace )>0?true:false;
	}
	
	//____ maxAdvance() ___________________________________________________
	
	int VectorFont::maxAdvance()
	{
		return m_ftFace->size->metrics.max_advance >> 6;
	}
	
	
	//____ hasGlyph() _____________________________________________________________
	
	bool VectorFont::hasGlyph( uint16_t ch )
	{
		int index = FT_Get_Char_Index( m_ftFace, ch );
		if( index == 0 )
			return false;
	
		return true;
	}
	
	//____ getGlyph() _____________________________________________________________
	
	Glyph_p VectorFont::getGlyph( uint16_t ch )
	{	
		// Get cached glyph if we have one
	
		MyGlyph * pGlyph = _findGlyph( ch, m_ftCharSize );
		if( pGlyph == 0 )
		{
			FT_Error err;
	
			// Load MyGlyph
	
			FT_UInt char_index = FT_Get_Char_Index( m_ftFace, ch );
			if( char_index == 0 )
				return 0;			// We got index for missing glyph.
	
			err = FT_Load_Glyph( m_ftFace, char_index, m_renderFlags );
			if( err )
				return 0;
	
			// Get some details about the glyph
	
			int advance = m_ftFace->glyph->advance.x >> 6;
	
			// Get a MyGlyph object and fill in details
	
			pGlyph = _addGlyph( ch, m_ftCharSize, advance, char_index );
		}
	
		return pGlyph;
	}
	
	
	
	/*
	Glyph_p VectorFont::getGlyph( uint16_t ch, int size )
	{
		size += m_sizeOffset;
	
		// Sanity check
	
		if( size > MaxFontSize || size < 0 )
			return 0;
	
		// Get cached glyph if we have one
	
		CacheSlot * pSlot = FindSlotInIndex( ch, size );
		if( pSlot == 0 )
		{
			FT_Error err;
	
			//-----------------------------------------
			// Get empty cache slot and fill with glyph
			//-----------------------------------------
	
			// Set size for FreeType
	
			if( m_ftCharSize != size )
				if( !SetCharSize( size ) )
					return 0;
	
			// Load MyGlyph
	
			FT_UInt char_index = FT_Get_Char_Index( m_ftFace, ch );
			if( char_index == 0 )
				return 0;			// We got index for missing glyph.
	
			err = FT_Load_Glyph( m_ftFace, char_index, m_renderFlags );
			if( err )
				return 0;
	
			// Get some details about the glyph
	
			int width = m_ftFace->glyph->bitmap.width;
			int height = m_ftFace->glyph->bitmap.rows;
	
			int advance = m_ftFace->glyph->advance.x >> 6;
			int xBearing = m_ftFace->glyph->bitmap_left;
			int yBearing = -m_ftFace->glyph->bitmap_top;
	
			// Get a cache slot
	
			pSlot = getCacheSlot( width, height );
			if( pSlot == 0 )
				return 0;
	
			// Fill in glyph details
	
			pSlot->pOwner = this;
			pSlot->size = size;
			pSlot->character = ch;
	
			pSlot->glyph.advance = advance;
			pSlot->glyph.bearingX = xBearing;
			pSlot->glyph.bearingY = yBearing;
			pSlot->glyph.kerningIndex = char_index;
			pSlot->glyph.pSurf = pSlot->pSurf->pSurf;
			pSlot->glyph.rect = Rect(pSlot->rect.x, pSlot->rect.y, width, height);
	
			//
	
			CopyBitmap( &m_ftFace->glyph->bitmap, pSlot );	// Copy our glyph bitmap to the slot
			AddSlotToIndex( ch, size, pSlot );				// Put a pointer in our index
		}
	
		TouchSlot( pSlot );								// Notify cache that we have accessed this one
		return &pSlot->glyph;
	}
	*/
	
	//____ _______________________________________________________
	
	VectorFont::CacheSlot * VectorFont::_generateBitmap( MyGlyph * pGlyph )
	{
		FT_Error err;
		
		// Load MyGlyph
	
		err = FT_Load_Glyph( m_ftFace, pGlyph->kerningIndex(), FT_LOAD_RENDER | m_renderFlags );
		if( err )
			return 0;
	
		// Get some details about the glyph
	
		int width = m_ftFace->glyph->bitmap.width;
		int height = m_ftFace->glyph->bitmap.rows;
	
		// Get a cache slot
	
		CacheSlot * pSlot = getCacheSlot( width, height );
	
		if( pSlot )
		{
			// Fill in missing slot details
	
			pSlot->pGlyph = pGlyph;
			pSlot->bitmap.rect = Rect(pSlot->rect.x, pSlot->rect.y, width, height);
			pSlot->bitmap.bearingX = m_ftFace->glyph->bitmap_left;
			pSlot->bitmap.bearingY = -m_ftFace->glyph->bitmap_top;
	
			//
	
			_copyBitmap( &m_ftFace->glyph->bitmap, pSlot );	// Copy our glyph bitmap to the slot
		}
	
		return pSlot;
	}
	
	
	
	//____ _copyBitmap() ____________________________________________________________
	
	// Currently only supports 32-bit RGBA surfaces!
	
	void VectorFont::_copyBitmap( FT_Bitmap * pBitmap, CacheSlot * pSlot )
	{
		Surface_p pSurf = pSlot->bitmap.pSurface;
	
		unsigned char * pDest = (unsigned char*) pSurf->lockRegion( AccessMode::WriteOnly, pSlot->bitmap.rect );
		assert( pDest != 0 );
		assert( pSurf->pixelFormat()->type == PixelType::BGRA_8 );
	
		int dest_pitch = pSurf->pitch();
	
		// Copy glyph bitmap into alpha channel of slot, making sure to clear any
		// left over area of slots alpha channel.
	
	
		switch( m_renderFlags )
		{
			case (FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO):
				_copyA1ToRGBA8( pBitmap->buffer, pBitmap->width, pBitmap->rows, pBitmap->pitch, pDest, pSlot->rect.w, pSlot->rect.h, dest_pitch );
				break;
			case (FT_LOAD_TARGET_NORMAL):
			case (FT_LOAD_TARGET_LIGHT):
				_copyA8ToRGBA8( pBitmap->buffer, pBitmap->width, pBitmap->rows, pBitmap->pitch, pDest, pSlot->rect.w, pSlot->rect.h, dest_pitch );
				break;
	
			default:
				assert( false );
		}
	
	
		// Testcode
	/*
		int i = 128;
		for( unsigned int y = 0 ; y < pSurf->getHeight() ; y++ )
		{
			for( unsigned int x = 0 ; x < pSurf->GetWidth() ; x++ )
				pBuffer[y*dest_pitch + x] = i++;
		}
	*/
	
		pSurf->unlock();
	}
	
	
	//____ _copyA8ToRGBA8() _____________________________________________________
	
	void VectorFont::_copyA8ToRGBA8( const uint8_t * pSrc, int src_width, int src_height, int src_pitch,
									    uint8_t * pDest, int dest_width, int dest_height, int dest_pitch )
	{
	
		int y = 0;
		for( ; y < src_height ; y++ )
		{
			int x = 0;
			for( ; x < src_width ; x++ )
				pDest[x*4+3] = pSrc[x];
	
			for( ; x < dest_width ; x++ )
				pDest[x*4+3] = 0;
	
			pSrc  += src_pitch;
			pDest += dest_pitch;
		}
	
		for( ; y < dest_height ; y++ )
		{
			for( int x = 0 ; x < dest_width ; x++ )
				pDest[x*4+3] = 0;
	
			pDest += dest_pitch;
		}
	}
	
	//____ _copyA1ToRGBA8() _____________________________________________________
	
	void VectorFont::_copyA1ToRGBA8( const uint8_t * pSrc, int src_width, int src_height, int src_pitch,
									    uint8_t * pDest, int dest_width, int dest_height, int dest_pitch )
	{
		uint8_t lookup[2] = { 0, 255 };
	
		int y = 0;
		for( ; y < src_height ; y++ )
		{
	
			int x = 0;
			for( ; x < src_width ; x++ )
			{
				pDest[x*4+3] = lookup[(((pSrc[x>>3])<<(x&7))&0xFF)>>7];
			}
	
			for( ; x < dest_width ; x++ )
				pDest[x*4+3] = 0;
	
			pSrc  += src_pitch;
			pDest += dest_pitch;
		}
	
		for( ; y < dest_height ; y++ )
		{
			for( int x = 0 ; x < dest_width ; x++ )
				pDest[x*4+3] = 0;
	
			pDest += dest_pitch;
		}
	}
	
	//___ _addGlyph() ________________________________________________________
	
	VectorFont::MyGlyph * VectorFont::_addGlyph( uint16_t ch, int size, int advance, uint32_t kerningIndex )
	{
		if( m_cachedGlyphsIndex[size] == 0 )
		{
			MyGlyph ** p = new MyGlyph*[256];
			memset( p, 0, 256*sizeof(MyGlyph*) );
	
			m_cachedGlyphsIndex[size] = p;
		}
	
		if( m_cachedGlyphsIndex[size][ch>>8] == 0 )
		{
			MyGlyph * p = new MyGlyph[256];
	
			m_cachedGlyphsIndex[size][ch>>8] = p;
		}
	
		assert( !m_cachedGlyphsIndex[size][ch>>8][ch&0xFF].isInitialized() );
	
		m_cachedGlyphsIndex[size][ch>>8][ch&0xFF] = MyGlyph( ch, size, advance, kerningIndex, this );
	
		return &m_cachedGlyphsIndex[size][ch>>8][ch&0xFF];
	}
	
	
	//____ setSurfaceFactory() ____________________________________________________
	
	void VectorFont::setSurfaceFactory( const SurfaceFactory_p& pFactory )
	{
		s_pSurfaceFactory = pFactory;
	}
	
	
	//____ clearCache() ___________________________________________________________
	
	void VectorFont::clearCache()
	{
		for( int i = 0 ; i < c_glyphSlotSizes ; i++ )
		{
			CacheSlot * p = s_cacheSlots[i].first();
			while( p )
			{
				if( p->pGlyph )
					p->pGlyph->slotLost();
				p = p->next();
			}
	
			s_cacheSlots[i].clear();
		}
	
		s_cacheSurfaces.clear();
	}
	
	
	//____ getCacheSlot() _________________________________________________________
	
	VectorFont::CacheSlot * VectorFont::getCacheSlot( int width, int height )
	{
		// Calculate size and index
	
		int size = ((width>height ? width:height)+c_glyphPixelSizeQuantization-1);
	
		if( size < c_minGlyphPixelSize )
			size = c_minGlyphPixelSize;
	
		assert( size <= c_maxGlyphPixelSize );
	
		int index = (size-c_minGlyphPixelSize) / c_glyphPixelSizeQuantization;
	
		// Make sure we have
	
		CacheSlot * pSlot = s_cacheSlots[index].last();
		if( pSlot == 0 || pSlot->pGlyph != 0 )
		{
			addCacheSlots( &s_cacheSlots[index], Size(size,size), 16 );
			pSlot = s_cacheSlots[index].last();
		}
	
		return pSlot;
	}
	
	
	//____ addCacheSlots() ___________________________________________________
	/*
		Creates a new cache surface of 2^x size big enough to at least hold the
		specified minimum amount of slots. Fills the surface with white and alpha 0.
		Creates all slots that can fit into this surface and adds them to the specified chain.
	*/
	
	int VectorFont::addCacheSlots( Chain<CacheSlot> * pChain, const Size& slotSize, int minSlots )
	{
		// Create and add the cache surface
	
		Size texSize = calcTextureSize( slotSize, 16 );
	
		Surface_p pSurf = s_pSurfaceFactory->createSurface( texSize );
		pSurf->fill( Color( 255,255,255,0 ) );
	
		CacheSurf * pCache = new CacheSurf( pSurf );
		s_cacheSurfaces.pushBack( pCache );
	
		// Create the slots
	
		Rect	slot( 0, 0, slotSize );
		int		nSlots = 0;
	
		for( slot.y = 0 ; slot.y + slotSize.h < texSize.h ; slot.y += slotSize.h + 1 )
		{
			for( slot.x = 0 ; slot.x + slotSize.w < texSize.w ; slot.x += slotSize.w + 1 )
			{
				CacheSlot * pSlot = new CacheSlot( pCache, slot );
				pChain->pushBack(pSlot);
				nSlots++;
			}
		}
	
		return nSlots;
	}
	
	
	
	//____ maxSlotsInSurface() ____________________________________________________
	
	int VectorFont::maxSlotsInSurface( const Size& surf, const Size& slot )
	{
		int rows = (surf.w+1)/(slot.w+1);			// +1 since we need one pixel spacing between each slot.
		int columns = (surf.h+1)/(surf.h+1);
	
		return rows*columns;
	}
	
	
	//____ calcTextureSize() ______________________________________________________
	
	Size VectorFont::calcTextureSize( const Size& slotSize, int nSlots )
	{
		Size	surfSize( 128, 128 );
	
		while( maxSlotsInSurface(surfSize, slotSize) < nSlots )
		{
			if( surfSize.w > surfSize.h )
				surfSize.h *= 2;
			else if( surfSize.w < surfSize.h )
				surfSize.w *= 2;
			else
			{
				if( maxSlotsInSurface( Size( surfSize.w, surfSize.h*2 ), slotSize ) >
					maxSlotsInSurface( Size( surfSize.w*2, surfSize.h ), slotSize ) )
					surfSize.h *= 2;
				else
					surfSize.w *= 2;
			}
		}
	
		return surfSize;
	}
	
	
	
	
	VectorFont::CacheSurf::~CacheSurf()
	{
	}
	
	VectorFont::MyGlyph::MyGlyph()
	: Glyph( 0, 0, 0 )
	{
		m_pSlot = 0;
		m_size = 0;
		m_character = 0;
	}
	
	
	VectorFont::MyGlyph::MyGlyph( uint16_t character, uint16_t size, int advance, uint32_t kerningIndex, Font * pFont )
	: Glyph( advance, kerningIndex, pFont )
	{
		m_pSlot = 0;
		m_size = size;
		m_character = character;
	}
	
	VectorFont::MyGlyph::~MyGlyph()
	{
		if(  m_pSlot != 0 )
		{
			m_pSlot->pGlyph = 0;
			m_pSlot->access = 0;
	
			m_pSlot->moveLast();
		}
	}
	
	const GlyphBitmap * VectorFont::MyGlyph::getBitmap()
	{
		if( !m_pSlot )
		{
			m_pSlot = ((VectorFont*)m_pFont)->_generateBitmap( this );
		}
	
		((VectorFont*)m_pFont)->_touchSlot(m_pSlot);
		return &m_pSlot->bitmap;
	}
	
} // namespace wg
#endif //USE_FREETYPE

