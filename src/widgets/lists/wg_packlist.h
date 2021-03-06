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
#ifndef WG_PACKLIST_DOT_H
#define WG_PACKLIST_DOT_H
#pragma once

#include <functional>

#include <wg_list.h>
#include <wg_columnheader.h>
#include <wg_selectablechildren.h>



namespace wg 
{

	class PackList;
	typedef	StrongPtr<PackList>		PackList_p;
	typedef	WeakPtr<PackList>		PackList_wp;
	
	
	
	//____ PackListSlot ____________________________________________________________

	class PackListSlot : public ListSlot	/** @private */
	{
	public:
		PackListSlot() {}

		int				ofs;				// Offset in pixels for start of this list item.
		int				length;				// Length in pixels of this list item. Includes widget padding.
		int				prefBreadth;		// Prefereed breadth of this widget.
	};

	//____ PackListChildrenHolder() ___________________________________________

	class PackListChildrenHolder : public SelectableChildrenHolder /** @private */
	{
	public:
		virtual int		_getInsertionPoint(const Widget * pWidget) const = 0;
		virtual bool	_sortEntries() = 0;
		virtual bool	_hasSortFunction() const = 0;
	};


	class PackListChildren;
	typedef	StrongInterfacePtr<PackListChildren>	PackListChildren_p;
	typedef	WeakInterfacePtr<PackListChildren>	PackListChildren_wp;

	//____ PackListChildren ______________________________________________________

	class PackListChildren : public SelectableChildren<PackListSlot, PackListChildrenHolder>
	{
	public:

		/** @private */

		PackListChildren(SlotArray<PackListSlot> * pSlotArray, PackListChildrenHolder * pHolder) : SelectableChildren<PackListSlot, PackListChildrenHolder>(pSlotArray, pHolder) {}

		//.____ Misc __________________________________________________________

		inline PackListChildren_p	ptr() { return PackListChildren_p(this); }

		//.____ Content _______________________________________________________

		iterator	insertSorted(Widget * pWidget);

		//.____ Ordering ______________________________________________________

		void		sort();

	};


	//____ PackList ____________________________________________________________

	/**
	 * @brief	List widget aligning its entries horizontally or vertically.
	 * 			
	 * PackList provides a standard list, where all contained children are
	 * aligned ("packed") either vertically or horizontally and can be selected 
	 * using mouse or keyboard.
	 * 
	 */

	class PackList : public List, protected PackListChildrenHolder
	{
		friend class PackListChildren;
		friend class SelectableChildren<PackListSlot, PackList>;
		friend class HideableChildren<PackListSlot,PackList>;
		friend class DynamicChildren<PackListSlot,PackList>;
		friend class Children<PackListSlot,PackList>;
//		template<class T, class P> friend class Children;
	public:

		//.____ Creation __________________________________________

		static PackList_p	create() { return PackList_p(new PackList()); }
	
		//.____ Interfaces _______________________________________

		ColumnHeader		header;
		PackListChildren	children;

		//.____ Identification __________________________________________
	
		virtual bool		isInstanceOf( const char * pClassName ) const;
		virtual const char *className( void ) const;
		static const char	CLASSNAME[];
		static PackList_p	cast( Object * pObject );
		
		//.____ Geometry ____________________________________________

		Size				preferredSize() const;
		int					matchingHeight(int width) const;
		int					matchingWidth(int height) const;

		void				setOrientation( Orientation orientation );
		Orientation			orientation() const { return m_bHorizontal?Orientation::Horizontal:Orientation::Vertical; }

		bool				setMinEntrySize(Size min);
		bool				setMaxEntrySize(Size max);
		Size				minEntrySize() const { return m_minEntrySize; }
		Size				maxEntrySize() const { return m_maxEntrySize; }

		//.____ Behavior ________________________________________________________

		void				setSortOrder( SortOrder order );
		SortOrder			sortOrder() const { return m_sortOrder; }
	
		void				setSortFunction( std::function<int(const Widget *, const Widget *)> func );
		std::function<int(const Widget *, const Widget *)> sortFunction() const { return m_sortFunc; }
	

	protected:
		PackList();
		virtual ~PackList();
		Widget*			_newOfMyType() const { return new PackList(); };

		// Overloaded from Widget

		void			_collectPatches( Patches& container, const Rect& geo, const Rect& clip );
		void			_maskPatches( Patches& patches, const Rect& geo, const Rect& clip, BlendMode blendMode );
		void			_cloneContent( const Widget * _pOrg );
		void			_renderPatches( GfxDevice * pDevice, const Rect& _canvas, const Rect& _window, Patches * _pPatches );
		void			_render( GfxDevice * pDevice, const Rect& _canvas, const Rect& _window, const Rect& _clip );
		void			_setSize( const Size& size );
		void			_refresh();
	
		void			_receive( Msg * pMsg );
		Size			_windowPadding() const;


		// Overloaded from PackListChildrenHolder

		void			_didAddSlots(Slot * pSlot, int nb);
		void			_didMoveSlots(Slot * pFrom, Slot * pTo, int nb);
		void			_willRemoveSlots(Slot * pSlot, int nb);

		void			_hideSlots(Slot * pSlot, int nb);
		void			_unhideSlots(Slot * pSlot, int nb);

		void			_selectSlots(Slot * pSlot, int nb);
		void			_unselectSlots(Slot * pSlot, int nb);
		Object *		_object() { return this; }
		WidgetHolder *	_widgetHolder() { return this; }
		int				_getInsertionPoint(const Widget * pWidget) const;
		bool			_hasSortFunction() const { return m_sortFunc != nullptr; }
		bool			_sortEntries();


		// Overloaded from List

		ListSlot *		_findEntry(const Coord& ofs);
		void			_getEntryGeo(Rect& geo, const ListSlot * pSlot) const;

		Rect			_listArea() const;
		Rect			_listWindow() const;
		Rect			_listCanvas() const;

		void			_onEntrySkinChanged(Size oldPadding, Size newPadding);
		void			_onLassoUpdated(const Rect& oldLasso, const Rect& newLasso);

		ListSlot *		_beginSlots() const;
		ListSlot *		_endSlots() const;


		// Overloaded from Container

		void			_firstSlotWithGeo(SlotWithGeo& package) const;
		void			_nextSlotWithGeo(SlotWithGeo& package) const;

		Widget * 		_findWidget(const Coord& ofs, SearchMode mode);


		// Overloaded from WidgetHolder

		Coord		_childPos(Slot * pSlot) const;
		Size		_childSize(Slot * pSlot) const;

		void		_childRequestRender(Slot * pSlot);
		void		_childRequestRender(Slot * pSlot, const Rect& rect);
		void		_childRequestResize(Slot * pSlot);

		Widget *	_prevChild(Slot * pSlot) const;
		Widget *	_nextChild(Slot * pSlot) const;


		// Overloaded from ItemHolder

		Coord	_itemPos(const Item * pItem) const;
		Size	_itemSize(const Item * pItem) const;
		Rect	_itemGeo(const Item * pItem) const;

		void	_itemNotified(Item * pItem, ItemNotif notification, void * pData);


		// Internal

		void			_requestRenderChildren(PackListSlot * pBegin, PackListSlot * pEnd);

		void			_updateChildOfsFrom( PackListSlot* pSlot );
	
	
		void			_getChildGeo( Rect& geo, const PackListSlot * pSlot ) const;
		int				_getEntryAt( int pixelofs ) const;
		Rect			_headerGeo() const;
	
		void			_refreshHeader();
		void			_refreshList();
	
		Size			_paddedLimitedPreferredSize( Widget * pChild );
		int				_paddedLimitedMatchingHeight( Widget * pChild, int paddedWidth );
		int				_paddedLimitedMatchingWidth( Widget * pChild, int paddedHeight );
	
		void			_addToContentPreferredSize(int length, int breadth);
		void			_subFromContentPreferredSize(int length, int breadth);

		ColumnHeaderItem	m_header;
		SlotArray<PackListSlot>	m_children;

		bool				m_bHorizontal;
	
		SortOrder			m_sortOrder;
		std::function<int(const Widget *,const Widget *)> m_sortFunc;
		
		int					m_contentBreadth;
		int					m_contentLength;
	
		Size				m_entryPadding;
		Size				m_minEntrySize;
		Size				m_maxEntrySize;
	
		//----
	
		int				m_contentPreferredLength;
		int				m_contentPreferredBreadth;
		int				m_nbPreferredBreadthEntries;			// Number of entries whose preferred breadth are the same as m_preferredSize.	
	};
	
	

} // namespace wg
#endif //WG_PACKLIST_DOT_H
