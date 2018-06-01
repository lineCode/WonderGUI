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

#include <memory.h>

#include <wg3_glsurface.h>
#include <wg3_util.h>
#include <wg3_blob.h>
#include <assert.h>



namespace wg
{
	const char GlSurface::CLASSNAME[] = {"GlSurface"};

	//____ maxSize() _______________________________________________________________

	Size GlSurface::maxSize()
	{
		GLint max = 1024;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		return Size(max,max);
	}

	//____ create ______________________________________________________________

    GlSurface_p	GlSurface::create( Size size, PixelType type, int hint )
    {
        if( type != PixelType::BGRA_8 && type != PixelType::BGR_8)
            return GlSurface_p();
        
        return GlSurface_p(new GlSurface(size,type,hint));
    }
    
    GlSurface_p	GlSurface::create( Size size, PixelType type, Blob * pBlob, int pitch, int hint )
    {
        if( (type != PixelType::BGRA_8 && type != PixelType::BGR_8) || !pBlob || pitch % 4 != 0 )
            return GlSurface_p();
        
        return GlSurface_p(new GlSurface(size,type,pBlob,pitch,hint));
    }
    
    GlSurface_p	GlSurface::create( Size size, PixelType type, uint8_t * pPixels, int pitch, const PixelFormat * pPixelFormat, int hint )
    {
        if( (type != PixelType::BGRA_8 && type != PixelType::BGR_8) || pPixels == 0 )
            return GlSurface_p();
        
        return  GlSurface_p(new GlSurface(size,type,pPixels,pitch, pPixelFormat,hint));
    };
    
    GlSurface_p	GlSurface::create( Surface * pOther, int hint )
    {
        return GlSurface_p(new GlSurface( pOther,hint ));
    }

    
    
	//____ Constructor _____________________________________________________________


    GlSurface::GlSurface( Size size, PixelType type, int hint )
    {
		assert( type == PixelType::BGR_8 || type == PixelType::BGRA_8 );

        _setPixelDetails(type);
        m_size	= size;
        m_pitch = ((size.w*m_pixelFormat.bits/8)+3)&0xFFFFFFFC;
    	m_pBlob = Blob::create(m_pitch*m_size.h);
		
        
        glGenTextures( 1, &m_texture );
        glBindTexture( GL_TEXTURE_2D, m_texture );
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
                     m_accessFormat, GL_UNSIGNED_BYTE, NULL );
    }
    
    
	GlSurface::GlSurface( Size size, PixelType type, Blob * pBlob, int pitch, int hint )
	{
		assert( (type == PixelType::BGR_8 || type == PixelType::BGRA_8) && pBlob && pitch % 4 == 0 );

        // Set general information
        
        _setPixelDetails(type);
        m_size	= size;
        m_pitch = pitch;
		m_pBlob = pBlob;
		
		glGenTextures( 1, &m_texture );
        glBindTexture( GL_TEXTURE_2D, m_texture );
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
			m_accessFormat, GL_UNSIGNED_BYTE, m_pBlob->data() );

		assert( glGetError() == 0);
	}
   
    GlSurface::GlSurface( Size size, PixelType type, uint8_t * pPixels, int pitch, const PixelFormat * pPixelFormat, int hint )
    {
		assert( (type == PixelType::BGR_8 || type == PixelType::BGRA_8) && pPixels != 0 );
		
       _setPixelDetails(type);
        m_size	= size;
        m_pitch = ((size.w*m_pixelFormat.bits/8)+3)&0xFFFFFFFC;
        m_pBlob = Blob::create(m_pitch*m_size.h);
        
        m_pPixels = (uint8_t *) m_pBlob->data();
        _copyFrom( pPixelFormat==0 ? &m_pixelFormat:pPixelFormat, pPixels, pitch, size, size );
        m_pPixels = 0;
                
        glGenTextures( 1, &m_texture );
        glBindTexture( GL_TEXTURE_2D, m_texture );
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		
        glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
                     m_accessFormat, GL_UNSIGNED_BYTE, m_pBlob->data() );
    
 		assert( glGetError() == 0);
            }


    GlSurface::GlSurface( Surface * pOther, int hint )
    {
		assert( pOther );
		
        _setPixelDetails(pOther->pixelFormat()->type);
        m_size	= pOther->size();
        m_pitch = m_size.w * m_pixelSize;
        m_pBlob = Blob::create(m_pitch*m_size.h);
        
        m_pPixels = (uint8_t *) m_pBlob->data();
        _copyFrom( pOther->pixelFormat(), (uint8_t*)pOther->pixels(), pOther->pitch(), m_size, m_size );
        m_pPixels = 0;
        
        glGenTextures( 1, &m_texture );
        glBindTexture( GL_TEXTURE_2D, m_texture );
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		
        glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
                     m_accessFormat, GL_UNSIGNED_BYTE, m_pBlob->data() );
        
		assert( glGetError() == 0);
    }
    
    

	void GlSurface::_setPixelDetails( PixelType type )
	{
        if( type == PixelType::BGR_8 )
        {
            m_internalFormat = GL_RGB8;
            m_accessFormat = GL_BGR;
            m_pixelSize = 3;
        }
        else
        {
            m_internalFormat = GL_RGBA8;
            m_accessFormat = GL_BGRA;
            m_pixelSize = 4;
        }
        
        Util::pixelTypeToFormat(type, m_pixelFormat);
	}

	//____ Destructor ______________________________________________________________

	GlSurface::~GlSurface()
	{
		// Free the stuff

		glDeleteTextures( 1, &m_texture );
	}

	//____ isInstanceOf() _________________________________________________________

	bool GlSurface::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;

		return Surface::isInstanceOf(pClassName);
	}

	//____ className() ____________________________________________________________

	const char * GlSurface::className( void ) const
	{ 
		return CLASSNAME; 
	}

	//____ cast() _________________________________________________________________

	GlSurface_p GlSurface::cast( Object * pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return GlSurface_p( static_cast<GlSurface*>(pObject) );

		return 0;
	}

	//____ setScaleMode() __________________________________________________________

	void GlSurface::setScaleMode( ScaleMode mode )
	{
		switch( mode )
		{
			case ScaleMode::Interpolate:
				glBindTexture( GL_TEXTURE_2D, m_texture );
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				break;
				
			case ScaleMode::Nearest:
			default:
				glBindTexture( GL_TEXTURE_2D, m_texture );
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				break;
		}
		
		Surface::setScaleMode(mode);
	}

	//____ size() ______________________________________________________________

	Size GlSurface::size() const
	{
		return m_size;
	}

	//____ isOpaque() ______________________________________________________________

	bool GlSurface::isOpaque() const
	{
		if( m_internalFormat == GL_RGB )
			return true;

		return false;
	}

	//____ lock() __________________________________________________________________

	uint8_t * GlSurface::lock( AccessMode mode )
	{
		if( m_accessMode != AccessMode::None || mode == AccessMode::None )
			return 0;

    	m_pPixels = (uint8_t*) m_pBlob->data();
		m_lockRegion = Rect(0,0,m_size);
		m_accessMode = mode;
		return m_pPixels;
	}

	//____ lockRegion() __________________________________________________________________

	uint8_t * GlSurface::lockRegion( AccessMode mode, const Rect& region )
	{
		if( m_accessMode != AccessMode::None || mode == AccessMode::None )
			return 0;

		if( region.x + region.w > m_size.w || region.y + region.w > m_size.h || region.x < 0 || region.y < 0 )
			return 0;

    	m_pPixels = (uint8_t*) m_pBlob->data();
		m_lockRegion = region;
		m_accessMode = mode;
		return m_pPixels += (m_size.w*region.y+region.x)*m_pixelSize;
	}


	//____ unlock() ________________________________________________________________

	void GlSurface::unlock()
	{
		if(m_accessMode == AccessMode::None )
			return;

		glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER );

		if( m_accessMode != AccessMode::ReadOnly )
		{
			glBindTexture( GL_TEXTURE_2D, m_texture );
        	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, m_size.w, m_size.h, m_accessFormat, GL_UNSIGNED_BYTE, m_pBlob->data() );
	//		glTexSubImage2D( GL_TEXTURE_2D, 0, m_lockRegion.x, m_lockRegion.y, m_lockRegion.w, m_lockRegion.h, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
		}
		m_accessMode = AccessMode::None;
		m_pPixels = 0;
		m_lockRegion.w = 0;
		m_lockRegion.h = 0;
	}


	//____ pixel() ______________________________________________________________

	uint32_t GlSurface::pixel( Coord coord ) const
	{
		if( m_accessMode != AccessMode::WriteOnly )
		{
			uint32_t val;

				uint8_t * pPixel = (uint8_t*) m_pBlob->data();

				switch( m_pixelSize )
				{
					case 1:
						val = (uint32_t) *pPixel;
					case 2:
						val = (uint32_t) ((uint16_t*) pPixel)[0];
					case 3:
						val = ((uint32_t) pPixel[0]) + (((uint32_t) pPixel[1]) << 8) + (((uint32_t) pPixel[2]) << 16);
					default:
						val = *((uint32_t*) pPixel);
			}

			return val;
		}

		return 0;
	}



	//____ alpha() ____________________________________________________________

	uint8_t GlSurface::alpha( Coord coord ) const
	{
	if( m_pixelFormat.type == PixelType::BGRA_8 )
        {
		uint8_t * p = (uint8_t*) m_pBlob->data();
        return p[coord.y*m_pitch+coord.x*4+3];            
    }
    else
        return 255;
            
}


bool GlSurface::unload()
{
	if( m_texture == 0 )
		return true;
	
    glDeleteTextures( 1, &m_texture );
	m_texture = 0;
	
	assert(glGetError() == 0);	
    return true;
}

bool GlSurface::isLoaded()
{
	return (m_texture == 0);
}

void GlSurface::reload()
{
	assert(glGetError() == 0);

    glGenTextures( 1, &m_texture );
    glBindTexture( GL_TEXTURE_2D, m_texture );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	
    glTexImage2D( GL_TEXTURE_2D, 0, m_internalFormat, m_size.w, m_size.h, 0,
                 m_accessFormat, GL_UNSIGNED_BYTE, m_pBlob->data() );
    

	assert( glGetError() == 0);	
	}

} // namespace wg
