/*=========================================================================

                         >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

                            -----------

  The WonderGUI Graphics Toolkit is free software you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation either
  version 2 of the License, or (at your option) any later version.

                            -----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_popuplayer.h>
#include <wg_util.h>
#include <wg_patches.h>
#include <wg_msgrouter.h>
#include <wg_panel.h>
#include <wg_base.h>
#include <wg_inputhandler.h>
#include <wg_slotarray.impl.h>


namespace wg 
{
	template class SlotArray<PopupSlot>;

	const char PopupLayer::CLASSNAME[] = {"PopupLayer"};

	//____ operator[] __________________________________________________________

	Widget& PopupChildren::operator[](int index) const
	{ 
		return * m_pHolder->m_popups.slot(index)->pWidget; 
	}

	//____ size() ______________________________________________________________
	
	int PopupChildren::size() const 
	{ 
		return m_pHolder->m_popups.size(); 
	} 

	//____ at() _______________________________________________________________
	
	Widget_p PopupChildren::at( int index) const 
	{ 
		if( index < 0 || index >= m_pHolder->m_popups.size() )
			return nullptr;

		return Widget_p(m_pHolder->m_popups.slot(index)->pWidget); 
	}
	
	//____ _object() ___________________________________________________________
	
	Object * PopupChildren::_object() const 
	{	
		return m_pHolder; 
	}


	//____ push() ________________________________________________

	void PopupChildren::push(Widget * _pPopup, Widget * _pOpener, const Rect& _launcherGeo, Origo _attachPoint, bool _bAutoClose, Size _maxSize )
	{
		m_pHolder->_addSlot( _pPopup, _pOpener, _launcherGeo, _attachPoint, _bAutoClose, _maxSize);
	}

	//____ pop() ________________________________________________

	void PopupChildren::pop(int nb)
	{
		if( nb <= 0 )
			return;
		
		nb = min(nb, m_pHolder->m_popups.size());
		
		m_pHolder->_removeSlots(0,nb); 
	}

	//____ pop() ________________________________________________

	void PopupChildren::pop(Widget * pPopup)
	{
		auto p = m_pHolder->m_popups.find(pPopup);

		if (p)
			m_pHolder->_removeSlots(0,m_pHolder->m_popups.index(p)+1);
	}



	//____ clear() ________________________________________________
	
	void PopupChildren::clear()
	{
		if( m_pHolder->m_popups.isEmpty() ) 
			return; 

		m_pHolder->_removeSlots(0,m_pHolder->m_popups.size()); 
	}


	
	//____ Constructor ____________________________________________________________
	
	PopupLayer::PopupLayer() : popups(this)
	{
	}
	
	//____ Destructor _____________________________________________________________
	
	PopupLayer::~PopupLayer()
	{
	}
	
	//____ isInstanceOf() _________________________________________________________
	
	bool PopupLayer::isInstanceOf( const char * pClassName ) const
	{ 
		if( pClassName==CLASSNAME )
			return true;
	
		return Layer::isInstanceOf(pClassName);
	}
	
	//____ className() ____________________________________________________________
	
	const char * PopupLayer::className( void ) const
	{ 
		return CLASSNAME; 
	}
	
	//____ cast() _________________________________________________________________
	
	PopupLayer_p PopupLayer::cast( Object * pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return PopupLayer_p( static_cast<PopupLayer*>(pObject) );
	
		return 0;
	}

	//____ _updateGeo() __________________________________________________________
	
	bool PopupLayer::_updateGeo(PopupSlot* pSlot, bool bInitialUpdate )
	{
		// Get size of parent and correct launcherGeo
	
	
		//
	
		Rect geo(0,0,Size::min(pSlot->pWidget->preferredSize(),Size::min(pSlot->maxSize,m_size)));
	
		switch( pSlot->attachPoint )
		{
			case Origo::NorthEast:					// Right side of launcherGeo, going down.
			{
				geo.x = pSlot->launcherGeo.right();
				geo.y = pSlot->launcherGeo.top();
				break;
			}
	
			case Origo::SouthEast:					// Right side of launcherGeo, going up.
			{
				geo.x = pSlot->launcherGeo.right();
				geo.y = pSlot->launcherGeo.bottom() - geo.h;
				break;
			}
	
			case Origo::NorthWest:					// Left-aligned above launcher.
			{
				geo.x = pSlot->launcherGeo.left();
				geo.y = pSlot->launcherGeo.top() - geo.h;
				break;
			}
	
			case Origo::SouthWest:					// Left-aligned below launcher.
			{
				geo.x = pSlot->launcherGeo.left();
				geo.y = pSlot->launcherGeo.bottom();
				break;
			}
	
			case Origo::West:						// Centered left of launcherGeo.
			{
				geo.x = pSlot->launcherGeo.left() - geo.w;
				geo.y = pSlot->launcherGeo.top() + pSlot->launcherGeo.h/2 - geo.h/2;
				break;
			}
	
			case Origo::North:						// Centered above launcherGeo.
			{
				geo.x = pSlot->launcherGeo.left() + pSlot->launcherGeo.w/2 + geo.w/2;
				geo.y = pSlot->launcherGeo.top() - geo.h;
				break;
			}
	
			case Origo::East:						// Centered right of launcherGeo.
			{
				geo.x = pSlot->launcherGeo.right();
				geo.y = pSlot->launcherGeo.top() + pSlot->launcherGeo.h/2 - geo.h/2;
				break;
			}
	
			case Origo::South:						// Centered below launcherGeo.
			{
				geo.x = pSlot->launcherGeo.left() + pSlot->launcherGeo.w/2 + geo.w/2;
				geo.y = pSlot->launcherGeo.bottom();
				break;
			}
	
		}
	
		// Adjust geometry to fit inside parent.
	
		if( geo.right() > m_size.w )
		{
			if( geo.left() == pSlot->launcherGeo.right() )
			{
				if( pSlot->launcherGeo.left() > m_size.w - pSlot->launcherGeo.right() )
				{
					geo.x = pSlot->launcherGeo.left() - geo.w;
					if( geo.x < 0 )
					{
						geo.x = 0;
						geo.w = pSlot->launcherGeo.left();
					}
				}
				else
					geo.w = m_size.w - geo.x;
			}
			else
				geo.x = m_size.w - geo.w;
		}
	
		if( geo.left() < 0 )
		{
			if( geo.right() == pSlot->launcherGeo.left() )
			{
				if( pSlot->launcherGeo.left() < m_size.w - pSlot->launcherGeo.right() )
				{
					geo.x = pSlot->launcherGeo.right();
					if( geo.right() > m_size.w )
						geo.w = m_size.w - geo.x;
				}
				else
				{
					geo.x = 0;
					geo.w = pSlot->launcherGeo.left();
				}
	
			}
			else
				geo.x = 0;
		}
	
		if( geo.bottom() > m_size.h )
		{
			if( geo.top() == pSlot->launcherGeo.bottom() )
			{
				if( pSlot->launcherGeo.top() > m_size.h - pSlot->launcherGeo.bottom() )
				{
					geo.y = pSlot->launcherGeo.top() - geo.h;
					if( geo.y < 0 )
					{
						geo.y = 0;
						geo.h = pSlot->launcherGeo.top();
					}
				}
				else
					geo.h = m_size.h - geo.y;
			}
			else
				geo.y = m_size.h - geo.h;
		}
	
		if( geo.top() < 0 )
		{
			if( geo.bottom() == pSlot->launcherGeo.top() )
			{
				if( pSlot->launcherGeo.top() < m_size.h - pSlot->launcherGeo.bottom() )
				{
					geo.y = pSlot->launcherGeo.bottom();
					if( geo.bottom() > m_size.h )
						geo.h = m_size.h - geo.y;
				}
				else
				{
					geo.y = 0;
					geo.h = pSlot->launcherGeo.bottom();
				}
			}
			else
				geo.y = 0;
		}
	
		// Update geometry if it has changed.
	
		if( geo != pSlot->geo )
		{
			if( !bInitialUpdate )
				_onRequestRender(pSlot->geo,pSlot);	
			pSlot->geo = geo;
			_onRequestRender(pSlot->geo,pSlot);	

			if( pSlot->pWidget->size() != geo.size() )
				pSlot->pWidget->_setSize(geo.size());

			return true;
		}
		else
			return false;
	}


	//____ _findWidget() ____________________________________________________________
	
	Widget *  PopupLayer::_findWidget( const Coord& ofs, SearchMode mode )
	{
		// MenuPanel has its own _findWidget() method since we need special treatment of
		// searchmode ACTION_TARGET when a menu is open.
	
		if( mode == SearchMode::ActionTarget && !m_popups.isEmpty() )
		{
			// In search mode ACTION_TARGET we limit our target to us, our menu-branches and the menu-opener if a menu is open.
	
			PopupSlot * pSlot = m_popups.first();
			Widget * pResult = 0;
	
			while( pSlot < m_popups.end() && !pResult )
			{
				if( pSlot->geo.contains( ofs ) )
				{
					if( pSlot->pWidget->isContainer() )
						pResult = static_cast<Container*>(pSlot->pWidget)->_findWidget( ofs - pSlot->geo.pos(), mode );
					else if( pSlot->pWidget->markTest( ofs - pSlot->geo.pos() ) )
						pResult = pSlot->pWidget;
				}
				pSlot++;
			}
	
			if( pResult == 0 )
			{
				// Check the root opener
				
				PopupSlot * pSlot = m_popups.last();
				if( pSlot->pOpener )
				{
					Widget * pOpener = pSlot->pOpener.rawPtr();
	
					Coord 	absPos 		= ofs + globalPos();
					Rect	openerGeo 	= pOpener->globalGeo();
	
					if( openerGeo.contains(absPos) && pOpener->markTest( absPos - openerGeo.pos() ) )
						pResult = pOpener;
				}
				
				// Fall back to us.
				
				if( pResult == 0 )
					pResult = this;
			}
			return pResult;
		}
		else
		{
			// For the rest of the modes we can rely on the default method.
	
			return Container::_findWidget( ofs, mode );
		}
	}
	
	//____ _onRequestRender() _____________________________________________________
	
	void PopupLayer::_onRequestRender( const Rect& rect, const LayerSlot * pSlot )
	{
		// Don not render anything if not visible

		if (pSlot && ((PopupSlot*)pSlot)->state == PopupSlot::State::OpeningDelay)
			return;

		// Clip our geometry and put it in a dirtyrect-list
	
		Patches patches;
		patches.add( Rect( rect, Rect(0,0,m_size)) );
	
		// Remove portions of dirty rect that are covered by opaque upper siblings,
		// possibly filling list with many small dirty rects instead.

		if( !m_popups.isEmpty() )
		{
			PopupSlot * pCover;

			if (pSlot)
				pCover = ((PopupSlot*)pSlot) - 1;
			else
				pCover = m_popups.last();

			while (pCover >= m_popups.begin())
			{
				if (pCover->geo.intersectsWith(rect) && pCover->state != PopupSlot::State::OpeningDelay && pCover->state != PopupSlot::State::Opening && pCover->state != PopupSlot::State::Closing)
					pCover->pWidget->_maskPatches(patches, pCover->geo, Rect(0, 0, INT_MAX, INT_MAX), _getBlendMode());

				pCover--;
			}
		}
		// Make request render calls
	
		for( const Rect * pRect = patches.begin() ; pRect < patches.end() ; pRect++ )
			_requestRender( * pRect );
	}

	//____ _renderPatches() ___________________________________________________

	class WidgetRenderContext
	{
	public:
		WidgetRenderContext() : pSlot(0) {}
		WidgetRenderContext(PopupSlot * pSlot, const Rect& geo) : pSlot(pSlot), geo(geo) {}

		PopupSlot *	pSlot;
		Rect		geo;
		Patches	patches;
	};

	void PopupLayer::_renderPatches(GfxDevice * pDevice, const Rect& _canvas, const Rect& _window, Patches * _pPatches)
	{

		// We start by eliminating dirt outside our geometry

		Patches 	patches(_pPatches->size());								// TODO: Optimize by pre-allocating?

		for (const Rect * pRect = _pPatches->begin(); pRect != _pPatches->end(); pRect++)
		{
			if (_canvas.intersectsWith(*pRect))
				patches.push(Rect(*pRect, _canvas));
		}


		// Render container itself

		for (const Rect * pRect = patches.begin(); pRect != patches.end(); pRect++)
			_render(pDevice, _canvas, _window, *pRect);


		// Render children

		Rect	dirtBounds = patches.getUnion();

		// Create WidgetRenderContext's for popups that might get dirty patches

		std::vector<WidgetRenderContext> renderList;

		auto pSlot = m_popups.begin();

		while (pSlot != m_popups.end() )
		{
			Rect geo = pSlot->geo + _canvas.pos();

			if (geo.intersectsWith(dirtBounds) && pSlot->state != PopupSlot::State::OpeningDelay)
				renderList.push_back(WidgetRenderContext(pSlot, geo));

			pSlot++;
		}

		// Go through WidgetRenderContexts, push and mask dirt

		for (unsigned int i = 0; i < renderList.size(); i++)
		{
			WidgetRenderContext * p = &renderList[i];

			p->patches.push(&patches);
			if( p->pSlot->state != PopupSlot::State::Opening && p->pSlot->state != PopupSlot::State::Closing )
				p->pSlot->pWidget->_maskPatches(patches, p->geo, p->geo, pDevice->blendMode());		//TODO: Need some optimizations here, grandchildren can be called repeatedly! Expensive!

			if (patches.isEmpty())
				break;
		}

		// Any dirt left in patches is for base child, lets render that first


		if (!patches.isEmpty())
			m_baseSlot.pWidget->_renderPatches(pDevice, _canvas, _window, &patches);


		// Go through WidgetRenderContexts and render the patches in reverse order (topmost popup rendered last).

		for (int i = renderList.size() - 1; i >= 0; i--)
		{
			WidgetRenderContext * p = &renderList[i];

			Color tint = Color::White;

			if (p->pSlot->state == PopupSlot::State::Opening)
				tint.a = 255 * p->pSlot->stateCounter / m_openingFadeMs;

			if (p->pSlot->state == PopupSlot::State::Closing)
				tint.a = 255 - (255 * p->pSlot->stateCounter / m_closingFadeMs);

			if (tint.a == 255)
				p->pSlot->pWidget->_renderPatches(pDevice, p->geo, p->geo, &p->patches);
			else
			{
				Color oldTint = pDevice->tintColor();
				pDevice->setTintColor(oldTint*tint);
				p->pSlot->pWidget->_renderPatches(pDevice, p->geo, p->geo, &p->patches);
				pDevice->setTintColor(oldTint);
			}
		}
	}


	//____ _maskPatches() _____________________________________________________

//	void PopupLayer::_maskPatches(Patches& patches, const Rect& geo, const Rect& clip, BlendMode blendMode)
//	{
//		Need to except children that are in states OpeningDelay, Opening and Closing.
//	}

	//____ _collectPatches() ___________________________________________________

//	void PopupLayer::_collectPatches(Patches& container, const Rect& geo, const Rect& clip)
//	{
//		Need to make sure patches are not collected for children in mode "OpeningDelay"
//		This might be handled by slotWithGeo() methods, depending on how what we choose.
//	}



	//____ _setSize() ___________________________________________________________
	
	void PopupLayer::_setSize( const Size& sz )
	{
		Layer::_setSize(sz);
	}
	
	//____ _cloneContent() ______________________________________________________
	
	void PopupLayer::_cloneContent( const Widget * _pOrg )
	{
		Layer::_cloneContent( _pOrg );
	}
	
	//____ _receive() ______________________________________________________________
	
	void PopupLayer::_receive( Msg * _pMsg )
	{
		Layer::_receive(_pMsg);
		
		int ms = static_cast<TickMsg*>(_pMsg)->timediff();

		switch( _pMsg->type() )
		{
			case MsgType::Tick:

				// Update state for all open popups

				for (auto& popup : m_popups)
				{
					switch (popup.state)
					{
					case PopupSlot::State::OpeningDelay:
						if (popup.stateCounter + ms < m_openingDelayMs)
						{
							popup.stateCounter += ms;
							break;
						}
						else
						{
							popup.state = PopupSlot::State::Opening;
							popup.stateCounter -= m_openingDelayMs;
							// No break here, let's continue down to opening...
						}
					case PopupSlot::State::Opening:
						popup.stateCounter += ms;
						_requestRender(popup.geo);
						if (popup.stateCounter >= m_openingFadeMs)
						{
							popup.stateCounter = 0;
							popup.state = popup.bAutoClose ? PopupSlot::State::PeekOpen : PopupSlot::State::FixedOpen;
						}
						break;

					case PopupSlot::State::ClosingDelay:
						if (popup.stateCounter + ms < m_closingDelayMs)
						{
							popup.stateCounter += ms;
							break;
						}
						else
						{
							popup.state = PopupSlot::State::Closing;
							popup.stateCounter -= m_closingDelayMs;
							// No break here, let's continue down to closing...
						}
					case PopupSlot::State::Closing:
						popup.stateCounter += ms;
						_requestRender(popup.geo);
						// Removing any closed popups is done in next loop
						break;
					default:
						break;
					}
					
				}

				// Close any popup that is due for closing.

				while (!m_popups.isEmpty() && m_popups.first()->state == PopupSlot::State::Closing && m_popups.first()->stateCounter >= m_closingFadeMs)
					_removeSlots(0, 1);

			break;

			case MsgType::MouseEnter:
			case MsgType::MouseMove:
			{
				if (m_popups.isEmpty())
					break;

				Coord 	pointerPos = InputMsg::cast(_pMsg)->pointerPos() - globalPos();

				// Top popup can be in state PeekOpen, which needs special attention.

				PopupSlot * pSlot = m_popups.first();
				if (pSlot && pSlot->state == PopupSlot::State::PeekOpen)
				{
					// Promote popup to state WeakOpen if pointer has entered its geo,
					// otherwise begin delayed closing if pointer has left launcherGeo.

					if (pSlot->geo.contains(pointerPos))
						pSlot->state = PopupSlot::State::WeakOpen;
					else if (!pSlot->launcherGeo.contains(pointerPos))
					{
						pSlot->state = PopupSlot::State::ClosingDelay;
						pSlot->stateCounter = 0;
					}
				}
			
				// A popup in state ClosingDelay should be promoted to
				// state PeekOpen if pointer has entered its launcherGeo and
				// to state WeakOpen if pointer has entered its geo.
				// Promoting to WeakOpen Should also promote any ancestor also in state ClosingDelay.

				for (auto& popup : m_popups)
				{
					if (popup.state == PopupSlot::State::ClosingDelay)
					{
						if (popup.launcherGeo.contains(pointerPos))
						{
							popup.state = PopupSlot::State::PeekOpen;
							popup.stateCounter = 0;
						}
						else if (popup.geo.contains(pointerPos))
						{
							PopupSlot * p = &popup;
							while (p != m_popups.end() && p->state == PopupSlot::State::ClosingDelay)
							{
								p->state = PopupSlot::State::WeakOpen;
								p->stateCounter = 0;
								p++;
							}
							break;		// Nothing more to do further down.
						}
					}
				}

				// If pointer has entered a selectable widget of a popup that isn't the top one
				// and all widgets between them have bAutoClose=true, they should all enter
				// state ClosingDelay (unless already in state Closing).


				Widget * pTop = m_popups.first()->pWidget;
				Widget * pMarked = _findWidget(pointerPos, SearchMode::ActionTarget);

				if (pMarked != this && pMarked->isSelectable() && m_popups.first()->bAutoClose)
				{
					// Trace hierarchy from marked to one of our children.

					while (pMarked->_parent() != this)
						pMarked = pMarked->_parent();

					//

					auto p = m_popups.first();
					while (p->bAutoClose && p->pWidget != pMarked)
					{
						if (p->state != PopupSlot::State::Closing && p->state != PopupSlot::State::ClosingDelay)
						{
							p->state = PopupSlot::State::ClosingDelay;
							p->stateCounter = 0;
						}
						p--;
					}
				}


			}				
			break;

/*
			case MsgType::MouseLeave:
			{
				// Top popup can be in state PeekOpen, which should begin closing when
				// pointer has left.

				PopupSlot * pSlot = m_popups.first();
				if (pSlot && pSlot->state == PopupSlot::State::PeekOpen)
				{
					pSlot->state = PopupSlot::State::ClosingDelay;
					pSlot->stateCounter = 0;
				}
			}
			break;
*/

			case MsgType::MouseRelease:
			{
				if (m_popups.isEmpty())
					break;					// Popup was removed already on the press.

				// Allow us to release the mouse within opener without closing any popups

				PopupSlot * pSlot = m_popups.first();
				if (pSlot->pOpener)
				{
					Widget * pOpener = pSlot->pOpener.rawPtr();

					Coord 	absPos = MouseReleaseMsg::cast(_pMsg)->pointerPos();
					Rect	openerGeo = pOpener->globalGeo();

					if (pOpener->markTest(absPos - openerGeo.pos()))
						break;
				}
						
				// DON'T BREAK! Continuing down to case MousePress on purpose.
			}
			case MsgType::MousePress:
			{
				auto pMsg = MousePressMsg::cast(_pMsg);

				auto pSource = Widget::cast(_pMsg->originalSource());
				if (!pSource || pSource == this )
					_removeSlots(0,m_popups.size());
				else if (pSource->isSelectable())
				{
					MsgRouter * pRouter = Base::msgRouter().rawPtr();

					if (pRouter)
						pRouter->post(SelectMsg::create(pSource));

					_removeSlots(0,m_popups.size());
				}

				_pMsg->swallow();
			}
			break;
	
			case MsgType::KeyPress:
			case MsgType::KeyRepeat:
			{
				KeyMsg_p pMsg = KeyMsg::cast(_pMsg);
	
				if( pMsg->translatedKeyCode() == Key::Escape )
				{
					if( !m_popups.isEmpty() )
					{
						_removeSlots(0,1);
						_pMsg->swallow();
					}
				}
			}
			break;
            
            default:
                break;
		}
	}
	
	//____ _stealKeyboardFocus() _________________________________________________
	
	void PopupLayer::_stealKeyboardFocus()
	{
		// Verify that we have a root
	
		if( !_root() )
			return;
	
		// Save old keyboard focus, which we assume belonged to previous menu in hierarchy.
	
		if( m_popups.size() < 2 )
			m_pKeyFocus = Base::inputHandler()->focusedWidget().rawPtr();
		else
			m_popups[1].pKeyFocus = Base::inputHandler()->focusedWidget().rawPtr();
	
		// Steal keyboard focus to top menu
	
		Widget * pWidget = m_popups.begin()->pWidget;

		_childRequestFocus( pWidget->_slot(), pWidget );
	}
	
	//____ _restoreKeyboardFocus() _________________________________________________
	
	void PopupLayer::_restoreKeyboardFocus()
	{
		// Verify that we have a root
	
		if( !_root() )
			return;
	
		//
	
		if( m_popups.isEmpty() )
			_holder()->_childRequestFocus( _slot(), m_pKeyFocus.rawPtr() );
		else
			_holder()->_childRequestFocus( _slot(),  m_popups.begin()->pKeyFocus.rawPtr() );
	}

	//____ _childRequestResize() _______________________________________________

	void PopupLayer::_childRequestResize(Slot * pSlot)
	{
		if( pSlot == &m_baseSlot )
			_requestResize();
		else
			_updateGeo( (PopupSlot *) pSlot );
	}

	//____ _addSlot() ____________________________________________________________

	void PopupLayer::_addSlot(Widget * _pPopup, Widget * _pOpener, const Rect& _launcherGeo, Origo _attachPoint, bool _bAutoClose, Size _maxSize)
	{
		PopupSlot * pSlot = m_popups.insert(0);
		pSlot->pOpener = _pOpener;
		pSlot->launcherGeo = _launcherGeo;
		pSlot->attachPoint = _attachPoint;
		pSlot->bAutoClose = _bAutoClose;
		pSlot->state = PopupSlot::State::OpeningDelay;
		pSlot->stateCounter = 0;
		pSlot->maxSize = _maxSize;

		pSlot->replaceWidget(this, _pPopup);

		_updateGeo(pSlot, true);
		_stealKeyboardFocus();

		if (m_tickRouteId == 0)
			m_tickRouteId = Base::msgRouter()->addRoute(MsgType::Tick, this);
	}


	//____ _removeSlots() __________________________________________________

	void PopupLayer::_removeSlots(int ofs, int nb)
	{
		MsgRouter * pEH = Base::msgRouter().rawPtr();
	
		PopupSlot * pSlot = m_popups.slot(ofs);

		nb = min(nb, m_popups.size());

		for(int i = 0 ; i < nb ; i++ )
		{
			if( pEH )
				pEH->post( new PopupClosedMsg( pSlot[i].pWidget, pSlot[i].pOpener ) );
			_requestRender(pSlot[i].geo);
		}
						
		m_popups.remove(ofs, nb);
		_restoreKeyboardFocus();

		if (m_popups.isEmpty())
		{
			Base::msgRouter()->deleteRoute(m_tickRouteId);
			m_tickRouteId = 0;
		}
	}

	//____ _beginLayerSlots() __________________________________________________

	LayerSlot * PopupLayer::_beginLayerSlots() const
	{
		return m_popups.begin();
	}

	//____ _endLayerSlots() __________________________________________________

	LayerSlot * PopupLayer::_endLayerSlots() const
	{
		return m_popups.end();
	}

	//____ _sizeOfLayerSlot() __________________________________________________

	int PopupLayer::_sizeOfLayerSlot() const
	{
		return sizeof(PopupSlot);
	}




} // namespace wg
