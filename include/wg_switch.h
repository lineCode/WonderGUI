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
#ifndef WG_SWITCH_DOT_H
#define WG_SWITCH_DOT_H


#ifndef WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif


//____ WgSwitch ____________________________________________________________

class WgSwitch : public WgWidget
{
public:
	WgSwitch();
	virtual ~WgSwitch();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgSwitch(); };

    void    SetValue( const float value );
    void    SetColor( WgColor color );
    void    SetOffColor(WgColor color );
    
	WgSize	PreferredSize() const;
    void    SetPreferredSize(WgSize size);

protected:
	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip  );
	bool	_onAlphaTest( const WgCoord& ofs );
	void	_onEnable();
	void	_onDisable();

private:
    int m_iValue;
    WgColor m_onColor;
    WgColor m_offColor;
    WgSize m_preferredSize;

};


#endif //WG_SWITCH_DOT_H
