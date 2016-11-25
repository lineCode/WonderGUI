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

#include <wg_extendedskin.h>


//____ SetContentPadding() ____________________________________________________

void WgExtendedSkin::SetContentPadding( WgBorders padding )
{
	m_contentPadding = padding;
}


//____ SetContentShift() ______________________________________________________

void WgExtendedSkin::SetContentShift( WgStateEnum state, WgCoord shift )
{
	int index = _stateToIndex(state);
	m_contentShift[index] = shift;
}

//____ SetHoveredContentShift() _______________________________________________

void WgExtendedSkin::SetHoveredContentShift( WgCoord shift )
{
	m_contentShift[_stateToIndex(WG_STATE_HOVERED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_FOCUSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_FOCUSED_SELECTED)] = shift;

	m_contentShift[_stateToIndex(WG_STATE_PRESSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED_SELECTED)] = shift;
}

//____ SetPressedContentShift() _______________________________________________

void WgExtendedSkin::SetPressedContentShift( WgCoord shift )
{
	m_contentShift[_stateToIndex(WG_STATE_PRESSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED_SELECTED)] = shift;
}

//____ SetSelectedContentShift() ______________________________________________

void WgExtendedSkin::SetSelectedContentShift( WgCoord shift )
{
	m_contentShift[_stateToIndex(WG_STATE_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_FOCUSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_FOCUSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED_SELECTED)] = shift;
}

//____ SetFocusedContentShift() _______________________________________________

void WgExtendedSkin::SetFocusedContentShift( WgCoord shift )
{
	m_contentShift[_stateToIndex(WG_STATE_FOCUSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_FOCUSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_FOCUSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_HOVERED_FOCUSED_SELECTED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED)] = shift;
	m_contentShift[_stateToIndex(WG_STATE_PRESSED_FOCUSED_SELECTED)] = shift;
}

//____ MinSize() ______________________________________________________________

WgSize WgExtendedSkin::MinSize() const
{
	return WgSize(m_contentPadding.Width(), m_contentPadding.Height() );
}

//____ MinSize() ______________________________________________________________

WgSize WgExtendedSkin::PreferredSize() const
{
	return WgSize(m_contentPadding.Width(), m_contentPadding.Height() );
}

//____ SizeForContent() _______________________________________________________

WgSize WgExtendedSkin::SizeForContent( const WgSize contentSize ) const
{
	return contentSize + m_contentPadding;
}

//____ ContentRect() __________________________________________________________

WgRect WgExtendedSkin::ContentRect( const WgRect& canvas, WgState state ) const
{
	return (canvas - m_contentPadding) + m_contentShift[_stateToIndex(state)];
}

