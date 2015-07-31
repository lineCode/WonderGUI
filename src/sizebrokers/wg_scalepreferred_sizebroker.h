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
#ifndef	WG_SCALEPREFERRED_SIZEBROKER_DOT_H
#define	WG_SCALEPREFERRED_SIZEBROKER_DOT_H

#ifndef WG_SIZEBROKER_DOT_H
#	include <wg_sizebroker.h>
#endif

namespace wg 
{
	
	class WgScalePreferredSizeBroker;
	typedef	WgStrongPtr<WgScalePreferredSizeBroker,WgSizeBroker_p>		WgScalePreferredSizeBroker_p;
	typedef	WgWeakPtr<WgScalePreferredSizeBroker,WgSizeBroker_wp>	WgScalePreferredSizeBroker_wp;
	
	class WgScalePreferredSizeBroker : public WgSizeBroker
	{
	public:
		static WgScalePreferredSizeBroker_p	create() { return WgScalePreferredSizeBroker_p(new WgScalePreferredSizeBroker()); }
	
		bool				isInstanceOf( const char * pClassName ) const;
		const char *		className( void ) const;
		static const char	CLASSNAME[];
		static WgScalePreferredSizeBroker_p	cast( const WgObject_p& pObject );
	
		int setItemLengths( WgSizeBrokerItem * pItems, int nItems, int totalLength ) const;
		int setPreferredLengths( WgSizeBrokerItem * pItems, int nItems ) const;
		bool mayAlterPreferredLengths() const;
	    
	protected:
		WgScalePreferredSizeBroker() {};
	    virtual ~WgScalePreferredSizeBroker() {};
	    
	};
	
	

} // namespace wg
#endif //WG_SCALEPREFERRED_SIZEBROKER_DOT_H
