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


#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <wg_glyphset.h>
#include <wg_texttool.h>


// TEMP PROFILING INCLUDES
//#include "Utilities/EProfiler.h"
//#include "Util/Util.hpp"






//____ WgGlyph::WgGlyph() _______________________________________________________

WgGlyph::WgGlyph()
{
	m_pGlyphset = 0;
	m_advance = 0;
	m_kerningIndex = 0;
}

//____ WgGlyph::WgGlyph() _______________________________________________________

WgGlyph::WgGlyph( int advance, Uint32 kerningIndex, WgGlyphset * pGlyphset )
{
	m_pGlyphset = pGlyphset;
	m_advance = advance;
	m_kerningIndex = kerningIndex;
}


