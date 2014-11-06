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

#ifndef WG_SLIDESPAN_DOT_H
#define WG_SLIDESPAN_DOT_H

#ifndef WG_SPAN_DOT_H
#	include <wg_span.h>
#endif


class WgSlideSpan;
typedef	WgIStrongPtr<WgSlideSpan,WgSpanPtr>		WgSlideSpanPtr;
typedef	WgIWeakPtr<WgSlideSpan,WgSpanWeakPtr>	WgSlideSpanWeakPtr;


class WgSlideSpan : public WgSpan
{
public:
	WgSlideSpan(WgSpanItem* pItem) : WgSpan(pItem) {}

	virtual bool				IsInstanceOf( const char * pClassName ) const;
	virtual const char *		ClassName( void ) const;
	static const char			CLASSNAME[];
	static WgSlideSpanPtr		Cast( const WgInterfacePtr& pInterface );				// Provided just for completeness sake.
	inline WgSlideSpanPtr		Ptr() { return WgSlideSpanPtr(_object(),this); }


	inline void	SetBegin( int begin ) { m_pItem->SetBegin(begin); }
	inline void	SetRelativePos( float pos ) { m_pItem->SetRelativePos(pos); }
	inline void	SetRelativeBegin( float begin ) { m_pItem->SetRelativeBegin(begin); }

	inline bool	StepForward() { return m_pItem->StepForward(); }
	inline bool	StepBackward() { return m_pItem->StepBackward(); }
	inline bool	SkipForward() { return m_pItem->SkipForward(); }
	inline bool	SkipBackward() { return m_pItem->SkipBackward(); }

};


#endif //WG_SPAN_DOT_H
