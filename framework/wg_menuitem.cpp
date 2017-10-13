#include <wg_menuitem.h>

#include <wg_char.h>
#include <wg_texttool.h>

#include <wg_menu.h>



//____ WgMenuItem::SetVisible() _______________________________________________

void WgMenuItem::SetVisible( bool bVisible )
{
	if( m_pMyMenu && m_bVisible != bVisible )
	{
		m_bVisible = bVisible;
		Modified();
	}
}

//____ WgMenuItem::Modified() ___________________________________________

void WgMenuItem::Modified()
{
	if(m_pMyMenu)
		m_pMyMenu->_itemModified();
}

//____ WgMenuSeparator::Constructor ___________________________________________

WgMenuSeparator::WgMenuSeparator():WgMenuItem(SEPARATOR,0)
{
}


//____ WgMenuEntry::Constructor _______________________________________________

WgMenuEntry::WgMenuEntry() : WgMenuItem(ENTRY,INT_MIN)
{
	m_bEnabled		= true;
	m_navKey		= 0;
	m_accelModif	= WG_MODKEY_NONE;
	m_accelKey		= 0;
}

WgMenuEntry::WgMenuEntry(	int id, const WgString& text, const WgString& helpText, const WgBlocksetPtr& pIcon, Uint16 navKey,
							WgModifierKeys accelModif, Uint16 accelKey, const WgString& accelText ):WgMenuItem(ENTRY,id)
{

	m_text			= text;
	m_accelText		= accelText;
	m_helpText		= helpText;

	m_pIcon			= pIcon;
	m_bEnabled		= true;

	m_navKey		= navKey;
	m_accelModif	= accelModif;
	m_accelKey		= accelKey;
}

void WgMenuEntry::SetText(const WgString& text)
{
	m_text = text;
	Modified();
}

void WgMenuEntry::SetHelpText(const WgString& helpText)
{
	m_helpText		= helpText;
	Modified();
}

void WgMenuEntry::SetIcon(const WgBlocksetPtr& pIcon)
{
	m_pIcon		= pIcon;
	Modified();
}

void WgMenuEntry::SetNavKey(Uint16 navKey)
{
	m_navKey		= navKey;
	Modified();
}

void WgMenuEntry::SetAccelModifier(WgModifierKeys accelModif)
{
	m_accelModif	= accelModif;
	Modified();
}

void WgMenuEntry::SetAccelKey(Uint16 accelKey)
{
	m_accelKey		= accelKey;
	Modified();
}

void WgMenuEntry::SetAccelText(const WgString& accelText)
{
	m_accelText		= accelText;
	Modified();
}

//____ WgMenuEntry::Destructor _______________________________________________

WgMenuEntry::~WgMenuEntry()
{
}


//____ WgMenuCheckBox::Constructor ____________________________________________

WgMenuCheckBox::WgMenuCheckBox()
{
	m_type = CHECKBOX;
	m_bChecked = false;
}

WgMenuCheckBox::WgMenuCheckBox(	int id, const WgString& text, const WgString& helpText, Uint16 navKey, bool bChecked,
								WgModifierKeys accelModif, Uint16 accelKey, const WgString& accelText )
						:WgMenuEntry( id, text, helpText, 0, navKey, accelModif, accelKey, accelText )
{
	m_type = CHECKBOX;
	m_bChecked = bChecked;
}

//____ WgMenuRadioButton::Constructor _________________________________________

WgMenuRadioButton::WgMenuRadioButton()
{
	m_type = RADIOBUTTON;
	m_bSelected = false;
}

WgMenuRadioButton::WgMenuRadioButton(	int id, const WgString& text, const WgString& helpText,
										Uint16 navKey, bool bSelected,
										WgModifierKeys accelModif, Uint16 accelKey, const WgString& accelText )
						:WgMenuEntry( id, text, helpText, 0, navKey, accelModif, accelKey, accelText )
{
	m_type = RADIOBUTTON;
	m_bSelected = bSelected;
}

//____ WgMenuRadioButton::Select() ____________________________________________

bool WgMenuRadioButton::Select()
{
	// Select me and unselect any adjacent RadioButton.

	m_bSelected = true;

	WgMenuItem * pItem = this->Next();
	while( pItem && pItem->GetType() == RADIOBUTTON )
	{
		((WgMenuRadioButton*)pItem)->m_bSelected = false;
		pItem = pItem->Next();
	}

	pItem = this->Prev();
	while( pItem && pItem->GetType() == RADIOBUTTON )
	{
		((WgMenuRadioButton*)pItem)->m_bSelected = false;
		pItem = pItem->Prev();
	}


	//TODO: Need to force a redraw here...

	return true;
}


//____ WgMenuSubMenu::Constructor _____________________________________________

WgMenuSubMenu::WgMenuSubMenu()
{
	m_type = SUBMENU;
	m_pSubMenu = 0;
}


WgMenuSubMenu::WgMenuSubMenu(	int id, const WgString& text, const WgString& helpText,
								const WgBlocksetPtr& pIcon, Uint16 navKey, WgMenu * pSubMenu,
								WgModifierKeys accelModif, Uint16 accelKey, const WgString& accelText )
						:WgMenuEntry( id, text, helpText, pIcon, navKey, accelModif, accelKey, accelText )
{
	m_type = SUBMENU;
	m_pSubMenu = pSubMenu;
}

void WgMenuSubMenu::SetSubMenu(WgMenu* subMenu)
{
	m_pSubMenu= subMenu;
};

void WgMenuSubMenu::SetMyMenu( WgMenu * pMenu )
{
	WgMenuItem::SetMyMenu( pMenu );
}

