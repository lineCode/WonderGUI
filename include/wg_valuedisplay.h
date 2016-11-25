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

#ifndef	WG_VALUEDISPLAY_DOT_H
#define	WG_VALUEDISPLAY_DOT_H

#ifndef	WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif

#ifndef	WG_TEXT_DOT_H
#	include <wg_text.h>
#endif

#ifndef WG_VALUEFORMAT_DOT_H
#	include <wg_valueformat.h>
#endif

#ifndef WG_TEXTPROP_DOT_H
#	include <wg_textprop.h>
#endif

#ifndef WG_INTERFACE_VALUEHOLDER_DOT_H
#	include <wg_interface_valueholder.h>
#endif

class	WgFont;

class WgValueDisplay : public WgWidget, public Wg_Interface_ValueHolder
{
public:
	WgValueDisplay();
	virtual ~WgValueDisplay();
	virtual const char * Type() const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgValueDisplay(); };


	//____ Methods __________________________________________

	void	SetTextProperties( const WgTextpropPtr& _pProps );
	void	SetFormat( const WgValueFormat& format );

	WgTextpropPtr	TextProperties() { return m_text.getProperties(); }
	WgValueFormat	Format() { return m_format; }
	virtual const WgValueFormat&	GetFormat() const { return m_format; }

	WgSize	PreferredSize() const;


protected:
	void	_onRefresh();
	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip );
	void	_onEnable();
	void	_onDisable();

	void	_regenText();

	WgWidget* _getWidget() { return this; }	// Needed for WgSilderTarget.

private:
	void	_valueModified();				///< Called when value has been modified.
	void	_rangeModified();				///< Called when range (and thus fractional value) has been modified.

	WgValueFormat	m_format;
	WgText			m_text;
};


#endif // WG_VALUEDISPLAY_DOT_H
