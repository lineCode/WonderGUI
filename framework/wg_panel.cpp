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

#include <vector>
#include <wg_panel.h>
#include <wg_util.h>

#include <wg_patches.h>

#ifndef WG_GFXDEVICE_DOT_H
#	include <wg_gfxdevice.h>
#endif

//____ Constructor _____________________________________________________________

WgPanel::WgPanel() : m_bFocusGroup(false), m_bRadioGroup(false), m_bTooltipGroup(false), m_maskOp(WG_MASKOP_RECURSE)
{
}

//____ IsPanel() ______________________________________________________________

bool WgPanel::IsPanel() const
{
	return true;
}

//____ CastToPanel() _______________________________________________________

WgPanel * WgPanel::CastToPanel()
{
	return this;
}

const WgPanel * WgPanel::CastToPanel() const
{
	return this;
}

//____ SetMaskOp() _____________________________________________________________

void WgPanel::SetMaskOp( WgMaskOp operation )
{
	if( operation != m_maskOp )
	{
		m_maskOp = operation;
		_requestRender();
	}
}

//____ SetSkin() _______________________________________________________________

void WgPanel::SetSkin( const WgSkinPtr& pSkin )
{
	m_pSkin = pSkin;
	_requestRender();
}

//____ _onCloneContent() _______________________________________________________

void WgPanel::_onCloneContent( const WgPanel * _pOrg )
{
	m_bFocusGroup 		= _pOrg->m_bFocusGroup;
	m_bRadioGroup 		= _pOrg->m_bRadioGroup;
	m_bTooltipGroup 	= _pOrg->m_bTooltipGroup;
	m_maskOp 			= _pOrg->m_maskOp;

	WgContainer::_onCloneContent( _pOrg );
}

//____ _onCollectPatches() _______________________________________________________

void WgPanel::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
	if( m_pSkin )
		container.Add( WgRect( geo, clip ) );
	else
		WgContainer::_onCollectPatches( container, geo, clip );

}


//____ _onMaskPatches() __________________________________________________________

void WgPanel::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
    if( m_pSkin && m_pSkin->IsOpaque() )
    {
        patches.Sub( WgRect(geo,clip) );
        return;
    }

    switch( m_maskOp )
    {
        case WG_MASKOP_RECURSE:
        {
            WgRect childGeo;
            WgPanelHook * p = static_cast<WgPanelHook*>(_firstHookWithGeo( childGeo ));
            WgRect myClip(geo, clip);

            while(p)
            {
                if( p->IsVisible() )
                    p->Widget()->_onMaskPatches( patches, childGeo + geo.Pos(), myClip, blendMode );
                p = static_cast<WgPanelHook*>(_nextHookWithGeo( childGeo, p ));
            }
            break;
        }
        case WG_MASKOP_SKIP:
            break;
        case WG_MASKOP_MASK:
            patches.Sub( WgRect(geo,clip) );
            break;
    }
}

//____ WgPanelHook::Parent() __________________________________________________

WgPanel* WgPanelHook::Parent() const 
{ 
	return static_cast<WgPanel*>(_parent()); 
}


//____ WgPanelHook::SetVisible() _____________________________________________________________

bool WgPanelHook::SetVisible( bool bVisible )
{
	if( bVisible != m_bVisible )
	{
		if( bVisible )
		{
			m_bVisible = true;
			_requestRender();
		}
		else
		{
			_requestRender();
			m_bVisible = false;
		}		
	}
	return true;
}

//____ WgPanelHook::SetPadding() ______________________________________________

bool WgPanelHook::SetPadding( WgBorders padding )
{
	if( padding != m_padding )
	{
		m_padding = padding;
		_requestResize();
	}
	return true;
}

//____ WgPanelHook::_sizeFromPolicy() ________________________________________________________

WgSize WgPanelHook::_sizeFromPolicy( WgSize specifiedSize, WgSizePolicy widthPolicy, WgSizePolicy heightPolicy, int scale ) const
{
	WgSize	defaultSize = _paddedPreferredPixelSize(scale);

	WgSize	sz;

	switch( widthPolicy )
	{
		case WG_DEFAULT:
		{
			sz.h = WgUtil::SizeFromPolicy( defaultSize.h, specifiedSize.h, heightPolicy );
			sz.w = _paddedMatchingPixelWidth(sz.h, scale);
			break;
		case WG_BOUND:
			sz.w = specifiedSize.w;
			sz.h = WgUtil::SizeFromPolicy( _paddedMatchingPixelHeight(sz.w, scale), specifiedSize.h, heightPolicy );
			break;
		case WG_CONFINED:
			if( defaultSize.w > specifiedSize.w )
			{
				sz.w = specifiedSize.w;
				sz.h = WgUtil::SizeFromPolicy( _paddedMatchingPixelHeight(sz.w, scale), specifiedSize.h, heightPolicy );
			}
			else
			{
				sz.h = WgUtil::SizeFromPolicy( defaultSize.h, specifiedSize.h, heightPolicy );
				sz.w = _paddedMatchingPixelWidth(sz.h, scale);
				if( sz.w > specifiedSize.w )
					sz.w = specifiedSize.w;
			}
			break;
		case WG_EXPANDED:
			if( defaultSize.w < specifiedSize.w )
			{
				sz.w = specifiedSize.w;
				sz.h = WgUtil::SizeFromPolicy( _paddedMatchingPixelHeight(sz.w, scale), specifiedSize.h, heightPolicy );
			}
			else
			{
				sz.h = WgUtil::SizeFromPolicy( defaultSize.h, specifiedSize.h, heightPolicy );
				sz.w = _paddedMatchingPixelWidth(sz.h, scale);
				if( sz.w < specifiedSize.w )
					sz.w = specifiedSize.w;
			}
			break;
		}
	}

	return sz;
}

WgSize WgPanelHook::_paddedPreferredPixelSize(int scale) const
{
    return m_pWidget->PreferredPixelSize() + m_padding.Scale(scale);
}

WgSize WgPanelHook::_paddedMinPixelSize(int scale) const
{
    return m_pWidget->MinPixelSize() + m_padding.Scale(scale);
}

WgSize WgPanelHook::_paddedMaxPixelSize(int scale) const
{
        return m_pWidget->MaxPixelSize() + m_padding.Scale(scale);
}

int WgPanelHook::_paddedMatchingPixelWidth( int paddedHeight, int scale ) const
{
    WgBorders padding = m_padding.Scale(scale);

    return m_pWidget->MatchingPixelWidth( paddedHeight - padding.Height() ) + padding.Width();
}

int WgPanelHook::_paddedMatchingPixelHeight( int paddedWidth, int scale ) const
{
    WgBorders padding = m_padding.Scale(scale);

    return m_pWidget->MatchingPixelHeight( paddedWidth - padding.Width() ) + padding.Height();
}
