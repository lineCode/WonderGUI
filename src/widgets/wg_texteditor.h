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

#ifndef	WG_TEXTEDITOR_DOT_H
#define	WG_TEXTEDITOR_DOT_H


#ifndef WG_WIDGET_DOT_H
#	include <wg_widget.h>
#endif

#ifndef	WG_EDITTEXT_DOT_H
#	include <wg_edittext.h>
#endif

#ifndef WG_CARETINSTANCE_DOT_H
#	include <wg_caretinstance.h>
#endif

namespace wg 
{
	
	class WgTextEditor;
	typedef	WgStrongPtr<WgTextEditor,WgWidget_p>		WgTextEditor_p;
	typedef	WgWeakPtr<WgTextEditor,WgWidget_wp>	WgTextEditor_wp;
	
	class WgTextEditor:public WgWidget, protected WgLegacyTextHolder
	{
	public:
		static WgTextEditor_p	create() { return WgTextEditor_p(new WgTextEditor()); }
	
		bool		isInstanceOf( const char * pClassName ) const;
		const char *className( void ) const;
		static const char	CLASSNAME[];
		static WgTextEditor_p	cast( const WgObject_p& pObject );
	
		//____ Interfaces ______________________________________
	
		WgEditText		text;
	
		//____ Methods __________________________________________
	
		inline void		SetMaxLines( int nLines ) { m_maxLines = nLines; }
		inline int		MaxLines() { return m_maxLines; }
	
		int		InsertTextAtCursor( const WgCharSeq& str );
		bool	InsertCharAtCursor( Uint16 c );
	
		virtual void			setEditMode(WgTextEditMode mode);
		virtual WgTextEditMode	editMode() const { return m_text.editMode(); }
	
		WgPointerStyle		PointerStyle() const;
		WgString			TooltipString() const;
	
		int		matchingHeight( int width ) const;
		WgSize	preferredSize() const;
		bool	IsAutoEllipsisDefault() const { return true; };
	
		bool	isEditable() const { return m_text.isEditable(); }
		bool	isSelectable() const { return m_text.isSelectable(); }
	
	protected:
		WgTextEditor();
		virtual ~WgTextEditor();
		virtual WgWidget* _newOfMyType() const { return new WgTextEditor(); };
	
		void	_onCloneContent( const WgWidget * _pOrg );
		void	_onRender( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, const WgRect& _clip );
		void	_onNewSize( const WgSize& size );
		void	_onRefresh();
		void	_onMsg( const WgMsg_p& pMsg );
		void	_onStateChanged( WgState oldState );
		void	_onSkinChanged( const WgSkin_p& pOldSkin, const WgSkin_p& pNewSkin );
	
		WgObject * _object() { return this; }
		void	_onFieldDirty( WgField * pField );
		void	_onFieldResize( WgField * pField );
	private:
	
		bool	_insertCharAtCursor( Uint16 c );
	
	
		WgLegacyTextField			m_text;
		bool				m_bHasFocus;
		int					m_maxLines;
		bool				m_bResetCursorOnFocus;
		WgRouteId			m_tickRouteId;
	};
	
	
	
	
	

} // namespace wg
#endif // WG_TEXTDISPLAY_DOT_H
