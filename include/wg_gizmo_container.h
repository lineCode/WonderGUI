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

#ifndef	WG_GIZMO_CONTAINER_DOT_H
#define	WG_GIZMO_CONTAINER_DOT_H

#ifndef WG_GIZMO_DOT_H
#	include <wg_gizmo.h>
#endif

#ifndef WG_GIZMO_PARENT_DOT_H
#	include <wg_gizmo_parent.h>
#endif

class WgPatches;

class WgGizmoContainer : public WgGizmoParent
{
	friend class WgHook;
	friend class WgFlexHook;
	friend class WgModalHook;

	friend class WgRoot;
	friend class WgGizmoFlexGeo;
	friend class WgGizmoModal;
	friend class WgOrderedLayout;
	friend class WgVBoxLayout;
	friend class WgMonotainer;

	public:

		void		SetFocusGroup( bool bFocusGroup ) { m_bFocusGroup = bFocusGroup; }
		bool		IsFocusGroup() const { return m_bFocusGroup; }

		void		SetRadioGroup( bool bRadioGroup ) { m_bRadioGroup = bRadioGroup; }
		bool		IsRadioGroup() const { return m_bRadioGroup; }

		void		SetTooltipGroup( bool bTooltipGroup ) { m_bTooltipGroup = bTooltipGroup; }
		bool		IsTooltipGroup() const { return m_bTooltipGroup; }

		void		SetMaskOp( WgMaskOp operation );
		WgMaskOp	MaskOp() const { return m_maskOp; }
		

		bool		IsGizmo() const;
		bool		IsRoot() const;

//		WgGizmo *	_castToGizmo();	TODO: Implement once we inherit from WgGizmo as we are supposed to.
		WgGizmoContainer *	CastToContainer();
		WgRoot *	CastToRoot();

	protected:
		WgGizmoContainer();
		virtual ~WgGizmoContainer() {};

		virtual void	_onEnable();
		virtual void	_onDisable();

		virtual void	_renderPatches( WgGfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches, Uint8 _layer );

	private:

		virtual WgHook* _firstHookWithGeo( WgRect& geo ) const = 0;
		virtual WgHook* _nextHookWithGeo( WgRect& geo, WgHook * pHook ) const = 0;

		bool 			_focusRequested( WgHook * pBranch, WgGizmo * pGizmoRequesting );	// Needed until WgGizmoContainer inherits from WgGizmo
		bool 			_focusReleased( WgHook * pBranch, WgGizmo * pGizmoReleasing );		// Needed until WgGizmoContainer inherits from WgGizmo

		virtual void	_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip );
		virtual void	_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip );
		virtual bool 	_onAlphaTest( const WgCoord& ofs );


		bool		m_bFocusGroup;
		bool		m_bRadioGroup;
		bool		m_bTooltipGroup;	// All Children+ belongs to the same tooltip group.
		bool		m_bSiblingsOverlap;	// Set if siblings (might be) overlapping each other (special considerations to be taken during rendering).
		WgMaskOp	m_maskOp;			// Specifies how container masks background.
};



#endif //WG_GIZMO_PARENT_DOT_H
