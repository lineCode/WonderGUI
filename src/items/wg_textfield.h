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

#ifndef	WG_TEXTFIELD_DOT_H
#define WG_TEXTFIELD_DOT_H

#ifndef	WG_TYPES_DOT_H
#	include <wg_types.h>
#endif

#ifndef WG_PRINTABLEFIELD_DOT_H
#	include <wg_printablefield.h>
#endif


#ifndef WG_BASE_DOT_H
#	include <wg_base.h>
#endif

namespace wg 
{
	
	class String;
	class CharSeq;
	class CharBuffer;
	
	//____ TextHolder ___________________________________________________________
	
	struct TextHolder : public PrintableHolder
	{
	};
	
	//____ TextField __________________________________________________________________
	
	class TextField : public PrintableField
	{
	public:
		TextField( TextHolder * pHolder );
	
		virtual void		clear();
	
		virtual void		set( const CharSeq& seq );
		virtual void		set( const CharBuffer * buffer );
		virtual void		set( const String& str );
	
		virtual int			append( const CharSeq& seq );
		virtual int			insert( int ofs, const CharSeq& seq );
		virtual int			replace( int ofs, int nDelete, const CharSeq& seq );
		virtual int			remove( int ofs, int len );
	
	
		inline int			length() const { return m_charBuffer.length(); }
		inline bool			isEmpty() const { return length()==0?true:false; }
		TextLink_p			getMarkedLink() const;
	
		
		virtual void		setCharStyle( const TextStyle_p& pStyle );
		virtual void		setCharStyle( const TextStyle_p& pStyle, int ofs, int len);

		virtual void		clearCharStyle();
		virtual void		clearCharStyle( int ofs, int len );
	
	protected:
	
	};
	

} // namespace wg
#endif //WG_TEXTFIELD_DOT_H
