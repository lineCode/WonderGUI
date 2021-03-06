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

#include <assert.h>
#include <limits.h>

#include <wg_resources_xml.h>
#include <wg_resource_factory_xml.h>
#include <wg_resourceserializer_xml.h>
#include <wg_font.h>
#include <wg_cursor.h>
#include <wg_vectorglyphs.h>
#include <wg_bitmapglyphs.h>
#include <wg_util.h>
#include <wg_valueformat.h>
#include <wg_textmanager.h>
#include <wg_skinmanager.h>
#include <wdg_button.h>
#include <wdg_checkbox2.h>
#include <wdg_combobox.h>
#include <wdg_container.h>
#include <wdg_fill.h>
#include <wdg_gridview.h>
#include <wdg_listview.h>
#include <wdg_lodder.h>
#include <wdg_menu.h>
#include <wdg_menubar.h>
#include <wdg_pixmap.h>
#include <wdg_radiobutton2.h>
#include <wdg_root.h>
#include <wdg_shader.h>
#include <wdg_tableview.h>
#include <wdg_tablist.h>
#include <wdg_text.h>
#include <wdg_editline.h>
#include <wdg_value.h>
#include <wdg_editvalue.h>
#include <wdg_textview.h>
#include <wdg_refreshbutton.h>
#include <wdg_ysplitter.h>

#include <wg_item_pixmap.h>
#include <wg_item_row.h>
#include <wg_item_stack.h>
#include <wg_item_text.h>
#include <wg_item_wraptext.h>

//
// TODO
//
// * Text (properly)
// * Localized text (properly)
// * Support for Wdg_TabList::SourceType sub tags
// * combine VDrag with HDrag. same code
// * additional info for items inside WgItemStack container: origo and offset
// * "unknown" tags and attributes should not be removed when existing metadata is serialized

#ifdef _FINAL
#	define ASSERT(expr, str) ((void)0)
#else
#	ifdef WIN32
#		define ASSERT(expr, str) do { if(!(expr)) {s.Error(str, __FILE__, __LINE__); __asm { int 3 } } } while(0)
#	else
#		define ASSERT(expr, str) do { if(!(expr)) {s.Error(str, __FILE__, __LINE__); } } while(0)
#	endif
#endif
#define VERIFY(expr, str) do { if(!(expr)) { s.Error(str, __FILE__, __LINE__); return; } } while(0)
#define WARNIF(expr, str) do { if(expr) { s.Warning(str, __FILE__, __LINE__); } } while(0)

namespace
{
	// convenience functions

	template<typename T1>
	bool WriteDiffAttr(WgResourceSerializerXML& s, const WgXmlNode& diffNode, const std::string& name, T1 a, T1 defA)
	{
		// Write if the attribute already exist or if the values differ from default
		if(diffNode.HasAttribute(name) || a != defA)
		{
			s.AddAttribute(name, WgUtil::ToString(a));
			return true;
		}
		return false;
	}

	template<typename T1>
	bool WriteDiffAttr(WgResourceSerializerXML& s, const WgXmlNode& diffNode, const std::string& name, T1 a, T1 b, T1 defA, T1 defB)
	{
		// Write if the attribute already exist or if the values differ from default
		if(diffNode.HasAttribute(name) || a != defA || b != defB)
		{
			s.AddAttribute(name, WgUtil::ToString(a, b));
			return true;
		}
		return false;
	}

	template<typename T1>
	bool WriteDiffAttr(WgResourceSerializerXML& s, const WgXmlNode& diffNode, const std::string& name, T1 a, T1 b, T1 c, T1 d, T1 defA, T1 defB, T1 defC, T1 defD)
	{
		if(diffNode.HasAttribute(name) || a != defA || b != defB || c != defC || d != defD)
		{
			s.AddAttribute(name, WgUtil::ToString(a, b, c, d));
			return true;
		}
		return false;
	}

	template<typename T1>
	bool WriteDiffAttr(WgResourceSerializerXML& s, const WgXmlNode& diffNode, const std::string& name, T1 a, T1 b, T1 c, T1 d, T1 e, T1 f, T1 defA, T1 defB, T1 defC, T1 defD, T1 defE, T1 defF)
	{
		if(diffNode.HasAttribute(name) || a != defA || b != defB || c != defC || d != defD || e != defE || f != defF)
		{
			s.AddAttribute(name, WgUtil::ToString(a, b, c, d, e, f));
			return true;
		}
		return false;
	}

	void WriteBlockSetAttr(WgResourceSerializerXML& s, WgBlockSetPtr blockSet, const std::string& attr)
	{
		if(blockSet)
		{
			std::string id = s.ResDb()->FindBlockSetId(blockSet);
			if(id.empty())
			{
				id = s.ResDb()->GenerateName(blockSet);
				s.ResDb()->AddBlockSet(id, blockSet);
			}
			s.AddAttribute(attr, id);
		}
	}

	void WriteColorSetAttr(WgResourceSerializerXML& s, WgColorSetPtr pColorSet, const std::string& attr)
	{
		if(pColorSet)
		{
			std::string id = s.ResDb()->FindColorSetId(pColorSet);
			if(id.empty())
			{
				id = s.ResDb()->GenerateName(pColorSet);
				s.ResDb()->AddColorSet(id, pColorSet);
			}
			s.AddAttribute(attr, id);
		}
	}


	void WriteTextPropAttr(WgResourceSerializerXML& s, WgTextPropPtr prop, const std::string& attr)
	{
		if(prop)
		{
			std::string id = s.ResDb()->FindTextPropId(prop);
			if(id.empty())
			{
				id = s.ResDb()->GenerateName(prop);
				s.ResDb()->AddTextProp(id, prop);
			}
			s.AddAttribute(attr, id);
		}
	}

	void WriteFontAttr(WgResourceSerializerXML& s, WgFont* font, const std::string& attr)
	{
		if(font)
		{
			std::string id = s.ResDb()->FindFontId(font);
			if(id.empty())
			{
				id = s.ResDb()->GenerateName(font);
				s.ResDb()->AddFont(id, font);
			}
			s.AddAttribute(attr, id);
		}
	}

	void WriteSurfaceAttr(WgResourceSerializerXML& s, const WgSurface* surf, const std::string& attr)
	{
		if(surf)
		{
			std::string id = s.ResDb()->FindSurfaceId(surf);
			if(id.empty())
			{
				id = s.ResDb()->GenerateName(surf);
				s.ResDb()->AddSurface(id, (WgSurface*)surf, id);// yes, ugly
			}
			s.AddAttribute(attr, id);
		}
	}

	void WriteGlyphSetAttr(WgResourceSerializerXML& s, const WgGlyphSet* glyphSet, const std::string& attr)
	{
		if(glyphSet)
		{
			std::string id = s.ResDb()->FindGlyphSetId(glyphSet);
			if(id.empty())
			{
				id = s.ResDb()->GenerateName(glyphSet);
				s.ResDb()->AddGlyphSet(id, (WgGlyphSet*)glyphSet, id);// yes, ugly
			}
			s.AddAttribute(attr, id);
		}
	}

	void WriteCursorAttr(WgResourceSerializerXML& s, const WgCursor* cursor, const std::string& attr)
	{
		if(cursor)
		{
			std::string id = s.ResDb()->FindCursorId(cursor);
			if(id.empty())
			{
				id = s.ResDb()->GenerateName(cursor);
				s.ResDb()->AddCursor(id, (WgCursor*)cursor);// yes, ugly
			}
			s.AddAttribute(attr, id);
		}
	}

	void WriteWidgetRefAttr(WgResourceSerializerXML& s, const WgWidget* wdg, const std::string& attr)
	{
		if(wdg)
		{
			std::string id = s.ResDb()->FindWidgetId(wdg);
			if(id.empty())
			{
				id = s.ResDb()->GenerateName(wdg);
				s.ResDb()->AddWidget(id, (WgWidget*)wdg);// yes, ugly
			}
			s.AddAttribute(attr, id);
		}
	}

	template<typename T>
	void WriteText(WgResourceSerializerXML& s, T* pText)
	{
		if(pText == 0)
			return;
		Uint32 nChars = WgTextTool::getTextSizeUTF8(pText);
		if(nChars == 0)
			return;
		char* pBuf = new char[nChars + 1];
		WgTextTool::getTextUTF8(pText, pBuf, nChars+1);
		pBuf[nChars] = 0;
		s.AddText(pBuf);
		delete[] pBuf;
	}

	template<typename T>
	void WriteTextAttrib(WgResourceSerializerXML& s, const T* pText, const std::string& attr)
	{
		if(pText == 0)
			return;
		Uint32 nChars = WgTextTool::getTextSizeUTF8(pText);
		if(nChars == 0)
			return;
		char* pBuf = new char[nChars + 1];
		WgTextTool::getTextUTF8(pText, pBuf, nChars+1);
		pBuf[nChars] = 0;
		s.AddAttribute(attr, pBuf);
		delete[] pBuf;
	}

	void WriteIDAttrib(WgResourceSerializerXML& s, const WgXmlNode& diffNode, const std::string& id)
	{
		if(diffNode.HasAttribute("id") || id.size())
			s.AddAttribute("id", id);
	}

	template< class T >	void WriteTextManager( T* pObject, WgResourceSerializerXML& s )
	{
		WgTextManager* pManager = pObject->GetTextManager();
		if( pManager )
		{
			std::string id = s.ResDb()->FindTextManagerId( pManager );

			if(id.empty())
			{
				id = s.ResDb()->GenerateName(pManager);
				s.ResDb()->AddTextManager(id, pManager);				//TODO: It's probably too late to add it here...
			}
			s.AddAttribute("textmanager", id);
		}
	}

	std::string ReadLocalizedString(const std::string& text, WgResourceSerializerXML& s)
	{
		if(text.size() > 0 && text[0] == ':')
			return s.ResDb()->LoadString(text.substr(1));
		return text;
	}

	template< class T >	void ReadTextManager(T* pObject, const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
	{
		std::string managerId = xmlNode["textmanager"];
		if( managerId.length() )
		{
			WgTextManager * pManager = s.ResDb()->GetTextManager(managerId);
			VERIFY(pManager != 0,  "unknown textmanager '" + managerId + "' specified");
			if( pManager )
				pObject->SetTextManager( pManager );
		}
	}
}

std::string WgXMLMetaData::GetAttribute(const std::string& name) const
{
	return xmlNode[name];
}

bool WgXMLMetaData::SetAttribute(const std::string& name, const std::string& value)
{
	xmlNode[name] = value;
	return true;
}

bool WgXMLMetaData::HasAttribute(const std::string& name) const
{
	return xmlNode.HasAttribute(name);
}

void WgResourceXML::OnDeserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	m_node = xmlNode;
	Deserialize(xmlNode, s);
}

void WgResourceXML::OnDeserialized(WgResourceSerializerXML& s)
{
	Deserialized(s);
}

void WgResourceXML::SetMetaData(WgResDB::MetaData* metaData)
{
	if(metaData)
		m_node = ((WgXMLMetaData*)metaData)->xmlNode;
}

void WgResourceXML::RegisterResources()
{
	if(WgResourceFactoryXML::Contains("xml"))
		return;

	WgResourceFactoryXML::Register<WgXmlRoot>			(WgXmlRoot::TagName());
	WgResourceFactoryXML::Register<WgReferenceRes>		(WgReferenceRes::TagName());
	WgResourceFactoryXML::Register<WgInterceptRes>		(WgInterceptRes::TagName());
	WgResourceFactoryXML::Register<WgModePropRes>		(WgModePropRes::TagName());
	WgResourceFactoryXML::Register<WgLegoRes>			(WgLegoRes::TagName());
	WgResourceFactoryXML::Register<WgBlockRes>			(WgBlockRes::TagName());
	WgResourceFactoryXML::Register<WgTextPropRes>		(WgTextPropRes::TagName());
	WgResourceFactoryXML::Register<WgValueFormatRes>	(WgValueFormatRes::TagName());
	//WgResourceFactoryXML::Register<WgPropRes>			(WgPropRes::TagName());
	WgResourceFactoryXML::Register<WgGeometryRes>		(WgGeometryRes::TagName());
	WgResourceFactoryXML::Register<WgColorRes>			(WgColorRes::TagName());
	WgResourceFactoryXML::Register<WgColorSetRes>		(WgColorSetRes::TagName());
	WgResourceFactoryXML::Register<WgSurfaceRes>		(WgSurfaceRes::TagName());
	WgResourceFactoryXML::Register<WgGlyphSetRes>		(WgGlyphSetRes::TagName());
	WgResourceFactoryXML::Register<WgFontRes>			(WgFontRes::TagName());
	WgResourceFactoryXML::Register<WgAnimRes>			(WgAnimRes::TagName());
	WgResourceFactoryXML::Register<WgKeyFrameRes>		(WgKeyFrameRes::TagName());
	WgResourceFactoryXML::Register<WgCursorRes>			(WgCursorRes::TagName());
	WgResourceFactoryXML::Register<WgTextManagerRes>	(WgTextManagerRes::TagName());
	WgResourceFactoryXML::Register<WgSkinManagerRes>	(WgSkinManagerRes::TagName());
	WgResourceFactoryXML::Register<WgBorderRes>			(WgBorderRes::TagName());
	WgResourceFactoryXML::Register<WgTileRes>			(WgTileRes::TagName());
	WgResourceFactoryXML::Register<WgBlockSetRes>		(WgBlockSetRes::TagName());
	WgResourceFactoryXML::Register<WgAltRes>			(WgAltRes::TagName());

	WgResourceFactoryXML::Register<WgItemPixmapRes>		(WgItemPixmapRes::TagName());
	WgResourceFactoryXML::Register<WgItemPixmapRes>		(WgItemPixmap::GetMyType());

	WgResourceFactoryXML::Register<WgItemRowRes>		(WgItemRowRes::TagName());
	WgResourceFactoryXML::Register<WgItemRowRes>		(WgItemRow::GetMyType());

	WgResourceFactoryXML::Register<WgItemStackRes>		(WgItemStackRes::TagName());
	WgResourceFactoryXML::Register<WgItemStackRes>		(WgItemStack::GetMyType());

	WgResourceFactoryXML::Register<WgItemTextRes>		(WgItemTextRes::TagName());
	WgResourceFactoryXML::Register<WgItemTextRes>		(WgItemText::GetMyType());

	WgResourceFactoryXML::Register<WgItemWrapTextRes>	(WgItemWrapTextRes::TagName());
	WgResourceFactoryXML::Register<WgItemWrapTextRes>	(WgItemWrapText::GetMyType());

	WgResourceFactoryXML::Register<WgMenuSeparatorRes>	(WgMenuSeparatorRes::TagName());
	WgResourceFactoryXML::Register<WgMenuEntryRes>		(WgMenuEntryRes::TagName());
	WgResourceFactoryXML::Register<WgMenuCheckBoxRes>	(WgMenuCheckBoxRes::TagName());
	WgResourceFactoryXML::Register<WgMenuRadioButtonRes>(WgMenuRadioButtonRes::TagName());
	WgResourceFactoryXML::Register<WgMenuSubMenuRes>	(WgMenuSubMenuRes::TagName());

	WgResourceFactoryXML::Register<Wdg_Button_Res>		(Wdg_Button_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_Button_Res>		(Wdg_Button::GetMyType());

	WgResourceFactoryXML::Register<Wdg_RefreshButton_Res>		(Wdg_RefreshButton_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_RefreshButton_Res>		(Wdg_RefreshButton::GetMyType());

	WgResourceFactoryXML::Register<Wdg_CheckBox2_Res>	(Wdg_CheckBox2_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_CheckBox2_Res>	(Wdg_CheckBox2::GetMyType());

	WgResourceFactoryXML::Register<Wdg_ComboBox_Res>	(Wdg_ComboBox_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_ComboBox_Res>	(Wdg_ComboBox::GetMyType());

	WgResourceFactoryXML::Register<Wdg_Container_Res>	(Wdg_Container_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_Container_Res>	(Wdg_Container::GetMyType());

	WgResourceFactoryXML::Register<Wdg_HDrag_Res>		(Wdg_HDrag_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_HDrag_Res>		(Wdg_HDrag::GetMyType());

	WgResourceFactoryXML::Register<Wdg_VDrag_Res>		(Wdg_VDrag_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_VDrag_Res>		(Wdg_VDrag::GetMyType());

	WgResourceFactoryXML::Register<Wdg_EditValue_Res>	(Wdg_EditValue_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_EditValue_Res>	(Wdg_EditValue::GetMyType());

	WgResourceFactoryXML::Register<Wdg_Fill_Res>		(Wdg_Fill_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_Fill_Res>		(Wdg_Fill::GetMyType());

	WgResourceFactoryXML::Register<Wdg_GridView_Res>	(Wdg_GridView_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_GridView_Res>	(Wdg_GridView::GetMyType());

	WgResourceFactoryXML::Register<Wdg_ListView_Res>	(Wdg_ListView_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_ListView_Res>	(Wdg_ListView::GetMyType());

	WgResourceFactoryXML::Register<Wdg_Menu_Res>		(Wdg_Menu_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_Menu_Res>		(Wdg_Menu::GetMyType());

	WgResourceFactoryXML::Register<Wdg_MenuBar_Res>		(Wdg_MenuBar_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_MenuBar_Res>		(Wdg_MenuBar::GetMyType());
	WgResourceFactoryXML::Register<WgMenuBarItemRes>	(WgMenuBarItemRes::TagName());

	WgResourceFactoryXML::Register<Wdg_Pixmap_Res>		(Wdg_Pixmap_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_Pixmap_Res>		(Wdg_Pixmap::GetMyType());

	WgResourceFactoryXML::Register<Wdg_Lodder_Res>		(Wdg_Lodder_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_Lodder_Res>		(Wdg_Lodder::GetMyType());

	WgResourceFactoryXML::Register<Wdg_RadioButton2_Res>(Wdg_RadioButton2_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_RadioButton2_Res>(Wdg_RadioButton2::GetMyType());

	WgResourceFactoryXML::Register<Wdg_Root_Res>		(Wdg_Root_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_Root_Res>		(Wdg_Root::GetMyType());

	WgResourceFactoryXML::Register<Wdg_Shader_Res>		(Wdg_Shader_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_Shader_Res>		(Wdg_Shader::GetMyType());

	WgResourceFactoryXML::Register<Wdg_TableView_Res>	(Wdg_TableView_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_TableView_Res>	(Wdg_TableView::GetMyType());

	WgResourceFactoryXML::Register<WgTableColumnRes>	(WgTableColumnRes::TagName());

	WgResourceFactoryXML::Register<Wdg_TabList_Res>		(Wdg_TabList_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_TabList_Res>		(Wdg_TabList::GetMyType());

	WgResourceFactoryXML::Register<WgTabRes>			(WgTabRes::TagName());

	WgResourceFactoryXML::Register<Wdg_Text_Res>		(Wdg_Text_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_Text_Res>		(Wdg_Text::GetMyType());

	WgResourceFactoryXML::Register<Wdg_EditLine_Res>	(Wdg_EditLine_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_EditLine_Res>	(Wdg_EditLine::GetMyType());

	WgResourceFactoryXML::Register<Wdg_TextView_Res>	(Wdg_TextView_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_TextView_Res>	(Wdg_TextView::GetMyType());

	WgResourceFactoryXML::Register<Wdg_YSplitter_Res>		(Wdg_YSplitter_Res::TagName());
	WgResourceFactoryXML::Register<Wdg_YSplitter_Res>		(Wdg_YSplitter::GetMyType());

	WgResourceFactoryXML::Register<WgBoldTextRes>		(WgBoldTextRes::TagName());
	WgResourceFactoryXML::Register<WgItalicTextRes>		(WgItalicTextRes::TagName());
	WgResourceFactoryXML::Register<WgUnderlinedTextRes>	(WgUnderlinedTextRes::TagName());
	WgResourceFactoryXML::Register<WgBreakTextRes>		(WgBreakTextRes::TagName());

	WgResourceFactoryXML::Register<WgConnectRes>		(WgConnectRes::TagName());

}

void WgResourceXML::UnregisterResources()
{
	WgResourceFactoryXML::Clear();
}

//////////////////////////////////////////////////////////////////////////
/// WgXmlRoot ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <xml>
// </xml>
void WgXmlRoot::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName());

	// references
	for(WgResDB::ResDBRes* res = s.ResDb()->GetFirstResDBRes(); res; res = res->Next())
	{
		WgReferenceRes refRes(this, res);
		refRes.SetMetaData(res->meta);
		refRes.Serialize(s);
	}

	// colors
	for(WgResDB::ColorRes* res = s.ResDb()->GetFirstResColor(); res; res = res->Next())
	{
		WgColorRes colorRes(this, res->res);
		colorRes.SetMetaData(res->meta);
		colorRes.Serialize(s);
	}

	// glyph sets
	if(s.ResDb()->GetFirstResGlyphSet())
	{
		s.AddText("\n");
		for(WgResDB::GlyphSetRes* res = s.ResDb()->GetFirstResGlyphSet(); res; res = res->Next())
		{
			WgGlyphSetRes glyphSetRes(this, res->res);
			glyphSetRes.SetMetaData(res->meta);
			glyphSetRes.Serialize(s);
		}
	}

	// surfaces
	if(s.ResDb()->GetFirstResSurface())
	{
		s.AddText("\n");
		for(WgResDB::SurfaceRes* res = s.ResDb()->GetFirstResSurface(); res; res = res->Next())
		{
			WgSurfaceRes surfaceRes(this, res->res);
			surfaceRes.SetMetaData(res->meta);
			surfaceRes.Serialize(s);
		}
	}

	// anims
	if(s.ResDb()->GetFirstResAnim())
	{
		s.AddText("\n");
		for(WgResDB::AnimRes* res = s.ResDb()->GetFirstResAnim(); res; res = res->Next())
		{
			WgAnimRes animRes(this, (WgGfxAnim*)res->res);
			animRes.SetMetaData(res->meta);
			animRes.Serialize(s);
		}
	}

	// cursors
	if(s.ResDb()->GetFirstResCursor())
	{
		s.AddText("\n");
		for(WgResDB::CursorRes* res = s.ResDb()->GetFirstResCursor(); res; res = res->Next())
		{
			WgCursorRes cursorRes(this, res->res);
			cursorRes.SetMetaData(res->meta);
			cursorRes.Serialize(s);
		}
	}

	// fonts
	if(s.ResDb()->GetFirstResFont())
	{
		s.AddText("\n");
		for(WgResDB::FontRes* res = s.ResDb()->GetFirstResFont(); res; res = res->Next())
		{
			WgFontRes fontRes(this, res->res);
			fontRes.SetMetaData(res->meta);
			fontRes.Serialize(s);
		}
	}

	// color sets
	if(s.ResDb()->GetFirstResColorSet())
	{
		s.AddText("\n");
		for(WgResDB::ColorSetRes* res = s.ResDb()->GetFirstResColorSet(); res; res = res->Next())
		{
			WgColorSetRes colorSetRes(this, res->res);
			colorSetRes.SetMetaData(res->meta);
			colorSetRes.Serialize(s);
		}
	}


	// block sets
	if(s.ResDb()->GetFirstResBlockSet())
	{
		s.AddText("\n");
		for(WgResDB::BlockSetRes* res = s.ResDb()->GetFirstResBlockSet(); res; res = res->Next())
		{
			WgBlockSetRes blockSetRes(this, res->res);
			blockSetRes.SetMetaData(res->meta);
			blockSetRes.Serialize(s);
		}
	}

	// text props
	if(s.ResDb()->GetFirstResTextProp())
	{
		s.AddText("\n");
		for(WgResDB::TextPropRes* res = s.ResDb()->GetFirstResTextProp(); res; res = res->Next())
		{
			WgTextPropRes textPropRes(this, res->res);
			textPropRes.SetMetaData(res->meta);
			textPropRes.Serialize(s);
		}
	}

	// text managers
	if(s.ResDb()->GetFirstResTextManager())
	{
		s.AddText("\n");
		for(WgResDB::TextManagerRes* res = s.ResDb()->GetFirstResTextManager(); res; res = res->Next())
		{
			WgTextManagerRes textManagerRes(this, res->res);
			textManagerRes.SetMetaData(res->meta);
			textManagerRes.Serialize(s);
		}
	}

	// skin managers
	if(s.ResDb()->GetFirstResSkinManager())
	{
		s.AddText("\n");
		for(WgResDB::SkinManagerRes* res = s.ResDb()->GetFirstResSkinManager(); res; res = res->Next())
		{
			WgSkinManagerRes skinManagerRes(this, res->res);
			skinManagerRes.SetMetaData(res->meta);
			skinManagerRes.Serialize(s);
		}
	}

	// widgets
	if(s.ResDb()->GetFirstResWidget())
	{
		s.AddText("\n");
		for(WgResDB::WidgetRes* res = s.ResDb()->GetFirstResWidget(); res; res = res->Next())
		{
			// Only write top level widgets. WgWidgetRes takes care of the subtree
			if(IsTopLevelWidget(res->res, s))
			{
				// HACK. assume that every menu is written by its holder (combobox, menubaritem, submenuitem, ...)
				if(res->res->Type() == Wdg_Menu::GetMyType())
					continue;

				WgResourceXML* xmlRes = WgResourceFactoryXML::Create(res->res->Type(), this);
				ASSERT(xmlRes, "unknown widget type");

				WgWidgetRes* widgetRes = WgResourceXML::Cast<WgWidgetRes>(xmlRes);
				if(widgetRes)
				{
					widgetRes->SetMetaData(res->meta);
					widgetRes->SetWidget(res->res);
					widgetRes->Serialize(s);
				}
				delete xmlRes;
			}
		}
	}

	s.EndTag();
}

void WgXmlRoot::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	// nothing to do yet
}

bool WgXmlRoot::IsTopLevelWidget(WgWidget* widget, WgResourceSerializerXML& s)
{
	for(WgResDB::WidgetRes* res = s.ResDb()->GetFirstResWidget(); res; res = res->Next())
	{
		if(widget->IsChildOf(res->res))
			return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
/// WgConnectRes /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <connect
//		id=[value]
//		from=[emitter]
//		action=[value]
//		to=[receiver]
// />

void WgConnectRes::Serialize(WgResourceSerializerXML& s, WgWidgetRes* widgetRes)
{
	WgWidget* widget = widgetRes->GetWidget();
	for(WgResDB::ConnectRes* res = s.ResDb()->GetFirstResConnect(); res; res = res->Next())
	{
		ConnectData* meta = (ConnectData*)res->meta;
		if(meta && meta->pEmitter == widget)
		{
			WgConnectRes connectRes(widgetRes);
			connectRes.SetMetaData(meta);
			connectRes.Serialize(s);
		}
	}
}

void WgConnectRes::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());
	s.EndTag();
}

void WgConnectRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgWidget* emitter = 0;
	WgWidgetRes* wdgRes = WgResourceXML::Cast<WgWidgetRes>(Parent());
	if(wdgRes)
	{
		emitter = wdgRes->GetWidget();
	}
	else
	{
		emitter = s.ResDb()->GetWidget(xmlNode["from"]);
		VERIFY(emitter, "missing 'from' on <connect>");
	}

	WgWidget* receiver = 0;
	if(xmlNode.HasAttribute("to"))
	{
		receiver = s.ResDb()->GetWidget(xmlNode["to"]);
		VERIFY(receiver, "cannot find " + xmlNode["to"] + " on <connect>");
	}

	if(emitter)
		s.ResDb()->AddConnect(new ConnectData(emitter, xmlNode));

	s.ResDb()->Connect(xmlNode["id"], emitter, xmlNode["action"], receiver);
}

//////////////////////////////////////////////////////////////////////////
/// WgReferenceRes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgReferenceRes::Serialize(WgResourceSerializerXML& s)
{
	WgResDB::ResDBRes* res = m_pResDbRes;
	VERIFY(res, "db reference does not exist in ResDb");
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());
	s.AddAttribute("file", res->file);
	bool bReq = WgUtil::ToBool(xmlNode["required"], true);
	WriteDiffAttr(s, xmlNode, "required", bReq, true);
	s.EndTag();
}

void WgReferenceRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	VERIFY(xmlNode.HasAttribute("file"), "invalid reference");
	bool bLoaded = s.ResDb()->AddResDb(xmlNode["file"], new WgXMLMetaData(xmlNode));
	VERIFY(WgUtil::ToBool(xmlNode["required"]) == false || bLoaded == true, "required database not found: " + xmlNode["file"]);
}

//////////////////////////////////////////////////////////////////////////
/// WgInterceptRes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <intercept
//		source=[value]
//		rule=[value]
// />

void WgInterceptRes::Serialize(WgResourceSerializerXML& s, WgWidget::ActionSource source, WgWidget::InterceptionRules rule )
{
	s.BeginTag(TagName());
	switch(source)
	{
	case WgWidget::POINTER: s.AddAttribute("source", "pointer"); break;
	case WgWidget::BUTTON1: s.AddAttribute("source", "button1"); break;
	case WgWidget::BUTTON2: s.AddAttribute("source", "button2"); break;
	case WgWidget::BUTTON3: s.AddAttribute("source", "button3"); break;
	case WgWidget::BUTTON4: s.AddAttribute("source", "button4"); break;
	case WgWidget::BUTTON5: s.AddAttribute("source", "button5"); break;
	case WgWidget::KEYBOARD:s.AddAttribute("source", "keyboard"); break;
	case WgWidget::WHEEL1: 	s.AddAttribute("source", "wheel1"); break;
	case WgWidget::WHEEL2: 	s.AddAttribute("source", "wheel2"); break;
	case WgWidget::WHEEL3: 	s.AddAttribute("source", "wheel3"); break;
	default: assert(0);
	}
	switch(rule)
	{
	case WgWidget::PASS:			s.AddAttribute("rule", "pass"); break;
	case WgWidget::BLOCK:			s.AddAttribute("rule", "block"); break;
	case WgWidget::INTERCEPT_PASS:	s.AddAttribute("rule", "intercept_pass"); break;
	case WgWidget::INTERCEPT_BLOCK: s.AddAttribute("rule", "intercept_block"); break;
	default: assert(0);
	}
	s.EndTag();
}

void WgInterceptRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgWidgetRes* widgetRes = WgResourceXML::Cast<WgWidgetRes>(Parent());
	VERIFY(widgetRes, "invalid parent for <intercept>. should be widget type");
	WgWidget* widget = widgetRes->GetWidget();
	const std::string& strSource = xmlNode["source"];

	WgWidget::ActionSource source;
	WgWidget::InterceptionRules rule;

	if(strSource == "pointer")			source = WgWidget::POINTER;
	else if(strSource == "button1")	source = WgWidget::BUTTON1;
	else if(strSource == "button2")	source = WgWidget::BUTTON2;
	else if(strSource == "button3")	source = WgWidget::BUTTON3;
	else if(strSource == "button4")	source = WgWidget::BUTTON4;
	else if(strSource == "button5")	source = WgWidget::BUTTON5;
	else if(strSource == "keyboard")	source = WgWidget::KEYBOARD;
	else if(strSource == "wheel1")		source = WgWidget::WHEEL1;
	else if(strSource == "wheel2")		source = WgWidget::WHEEL2;
	else if(strSource == "wheel3")		source = WgWidget::WHEEL3;
	else ASSERT(0, "invalid intercept source");

	const std::string& strRule = xmlNode["rule"];

	if(strRule == "pass")					rule = WgWidget::PASS;
	else if(strRule == "block")			rule = WgWidget::BLOCK;
	else if(strRule == "intercept_pass")	rule = WgWidget::INTERCEPT_PASS;
	else if(strRule == "intercept_block")	rule = WgWidget::INTERCEPT_BLOCK;
	else ASSERT(0, "invalid intercept rule");

	widget->Intercept(source, rule);
}

//////////////////////////////////////////////////////////////////////////
/// WgPointerMaskRes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// pointermask=[value]
void WgPointerMaskRes::Serialize(WgResourceSerializerXML& s, const WgXmlNode& xmlNode, const std::string& attr, WgWidget::PointerMask mask, WgWidget::PointerMask def)
{
	if(xmlNode.HasAttribute(attr) || mask != def)
	{
		switch(mask)
		{
		case WgWidget::POINTER_TRANSPARENT:		s.AddAttribute(attr, "transparent"); break;
		case WgWidget::POINTER_OPAQUE:			s.AddAttribute(attr, "opaque"); break;
		case WgWidget::POINTER_SOURCE_ALPHA:	s.AddAttribute(attr, "sourcealpha"); break;
		case WgWidget::POINTER_SOFT_OPAQUE:		s.AddAttribute(attr, "softopaque"); break;
		default: assert(0);
		};
	}
}

WgWidget::PointerMask WgPointerMaskRes::Deserialize(const WgXmlNode& xmlNode, const std::string& attr)
{
	WgWidget::PointerMask mask = WgWidget::POINTER_SOURCE_ALPHA;
	const std::string& val = xmlNode[attr];
	if(val.empty() || val == "sourcealpha")	mask = WgWidget::POINTER_SOURCE_ALPHA;
	else if(val == "opaque")				mask = WgWidget::POINTER_OPAQUE;
	else if(val == "transparent")			mask = WgWidget::POINTER_TRANSPARENT;
	else if(val == "softopaque")			mask = WgWidget::POINTER_SOFT_OPAQUE;
	else assert(0);
	return mask;
}


//////////////////////////////////////////////////////////////////////////
/// WgPointerStyleRes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// pointerstyle=[value]
void WgPointerStyleRes::Serialize(WgResourceSerializerXML& s, const WgXmlNode& xmlNode, const std::string& attr, WgPointerStyle style, WgPointerStyle def)
{
	if(xmlNode.HasAttribute(attr) || style != def)
	{
		switch(style)
		{
			case WG_POINTER_ARROW:			s.AddAttribute(attr, "arrow"); break;
			case WG_POINTER_HOURGLASS:		s.AddAttribute(attr, "hourglass"); break;
			case WG_POINTER_HAND:			s.AddAttribute(attr, "hand"); break;
			case WG_POINTER_CROSSHAIR:		s.AddAttribute(attr, "crosshair"); break;
			case WG_POINTER_HELP:			s.AddAttribute(attr, "help"); break;
			case WG_POINTER_IBEAM:			s.AddAttribute(attr, "ibeam"); break;
			case WG_POINTER_STOP:			s.AddAttribute(attr, "stop"); break;
			case WG_POINTER_UP_ARROW:		s.AddAttribute(attr, "up_arrow"); break;
			case WG_POINTER_SIZE_ALL:		s.AddAttribute(attr, "size_all"); break;
			case WG_POINTER_SIZE_NE_SW:		s.AddAttribute(attr, "size_ne_sw"); break;
			case WG_POINTER_SIZE_NW_SE:		s.AddAttribute(attr, "size_nw_se"); break;
			case WG_POINTER_SIZE_N_S:		s.AddAttribute(attr, "size_n_s"); break;
			case WG_POINTER_SIZE_W_E:		s.AddAttribute(attr, "size_w_e"); break;
			default: assert(0);
		}
	}
}

WgPointerStyle WgPointerStyleRes::Deserialize(const WgXmlNode& xmlNode, const std::string& attr)
{
	WgPointerStyle style = WG_POINTER_DEFAULT;
	const std::string& val = xmlNode[attr];
	if(val.empty() || val == "default") style = WG_POINTER_DEFAULT;
	else if(val == "arrow")				style = WG_POINTER_ARROW;
	else if(val == "default")			style = WG_POINTER_DEFAULT;
	else if(val == "hourglass")			style = WG_POINTER_HOURGLASS;
	else if(val == "hand")				style = WG_POINTER_HAND;
	else if(val == "crosshair")			style = WG_POINTER_CROSSHAIR;
	else if(val == "help")				style = WG_POINTER_HELP;
	else if(val == "ibeam")				style = WG_POINTER_IBEAM;
	else if(val == "stop")				style = WG_POINTER_STOP;
	else if(val == "up_arrow")			style = WG_POINTER_UP_ARROW;
	else if(val == "size_all")			style = WG_POINTER_SIZE_ALL;
	else if(val == "size_ne_sw")		style = WG_POINTER_SIZE_NE_SW;
	else if(val == "size_nw_se")		style = WG_POINTER_SIZE_NW_SE;
	else if(val == "size_n_s")			style = WG_POINTER_SIZE_N_S;
	else if(val == "size_w_e")			style = WG_POINTER_SIZE_W_E;
	else assert(0);
	return style;
}

//////////////////////////////////////////////////////////////////////////
/// WgButtonLayoutRes ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// layout=[value]
void WgButtonLayoutRes::Serialize(WgResourceSerializerXML& s, const WgXmlNode& xmlNode, const std::string& attr, WgGizmoSlider::ButtonLayout layout, WgGizmoSlider::ButtonLayout def)
{
	if(xmlNode.HasAttribute(attr) || layout != def)
	{
		switch(layout)
		{
		case WgGizmoSlider::NONE:			s.AddAttribute(attr, "none"); break;
		case WgGizmoSlider::HEADER_FWD:		s.AddAttribute(attr, "head_fwd"); break;
		case WgGizmoSlider::HEADER_BWD:		s.AddAttribute(attr, "head_bwd"); break;
		case WgGizmoSlider::FOOTER_FWD:		s.AddAttribute(attr, "foot_fwd"); break;
		case WgGizmoSlider::FOOTER_BWD:		s.AddAttribute(attr, "foot_bwd"); break;
		case WgGizmoSlider::WINDOWS:		s.AddAttribute(attr, "windows"); break;
		case WgGizmoSlider::NEXT_VERT:		s.AddAttribute(attr, "next_vert"); break;
		case WgGizmoSlider::NEXT_HORR:		s.AddAttribute(attr, "next_horr"); break;
		case WgGizmoSlider::ALL:			s.AddAttribute(attr, "all"); break;
		default:							s.AddAttribute(attr, "default"); break;
		}
	}
}

WgGizmoSlider::ButtonLayout WgButtonLayoutRes::Deserialize(const WgXmlNode& xmlNode, const std::string& attr)
{
	WgGizmoSlider::ButtonLayout layout = WgGizmoSlider::DEFAULT;
	const std::string& val = xmlNode[attr];
	if(val.empty() || val == "default") layout = WgGizmoSlider::DEFAULT;
	else if(val == "none")		layout = WgGizmoSlider::NONE;
	else if(val == "default")	layout = WgGizmoSlider::DEFAULT;
	else if(val == "head_fwd")	layout = WgGizmoSlider::HEADER_FWD;
	else if(val == "head_bwd")	layout = WgGizmoSlider::HEADER_BWD;
	else if(val == "foot_fwd")	layout = WgGizmoSlider::FOOTER_FWD;
	else if(val == "foot_bwd")	layout = WgGizmoSlider::FOOTER_BWD;
	else if(val == "windows")	layout = WgGizmoSlider::WINDOWS;
	else if(val == "next_vert")	layout = WgGizmoSlider::NEXT_VERT;
	else if(val == "next_horr")	layout = WgGizmoSlider::NEXT_HORR;
	else if(val == "all")		layout = WgGizmoSlider::ALL;
	else assert(0);
	return layout;
}

//////////////////////////////////////////////////////////////////////////
/// WgModeRes ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// mode=[value]
std::string WgModeRes::Serialize(WgMode mode)
{
	switch(mode)
	{
	case WG_MODE_NORMAL:	return "normal";
	case WG_MODE_MARKED:	return "marked";
	case WG_MODE_SELECTED:	return "selected";
	case WG_MODE_DISABLED:	return "disabled";
	case WG_MODE_SPECIAL:	return "special";
	case WG_MODE_ALL:		return "all";
	default:
		assert(0);
	}
	return "normal";
}

WgMode WgModeRes::Deserialize(const std::string& value)
{
	if(value.empty() || value == "normal")	return WG_MODE_NORMAL;
	else if(value == "marked")				return WG_MODE_MARKED;
	else if(value == "selected")			return WG_MODE_SELECTED;
	else if(value == "disabled")			return WG_MODE_DISABLED;
	else if(value == "special")				return WG_MODE_SPECIAL;
	else if(value == "all")					return WG_MODE_ALL;
	else assert(0);
	return WG_MODE_NORMAL;
}


//////////////////////////////////////////////////////////////////////////
/// WgModePropRes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <mode
//		id=[mode]
//		style=[fontstyle]
//		col=[color]
//		bg_col=[background color]
//		size=[size]
//		underlined=[true | false]
void WgModePropRes::Serialize(WgResourceSerializerXML& s)
{
	WgTextPropRes* textPropRes = WgResourceXML::Cast<WgTextPropRes>(Parent());
	WgColorSetRes* colorSetRes = WgResourceXML::Cast<WgColorSetRes>(Parent());

	ASSERT(textPropRes||colorSetRes, "invalid parent for <mode>");

	if( colorSetRes )
	{
		// Only write down if we are different from color of WG_MODE_NORMAL

		WgColorSetPtr p = colorSetRes->GetColorSet();
		WgColor color = p->Color(m_mode);

		if(color != p->Color(WG_MODE_NORMAL))
		{
			s.BeginTag(TagName());
			s.AddAttribute("id", WgModeRes::Serialize(m_mode));
			WgColorRes::Serialize(s, XmlNode(), "col", color, WgColor(0, color.a+1)); // force write by making color != default
			s.EndTag();
		}
	}
	else
	{
		s.BeginTag(TagName());
		s.AddAttribute("id", WgModeRes::Serialize(m_mode));

		WgTextProp* textProp = textPropRes->GetTextProp();

		if(textProp->Size(m_mode) != textPropRes->GetSize())
			s.AddAttribute("size", WgUtil::ToString(textProp->Size(m_mode)));

		if(textProp->Style(m_mode) != textPropRes->GetStyle())
			s.AddAttribute("style", WgFontStyleRes::Serialize(textProp->Style(m_mode), s));

		WgColor color = textProp->Color(m_mode);
		if(color != textPropRes->GetColor())
			WgColorRes::Serialize(s, XmlNode(), "col", color, WgColor(0, color.a+1)); // force write by making color != default

		WgColor bgColor = textProp->BgColor(m_mode);
		if(bgColor != textPropRes->GetBgColor())
			WgColorRes::Serialize(s, XmlNode(), "bg_col", bgColor, WgColor(0, bgColor.a+1)); // force write by making color != default


		if(textProp->IsUnderlined(m_mode) != textPropRes->IsUnderlined())
			s.AddAttribute("underlined", WgUtil::ToString(textProp->IsUnderlined(m_mode)));

		s.EndTag();
	}
}


void WgModePropRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgTextPropRes* textPropRes = WgResourceXML::Cast<WgTextPropRes>(Parent());
	WgColorSetRes* colorSetRes = WgResourceXML::Cast<WgColorSetRes>(Parent());

	ASSERT(textPropRes||colorSetRes, "invalid parent for <mode>");
	ASSERT(xmlNode.HasAttribute("id"), "missing id on <mode>");

	m_mode = WgModeRes::Deserialize(xmlNode["id"]);

	if( colorSetRes )
	{
		WgColorSetPtr pColorSet = colorSetRes->GetColorSet();
		WgColor color;

		if(xmlNode.HasAttribute("col"))
			color = WgColorRes::Deserialize(s, xmlNode["col"]);
		else
		{
			color.r = WgUtil::ToUint8(xmlNode["r"], 0);
			color.g = WgUtil::ToUint8(xmlNode["g"], 0);
			color.b = WgUtil::ToUint8(xmlNode["b"], 0);
			color.a = WgUtil::ToUint8(xmlNode["a"], 0xff);
		}
		pColorSet->SetColor(color, m_mode);
	}
	else
	{
		WgTextProp* textProp = textPropRes->GetTextProp();

		if(xmlNode.HasAttribute("size"))
			textProp->SetSize(WgUtil::ToUint32(xmlNode["size"]), m_mode);

		if(xmlNode.HasAttribute("style"))
			textProp->SetStyle(WgFontStyleRes::Deserialize(xmlNode, s), m_mode);

		if(xmlNode.HasAttribute("col"))
			textProp->SetColor(WgColorRes::Deserialize(s, xmlNode["col"]), m_mode);

		if(xmlNode.HasAttribute("bg_col"))
			textProp->SetBgColor(WgColorRes::Deserialize(s, xmlNode["bg_col"]), m_mode);

		if(xmlNode.HasAttribute("underlined"))
		{
			if(WgUtil::ToBool(xmlNode["underlined"]))
				textProp->SetUnderlined(m_mode);
			else 
				textProp->ClearUnderlined(m_mode);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
/// WgTextPropRes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <textprop
//		id=[string]
//		font=[fontname]
//		size=[default size]
//		style=[default fontstyle]
//		col=[default color]
//		bg_col=[default background color]
//		underlined=[default true | false]
//		breaklevel=[value]
void WgTextPropRes::Serialize(WgResourceSerializerXML& s)
{
	VERIFY(m_pProp, "no text prop defined");
	WgResDB::FontRes* fontRes = s.ResDb()->FindResFont(m_pProp->Font());
	VERIFY(fontRes, "font required by <textprop> does not exist in resdb");
	VERIFY(fontRes->id.size(), "<textprop> id witth 0 length");

	m_bColored = m_pProp->IsColored(WG_MODE_NORMAL);
	m_bBgColor = m_pProp->IsBgColored(WG_MODE_NORMAL);
	m_color = m_pProp->Color(WG_MODE_NORMAL);
	m_bgColor = m_pProp->BgColor(WG_MODE_NORMAL);
	m_style = m_pProp->Style(WG_MODE_NORMAL);
	m_underlined = m_pProp->IsUnderlined(WG_MODE_NORMAL);
	m_size = m_pProp->Size(WG_MODE_NORMAL);

	s.BeginTag(TagName());

	s.AddAttribute("id", s.ResDb()->FindTextPropId(m_pProp));
	s.AddAttribute("font", fontRes->id);
	s.AddAttribute("size", WgUtil::ToString(m_size) );

	if( m_pProp->BreakLevel() != -1 )
		s.AddAttribute( "breaklevel", WgUtil::ToString(m_pProp->BreakLevel()) );

	if(m_style != WG_STYLE_NORMAL)
		s.AddAttribute("style", WgFontStyleRes::Serialize(m_style, s));

	if(m_bColored)
		WgColorRes::Serialize(s, XmlNode(), "col", m_color, WgColor(0, m_color.a+1)); // force write by making m_color != default

	if(m_bBgColor)
		WgColorRes::Serialize(s, XmlNode(), "bg_col", m_bgColor, WgColor(0, m_bgColor.a+1)); // force write by making m_color != default

	if(m_underlined)
		s.AddAttribute("underlined", "true");

	if(!m_pProp->IsEqual(WG_MODE_NORMAL, WG_MODE_MARKED))
		WgModePropRes(this, WG_MODE_MARKED).Serialize(s);
	if(!m_pProp->IsEqual(WG_MODE_NORMAL, WG_MODE_SELECTED))
		WgModePropRes(this, WG_MODE_SELECTED).Serialize(s);
	if(!m_pProp->IsEqual(WG_MODE_NORMAL, WG_MODE_DISABLED))
		WgModePropRes(this, WG_MODE_DISABLED).Serialize(s);
	if(!m_pProp->IsEqual(WG_MODE_NORMAL, WG_MODE_SPECIAL))
		WgModePropRes(this, WG_MODE_SPECIAL).Serialize(s);

	s.EndTag();
}

void WgTextPropRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	std::string fontId = xmlNode["font"];

	if( !fontId.empty() )
	{
		WgFont * pFont = s.ResDb()->GetFont( fontId );
		ASSERT(pFont, "font '" + fontId + "' doesn't exist");
		if( pFont )
			m_prop.SetFont( pFont );
	}

	m_prop.SetBreakLevel( WgUtil::ToSint32( xmlNode["breaklevel"], -1 ) );

	m_bColored = xmlNode.HasAttribute("col");
	if(m_bColored)
		m_color = WgColorRes::Deserialize(s, xmlNode["col"]);

	m_bBgColor = xmlNode.HasAttribute("bg_col");
	if(m_bBgColor)
		m_bgColor = WgColorRes::Deserialize(s, xmlNode["bg_col"]);

	m_style = WgFontStyleRes::Deserialize(xmlNode, s);
	m_underlined = WgUtil::ToBool(xmlNode["underlined"]);
	m_size = WgUtil::ToUint32(xmlNode["size"]);


	m_prop.SetStyle( m_style );
	m_prop.SetSize( m_size );

	if(m_bColored)
		m_prop.SetColor(m_color);

	if(m_bBgColor)
		m_prop.SetBgColor(m_bgColor);

	if( m_underlined )
		m_prop.SetUnderlined();
	else
		m_prop.ClearUnderlined();
}

void WgTextPropRes::Deserialized(WgResourceSerializerXML& s)
{
	std::string id = XmlNode()["id"];
	VERIFY(id.size() != 0, "<textprop> without id attribute");

	// Register WgTextProp and save our reference.
	m_pProp = m_prop.Register();
	s.ResDb()->AddTextProp( id, m_pProp, new WgXMLMetaData(XmlNode()) );
}

//////////////////////////////////////////////////////////////////////////
/// WgValueFormatRes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <valueformat
//		integers=[value]
//		decimals=[value]
//		grouping=[value]
//		separator=[char]
//		period=[char]
//		prefix=[string]
//		suffix=[string]
//		plus_sign=[true | false]
//		negative_zero=[true | false]
//		force_period=[true | false]
//		force_decimals=[true | false]
//		no_decimal_tresh=[value]

void WgValueFormatRes::Serialize(WgResourceSerializerXML& s)
{
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());

	WriteDiffAttr<Uint8>(s, xmlNode, "integers", m_format->getIntegers(), 1);
	WriteDiffAttr<Uint8>(s, xmlNode, "decimals", m_format->getDecimals(), 0);
	WriteDiffAttr<Uint8>(s, xmlNode, "grouping", m_format->getGrouping(), 0);
	WriteDiffAttr<Uint16>(s, xmlNode, "separator", m_format->getSeparator(), 0xA0);
	WriteDiffAttr<Uint16>(s, xmlNode, "period", m_format->getPeriod(), 0x2e);


	std::wstring	pre = WgCharSeq(m_format->getPrefix()).GetStdWstring();
	std::wstring	suf = WgCharSeq(m_format->getSuffix()).GetStdWstring();

	if(!pre.empty() || xmlNode.HasAttribute("prefix"))
		WriteTextAttrib(s, m_format->getPrefix().Chars(), "prefix");

	if(!suf.empty() || xmlNode.HasAttribute("suffix"))
		WriteTextAttrib(s, m_format->getSuffix().Chars(), "suffix");

	WriteDiffAttr(s, xmlNode, "plus_sign", m_format->getPlus(), false);
	WriteDiffAttr(s, xmlNode, "negative_zero", m_format->getZeroIsNegative(), false);
	WriteDiffAttr(s, xmlNode, "force_period", m_format->getForcePeriod(), false);
	WriteTextPropAttr(s, m_format->getTextProperties(), "prop");
	WriteDiffAttr(s, xmlNode, "force_decimals", m_format->getForceDecimals(), true);
	WriteDiffAttr(s, xmlNode, "no_decimal_thres", m_format->getNoDecimalThreshold(), 0);

	s.EndTag();
}

void WgValueFormatRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgValueHolderRes* parentRes = WgResourceXML::Cast<WgValueHolderRes>(Parent());
	VERIFY(parentRes, "invalid parent for <valueformat>. should be valueholder type");

	WgValueFormat format;
	format.setIntegers(WgUtil::ToSint32(xmlNode["integers"], 1));
	format.setDecimals(WgUtil::ToSint32(xmlNode["decimals"], 0));
	format.setGrouping(WgUtil::ToSint32(xmlNode["grouping"], 0));
	format.setSeparator(WgUtil::ToUint16(xmlNode["separator"], 0xA0));
	format.setPeriod(WgUtil::ToUint16(xmlNode["period"], 0x2e));

	format.setPrefix(xmlNode["prefix"].c_str());
	format.setSuffix(xmlNode["suffix"].c_str());

	format.setPlus(WgUtil::ToBool(xmlNode["plus_sign"], false));
	format.setZeroIsNegative(WgUtil::ToBool(xmlNode["negative_zero"], false));
	format.setForcePeriod(WgUtil::ToBool(xmlNode["force_period"], false));
	WgTextPropPtr prop = s.ResDb()->GetTextProp(xmlNode["prop"]);
	if(prop)
		format.setTextProperties(prop);
	format.setForceDecimals(WgUtil::ToBool(xmlNode["force_decimals"], true));
	format.setNoDecimalThreshold(WgUtil::ToSint32(xmlNode["no_decimal_thres"], 0));

	parentRes->GetHolder()->SetFormat(format);
}

//////////////////////////////////////////////////////////////////////////
/// WgValueHolderRes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	Not a tag, attributes are included in subclasses.
//
//	val=[value]
//	unitsize=[value]
//	stepsize=[value]
//	minlimit=[value]
//	maxlimit=[value]
//	valuedigits=[value]
//	modulator=[value]

void WgValueHolderRes::Serialize(WgResourceXML* pThis, const WgXmlNode& xmlNode, WgResourceSerializerXML& s, class Wg_Interface_ValueHolder* holder)
{
	WgValueFormatRes format(pThis, &holder->GetFormat());
	format.Serialize(s);
	WriteDiffAttr(s, xmlNode, "val", holder->Value(), (Sint64)0);
	WriteDiffAttr<Uint32>(s, xmlNode, "unitsize", holder->UnitSize(), 1);
	WriteDiffAttr<Uint32>(s, xmlNode, "stepsize", holder->StepSize(), 1);
	WriteDiffAttr(s, xmlNode, "minlimit", holder->MinLimit(), (Sint64)0xC0000000);
	WriteDiffAttr(s, xmlNode, "maxlimit", holder->MaxLimit(), (Sint64)0x3FFFFFFF);
	WriteDiffAttr<Uint32>(s, xmlNode, "valuedigits", holder->ValueDigits(), 0);
	WriteDiffAttr<Uint32>(s, xmlNode, "modulator", holder->Modulator(), 1);
}

void WgValueHolderRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s, Wg_Interface_ValueHolder* holder)
{
	m_holder = holder;
	holder->SetValue(WgUtil::ToSint64(xmlNode["val"]));
	holder->SetFractionalRounding(WgUtil::ToUint32(xmlNode["valuedigits"], 0), WgUtil::ToUint32(xmlNode["modulator"], 1) );
	holder->SetUnitSize(WgUtil::ToUint32(xmlNode["unitsize"], 1));
	holder->SetStepSize(WgUtil::ToUint32(xmlNode["stepsize"], 1));
	holder->SetRange(WgUtil::ToSint32(xmlNode["minlimit"], (Sint32)0xC0000000), WgUtil::ToSint32(xmlNode["maxlimit"], (Sint32)0x3FFFFFFF) );
}

//////////////////////////////////////////////////////////////////////////
/// WgFontStyleRes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// style=[value]
// value:
//		normal, bold, bold_italic, italic, superscript, subscript, monospace,
//		heading_1, heading_2, heading_3, heading_4, heading_5,
//		user_1, user_2, user_3, user_4, user_5
std::string WgFontStyleRes::Serialize(WgFontStyle style, WgResourceSerializerXML& s)
{
	switch(style)
	{
	case WG_STYLE_NORMAL:		return "normal";
	case WG_STYLE_BOLD:			return "bold";
	case WG_STYLE_BOLD_ITALIC:	return "bold_italic";
	case WG_STYLE_ITALIC:		return "italic";
	case WG_STYLE_SUPERSCRIPT:	return "superscript";
	case WG_STYLE_SUBSCRIPT:	return "subscript";
	case WG_STYLE_MONOSPACE:	return "monospace";
	case WG_STYLE_HEADING_1:	return "heading_1";
	case WG_STYLE_HEADING_2:	return "heading_2";
	case WG_STYLE_HEADING_3:	return "heading_3";
	case WG_STYLE_HEADING_4:	return "heading_4";
	case WG_STYLE_HEADING_5:	return "heading_5";
	case WG_STYLE_USER_1:		return "user_1";
	case WG_STYLE_USER_2:		return "user_2";
	case WG_STYLE_USER_3:		return "user_3";
	case WG_STYLE_USER_4:		return "user_4";
	case WG_STYLE_USER_5:		return "user_5";
	default: ASSERT(0, "invalid style value");
	}
	return "normal";
}

WgFontStyle WgFontStyleRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgFontStyle style = WG_STYLE_NORMAL;
	const std::string& value = xmlNode["style"];
	if(value.empty() || value == "normal")	style = WG_STYLE_NORMAL;
	else if(value == "bold")				style = WG_STYLE_BOLD;
	else if(value == "bold_italic")			style = WG_STYLE_BOLD_ITALIC;
	else if(value == "italic")				style = WG_STYLE_ITALIC;
	else if(value == "superscript")			style = WG_STYLE_SUPERSCRIPT;
	else if(value == "subscript")			style = WG_STYLE_SUBSCRIPT;
	else if(value == "monospace")			style = WG_STYLE_MONOSPACE;
	else if(value == "heading_1")			style = WG_STYLE_HEADING_1;
	else if(value == "heading_2")			style = WG_STYLE_HEADING_2;
	else if(value == "heading_3")			style = WG_STYLE_HEADING_3;
	else if(value == "heading_4")			style = WG_STYLE_HEADING_4;
	else if(value == "headling_5")			style = WG_STYLE_HEADING_5;
	else if(value == "user_1")				style = WG_STYLE_USER_1;
	else if(value == "user_2")				style = WG_STYLE_USER_2;
	else if(value == "user_3")				style = WG_STYLE_USER_3;
	else if(value == "user_4")				style = WG_STYLE_USER_4;
	else if(value == "user_5")				style = WG_STYLE_USER_5;
	else ASSERT(0, "invalid style");
	return style;
}

//////////////////////////////////////////////////////////////////////////
/// WgSizeRes ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// size=[w, h]	OR	w=[] h=[]
void WgSizeRes::Serialize(WgResourceSerializerXML& s, const WgXmlNode& xmlNode, const std::string& attr, WgSize size, WgSize def)
{
	if(xmlNode.HasAttribute(attr) || size != def)
	{
		s.AddAttribute(attr, WgUtil::ToString(size.w, size.h));
	}
}

WgSize WgSizeRes::Deserialize(const WgXmlNode& xmlNode, const std::string& attr, WgSize def)
{
	WgSize size;
	if(WgUtil::FromString(xmlNode[attr], size.w, size.h) == 2)
		return size;
	return def;
}

//////////////////////////////////////////////////////////////////////////
/// WgGeometryRes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// <geo
//		origo=[name / anchorx, anchory, hotspotx, hotspoty]
//		x=[integer]
//		y=[integer]
//		w=[integer]
//		h=[integer]
//
//		OR
//
//		origo=[name / anchorx, anchory, hotspotx, hotspoty]
//		rect=[x, y, w, h]
//
//		OR
//
//		left  =[relative(, offset)]
//		top   =[relative(, offset)]
//		right =[relative(, offset)]
//		bottom=[relative(, offset)]
// />
void WgGeometryRes::Serialize(WgResourceSerializerXML& s)
{
	WgWidgetRes* parentRes = WgResourceXML::Cast<WgWidgetRes>(Parent());
	VERIFY(parentRes, "invalid parent for <geo>. should be widget type");

	WgOrigo o1 = parentRes->GetWidget()->OrigoTopLeft();
	WgOrigo o2 = parentRes->GetWidget()->OrigoBottomRight();
	Sint32 x1 = parentRes->GetWidget()->PosX();
	Sint32 y1 = parentRes->GetWidget()->PosY();
	Sint32 x2 = parentRes->GetWidget()->PosX2();
	Sint32 y2 = parentRes->GetWidget()->PosY2();

	const WgXmlNode& xmlNode = XmlNode();

	bool bRect = xmlNode.HasAttribute("rect");
	bool bWriteAsTag = !xmlNode.HasAttribute("origo") && !xmlNode.HasAttribute("w") && !bRect && !xmlNode.HasAttribute("left");
	bool bDefault = o1 == WgOrigo::topLeft() && o2 == WgOrigo::bottomRight() && x1 == 0 && x2 == 0 && y1 == 0 && y2 == 0;

	// clear out the old attributes
	s.RemoveAttribute("x");
	s.RemoveAttribute("y");
	s.RemoveAttribute("w");
	s.RemoveAttribute("h");
	s.RemoveAttribute("rect");
	s.RemoveAttribute("left");
	s.RemoveAttribute("top");
	s.RemoveAttribute("right");
	s.RemoveAttribute("bottom");

	// don't write default values unless attributes already exist
	if(bDefault && bWriteAsTag)
		return;

	if(bWriteAsTag)
		s.BeginTag(TagName());

	if(o1 == o2)
	{
		WriteDiffAttr(s, xmlNode, "origo", o1, WgOrigo::topLeft());

		if(bRect)
		{
			s.AddAttribute("rect", WgUtil::ToString(x1, y1, x2 - x1, y2 - y1));
		}
		else
		{
			WriteDiffAttr(s, xmlNode, "x", x1, 0);
			WriteDiffAttr(s, xmlNode, "y", y1, 0);
			s.AddAttribute("w", WgUtil::ToString(x2 - x1));
			s.AddAttribute("h", WgUtil::ToString(y2 - y1));
		}
	}
	else
	{
		// left, top, right, bottom
		s.AddAttribute("left", WgUtil::ToString(o1.anchorX(), x1));
		s.AddAttribute("top", WgUtil::ToString(o1.anchorY(), y1));
		s.AddAttribute("right", WgUtil::ToString(o2.anchorX(), x2));
		s.AddAttribute("bottom", WgUtil::ToString(o2.anchorY(), y2));
	}

	if(bWriteAsTag)
		s.EndTag();
}

void WgGeometryRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgWidgetRes* widgetRes = WgResourceXML::Cast<WgWidgetRes>(Parent());
	VERIFY(widgetRes, "invalid parent for <geo>. should be a widget type");
	WgWidget* widget = widgetRes->GetWidget();

	WgOrigo o1, o2;
	Sint32 x1, y1, x2, y2;

	if(xmlNode.HasAttribute("w") || xmlNode.HasAttribute("rect"))
	{
		o1 = o2 = WgUtil::ToOrigo(xmlNode["origo"]);
		WgRect rect = WgRectRes::Deserialize(xmlNode);
		x1 = rect.x;
		y1 = rect.y;
		x2 = rect.x + rect.w;
		y2 = rect.y + rect.h;
		widget->SetGeometry(o1, rect);
	}
	else
	{
		ASSERT(!(xmlNode.HasAttribute("origo") || xmlNode.HasAttribute("rect") || xmlNode.HasAttribute("x") || xmlNode.HasAttribute("y") || xmlNode.HasAttribute("w") || xmlNode.HasAttribute("h")),
			"use either (origo,x,y,w,h) or (origo,rect) or (top,bottom,left,right) to define geo");
		float t=0.f; Sint32 to=0;
		float b=1.f; Sint32 bo=0;
		float l=0.f; Sint32 lo=0;
		float r=1.f; Sint32 ro=0;
		WgUtil::FromString(xmlNode["left"], l, lo);
		WgUtil::FromString(xmlNode["top"], t, to);
		WgUtil::FromString(xmlNode["right"], r, ro);
		WgUtil::FromString(xmlNode["bottom"], b, bo);
		ASSERT(t <= b && l <= r, "top,left must be smaller than or equal to bottom,right");
		o1 = WgOrigo::specific(l, t, 0, 0);
		o2 = WgOrigo::specific(r, b, 1, 1);
		x1 = lo;
		y1 = to;
		x2 = ro;
		y2 = bo;
		widget->SetGeometry(o1, x1, y1, o2, x2, y2);
	}
}

//////////////////////////////////////////////////////////////////////////
/// WgColorRes ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
char* WgColorRes::ToHexString(WgColor color)
{
	static char hex[9];
	WgTextTool::Uint8ToAscii(color.r, &hex[0], 2);
	WgTextTool::Uint8ToAscii(color.g, &hex[2], 2);
	WgTextTool::Uint8ToAscii(color.b, &hex[4], 2);
	if(color.a != 0xff)
		WgTextTool::Uint8ToAscii(color.a, &hex[6], 2);
	else
		hex[6] = 0;
	return hex;
}

void WgColorRes::Serialize(WgResourceSerializerXML& s, const WgXmlNode& xmlNode, const std::string& attr, WgColor color, WgColor def)
{
	const std::string& oldName = xmlNode[attr];
	if(oldName.size() && oldName[0] == '#')
	{
		WgColor oldCol = s.ResDb()->GetColor(oldName.substr(1));
		if(oldCol == color)
		{
			s.AddAttribute(attr, oldName);
			return;
		}
	}

	if(oldName.size() || color != def)
	{
		s.AddAttribute(attr, ToHexString(color));
	}
}

WgColor WgColorRes::Deserialize(WgResourceSerializerXML& s, const std::string& value, WgColor def)
{
	if(value.empty())
		return def;

	bool bValid = value.length() == 6 || value.length() == 8 || value[0] == '#';
	ASSERT(bValid, "invalid color. should be RRGGBB or RRGGBBAA or #name");

	if(!bValid)
		return def;

	if( value[0] == '#' )
	{
		WgResDB::ColorRes* colorRes = s.ResDb()->GetResColor(value.substr(1));
//		ASSERT(colorRes, "undefined color: " + value.substr(1));
		if(colorRes)
			return colorRes->res;
		return def;
	}

	WgColor col;

	col.r = WgTextTool::AsciiToUint8( value.c_str() );
	col.g = WgTextTool::AsciiToUint8( value.c_str()+2 );
	col.b = WgTextTool::AsciiToUint8( value.c_str()+4 );

	if( value.length() == 8 )
		col.a = WgTextTool::AsciiToUint8( value.c_str()+6 );
	else
		col.a = 255;

	return col;
}

void WgColorRes::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	WriteIDAttrib(s, XmlNode(), XmlNode()["id"]);

	if(XmlNode().HasAttribute("col"))
	{
		char hex[9] = {0};
		WgTextTool::Uint8ToAscii(m_color.r, &hex[0], 2);
		WgTextTool::Uint8ToAscii(m_color.g, &hex[2], 2);
		WgTextTool::Uint8ToAscii(m_color.b, &hex[4], 2);
		WgTextTool::Uint8ToAscii(m_color.a, &hex[6], 2);
		s.AddAttribute("col", hex);
	}
	else
	{
		s.AddAttribute("r", WgUtil::ToString(m_color.r));
		s.AddAttribute("g", WgUtil::ToString(m_color.g));
		s.AddAttribute("b", WgUtil::ToString(m_color.b));
		WriteDiffAttr(s, XmlNode(), "a", m_color.a, (Uint8)0xff);
	}
	s.EndTag();
}

void WgColorRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	ASSERT(xmlNode.HasAttribute("r") || xmlNode.HasAttribute("g") || xmlNode.HasAttribute("b") || xmlNode.HasAttribute("col"), "only [r],[g],[b] or [col] supported in <color>");

	if(xmlNode.HasAttribute("col"))
	{
		m_color = Deserialize(s, xmlNode["col"]);
	}
	else
	{
		m_color.r = WgUtil::ToUint8(xmlNode["r"], 0);
		m_color.g = WgUtil::ToUint8(xmlNode["g"], 0);
		m_color.b = WgUtil::ToUint8(xmlNode["b"], 0);
		m_color.a = WgUtil::ToUint8(xmlNode["a"], 0xff);
	}

	const std::string& id = xmlNode["id"];
	if(id.size())
	{
		s.ResDb()->AddColor(id, m_color, new WgXMLMetaData(XmlNode()));
	}
}


//////////////////////////////////////////////////////////////////////////
/// WgSurfaceRes /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <surface id="" file="" />
void WgSurfaceRes::Serialize(WgResourceSerializerXML& s)
{
	WgResDB::SurfaceRes* res = s.ResDb()->FindResSurface(m_pSurf);
	VERIFY(res, "surface does not exist in ResDb");
	s.BeginTag(TagName(), XmlNode());
	s.AddAttribute("id", res->id);
	s.AddAttribute("file", res->file);
	s.EndTag();
}

void WgSurfaceRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	std::string filename = xmlNode["file"];
	std::string id = xmlNode["id"];
	bool bRequired = WgUtil::ToBool(xmlNode["required"], true);
	WgXMLMetaData* pMeta = new WgXMLMetaData(XmlNode());
	if(!s.ResDb()->AddSurface(id, filename, pMeta, bRequired))
	{
		delete pMeta;
		if( bRequired )
			s.Warning("Missing surface '" + filename + "'");
		return;
	}
	m_pSurf = s.ResDb()->GetSurface(id);
	ASSERT(m_pSurf, "invalid <surface>");
}

//////////////////////////////////////////////////////////////////////////
/// WgGlyphSetRes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <glyphset id="" file="" />	when defining glyphsets
// <glyphset id="" style="" />	when referencing glyphsets as child to <font>
void WgGlyphSetRes::Serialize(WgResourceSerializerXML& s)
{
	WgResDB::GlyphSetRes* res = s.ResDb()->FindResGlyphSet(m_pGlyphSet);
	ASSERT(res, "glyphset does not exist in ResDb");
	if(res == 0)
	{
		std::string id = WgResDB::GenerateName(m_pGlyphSet);
		s.ResDb()->AddGlyphSet(id, m_pGlyphSet, id);
		res = s.ResDb()->FindResGlyphSet(m_pGlyphSet);
	}
	s.BeginTag(TagName());
	s.AddAttribute("id", res->id);
	if(WgResourceXML::Cast<WgFontRes>(Parent()))
	{
		if( m_bHasStyle )
			s.AddAttribute("style", WgFontStyleRes::Serialize(m_style, s));

		if( m_size != 0 )
			s.AddAttribute("size", WgUtil::ToString(m_size));
	}
	else
	{
		s.AddAttribute("file", res->file);

#ifdef WG_USE_FREETYPE
		if( res->res->GetType() == WgGlyphSet::VECTOR )
		{
			// Check and add our render_mode by seeing what else we have
			// except monochrome.

			WgVectorGlyphs::RenderMode mode = WgVectorGlyphs::MONOCHROME;

			for( int i = 0 ; i <= WG_MAX_FONTSIZE && mode == WgVectorGlyphs::MONOCHROME; i++ )
				mode = ((WgVectorGlyphs*)(res->res))->GetRenderMode( i );

			switch( mode )
			{
				case WgVectorGlyphs::MONOCHROME:
					s.AddAttribute("render_mode", "monochrome" );
					break;
				case WgVectorGlyphs::CRISP_EDGES:
					s.AddAttribute("render_mode", "crisp_edges" );
					break;
				case WgVectorGlyphs::BEST_SHAPES:
					s.AddAttribute("render_mode", "best_shapes" );
					break;
			}

			// Write down size_offset if we have any.

			int	sizeOffset = ((WgVectorGlyphs*)(res->res))->GetSizeOffset();
			if( sizeOffset != 0 )
				s.AddAttribute("size_offset", WgUtil::ToString( sizeOffset ) );

			// Write down monochrome_sizes if we have any.

			if( mode != WgVectorGlyphs::MONOCHROME )
			{
				std::string	str;

				for( int i = 0 ; i <= WG_MAX_FONTSIZE ; i++ )
				{
					WgVectorGlyphs::RenderMode m = ((WgVectorGlyphs*)(res->res))->GetRenderMode( i );
					if( m == WgVectorGlyphs::MONOCHROME )
					{
						if( !str.empty() )
							str += ",";

						str += WgUtil::ToString( i );
					}

				}

				if( !str.empty() )
					s.AddAttribute( "monochrome_sizes", str );
			}
		}
#endif
	}
	s.EndTag();
}

void WgGlyphSetRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	const std::string& id = xmlNode["id"];
	m_pGlyphSet = s.ResDb()->GetGlyphSet(id);


	WgFontRes* fontRes = WgResourceXML::Cast<WgFontRes>(Parent());
	if( fontRes == 0 )
	{
		ASSERT(m_pGlyphSet == 0, "glyphset already exist");
		const std::string& filename = xmlNode["file"];

		s.ResDb()->AddGlyphSet(id, filename, new WgXMLMetaData(XmlNode()));
		m_pGlyphSet = s.ResDb()->GetGlyphSet(id);
		VERIFY(m_pGlyphSet, "invalid <glyphset>");

#ifdef WG_USE_FREETYPE

		if( m_pGlyphSet->GetType() == WgGlyphSet::VECTOR )
		{
			WgVectorGlyphs * pVectorGlyphs = (WgVectorGlyphs*)m_pGlyphSet;

			const std::string& mode = xmlNode["render_mode"];
			if( !mode.empty() )
			{
				if( mode == "monochrome" )
					pVectorGlyphs->SetRenderMode( WgVectorGlyphs::MONOCHROME );
				else if( mode == "crisp_edges" )
					pVectorGlyphs->SetRenderMode( WgVectorGlyphs::CRISP_EDGES );
				else if( mode == "best_shapes" )
					pVectorGlyphs->SetRenderMode( WgVectorGlyphs::BEST_SHAPES );
				else
					s.Warning("Unknown glyphset render_mode '" + mode + "'");

			}

			//

			const std::string& monosizes = xmlNode["monochrome_sizes"];
			if( !monosizes.empty() )
			{
				std::vector<std::string> tokens;
				WgUtil::Tokenize(monosizes, tokens);

				for( unsigned int i = 0 ; i < tokens.size() ; i++ )
					pVectorGlyphs->SetRenderMode( WgVectorGlyphs::MONOCHROME, WgUtil::ToSint32( tokens[i] ) );
			}

			//

			int size_offset = WgUtil::ToSint32( xmlNode["size_offset"] );
			pVectorGlyphs->SetSizeOffset( size_offset );
		}
#endif
	}
	else
	{
		const std::string& style = xmlNode["style"];
		const std::string& size = xmlNode["size"];
		VERIFY(m_pGlyphSet, "glyphset not set");

		WgFontStyle styleInt	= WgFontStyleRes::Deserialize(xmlNode, s);
		Uint32		sizeInt		= WgUtil::ToUint32(size);
		WgFont *	pFont		= fontRes->GetFont();

		switch( m_pGlyphSet->GetType() )
		{
#ifdef WG_USE_FREETYPE
			case WgGlyphSet::VECTOR:
				if( style.length() == 0 )
				{
					VERIFY( pFont->GetDefaultVectorGlyphs() == 0, "default vector glyphs defined more than once" );
					pFont->SetDefaultVectorGlyphs( static_cast<WgVectorGlyphs*>(m_pGlyphSet) );
				}
				else
				{
//					VERIFY( pFont->GetVectorGlyphs( styleInt, sizeInt ) == 0, "vector glyphs defined more than once for same style/size" );

					if( size.empty() )
						pFont->SetVectorGlyphs( static_cast<WgVectorGlyphs*>(m_pGlyphSet), styleInt );
					else
						pFont->SetVectorGlyphs( static_cast<WgVectorGlyphs*>(m_pGlyphSet), styleInt, sizeInt );
				}
				break;
#endif
			case WgGlyphSet::BITMAP:
				if( style.length() == 0 )
				{
					VERIFY( pFont->GetDefaultBitmapGlyphs( sizeInt ) == 0, "default bitmap glyphs for same size defined more than once" );
					pFont->SetDefaultBitmapGlyphs( static_cast<WgBitmapGlyphs*>(m_pGlyphSet), sizeInt );
				}
				else
				{
					VERIFY( pFont->GetBitmapGlyphs( styleInt, sizeInt ) == 0, "bitmap glyphs defined more than once for same style and size" );
					pFont->SetBitmapGlyphs( static_cast<WgBitmapGlyphs*>(m_pGlyphSet), styleInt, sizeInt );
				}
				break;

			default:
				VERIFY( false, "Unknown or unsupported glyphset subtype" );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
/// WgFontRes ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <font
//		id=[name]
//		glyphset=[default glyphset name]
// />
void WgFontRes::Serialize(WgResourceSerializerXML& s)
{
	WgResDB::FontRes* res = s.ResDb()->FindResFont(m_pFont);
	VERIFY(res, "font does not exist in ResDb");
	s.BeginTag(TagName());
	if(res->id.size())
		s.AddAttribute("id", res->id);

//	WriteCursorAttr(s, m_pFont->GetCursor(), "cursor");

	// Write default vector glyphs

#ifdef WG_USE_FREETYPE
	if( m_pFont->GetDefaultVectorGlyphs() )
		WriteGlyphSetAttr(s, m_pFont->GetDefaultVectorGlyphs(), "glyphset");

	// Write additional vector glyphs

	for(int style = WG_STYLE_NORMAL; style < WG_NB_FONTSTYLES; style++)
	{
		WgGlyphSet* pGlyphSet = m_pFont->GetVectorGlyphs((WgFontStyle)style,0);
		if(pGlyphSet)
			WgGlyphSetRes(this, pGlyphSet, (WgFontStyle)style).Serialize(s);
	}
#endif

	// Write bitmap glyphs


	for(int size = 0; size <= WG_MAX_FONTSIZE; size++)
	{
		WgGlyphSet* pGlyphSet = m_pFont->GetDefaultBitmapGlyphs(size);
		if(pGlyphSet)
			WgGlyphSetRes(this, pGlyphSet, size).Serialize(s);


		for(int style = WG_STYLE_NORMAL; style < WG_NB_FONTSTYLES; style++)
		{
			WgGlyphSet* pGlyphSet = m_pFont->GetBitmapGlyphs((WgFontStyle)style, size);
			if(pGlyphSet)
				WgGlyphSetRes(this, pGlyphSet, (WgFontStyle)style, size).Serialize(s);
		}
	}

	s.EndTag();
}

void WgFontRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	m_pFont = new WgFont();
	WgGlyphSet * p = s.ResDb()->GetGlyphSet(xmlNode["glyphset"]);

	if( p )
	{
		switch( p->GetType() )
		{
#ifdef WG_USE_FREETYPE
		case WgGlyphSet::VECTOR:
				m_pFont->SetDefaultVectorGlyphs( (WgVectorGlyphs*) p);
				break;
#endif
		case WgGlyphSet::BITMAP:
				m_pFont->SetDefaultBitmapGlyphs( (WgBitmapGlyphs*) p);			// Default for size 0, making it default for all sizes...
				break;

		default:
				VERIFY( false, "Unknown or unsupported glyphset subtype" );
		}
	}

//	m_pFont->SetCursor(s.ResDb()->GetCursor(xmlNode["cursor"]));
	s.ResDb()->AddFont(xmlNode["id"], m_pFont, new WgXMLMetaData(XmlNode()));
}

//////////////////////////////////////////////////////////////////////////
/// WgAnimRes ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgAnimRes::Serialize(WgResourceSerializerXML& s)
{
	const int legoMargin = 2;
	s.BeginTag(TagName(), XmlNode());
	WriteDiffAttr<Uint32>(s, XmlNode(), "state_w", m_pAnim->Size().w - legoMargin, 0);
	WriteDiffAttr<Uint32>(s, XmlNode(), "state_h", m_pAnim->Size().h, 0);
	WriteDiffAttr(s, XmlNode(), "playmode", FromPlayMode(m_pAnim->PlayMode()), FromPlayMode(WG_FORWARD_ONCE));
	WriteDiffAttr(s, XmlNode(), "timescale", m_pAnim->TimeScaler(), 1.f);
	WriteDiffAttr(s, XmlNode(), "duration", (int) m_pAnim->Duration(), (int)0);

	WgBorderRes::Serialize(s, XmlNode(), "borders", m_pAnim->GfxBorders());
	WgBlockFlagsRes::Serialize(s, this, XmlNode(), m_pAnim->BlockFlags() );
	s.EndTag();
}

void WgAnimRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	m_pAnim = new WgGfxAnim();
	m_pAnim->SetTimeScaler(WgUtil::ToFloat(xmlNode["timescale"]));
	m_pAnim->SetPlayMode(ToPlayMode(xmlNode["playmode"]));
	m_pAnim->SetGfxBorders( WgBorderRes::Deserialize(s, xmlNode["borders"]) );
	m_pAnim->SetBlockFlags( WgBlockFlagsRes::Deserialize(s, xmlNode) );

	const int legoMargin = 2;
	Uint32 duration = WgUtil::ToUint32(xmlNode["duration"]);

	WgResDB::LegoSource* source =  s.ResDb()->GetLegoSource(xmlNode["source"]);
	if(source)
	{
		m_pAnim->SetSize( source->rect.Size());

		WgSurface* pSurf = s.ResDb()->GetSurface(source->surface);
		VERIFY(pSurf, "surface doesn't exist");

		VERIFY(m_pAnim->AddFrames(pSurf, source->rect.Pos(), WgSize(source->nStates,1), duration, source->nStates, WgSize(legoMargin,0) ), "could not add frame to anim");

	}
	else
	{
		WgSurface* pSurf = s.ResDb()->GetSurface(xmlNode["surface"]);
		VERIFY(pSurf, "neither 'source' nor 'surface' defined in anim");

		int stateW = WgUtil::ToUint32(xmlNode["state_w"]);
		int stateH = WgUtil::ToUint32(xmlNode["state_h"]);
		int nStates = WgUtil::ToUint32(xmlNode["state_count"]);

		m_pAnim->SetSize( WgSize(stateW, stateH) );


		VERIFY(m_pAnim->AddFrames(pSurf, duration, nStates, WgSize(legoMargin,legoMargin) ), "could not add frame to anim");
	}

	s.ResDb()->AddAnim(xmlNode["id"], m_pAnim, new WgXMLMetaData(xmlNode));
}

std::string WgAnimRes::FromPlayMode(WgAnimMode mode)
{
	switch(mode)
	{
		case WG_FORWARD_ONCE:		return "fwd_once";
		case WG_BACKWARD_ONCE:		return "bwd_once";
		case WG_FORWARD_LOOPING:	return "fwd_loop";
		case WG_BACKWARD_LOOPING:	return "bwd_loop";
		case WG_FORWARD_PINGPONG:	return "fwd_pong";
		case WG_BACKWARD_PINGPONG:	return "bwd_pong";
		default:
			return "fwd_once";
	}
}

WgAnimMode WgAnimRes::ToPlayMode(const std::string& playmode)
{
	if(playmode.empty() ||
		playmode =="fwd_once")	return WG_FORWARD_ONCE;
	if(playmode == "bwd_once")	return WG_BACKWARD_ONCE;
	if(playmode == "fwd_loop")	return WG_FORWARD_LOOPING;
	if(playmode == "bwd_loop")	return WG_BACKWARD_LOOPING;
	if(playmode == "fwd_pong")	return WG_FORWARD_PINGPONG;
	if(playmode == "bwd_pong")	return WG_BACKWARD_PINGPONG;
	return WG_FORWARD_ONCE;
}

//////////////////////////////////////////////////////////////////////////
/// WgKeyFrameRes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgKeyFrameRes::Serialize(WgResourceSerializerXML& s)
{
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());
	WriteSurfaceAttr(s, m_pFrame->pSurf, "surface");
	WriteDiffAttr<Uint16>(s, xmlNode, "xofs", m_pFrame->ofs.x, 0);
	WriteDiffAttr<Uint16>(s, xmlNode, "yofs", m_pFrame->ofs.y, 0);
	WriteDiffAttr<Uint32>(s, xmlNode, "duration", m_pFrame->Duration(), 0);
	s.EndTag();
}

void WgKeyFrameRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgAnimRes* animRes = WgResourceXML::Cast<WgAnimRes>(Parent());
	VERIFY(animRes, "invalid parent for <frame>. should be <anim>");

	WgGfxAnim* anim = animRes->GetAnim();

	WgSurface* pSurf = s.ResDb()->GetSurface(xmlNode["surface"]);
	WgResDB::LegoSource* source =  s.ResDb()->GetLegoSource(xmlNode["source"]);
	VERIFY(source, "no source defined");
	VERIFY(pSurf, "no surface defined");
	Uint16 xOfs = WgUtil::ToUint16(xmlNode["xofs"]);
	Uint16 yOfs = WgUtil::ToUint16(xmlNode["yofs"]);
	Uint32 duration = WgUtil::ToUint32(xmlNode["duration"]);

	const int legoMargin = 2;
//	VERIFY(anim->addHorrTiledFrames(source->nStates, pSurf, source->rect.x, source->rect.y, duration, legoMargin), "could not add frame to anim");
	VERIFY(anim->AddFrames(pSurf, source->rect.Pos(), WgSize(source->nStates,1), duration, source->nStates, WgSize(legoMargin,0) ), "could not add frame to anim");
}

//////////////////////////////////////////////////////////////////////////
/// WgCursorRes //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgCursorRes::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	s.AddAttribute("eol", WgUtil::ToString(s.ResDb()->FindAnimId(m_pCursor->Anim(WgCursor::EOL)), m_pCursor->BearingX(WgCursor::EOL), m_pCursor->BearingY(WgCursor::EOL), m_pCursor->Advance(WgCursor::EOL) ));
	s.AddAttribute("ins", WgUtil::ToString(s.ResDb()->FindAnimId(m_pCursor->Anim(WgCursor::INS)), m_pCursor->BearingX(WgCursor::INS), m_pCursor->BearingY(WgCursor::INS), m_pCursor->Advance(WgCursor::INS) ));
	s.AddAttribute("ovr", WgUtil::ToString(s.ResDb()->FindAnimId(m_pCursor->Anim(WgCursor::OVR)), m_pCursor->BearingX(WgCursor::OVR), m_pCursor->BearingY(WgCursor::OVR), m_pCursor->Advance(WgCursor::OVR) ));

	s.AddAttribute("blitmode", BlitModeToString(m_pCursor->GetBlitMode()) );
	s.EndTag();
}

void WgCursorRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	m_pCursor = new WgCursor();
	std::string anim;

	WgCoord	bearing;
	int		spacing;

	VERIFY(WgUtil::FromString(xmlNode["eol"], anim, bearing.x, bearing.y, spacing ) == 4, "invalid EOL spec");
	m_pCursor->SetMode(WgCursor::EOL, (WgGfxAnim*)s.ResDb()->GetAnim(anim), bearing, spacing );

	VERIFY(WgUtil::FromString(xmlNode["ins"], anim, bearing.x, bearing.y, spacing ) == 4, "invalid INS spec");
	m_pCursor->SetMode(WgCursor::INS, (WgGfxAnim*)s.ResDb()->GetAnim(anim), bearing, spacing );

	VERIFY(WgUtil::FromString(xmlNode["ovr"], anim, bearing.x, bearing.y, spacing ) == 4, "invalid OVR spec");
	m_pCursor->SetMode(WgCursor::OVR, (WgGfxAnim*)s.ResDb()->GetAnim(anim), bearing, spacing );

	m_pCursor->SetBlitMode( StringToBlitMode(xmlNode["blitmode"]) );

	s.ResDb()->AddCursor(xmlNode["id"], m_pCursor, new WgXMLMetaData(xmlNode));
}


std::string	WgCursorRes::BlitModeToString( WgCursor::BlitMode mode )
{
	switch( mode )
	{
		case WgCursor::NORMAL:
			return "normal";
		case WgCursor::TINTED:
			return "tinted";
		case WgCursor::INVERT_BG:
			return "invert_bg";
	}

	return "normal";
}

WgCursor::BlitMode	WgCursorRes::StringToBlitMode( const std::string& str )
{
	if( str == "normal" )
	{
		return WgCursor::NORMAL;
	}
	else if( str == "tinted" )
	{
		return WgCursor::TINTED;
	}
	else if( str == "invert_bg" )
	{
		return WgCursor::INVERT_BG;
	}

	return WgCursor::NORMAL;
}




//////////////////////////////////////////////////////////////////////////
/// WgTextManagerRes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgTextManagerRes::Serialize(WgResourceSerializerXML& s)
{
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), xmlNode);

	WriteDiffAttr<float>(s, xmlNode, "size_stepping", m_pTextManager->GetSizeStepping(), 0.f);

	//

	if( m_pTextManager->GetSizeRounding() != WgTextManager::ROUND_NEAREST )
	{
		std::string	value;
		switch( m_pTextManager->GetSizeRounding() )
		{
			case WgTextManager::ROUND_NEAREST:
				value = "nearest";
				break;
			case WgTextManager::ROUND_UP:
				value = "up";
				break;
			case WgTextManager::ROUND_DOWN:
				value = "down";
				break;
		}
		s.AddAttribute( "size_rounding", value );
	}

	//

	if( m_pTextManager->GetAllowedSizes() )
	{
		float * pSizes = m_pTextManager->GetAllowedSizes();

		std::string	str;
		while( * pSizes != 0 )
		{
			if( !str.empty() )
				str += ",";

			str += WgUtil::ToString( * pSizes++ );
		}

		s.AddAttribute( "allowed_sizes", str );
	}

	//

	WriteDiffAttr<float>(s, xmlNode, "grow_treshold", m_pTextManager->GetGrowTreshold(), 1.f);
	WriteDiffAttr<float>(s, xmlNode, "grow_ratio", m_pTextManager->GetGrowRatio(), 1.f);
	WriteDiffAttr<float>(s, xmlNode, "grow_limit", m_pTextManager->GetGrowLimit(), 0.f);

	//

	WriteDiffAttr<float>(s, xmlNode, "shrink_treshold", m_pTextManager->GetShrinkTreshold(), 1.f);
	WriteDiffAttr<float>(s, xmlNode, "shrink_ratio", m_pTextManager->GetShrinkRatio(), 1.f);
	WriteDiffAttr<float>(s, xmlNode, "shrink_limit", m_pTextManager->GetShrinkLimit(), 0.f);

	//

	s.EndTag();
}

void WgTextManagerRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	m_pTextManager = new WgTextManager();

	float stepping = WgUtil::ToFloat(xmlNode["size_stepping"], 0.f );
	VERIFY( stepping >= 0.f, "size_stepping out of allowed range" );
	m_pTextManager->SetSizeStepping( stepping );

	if( xmlNode.HasAttribute("size_rounding") )
	{
		WgTextManager::Rounding rounding = WgTextManager::ROUND_NEAREST;
		std::string value = xmlNode["size_rounding"];

		if( value == "nearest" )
			rounding = WgTextManager::ROUND_NEAREST;
		else if( value == "up" )
			rounding = WgTextManager::ROUND_UP;
		else if( value == "down" )
			rounding = WgTextManager::ROUND_DOWN;
		else
			s.Error( "size_rounding set to unknown value '" + value + "', allowed values are 'nearest', 'up' and 'down'" );

		m_pTextManager->SetSizeRounding( rounding );
	}

	if( xmlNode.HasAttribute("allowed_sizes") )
	{
		std::vector<std::string> tokens;
		WgUtil::Tokenize(xmlNode["allowed_sizes"], tokens);

		float * pFloats = new float[tokens.size()];

		for( unsigned int i = 0 ; i < tokens.size() ; i++ )
			pFloats[i] = WgUtil::ToFloat( tokens[i] );

		bool res = m_pTextManager->SetAllowedSizes(tokens.size(), pFloats);
		delete [] pFloats;

		if( !res )
			s.Error( "allowed_sizes can not be set to '" + xmlNode["allowed_sizes"] + "'" );
	}

	float	treshold, ratio, limit;

	treshold	= WgUtil::ToFloat(xmlNode["grow_treshold"], 1.f );
	ratio		= WgUtil::ToFloat(xmlNode["grow_ratio"], 1.f );
	limit		= WgUtil::ToFloat(xmlNode["grow_limit"], 0.f );

	VERIFY( treshold >= 1.f , "grow_treshold must be >= 1.0" );
	VERIFY( ratio >= 0.f , "grow_ratio may not be negative" );
	VERIFY( limit == 0.f || limit >= 1.f , "grow_limit must be 0 or >= 1.0" );

	m_pTextManager->SetGrowFormula( treshold, ratio, limit );



	treshold	= WgUtil::ToFloat(xmlNode["shrink_treshold"], 1.f );
	ratio		= WgUtil::ToFloat(xmlNode["shrink_ratio"], 1.f );
	limit		= WgUtil::ToFloat(xmlNode["shrink_limit"], 0.f );

	VERIFY( treshold <= 1.f , "shrink_treshold must be <= 1.0" );
	VERIFY( ratio >= 0.f , "shrink_ratio may not be negative" );
	VERIFY( limit <= 1.f , "shrink_limit must be <= 1.0" );

	m_pTextManager->SetShrinkFormula( treshold, ratio, limit );


	s.ResDb()->AddTextManager(xmlNode["id"], m_pTextManager, new WgXMLMetaData(xmlNode));
}


//////////////////////////////////////////////////////////////////////////
/// WgSkinManagerRes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgSkinManagerRes::Serialize(WgResourceSerializerXML& s)
{
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), xmlNode);

	//

	//

	s.EndTag();
}

void WgSkinManagerRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	m_pSkinManager = new WgSkinManager();

	//

	//

	s.ResDb()->AddSkinManager(xmlNode["id"], m_pSkinManager, new WgXMLMetaData(xmlNode));
}



//////////////////////////////////////////////////////////////////////////
/// WgRectRes ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgRectRes::Serialize(WgResourceSerializerXML& s, const WgXmlNode& xmlNode, const WgRect& rect)
{
	bool bShort = xmlNode.HasAttribute("rect");
	if(bShort)
	{
		s.AddAttribute("rect", WgUtil::ToString(rect.x, rect.y, rect.w, rect.h));
	}
	else
	{
		WriteDiffAttr(s, xmlNode, "x", rect.x, 0);
		WriteDiffAttr(s, xmlNode, "y", rect.y, 0);
		s.AddAttribute("w", WgUtil::ToString(rect.w));
		s.AddAttribute("h", WgUtil::ToString(rect.h));
	}
}

WgRect WgRectRes::Deserialize(const WgXmlNode& xmlNode)
{
	WgRect rect;
	if(WgUtil::FromString(xmlNode["rect"], rect.x, rect.y, rect.w, rect.h) != 4)
	{
		WgUtil::FromString(xmlNode["x"], rect.x);
		WgUtil::FromString(xmlNode["y"], rect.y);
		WgUtil::FromString(xmlNode["w"], rect.w);
		WgUtil::FromString(xmlNode["h"], rect.h);
	}
	return rect;
}

//////////////////////////////////////////////////////////////////////////
/// WgBorderRes //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <borders left="" top="" bottom="" right="" all="" />
void WgBorderRes::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName());
	Sint32 b[4] = {m_Borders.left, m_Borders.top, m_Borders.right, m_Borders.bottom};
	if(b[0] == b[1] && b[0] == b[2] && b[0] == b[3])
	{
		s.AddAttribute("all", WgUtil::ToString(b[0]));
	}
	else
	{
		s.AddAttribute("left", WgUtil::ToString(b[0]));
		s.AddAttribute("top", WgUtil::ToString(b[1]));
		s.AddAttribute("right", WgUtil::ToString(b[2]));
		s.AddAttribute("bottom", WgUtil::ToString(b[3]));
	}
	s.EndTag();
}

void WgBorderRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgBlockSetRes* blockSetRes = WgResourceXML::Cast<WgBlockSetRes>(Parent());
	VERIFY(blockSetRes, "invalid parent for <borders>. should be <blockset>");

	WgBlockSetPtr blockSet = blockSetRes->GetBlockSet();

	m_Borders.Set(WgUtil::ToSint32(xmlNode["all"], 0));
	m_Borders.left = WgUtil::ToSint32(xmlNode["left"]);
	m_Borders.top = WgUtil::ToSint32(xmlNode["top"]);
	m_Borders.right = WgUtil::ToSint32(xmlNode["right"]);
	m_Borders.bottom = WgUtil::ToSint32(xmlNode["bottom"]);

	blockSet->SetGfxBorders(m_Borders);
}

void WgBorderRes::Serialize(WgResourceSerializerXML& s, const WgXmlNode& xmlNode, const std::string& attr, const WgBorders& borders, const WgBorders& def)
{
	if(xmlNode.HasAttribute(attr) || borders != def)
	{
		Sint32 b[4] = {borders.left, borders.top, borders.right, borders.bottom};
		if(b[0] == b[1] && b[0] == b[2] && b[0] == b[3])
			s.AddAttribute(attr, WgUtil::ToString(b[0]));
		else
			s.AddAttribute(attr, WgUtil::ToString(b[0], b[1], b[2], b[3]));
	}

}

WgBorders WgBorderRes::Deserialize(WgResourceSerializerXML& s, const std::string& value, const WgBorders& def)
{
	WgBorders borders;
	int n = WgUtil::FromString(value, borders.left, borders.top, borders.right, borders.bottom);
	if(n == 1)
		borders.top = borders.right = borders.bottom = borders.left;
	else if(n != 4)
		borders = def;
	return borders;
}


//////////////////////////////////////////////////////////////////////////
/// WgBlockFlagsRes //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgBlockFlagsRes::Serialize(WgResourceSerializerXML& s, WgResourceXML* tag, const WgXmlNode& xmlNode, Uint32 flags)
{
	Uint32 tileFlags = flags & WG_TILE_ALL;
	if(xmlNode.HasAttribute("tile") && (tileFlags == WG_TILE_ALL || tileFlags == 0))
		s.AddAttribute("tile", WgUtil::ToString(tileFlags ? true : false));
	else if(tileFlags)
		WgTileRes(tag, tileFlags).Serialize(s);			// Only allowed for blocksets.

	WriteDiffAttr(s, xmlNode, "scale", (flags & WG_SCALE_CENTER) != 0, false);
	WriteDiffAttr(s, xmlNode, "fixed_size", (flags & WG_FIXED_CENTER) != 0, false);

	if( flags & WG_SKIP_NORMAL )
		s.AddAttribute("skip_normal", "true" );
	if( flags & WG_SKIP_MARKED )
		s.AddAttribute("skip_marked", "true" );
	if( flags & WG_SKIP_SELECTED )
		s.AddAttribute("skip_selected", "true" );
	if( flags & WG_SKIP_DISABLED )
		s.AddAttribute("skip_disabled", "true" );
	if( flags & WG_SKIP_SPECIAL )
		s.AddAttribute("skip_special", "true" );
}

Uint32 WgBlockFlagsRes::Deserialize(WgResourceSerializerXML& s, const WgXmlNode& xmlNode)
{
	Uint32 flags = 0;WgUtil::ToBool(xmlNode["tile"]) ? WG_TILE_ALL : 0;
	bool scale = WgUtil::ToBool(xmlNode["scale"]);
	bool fixedSize = WgUtil::ToBool(xmlNode["fixed_size"]);


	if( WgUtil::ToBool(xmlNode["tile"]) )
		flags |= WG_TILE_ALL;
	if( WgUtil::ToBool(xmlNode["scale"]) )
		flags |= WG_SCALE_CENTER;
	if( WgUtil::ToBool(xmlNode["fixed_size"]) )
		flags |= WG_FIXED_CENTER;

	WARNIF((flags != 0 && flags != WG_TILE_ALL && flags != WG_SCALE_CENTER && flags != WG_FIXED_CENTER),
		"Cannot set more than one of scale, tile and fixed_size" );

	if( WgUtil::ToBool(xmlNode["skip_normal"]) )
		flags |= WG_SKIP_NORMAL;
	if( WgUtil::ToBool(xmlNode["skip_marked"]) )
		flags |= WG_SKIP_MARKED;
	if( WgUtil::ToBool(xmlNode["skip_selected"]) )
		flags |= WG_SKIP_SELECTED;
	if( WgUtil::ToBool(xmlNode["skip_disabled"]) )
		flags |= WG_SKIP_DISABLED;
	if( WgUtil::ToBool(xmlNode["skip_special"]) )
		flags |= WG_SKIP_SPECIAL;

	return flags;
}



//////////////////////////////////////////////////////////////////////////
/// WgOrientationRes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgOrientationRes::Serialize(WgResourceSerializerXML& s, const WgXmlNode& xmlNode, const std::string& attr, WgOrientation orientation, WgOrientation def)
{
	if( xmlNode.HasAttribute(attr) || orientation != def )
	{
		s.AddAttribute(attr, WgUtil::ToString(orientation));
	}
}

WgOrientation WgOrientationRes::Deserialize(WgResourceSerializerXML& s, const std::string& value, WgOrientation def)
{
	WgOrientation orientation;

	if( value.empty() )
		return def;

	bool b = WgUtil::FromString(value, orientation);
	WARNIF(!b, "invalid value for orientation. defaulting to northwest" );

	return orientation;
}


//////////////////////////////////////////////////////////////////////////
/// WgTileRes ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <tile
//		all=[true | false]
//		borders=[true | false]
//		center=[true | false]
//		left=[true | false]
//		top=[true | false]
//		bottom=[true | false]
//		right=[true | false]
// />
void WgTileRes::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName());

//	const WgXmlNode& xmlNode = XmlNode();

	if(m_tile == All)
	{
		s.AddAttribute("all", "true");
	}
	else
	{
		if(m_tile & Center) s.AddAttribute("center", "true");
		if(m_tile & Left) s.AddAttribute("left", "true");
		if(m_tile & Top) s.AddAttribute("top", "true");
		if(m_tile & Bottom) s.AddAttribute("bottom", "true");
		if(m_tile & Right) s.AddAttribute("right", "true");
	}

	s.EndTag();
}

void WgTileRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgBlockSetRes* blockSetRes = WgResourceXML::Cast<WgBlockSetRes>(Parent());
	VERIFY(blockSetRes, "invalid parent for <tile>. should be <blockset>");

	WgBlockSetPtr blockSet = blockSetRes->GetBlockSet();

	m_tile = 0;
	if(WgUtil::ToBool(xmlNode["all"]))		m_tile = All;
	if(WgUtil::ToBool(xmlNode["center"]))	m_tile |= Center;
	if(WgUtil::ToBool(xmlNode["left"]))	m_tile |= Left;
	if(WgUtil::ToBool(xmlNode["top"]))		m_tile |= Top;
	if(WgUtil::ToBool(xmlNode["bottom"]))	m_tile |= Bottom;
	if(WgUtil::ToBool(xmlNode["right"]))	m_tile |= Right;

	VERIFY(blockSet->SetTile(m_tile, true), "Invalid tile flags (is it already scaled?)");
}

//////////////////////////////////////////////////////////////////////////
/// WgBlockRes ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <block
//		mode=[WgMode]
//		x=[integer]
//		y=[integer]
// />
void WgBlockRes::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

//	const WgXmlNode& xmlNode = XmlNode();

	s.AddAttribute("mode", WgModeRes::Serialize(m_mode));
	s.AddAttribute("x", WgUtil::ToString(m_x));
	s.AddAttribute("y", WgUtil::ToString(m_y));

	s.EndTag();
}

void WgBlockRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgBlockSetRes* blockSetRes;
	VERIFY(blockSetRes = WgResourceXML::Cast<WgBlockSetRes>(Parent()), "invalid parent for <block>, should be <blockset>");

	m_mode = WgModeRes::Deserialize(xmlNode["mode"]);
	m_x = WgUtil::ToSint32(xmlNode["x"]);
	m_y = WgUtil::ToSint32(xmlNode["y"]);

	blockSetRes->GetBlockSet()->SetPos(m_mode, WgCoord(m_x, m_y) );
}


//////////////////////////////////////////////////////////////////////////
/// WgLegoRes ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgLegoRes::Serialize(WgResourceSerializerXML& s)
{
	assert(0);
}

void WgLegoRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	std::string name = xmlNode["id"];
	WgRect rect = WgRectRes::Deserialize(xmlNode);
	Uint32 nStates = WgUtil::ToUint32(xmlNode["states"], 1);
	std::string surf = xmlNode["surf"];
	rect.w = (rect.w + 2) / nStates - 2;
	s.ResDb()->AddLegoSource(name, surf, rect, nStates);
}

//////////////////////////////////////////////////////////////////////////
/// WgColorSetRes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//	<colorset
//		id=[name]
//		col=[color]
//	/>
void WgColorSetRes::Serialize(WgResourceSerializerXML& s)
{
	VERIFY(m_pColorSet, "null-ptr colorset");

	s.BeginTag(TagName());

	const WgXmlNode& xmlNode = XmlNode();

	WriteColorSetAttr(s, m_pColorSet, "id");

	WgColor	normalColor = m_pColorSet->Color(WG_MODE_NORMAL);

	WgColorRes::Serialize( s, xmlNode, "col", normalColor );

	if(m_pColorSet->Color(WG_MODE_MARKED) != normalColor )
		WgModePropRes(this, WG_MODE_MARKED).Serialize(s);
	if(m_pColorSet->Color(WG_MODE_SELECTED) != normalColor )
		WgModePropRes(this, WG_MODE_SELECTED).Serialize(s);
	if(m_pColorSet->Color(WG_MODE_DISABLED) != normalColor )
		WgModePropRes(this, WG_MODE_DISABLED).Serialize(s);
	if(m_pColorSet->Color(WG_MODE_SPECIAL) != normalColor )
		WgModePropRes(this, WG_MODE_SPECIAL).Serialize(s);

	s.EndTag();
}

void WgColorSetRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	std::string id = xmlNode["id"];
	VERIFY(id.size() != 0, "missing id in <colorset>");

	if(xmlNode.HasAttribute("col"))
		m_pColorSet = WgColorSet::Create( WgColorRes::Deserialize(s, xmlNode["col"]) );
	else
	{
		WgColor color;
		color.r = WgUtil::ToUint8(xmlNode["r"], 0);
		color.g = WgUtil::ToUint8(xmlNode["g"], 0);
		color.b = WgUtil::ToUint8(xmlNode["b"], 0);
		color.a = WgUtil::ToUint8(xmlNode["a"], 0xff);
		m_pColorSet = WgColorSet::Create(color);
	}
	s.ResDb()->AddColorSet( id, m_pColorSet, new WgXMLMetaData(XmlNode()) );
}



//////////////////////////////////////////////////////////////////////////
/// WgBlockSetRes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//	<blockset
//		id=[name]
//		surface=[name]
//		x=[integer]
//		y=[integer]
//		w=[integer]
//		h=[integer]
//		rect=[x,y,w,h]
//		blocks=[integer]
//		margin=[integer]
//		borders=[all]
//		borders=[left,right,top,bottom]
//		content_borders=[all]
//		content_borders=[left,right,top,bottom]
//		content_shift_marked=[x,y]
//		content_shift_pressed=[x,y]
//		tile=[true | false]
//		scale=[true | false]
//		fixed_size=[true | false]
//		skip_normal=[true | false]
//		skip_marked=[true | false]
//		skip_selected=[true | false]
//		skip_disabled=[true | false]
//		skip_special=[true | false]
//		text_colors=[colorprop]
// />
void WgBlockSetRes::Serialize(WgResourceSerializerXML& s)
{
	VERIFY(m_pBlockSet, "null-ptr blockset");

	s.BeginTag(TagName());

	const WgXmlNode& xmlNode = XmlNode();

	WriteBlockSetAttr(s, m_pBlockSet, "id");

	WgBorderRes::Serialize(s, xmlNode, "borders", m_pBlockSet->GfxBorders());
	WgBorderRes::Serialize(s, xmlNode, "content_borders", m_pBlockSet->ContentBorders());

	WgCoord shiftMarked = m_pBlockSet->ContentShift(WG_MODE_MARKED);
	WriteDiffAttr<Sint32>(s, xmlNode, "content_shift_marked", shiftMarked.x, shiftMarked.y, 0, 0 );
	WgCoord shiftPressed = m_pBlockSet->ContentShift(WG_MODE_SELECTED);
	WriteDiffAttr<Sint32>(s, xmlNode, "content_shift_pressed", shiftPressed.x, shiftPressed.y, 0, 0 );

	WgBlockFlagsRes::Serialize(s, this, xmlNode, m_pBlockSet->Flags() );

	/*
	Uint32 tileFlags = m_pBlockSet->Flags() & WG_TILE_ALL;
	if(xmlNode.HasAttribute("tile") && (tileFlags == WG_TILE_ALL || tileFlags == 0))
		s.AddAttribute("tile", WgUtil::ToString(tileFlags ? true : false));
	else if(tileFlags)
		WgTileRes(this, tileFlags).Serialize(s);

	WriteDiffAttr(s, xmlNode, "scale", (m_pBlockSet->Flags() & WG_SCALE_CENTER) != 0, false);
	WriteDiffAttr(s, xmlNode, "fixed_size", (m_pBlockSet->Flags() & WG_FIXED_CENTER) != 0, false);

	Uint32 flags = m_pBlockSet->Flags();
	if( flags & WG_SKIP_NORMAL )
		s.AddAttribute("skip_normal", "true" );
	if( flags & WG_SKIP_MARKED )
		s.AddAttribute("skip_marked", "true" );
	if( flags & WG_SKIP_SELECTED )
		s.AddAttribute("skip_selected", "true" );
	if( flags & WG_SKIP_DISABLED )
		s.AddAttribute("skip_disabled", "true" );
	if( flags & WG_SKIP_SPECIAL )
		s.AddAttribute("skip_special", "true" );
*/
	WriteColorSetAttr(s, m_pBlockSet->TextColors(), "text_colors" );

	WgRect rect[5];
	rect[WG_MODE_NORMAL] = m_pBlockSet->Rect(WG_MODE_NORMAL);
	rect[WG_MODE_MARKED] = m_pBlockSet->Rect(WG_MODE_MARKED);
	rect[WG_MODE_SELECTED] = m_pBlockSet->Rect(WG_MODE_SELECTED);
	rect[WG_MODE_DISABLED] = m_pBlockSet->Rect(WG_MODE_DISABLED);
	rect[WG_MODE_SPECIAL] = m_pBlockSet->Rect(WG_MODE_SPECIAL);

	bool bUsed[5] =
	{
		true,
		rect[WG_MODE_MARKED] != rect[WG_MODE_NORMAL],
		rect[WG_MODE_SELECTED] != rect[WG_MODE_NORMAL],
		rect[WG_MODE_DISABLED] != rect[WG_MODE_NORMAL],
		rect[WG_MODE_SPECIAL] != rect[WG_MODE_NORMAL],
	};

	int margin = 0;
	int nBlocks = 1;
	int iLastUsed = 0;
	bool bSubBlocks = false;
	for(int i = 1; i < 5; i++)
	{
		if(bUsed[i])
		{
			iLastUsed = i;
			nBlocks++;
			int m = rect[i].x - rect[iLastUsed].x - rect[iLastUsed].w;
			if(m != margin)
				bSubBlocks = true;
		}
	}

	if(nBlocks == 2 && bUsed[WG_MODE_DISABLED] == false)
		bSubBlocks = true;
	else if(nBlocks != iLastUsed + 1)
		bSubBlocks = true;

	if(xmlNode.HasAttribute("lego"))
	{
		WgResDB::LegoSource* source = s.ResDb()->GetLegoSource(xmlNode["lego"]);
		WgSurface* pSurface = s.ResDb()->GetSurface(source->surface);

		VERIFY(source != 0, "lego source '" + xmlNode["lego"] + "' not found");
		VERIFY(pSurface != 0, "surface '" + source->surface +  "' not found");

		s.AddAttribute("lego", source->id);
		s.AddAttribute("surface", source->surface);

		int iState[5];
		iState[WG_MODE_NORMAL] = StateFromRect(source->rect, rect[WG_MODE_NORMAL]);
		iState[WG_MODE_MARKED] = StateFromRect(source->rect, rect[WG_MODE_MARKED]);
		iState[WG_MODE_SELECTED] = StateFromRect(source->rect, rect[WG_MODE_SELECTED]);
		iState[WG_MODE_DISABLED] = StateFromRect(source->rect, rect[WG_MODE_DISABLED]);
		iState[WG_MODE_SPECIAL] = StateFromRect(source->rect, rect[WG_MODE_SPECIAL]);

		s.AddAttribute("states", WgUtil::ToString(iState[0], iState[1], iState[2], iState[3], iState[4]));
	}
	else
	{
		WriteSurfaceAttr(s, m_pBlockSet->Surface(), "surface");

		WgRectRes::Serialize(s, xmlNode, rect[WG_MODE_NORMAL]);

		if(bSubBlocks || !xmlNode.HasAttribute("blocks"))
		{
			for(int i = 1; i < 5; i++)
			{
				if(bUsed[i])
				{
					WgBlockRes b(this, WgMode(i), rect[i].x, rect[i].y);
					b.Serialize(s);
				}
			}
		}
		else
		{
			// compact mode
			WriteDiffAttr(s, xmlNode, "blocks", nBlocks, 1);
			WriteDiffAttr(s, xmlNode, "margin", margin, 2);
		}
	}
	s.EndTag();
}

WgRect WgBlockSetRes::StateRect(const WgRect& src, int iState)
{
	const int legoMargin = 2;
	return WgRect(src.x + (src.w + legoMargin) * iState, src.y, src.w, src.h);
}

int WgBlockSetRes::StateFromRect(const WgRect& src, const WgRect& stateRect)
{
	const int legoMargin = 2;
	return (stateRect.x - src.x) / (src.w + legoMargin);
}


void WgBlockSetRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	ASSERT(xmlNode.HasAttribute("lego") || xmlNode.HasAttribute("surface"), "no surface or lego tag in <blockset>");

	std::string id = xmlNode["id"];
	if(id.size() == 0)
		id = xmlNode["lego"];
	VERIFY(id.size() != 0, "missing id in <blockset>");

	bool bRequired = WgUtil::ToBool(xmlNode["required"], true);

	WgBorders gfxBorders = WgBorderRes::Deserialize(s, xmlNode["borders"]);
	WgBorders contentBorders = WgBorderRes::Deserialize(s, xmlNode["content_borders"]);

	WgCoord	shiftMarked;
	WgUtil::FromString(xmlNode["content_shift_marked"], shiftMarked.x, shiftMarked.y);
	
	WgCoord	shiftPressed;
	WgUtil::FromString(xmlNode["content_shift_pressed"], shiftPressed.x, shiftPressed.y);

	Uint32 flags = WgBlockFlagsRes::Deserialize(s,xmlNode);
/*
	Uint32 flags = WgUtil::ToBool(xmlNode["tile"]) ? WG_TILE_ALL : 0;
	bool scale = WgUtil::ToBool(xmlNode["scale"]);
	bool fixedSize = WgUtil::ToBool(xmlNode["fixed_size"]);



	VERIFY(!((scale && fixedSize) || (scale && (flags & WG_TILE_CENTER)) || (fixedSize && (flags & WG_TILE_CENTER))),
		"Cannot set more than one of scale, tile center and fixed_size" );

	if( WgUtil::ToBool(xmlNode["skip_normal"]) )
		flags |= WG_SKIP_NORMAL;
	if( WgUtil::ToBool(xmlNode["skip_marked"]) )
		flags |= WG_SKIP_MARKED;
	if( WgUtil::ToBool(xmlNode["skip_selected"]) )
		flags |= WG_SKIP_SELECTED;
	if( WgUtil::ToBool(xmlNode["skip_disabled"]) )
		flags |= WG_SKIP_DISABLED;
	if( WgUtil::ToBool(xmlNode["skip_special"]) )
		flags |= WG_SKIP_SPECIAL;
*/
	WgColorSetPtr pTextColors =	s.ResDb()->GetColorSet(xmlNode["text_colors"]);

	if(xmlNode.HasAttribute("lego"))
	{
		WgResDB::LegoSource* source = s.ResDb()->GetLegoSource(xmlNode["lego"]);
		VERIFY(source != 0, "lego source '" + xmlNode["lego"] + "' not found");

		WgSurface* pSurface = s.ResDb()->GetSurface(source->surface);
		VERIFY(pSurface != 0, "surface '" + source->surface +  "' not found");

		int iState0 = 0;
		int iState1 = 1;
		int iState2 = 2;
		int iState3 = 3;
		int iState4 = 4;
		int dummy = 0;

		WgRect rect = source->rect;

		int nStates = WgUtil::FromString(xmlNode["states"], iState0, iState1, iState2, iState3, iState4, dummy);
		if(nStates == 0)
			nStates = source->nStates;

		switch(nStates)
		{
		case 1: m_pBlockSet = pSurface->defineBlockSet(StateRect(rect, iState0), gfxBorders, contentBorders, pTextColors, flags); break;
		case 2: m_pBlockSet = pSurface->defineBlockSet(StateRect(rect, iState0), StateRect(rect, iState1), gfxBorders, contentBorders, pTextColors, flags); break;
		case 3: m_pBlockSet = pSurface->defineBlockSet(StateRect(rect, iState0), StateRect(rect, iState1), StateRect(rect, iState2), gfxBorders, contentBorders, pTextColors, flags); break;
		case 4: m_pBlockSet = pSurface->defineBlockSet(StateRect(rect, iState0), StateRect(rect, iState1), StateRect(rect, iState2), StateRect(rect, iState3), gfxBorders, contentBorders, pTextColors, flags); break;
		case 5: m_pBlockSet = pSurface->defineBlockSet(StateRect(rect, iState0), StateRect(rect, iState1), StateRect(rect, iState2), StateRect(rect, iState3), StateRect(rect, iState4), gfxBorders, contentBorders, pTextColors, flags); break;
		default:
			ASSERT(0, "invalid states definition in <blockset>");
		}
	}
	else
	{
		WgRect rect[5];
		rect[0] = WgRectRes::Deserialize(xmlNode);

		WgSurface* pSurface = s.ResDb()->GetSurface(xmlNode["surface"]);

		if( !bRequired && !pSurface )
			return;

		VERIFY(pSurface != 0, "missing surface '" + xmlNode["surface"] + "' in <blockset>");

		if(rect[0].w == 0 && rect[0].h == 0)
		{
			rect[0] = WgRect(0, 0, pSurface->Width(), pSurface->Height());
		}


		int nBlocks = WgUtil::ToSint32(xmlNode["blocks"]);
		int margin = WgUtil::ToSint32(xmlNode["margin"], 2);

		for(int i = 1; i < nBlocks; i++)
			rect[i] = WgRect(rect[i-1].x + rect[i-1].w + margin, rect[0].y, rect[0].w, rect[0].h);
		switch(nBlocks)
		{
		case 0:
		case 1: m_pBlockSet = pSurface->defineBlockSet(rect[0], gfxBorders, contentBorders, pTextColors, flags); break;
		case 2: m_pBlockSet = pSurface->defineBlockSet(rect[0], rect[1], gfxBorders, contentBorders, pTextColors, flags); break;
		case 3: m_pBlockSet = pSurface->defineBlockSet(rect[0], rect[1], rect[2], gfxBorders, contentBorders, pTextColors, flags); break;
		case 4: m_pBlockSet = pSurface->defineBlockSet(rect[0], rect[1], rect[2], rect[3], gfxBorders, contentBorders, pTextColors, flags); break;
		case 5: m_pBlockSet = pSurface->defineBlockSet(rect[0], rect[1], rect[2], rect[3], rect[4], gfxBorders, contentBorders, pTextColors, flags); break;
		}
	}
/*
	if(scale)
		m_pBlockSet->SetScale(scale);
	else if( fixedSize )
		m_pBlockSet->SetFixedSize(fixedSize);
*/

	m_pBlockSet->SetContentShift(WG_MODE_MARKED, shiftMarked);
	m_pBlockSet->SetContentShift(WG_MODE_SELECTED, shiftPressed);

	s.ResDb()->AddBlockSet( id, m_pBlockSet, new WgXMLMetaData(XmlNode()) );
}


//////////////////////////////////////////////////////////////////////////
/// WgAltRes /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//	<alt
//		activation_size=[w,h]
//		activation_width=[w]
//		activation_height=[h]
//		surface=[name]
//		x=[integer]
//		y=[integer]
//		w=[integer]
//		h=[integer]
//		rect=[x,y,w,h]
//		blocks=[integer]
//		margin=[integer]
//		borders=[all]
//		borders=[left,right,top,bottom]
//		content_borders=[all]
//		content_borders=[left,right,top,bottom]
//		content_shift_marked=[x,y]
//		content_shift_pressed=[x,y]
//		
// />
void WgAltRes::Serialize(WgResourceSerializerXML& s, int altNb )
{
	ASSERT(altNb > 0, "incorrect call to WgAltRes::Serialize()" );

	WgBlockSetRes* blockSetRes = WgResourceXML::Cast<WgBlockSetRes>(Parent());
	VERIFY(blockSetRes, "invalid parent for <alt>. should be <blockset>");
	WgBlockSetPtr pBlockSet = blockSetRes->GetBlockSet();


	s.BeginTag(TagName());

	const WgXmlNode& xmlNode = XmlNode();

	WgSize actSize = pBlockSet->ActivationSize(altNb);

	if( actSize.w !=0 && actSize.h != 0 )
		s.AddAttribute( "activation_size", WgUtil::ToString( actSize.w, actSize.h ));
	else if( actSize.w != 0 )
		s.AddAttribute( "activation_width", WgUtil::ToString( actSize.w ) );
	else if( actSize.h != 0 )
		s.AddAttribute( "activation_height", WgUtil::ToString( actSize.h ) );
	else
		assert(0);		// Invalid activation size, both set to 0!

	WgBorderRes::Serialize(s, xmlNode, "borders", pBlockSet->GfxBorders(altNb));
	WgBorderRes::Serialize(s, xmlNode, "content_borders", pBlockSet->ContentBorders(altNb));

	WgCoord shiftMarked = pBlockSet->ContentShift(WG_MODE_MARKED,altNb);
	WriteDiffAttr<Sint32>(s, xmlNode, "content_shift_marked", shiftMarked.x, shiftMarked.y, 0, 0 );
	WgCoord shiftPressed = pBlockSet->ContentShift(WG_MODE_SELECTED,altNb);
	WriteDiffAttr<Sint32>(s, xmlNode, "content_shift_pressed", shiftPressed.x, shiftPressed.y, 0, 0 );

	WgRect rect[5];
	rect[WG_MODE_NORMAL] = pBlockSet->Rect(WG_MODE_NORMAL,altNb);
	rect[WG_MODE_MARKED] = pBlockSet->Rect(WG_MODE_MARKED,altNb);
	rect[WG_MODE_SELECTED] = pBlockSet->Rect(WG_MODE_SELECTED,altNb);
	rect[WG_MODE_DISABLED] = pBlockSet->Rect(WG_MODE_DISABLED,altNb);
	rect[WG_MODE_SPECIAL] = pBlockSet->Rect(WG_MODE_SPECIAL,altNb);

	bool bUsed[5] =
	{
		true,
		rect[WG_MODE_MARKED] != rect[WG_MODE_NORMAL],
		rect[WG_MODE_SELECTED] != rect[WG_MODE_NORMAL],
		rect[WG_MODE_DISABLED] != rect[WG_MODE_NORMAL],
		rect[WG_MODE_SPECIAL] != rect[WG_MODE_NORMAL],
	};

	int margin = 0;
	int nBlocks = 1;
	int iLastUsed = 0;
	bool bSubBlocks = false;
	for(int i = 1; i < 5; i++)
	{
		if(bUsed[i])
		{
			iLastUsed = i;
			nBlocks++;
			int m = rect[i].x - rect[iLastUsed].x - rect[iLastUsed].w;
			if(m != margin)
				bSubBlocks = true;
		}
	}

	if(nBlocks == 2 && bUsed[WG_MODE_DISABLED] == false)
		bSubBlocks = true;
	else if(nBlocks != iLastUsed + 1)
		bSubBlocks = true;

	if(xmlNode.HasAttribute("lego"))
	{
		WgResDB::LegoSource* source = s.ResDb()->GetLegoSource(xmlNode["lego"]);
		WgSurface* pSurface = s.ResDb()->GetSurface(source->surface);

		VERIFY(source != 0, "lego source '" + xmlNode["lego"] + "' not found");
		VERIFY(pSurface != 0, "surface '" + source->surface +  "' not found");

		s.AddAttribute("lego", source->id);
		s.AddAttribute("surface", source->surface);

		int iState[5];
		iState[WG_MODE_NORMAL] = StateFromRect(source->rect, rect[WG_MODE_NORMAL]);
		iState[WG_MODE_MARKED] = StateFromRect(source->rect, rect[WG_MODE_MARKED]);
		iState[WG_MODE_SELECTED] = StateFromRect(source->rect, rect[WG_MODE_SELECTED]);
		iState[WG_MODE_DISABLED] = StateFromRect(source->rect, rect[WG_MODE_DISABLED]);
		iState[WG_MODE_SPECIAL] = StateFromRect(source->rect, rect[WG_MODE_SPECIAL]);

		s.AddAttribute("states", WgUtil::ToString(iState[0], iState[1], iState[2], iState[3], iState[4]));
	}
	else
	{
		WriteSurfaceAttr(s, pBlockSet->Surface(altNb), "surface");

		WgRectRes::Serialize(s, xmlNode, rect[WG_MODE_NORMAL]);

		if(bSubBlocks || !xmlNode.HasAttribute("blocks"))
		{
			for(int i = 1; i < 5; i++)
			{
				if(bUsed[i])
				{
					WgBlockRes b(this, WgMode(i), rect[i].x, rect[i].y);
					b.Serialize(s);
				}
			}
		}
		else
		{
			// compact mode
			WriteDiffAttr(s, xmlNode, "blocks", nBlocks, 1);
			WriteDiffAttr(s, xmlNode, "margin", margin, 2);
		}
	}
	s.EndTag();
}

WgRect WgAltRes::StateRect(const WgRect& src, int iState)
{
	const int legoMargin = 2;
	return WgRect(src.x + (src.w + legoMargin) * iState, src.y, src.w, src.h);
}

int WgAltRes::StateFromRect(const WgRect& src, const WgRect& stateRect)
{
	const int legoMargin = 2;
	return (stateRect.x - src.x) / (src.w + legoMargin);
}

void WgAltRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	ASSERT(xmlNode.HasAttribute("lego") || xmlNode.HasAttribute("rect") || (xmlNode.HasAttribute("w") && xmlNode.HasAttribute("h")), "missing rectangle in <alt>");

	WgBlockSetRes* blockSetRes = WgResourceXML::Cast<WgBlockSetRes>(Parent());
	VERIFY(blockSetRes, "invalid parent for <alt>. should be <blockset>");

	WgBlockSetPtr pBlockSet = blockSetRes->GetBlockSet();

	WgBorders gfxBorders = WgBorderRes::Deserialize(s, xmlNode["borders"]);
	WgBorders contentBorders = WgBorderRes::Deserialize(s, xmlNode["content_borders"]);


	WgCoord	shiftMarked;
	WgUtil::FromString(xmlNode["content_shift_marked"], shiftMarked.x, shiftMarked.y);
	
	WgCoord	shiftPressed;
	WgUtil::FromString(xmlNode["content_shift_pressed"], shiftPressed.x, shiftPressed.y);

	//

	WgSize activationSize = WgSize(0,0);

	if( xmlNode.HasAttribute("activation_size") )
	{
		WgUtil::FromString(xmlNode["activation_size"], activationSize.w, activationSize.h );
	}
	else
	{
		activationSize.w = WgUtil::ToSint32(xmlNode["activation_width"]);
		activationSize.h = WgUtil::ToSint32(xmlNode["activation_height"]);
	}

	// Fill in the surface and rectangles

	WgSurface * pSurface = 0;
	WgRect rect[5];		// normal, marked, selected, disabled, special
	int		nRects = 0;

	if(xmlNode.HasAttribute("lego"))
	{
		WgResDB::LegoSource* source = s.ResDb()->GetLegoSource(xmlNode["lego"]);
		VERIFY(source != 0, "lego source '" + xmlNode["lego"] + "' not found");

		pSurface = s.ResDb()->GetSurface(source->surface);
		VERIFY(pSurface != 0, "surface '" + source->surface +  "' not found");

		// Create a remap table "iStates" for what rectangle to use for which state.

		int	iStates[5] = { 0,1,2,3,4 };					// normal, marked, selected, disabled, special
		int dummy = 0;

		WgRect big_rect = source->rect;

		int nStates = WgUtil::FromString(xmlNode["states"], iStates[0], iStates[1], iStates[2], iStates[3], iStates[4], dummy);
		if(nStates == 0)
			nStates = source->nStates;

		ASSERT( nStates > 0 && nStates <= 5, "invalid states definition in <alt>");

		// Fill in the rectangles

		for( int i = 0 ; i < nStates ; i++ )
			rect[i] = StateRect( big_rect, iStates[i] );

		nRects = nStates;
	}
	else
	{
		rect[0] = WgRectRes::Deserialize(xmlNode);

		pSurface = s.ResDb()->GetSurface(xmlNode["surface"]);
		VERIFY(pSurface != 0, "missing surface '" + xmlNode["surface"] + "' in <blockset>");

		if(rect[0].w == 0 && rect[0].h == 0)
		{
			rect[0] = WgRect(0, 0, pSurface->Width(), pSurface->Height());
		}

		int nBlocks = WgUtil::ToSint32(xmlNode["blocks"]);
		int margin = WgUtil::ToSint32(xmlNode["margin"], 2);

		for(int i = 1; i < nBlocks; i++)
			rect[i] = WgRect(rect[i-1].x + rect[i-1].w + margin, rect[0].y, rect[0].w, rect[0].h);

		nRects = nBlocks;
	}

	for( int i = nRects ; i < 5 ; i++ )
		rect[i] = rect[0];					// Fill up with normal for unspecified ones.

	pBlockSet->AddAlternative( activationSize, pSurface, rect[0], rect[1], rect[2],
							   rect[3], rect[4], gfxBorders, contentBorders, shiftMarked, shiftPressed );
}


//////////////////////////////////////////////////////////////////////////
/// WgIconHolderRes //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	icon_borders		= [borders]
//	icon_orientation	= [orientation]
//	icon_scale			= [0 - 1.0]
//	icon_push_text		= [bool]

void WgIconHolderRes::Serialize(WgResourceXML* pThis, const WgXmlNode& xmlNode, WgResourceSerializerXML& s, WgIconHolder* holder)
{
	WgBorderRes::Serialize(s, xmlNode, "icon_borders", holder->IconBorders());
	WgOrientationRes::Serialize(s, xmlNode, "icon_orientation", holder->IconOrientation(), WG_WEST);
	WriteDiffAttr(s, xmlNode, "icon_scale", holder->IconScale(), 0.f);
	WriteDiffAttr(s, xmlNode, "icon_push_text", holder->IsIconPushingText(), true);
}

void WgIconHolderRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s, WgIconHolder* holder)
{
	holder->SetIconBorders(WgBorderRes::Deserialize(s, xmlNode["icon_borders"]));
	holder->SetIconOrientation(WgOrientationRes::Deserialize(s, xmlNode["icon_orientation"], WG_WEST));

	holder->SetIconScale(WgUtil::ToFloat(xmlNode["icon_scale"], 0.f));
	holder->SetIconPushingText(WgUtil::ToBool(xmlNode["icon_push_text"], true));
}

//////////////////////////////////////////////////////////////////////////
/// WgTileHolderRes //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	tile_colors			= [colorset]
//	odd_tile_colors		= [colorset]
//	even_tile_colors	= [colorset]
//	tile_blocks			= [blockset]
//	odd_tile_blocks		= [blockset]
//	even_tile_blocks	= [blockset]

void WgTileHolderRes::Serialize(WgResourceXML* pThis, const WgXmlNode& xmlNode, WgResourceSerializerXML& s, WgTileHolder* holder)
{
	// row colors

	if( holder->OddTileColors() == holder->EvenTileColors() )
		WriteColorSetAttr(s, holder->OddTileColors(), "tile_colors" );
	else
	{
		WriteColorSetAttr(s, holder->OddTileColors(), "odd_tile_colors" );
		WriteColorSetAttr(s, holder->EvenTileColors(), "even_tile_colors" );
	}

	// row blocks

	if( holder->OddTileBlocks() == holder->EvenTileBlocks() )
		WriteBlockSetAttr(s, holder->OddTileBlocks(), "tile_blocks" );
	else
	{
		WriteBlockSetAttr(s, holder->OddTileBlocks(), "odd_tile_blocks" );
		WriteBlockSetAttr(s, holder->EvenTileBlocks(), "even_tile_blocks" );
	}
}

void WgTileHolderRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s, WgTileHolder* holder)
{
	if( xmlNode.HasAttribute("tile_blocks") )
		holder->SetTileBlocks( s.ResDb()->GetBlockSet(xmlNode["tile_blocks"]) );
	else
		holder->SetTileBlocks( s.ResDb()->GetBlockSet(xmlNode["odd_tile_blocks"]), s.ResDb()->GetBlockSet(xmlNode["even_tile_blocks"]) );

	if( xmlNode.HasAttribute("tile_colors") )
		holder->SetTileColors( s.ResDb()->GetColorSet(xmlNode["tile_colors"]) );
	else
		holder->SetTileColors( s.ResDb()->GetColorSet(xmlNode["odd_tile_colors"]), s.ResDb()->GetColorSet(xmlNode["even_tile_colors"]) );
}


//////////////////////////////////////////////////////////////////////////
/// WgTextHolderRes //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// prop=[textprop]
// selection_prop=[textprop]
// link_prop=[textprop]
// base_colors=[colorset]
// cursor=[ref]
// text=[string]
// textalign=[origo]
// tint=[mul | opaque]
// linespaceadjustment=[value]
// max_chars=[value]
// wrap=[true | false]
// ellipsis=[true | false]
// textmanager=[textmanager]

void WgTextHolderRes::Serialize(WgResourceXML* pThis, const WgXmlNode& xmlNode, WgResourceSerializerXML& s, Wg_Interface_TextHolder* holder)
{

	WriteTextPropAttr(s, holder->GetTextProperties(), "prop");
	WriteTextPropAttr(s, holder->GetSelectionProperties(), "selection_prop");
	WriteTextPropAttr(s, holder->GetLinkProperties(), "link_prop");
	WriteColorSetAttr(s, holder->TextBaseColors(), "base_colors" );

	WgCursor * pCursor = holder->GetCursor();
	if( pCursor )
	{
		std::string id = s.ResDb()->FindCursorId(pCursor);
		if( id.empty() )
		{
			id = s.ResDb()->GenerateName(pCursor);
			s.ResDb()->AddCursor(id, pCursor);				//TODO: It's probably too late to add it here...
		}

		s.AddAttribute("cursor", id);
	}
	std::string cursorId = xmlNode["cursor"];
	if( cursorId.length() )
	{
		WgCursor * pCursor = s.ResDb()->GetCursor(cursorId);
		if( pCursor )
			holder->SetCursor(pCursor);
	}


	WgText *pTextObj = holder->TextObj();
	const WgChar *pText = pTextObj->getText();
	WriteTextAttrib(s, pText, "text");

	WriteDiffAttr(s, xmlNode, "textalign", holder->TextAlignment(), WgOrigo::topLeft());
/*
	WgColor defSel(0);
	WgResDB::ColorRes* colorRes = s.ResDb()->GetResColor("TextSelectionColor");
	if(colorRes)
		defSel = colorRes->res;
	WgColorRes::Serialize(s, xmlNode, "selection_color", holder->GetSelectionColor(), defSel);
*/
	if(xmlNode.HasAttribute("tint") || holder->TextTintMode() != WG_TINTMODE_MULTIPLY)
	{
		if(holder->TextTintMode() == WG_TINTMODE_MULTIPLY)
			s.AddAttribute("tint", "mul");
		else
			s.AddAttribute("tint", "opaque");
	}

	WriteDiffAttr<Sint8>(s, xmlNode, "linespaceadjustment", holder->GetLineSpaceAdjustment(), 0);
	WriteDiffAttr(s, xmlNode, "max_chars", holder->MaxChars(), INT_MAX);
	WriteDiffAttr(s, xmlNode, "wrap", holder->GetTextWrap(), true);
	WriteDiffAttr(s, xmlNode, "ellipsis", holder->AutoEllipsis(), holder->IsAutoEllipsisDefault() );

	WriteTextManager(holder, s);

/*	WgTextPropPtr pCurProp = 0;

	Uint16* textBuf = new Uint16[pTextObj->nbChars()];
	int iBuf = 0;
	for(; pText->GetGlyph(); pText++)
	{
		// diff with default prop
		if(pCurProp != pText->GetProperties())
		{
			textBuf[iBuf] = 0;
			if(!pCurProp)
			{
				WriteText(s, textBuf);
			}
			else
			{
				// add <prop> tag with new text attributes for this sequence
				WgPropRes(pThis, holder->GetTextDefaultProperties(), pCurProp, textBuf).Serialize(s);
			}
			iBuf = 0;
			pCurProp = pText->GetProperties();
			if(pCurProp == holder->GetTextDefaultProperties())
				pCurProp = 0;
		}

		textBuf[iBuf++] = pText->GetGlyph();
	}
	if(iBuf > 0)
	{
		textBuf[iBuf] = 0;
		WriteText(s, textBuf);
	}
	delete[] textBuf;
	*/
}

void WgTextHolderRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s, Wg_Interface_TextHolder* holder)
{
	WgTextPropPtr prop = s.ResDb()->GetTextProp(xmlNode["prop"]);
	if(prop)
		holder->SetTextProperties(prop);

	prop = s.ResDb()->GetTextProp(xmlNode["selection_prop"]);
	if(prop)
		holder->SetSelectionProperties(prop);

	prop = s.ResDb()->GetTextProp(xmlNode["link_prop"]);
	if(prop)
		holder->SetLinkProperties(prop);

	std::string cursorId = xmlNode["cursor"];
	if( cursorId.length() )
	{
		WgCursor * pCursor = s.ResDb()->GetCursor(cursorId);
		if( pCursor )
			holder->SetCursor(pCursor);
	}

	std::string text = ReadLocalizedString(xmlNode["text"], s);
	if(text.length())
	{
		if(text[0] == ':')
			text = s.ResDb()->LoadString(text.substr(1));
		holder->SetText( WgCharSeq( text, s.ResDb() ) );
	}

	holder->SetTextAlignment(WgUtil::ToOrigo(xmlNode["textalign"]));
/*
	if( xmlNode.HasAttribute("selection_color") )
		holder->SetSelectionColor( WgColorRes::Deserialize(s, xmlNode["selection_color"]) );
	else
		holder->SetSelectionColor( WgColorRes::Deserialize(s, "#TextSelectionColor") );
*/
	WgTintMode tint = WG_TINTMODE_MULTIPLY;
	if(xmlNode["tint"] == "opaque")
		tint = WG_TINTMODE_OPAQUE;
	holder->SetTextTintMode(tint);

	holder->SetLineSpaceAdjustment(WgUtil::ToSint8(xmlNode["linespaceadjustment"]));
	holder->SetTextWrap(WgUtil::ToBool(xmlNode["wrap"], true));

	if( xmlNode.HasAttribute("max_chars") )
		holder->SetMaxChars(WgUtil::ToSint32(xmlNode["max_chars"]));

	if( xmlNode.HasAttribute("ellipsis"))
		holder->SetAutoEllipsis(WgUtil::ToBool(xmlNode["ellipsis"]));

	if(xmlNode.HasAttribute("base_colors"))
	{
		holder->SetTextBaseColors(s.ResDb()->GetColorSet(xmlNode["base_colors"]));

	}

	ReadTextManager(holder, xmlNode, s);
}

void WgTextHolderRes::DeserializeText(const char * pChars, int len)
{
	GetCharBuffer()->PushBack(WgCharSeq(pChars, len));
}

//////////////////////////////////////////////////////////////////////////
/// WgEditTextRes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgEditTextRes::Serialize(WgResourceXML* pThis, const WgXmlNode& xmlNode, WgResourceSerializerXML& s, WgInterfaceEditText* holder)
{
	WgTextHolderRes::Serialize( pThis, xmlNode, s, holder );

	WgTextEditMode mode = holder->GetEditMode();

	std::string defMode = "static";
	std::string modeName;
	switch(mode)
	{
	case WG_TEXT_SELECTABLE: modeName = "selectable"; break;
	case WG_TEXT_EDITABLE: modeName = "editable"; break;
	default: modeName = defMode;
	}

	WriteDiffAttr(s, xmlNode, "editmode", modeName, defMode);
}

void WgEditTextRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s, WgInterfaceEditText* holder)
{
	WgTextHolderRes::Deserialize( xmlNode, s, holder );
	bool bEditable = WgUtil::ToBool(xmlNode["edit"], false);

	if(bEditable)
	{
		holder->SetEditMode(WG_TEXT_EDITABLE);
	}
	else
	{
		std::string modeName = xmlNode["editmode"];
		if(modeName == "selectable")
			holder->SetEditMode(WG_TEXT_SELECTABLE);
		else if(modeName == "editable")
			holder->SetEditMode(WG_TEXT_EDITABLE);
	}
}


//////////////////////////////////////////////////////////////////////////


void WgPropRes::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName());

	std::string id = s.ResDb()->FindTextPropId(m_pProp);

	if(id.size())
	{
		s.AddAttribute("id", id);
	}
	else
	{
		if(m_pProp->Font() != m_pDefaultProp->Font())
		{
			WriteFontAttr(s, m_pProp->Font(), "font");
		}

		if(m_pProp->Style() != m_pDefaultProp->Style())
		{
			s.AddAttribute("style", WgFontStyleRes::Serialize(m_pProp->Style(), s));
		}

		if(m_pProp->IsUnderlined() != m_pDefaultProp->IsUnderlined())
		{
			s.AddAttribute("underlined", WgUtil::ToString(m_pProp->IsUnderlined()));
		}

		// have to write color if it's colored
		if(m_pProp->IsColored())
		{
			WgColorRes::Serialize(s, XmlNode(), "col", m_pProp->Color(), WgColor(0, m_pProp->Color().a + 1));
		}
	}

	WriteText(s, m_textOut);

	s.EndTag();
}

void WgPropRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgTextHolderRes* textHolder = WgResourceXML::Cast<WgTextHolderRes>(Parent());
	VERIFY(textHolder, "invalid parent for <prop>. should be text holder");

	m_pProp = 0;

	std::string id = xmlNode["id"];
	if(id.size())
	{
		m_pProp = s.ResDb()->GetTextProp(id);
		ASSERT(m_pProp, "undefined reference to textprop: " + id);
	}
	else
	{
		WgFont * pFont = s.ResDb()->GetFont( xmlNode["font"] );
		if( pFont )
			m_prop.SetFont( pFont );

		if(xmlNode.HasAttribute("col"))
			m_prop.SetColor( WgColorRes::Deserialize(s, xmlNode["col"]) );

		m_prop.SetStyle( WgFontStyleRes::Deserialize(xmlNode, s) );

		if( WgUtil::ToBool(xmlNode["underlined"]) )
			m_prop.SetUnderlined();
		else
			m_prop.ClearUnderlined();
	}
}

void WgPropRes::Deserialized(WgResourceSerializerXML& s)
{
	WgTextHolderRes* textHolder = WgResourceXML::Cast<WgTextHolderRes>(Parent());
	if(textHolder)
	{
		if(m_pProp)
		{
			// Register WgTextProp and save our reference.
			m_pProp = m_prop.Register();
		}

		int ofs = textHolder->GetCharBuffer()->NbChars();
		int len = m_textIn.size();
		textHolder->GetCharBuffer()->PushBack(WgCharSeq(m_textIn.c_str(), len));
		textHolder->GetCharBuffer()->SetProperties(m_pProp, ofs, len);
	}
}

void WgPropRes::DeserializeText(const char * pChars, int len)
{
	m_textIn.insert(m_textIn.size(), pChars, len);
}

WgCharBuffer* WgPropRes::GetCharBuffer()
{
	WgTextHolderRes* textRes = WgResourceXML::Cast<WgTextHolderRes>(Parent());
	return textRes ? textRes->GetCharBuffer() : 0;
}

//////////////////////////////////////////////////////////////////////////
void WgBoldTextRes::DeserializeText(const char * pChars, int len)
{
	GetCharBuffer()->PushBack(WgCharSeq(pChars, len));
}

void WgBoldTextRes::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());
	WgTextHolderRes::Serialize(this, XmlNode(), s, 0);
	s.EndTag();
}

void WgBoldTextRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgTextHolderRes* textRes = WgResourceXML::Cast<WgTextHolderRes>(Parent());
	VERIFY(textRes, "<b> tag requires a parent tag that support text");
}

WgCharBuffer* WgBoldTextRes::GetCharBuffer()
{
	WgTextHolderRes* textRes = WgResourceXML::Cast<WgTextHolderRes>(Parent());
	return textRes ? textRes->GetCharBuffer() : 0;
}

//////////////////////////////////////////////////////////////////////////

void WgItalicTextRes::DeserializeText(const char * pChars, int len)
{
	Uint32 ofs = GetCharBuffer()->Size();
	GetCharBuffer()->PushBack(WgCharSeq(pChars, len));
	GetCharBuffer()->SetStyle(WG_STYLE_ITALIC, ofs, len);
}

void WgItalicTextRes::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());
	WgTextHolderRes::Serialize(this, XmlNode(), s, 0);
	s.EndTag();
}

void WgItalicTextRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgTextHolderRes* textRes = WgResourceXML::Cast<WgTextHolderRes>(Parent());
	VERIFY(textRes, "<i> tag requires a parent tag that support text");
}

WgCharBuffer* WgItalicTextRes::GetCharBuffer()
{
	WgTextHolderRes* textRes = WgResourceXML::Cast<WgTextHolderRes>(Parent());
	return textRes ? textRes->GetCharBuffer() : 0;
}

//////////////////////////////////////////////////////////////////////////

void WgUnderlinedTextRes::DeserializeText(const char * pChars, int len)
{
	Uint32 ofs = GetCharBuffer()->Size();
	GetCharBuffer()->PushBack(WgCharSeq(pChars, len));
	GetCharBuffer()->SetUnderlined(ofs, len);
}

void WgUnderlinedTextRes::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());
	WgTextHolderRes::Serialize(this, XmlNode(), s, 0);
	s.EndTag();
}

void WgUnderlinedTextRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgTextHolderRes* textRes = WgResourceXML::Cast<WgTextHolderRes>(Parent());
	VERIFY(textRes, "<u> tag requires a parent tag that support text");
}

WgCharBuffer* WgUnderlinedTextRes::GetCharBuffer()
{
	WgTextHolderRes* textRes = WgResourceXML::Cast<WgTextHolderRes>(Parent());
	return textRes ? textRes->GetCharBuffer() : 0;
}

//////////////////////////////////////////////////////////////////////////

void WgBreakTextRes::DeserializeText(const char * pChars, int len)
{
	assert(0);
	WgTextHolderRes::DeserializeText(pChars, len);
}

void WgBreakTextRes::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());
	s.EndTag();
}

void WgBreakTextRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgTextHolderRes* textRes = WgResourceXML::Cast<WgTextHolderRes>(Parent());
	VERIFY(textRes, "<br> tag requires a parent tag that support text");
	GetCharBuffer()->PushBack(WgCharSeq("\n"));
}

WgCharBuffer* WgBreakTextRes::GetCharBuffer()
{
	WgTextHolderRes* textRes = WgResourceXML::Cast<WgTextHolderRes>(Parent());
	return textRes ? textRes->GetCharBuffer() : 0;
}

//////////////////////////////////////////////////////////////////////////
/// WgWidgetRes //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// < ...
//		tooltipdelay=[integer]
//		layer=[integer]
//		pointermask=[mask]
//		pointerstyle=[style]
//		minsize=[w, h]
//		maxsize=[w, h]
//		enabled=[true | false]
//		modal=[true | false]
//		geo=[x, y, w, h]
//		id=[name]
//		uid=[integer]
//		visible=[true | false]
// ... />

WgWidgetRes::~WgWidgetRes()
{
}

void WgWidgetRes::Serialize(WgResourceSerializerXML& s)
{
	const WgXmlNode& xmlNode = XmlNode();

	WriteDiffAttr(s, xmlNode, "tooltipdelay", m_Widget->TooltipDelay(), 500);
	WriteDiffAttr(s, xmlNode, "layer", m_Widget->Layer(), (Uint8)WG_DEFAULT_LAYER);
	WgPointerMaskRes::Serialize(s, xmlNode, "pointermask", m_Widget->GetPointerMask());
	WgPointerStyleRes::Serialize(s, xmlNode, "pointerstyle", m_Widget->GetPointerStyle());
	WgSizeRes::Serialize(s, xmlNode, "minsize", m_Widget->GetMinSizeUser(), WgSize(1, 1));
	WgSizeRes::Serialize(s, xmlNode, "maxsize", m_Widget->GetMaxSizeUser(), WgSize(10000, 10000));
	WriteDiffAttr(s, xmlNode, "enabled", m_Widget->IsEnabled(), true);
	WriteDiffAttr(s, xmlNode, "uid", m_Widget->Id64(), (Sint64)0);
	WriteDiffAttr(s, xmlNode, "modal", m_Widget->IsModal(), false);
	WriteDiffAttr(s, xmlNode, "visible", !m_Widget->IsSetToHide(), true);

	WgGeometryRes(this, xmlNode).Serialize(s);

	std::string id = s.ResDb()->FindWidgetId(m_Widget);
	if(id.size())
		s.AddAttribute("id", id);
	else
		s.RemoveAttribute("id");

	// Parameters that only exist for gizmos.

	WgGizmo * pGizmo = m_Widget->GetGizmo();
	if( pGizmo )
	{
		if( pGizmo->GetSkinManager() )
		{
			std::string skinManagerId = s.ResDb()->FindSkinManagerId( pGizmo->GetSkinManager() );
			WriteDiffAttr( s, xmlNode, "skinmanager", skinManagerId, std::string("") );
		}
	}

	//

	WgConnectRes::Serialize(s, this);

	if( m_Widget->GetInterceptionRules(WgWidget::POINTER) != WgWidget::PASS )
		WgInterceptRes::Serialize(s, WgWidget::POINTER, m_Widget->GetInterceptionRules(WgWidget::POINTER));
	if( m_Widget->GetInterceptionRules(WgWidget::BUTTON1) != WgWidget::PASS )
		WgInterceptRes::Serialize(s, WgWidget::BUTTON1, m_Widget->GetInterceptionRules(WgWidget::POINTER));
	if( m_Widget->GetInterceptionRules(WgWidget::BUTTON2) != WgWidget::PASS )
		WgInterceptRes::Serialize(s, WgWidget::BUTTON2, m_Widget->GetInterceptionRules(WgWidget::POINTER));
	if( m_Widget->GetInterceptionRules(WgWidget::BUTTON3) != WgWidget::PASS )
		WgInterceptRes::Serialize(s, WgWidget::BUTTON3, m_Widget->GetInterceptionRules(WgWidget::POINTER));
	if( m_Widget->GetInterceptionRules(WgWidget::BUTTON4) != WgWidget::PASS )
		WgInterceptRes::Serialize(s, WgWidget::BUTTON4, m_Widget->GetInterceptionRules(WgWidget::POINTER));
	if( m_Widget->GetInterceptionRules(WgWidget::BUTTON5) != WgWidget::PASS )
		WgInterceptRes::Serialize(s, WgWidget::BUTTON5, m_Widget->GetInterceptionRules(WgWidget::POINTER));
	if( m_Widget->GetInterceptionRules(WgWidget::KEYBOARD) != WgWidget::PASS )
		WgInterceptRes::Serialize(s, WgWidget::KEYBOARD, m_Widget->GetInterceptionRules(WgWidget::POINTER));
	if( m_Widget->GetInterceptionRules(WgWidget::WHEEL1) != WgWidget::PASS )
		WgInterceptRes::Serialize(s, WgWidget::WHEEL1, m_Widget->GetInterceptionRules(WgWidget::POINTER));
	if( m_Widget->GetInterceptionRules(WgWidget::WHEEL2) != WgWidget::PASS )
		WgInterceptRes::Serialize(s, WgWidget::WHEEL2, m_Widget->GetInterceptionRules(WgWidget::POINTER));
	if( m_Widget->GetInterceptionRules(WgWidget::WHEEL3) != WgWidget::PASS )
		WgInterceptRes::Serialize(s, WgWidget::WHEEL3, m_Widget->GetInterceptionRules(WgWidget::POINTER));

	for(WgWidget* child = m_Widget->FirstChild(); child; child = child->NextSibling())
	{
		WgResDB::WidgetRes* widgetDb = s.ResDb()->FindResWidget(child);
		WgResourceXML* xmlRes = WgResourceFactoryXML::Create(child->Type(), this);
		WgWidgetRes* widgetRes = WgResourceXML::Cast<WgWidgetRes>(xmlRes);
		ASSERT(widgetRes, std::string("invalid widget type: ") + child->Type());
		if(widgetRes)
		{
			if(widgetDb)
				widgetRes->SetMetaData(widgetDb->meta);
			widgetRes->SetWidget(child);
			widgetRes->Serialize(s);
		}
		delete xmlRes;
	}
}

void WgWidgetRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	assert(m_Widget);

	m_Widget->SetTooltipDelay(WgUtil::ToSint32(xmlNode["tooltipdelay"], 500));
	m_Widget->SetLayer(WgUtil::ToUint8(xmlNode["layer"], WG_DEFAULT_LAYER));
	m_Widget->SetPointerMask(WgPointerMaskRes::Deserialize(xmlNode, "pointermask"));
	m_Widget->SetPointerStyle(WgPointerStyleRes::Deserialize(xmlNode, "pointerstyle"));
	m_Widget->MinSize(WgSizeRes::Deserialize(xmlNode, "minsize", WgSize(1, 1)));
	m_Widget->MaxSize(WgSizeRes::Deserialize(xmlNode, "maxsize", WgSize(10000, 10000)));
	m_Widget->SetEnabled(WgUtil::ToBool(xmlNode["enabled"], true));
	m_Widget->SetId(WgUtil::ToSint32(xmlNode["uid"], 0));
	if(WgUtil::ToBool(xmlNode["modal"], false))
		m_Widget->Modal();
	if(!WgUtil::ToBool(xmlNode["visible"], true))
		m_Widget->HideBranch();

	WgGeometryRes(this).Deserialize(xmlNode, s);

	WgWidgetRes* parentRes = WgResourceXML::Cast<WgWidgetRes>(Parent());
	if(parentRes)
	{
		m_Widget->SetParent(parentRes->GetWidget());
	}

	std::string skinmanager = xmlNode["skinmanager"];
	if( !skinmanager.empty() )
	{
		WgGizmo * pGizmo = m_Widget->GetGizmo();
		WgSkinManager * pManager = s.ResDb()->GetSkinManager( skinmanager );

		VERIFY(pGizmo, "Skinmanager set on non-gizmo widget");
		VERIFY(pManager, "Referenced skinmanager not found");

		pGizmo->SetSkinManager( pManager );
	}

	s.ResDb()->AddWidget(xmlNode["id"], m_Widget, new WgXMLMetaData(XmlNode()));
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_Button_Res ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <button [widget-attribs]
//		blockset=[name]
//		text=[string]
//		pressanim=[button1, button2, button3, bDownWhenMouseOutside]
//	/>
Wdg_Button_Res::Wdg_Button_Res(WgResourceXML* parent, Wdg_Button* widget) :
	WgWidgetRes(parent, widget),
	m_pCharBuffer(0)
{
}

Wdg_Button_Res::~Wdg_Button_Res()
{
	delete m_pCharBuffer;
}

void Wdg_Button_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	const WgXmlNode& xmlNode = XmlNode();
	Wdg_Button* widget = GetWidget();
	WgWidgetRes::Serialize(s);

	WgTextHolderRes::Serialize(this, xmlNode, s, widget);
	WgIconHolderRes::Serialize(this, xmlNode, s, widget);

	WriteBlockSetAttr(s, widget->GetSource(), "blockset");

	bool b1, b2, b3, bo;
	widget->GetPressAnim( b1, b2, b3, bo );
	WriteDiffAttr<bool>(s, xmlNode, "pressanim", b1, b2, b3, bo, true, false, false, false);
	WriteBlockSetAttr(s, widget->GetIconSource(), "icon");
//	WriteDiffAttr(s, xmlNode, "iconorigo", widget->GetIconOrigo(), WgOrigo::topLeft());
//	WriteDiffAttr<Sint8>(s, xmlNode, "iconofs", widget->GetIconOfsX(), widget->GetIconOfsY(), 0, 0);
	WriteTextAttrib(s, widget->GetRealTooltipString().Chars(), "tooltip");

	s.EndTag();
}

void Wdg_Button_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_Button* widget = new Wdg_Button();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);

	WgTextHolderRes::Deserialize(xmlNode, s, widget);
	WgIconHolderRes::Deserialize(xmlNode, s, widget);

	widget->SetSource(s.ResDb()->GetBlockSet(xmlNode["blockset"]));
//	widget->SetText(xmlNode["text"].c_str());

	bool b1 = true, b2 = false, b3 = false, bo = false;
	WgUtil::FromString(xmlNode["pressanim"], b1, b2, b3, bo);
	widget->SetPressAnim(b1, b2, b3, bo);

	Sint8 xOfs = 0, yOfs = 0;
	widget->SetIcon(s.ResDb()->GetBlockSet(xmlNode["icon"]));
	widget->SetTooltipString( WgCharSeq(ReadLocalizedString(xmlNode["tooltip"], s).c_str()));
}

WgCharBuffer* Wdg_Button_Res::GetCharBuffer()
{
	if(m_pCharBuffer == 0)
		m_pCharBuffer = new WgCharBuffer();

	return m_pCharBuffer;
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_RefreshButton_Res ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <refreshbutton [widget-attribs]
//		blockset=[name]
//		normal_text=[string]
//		textborder=[left, right, up, down]
//		pressanim=[button1, button2, button3, bDownWhenMouseOutside]
//
//		refresh_anim=[string]
//		refresh_mode=[spinning/progress]
//		refresh_text=[string]
//		anim_target=[icon/button_centered/button_stretched]
//		restartable=[bool]
//
//	/>
Wdg_RefreshButton_Res::Wdg_RefreshButton_Res(WgResourceXML* parent, Wdg_RefreshButton* widget) :
	WgWidgetRes(parent, widget),
	m_pCharBuffer(0)
{
}

Wdg_RefreshButton_Res::~Wdg_RefreshButton_Res()
{
	delete m_pCharBuffer;
}

void Wdg_RefreshButton_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	const WgXmlNode& xmlNode = XmlNode();
	Wdg_RefreshButton* widget = GetWidget();
	WgWidgetRes::Serialize(s);

	WgTextHolderRes::Serialize(this, xmlNode, s, widget);
	WgIconHolderRes::Serialize(this, xmlNode, s, widget);

	WriteBlockSetAttr(s, widget->GetSource(), "blockset");

	bool b1, b2, b3, bo;
	widget->GetPressAnim( b1, b2, b3, bo );
	WriteDiffAttr<bool>(s, xmlNode, "pressanim", b1, b2, b3, bo, true, false, false, false);
	WriteBlockSetAttr(s, widget->GetIconSource(), "icon");
	WriteTextAttrib(s, widget->GetRealTooltipString().Chars(), "tooltip");

	if( widget->GetRefreshAnimation() )
		WriteDiffAttr( s, xmlNode, "refresh_anim", s.ResDb()->FindAnimId(widget->GetRefreshAnimation()).c_str(), "" );

	WriteDiffAttr( s, xmlNode, "refresh_mode", widget->GetRefreshMode()==WgGizmoRefreshButton::SPINNING? "spinning":"progress", "" );
	WriteTextAttrib(s, widget->GetRefreshText(), "refresh_text");


	WgGizmoRefreshButton::AnimTarget	t = widget->GetAnimTarget();
	std::string	target;
	switch( t )
	{
		case WgGizmoRefreshButton::ICON:
			target = "icon";
			break;
		case WgGizmoRefreshButton::BUTTON_CENTERED:
			target = "button_centered";
			break;
		case WgGizmoRefreshButton::BUTTON_STRETCHED:
			target = "button_stretched";
			break;
		default:
			assert(false);
			break;
	}
	WriteDiffAttr( s, xmlNode, "anim_target", target.c_str(), "" );
	WriteDiffAttr<bool>(s, xmlNode, "restartable", widget->IsRestartable(), false );

	WriteTextPropAttr(s, widget->GetRefreshTextProperties(), "refresh_prop");

	s.EndTag();
}

void Wdg_RefreshButton_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_RefreshButton* widget = new Wdg_RefreshButton();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);

	WgTextHolderRes::Deserialize(xmlNode, s, widget);
	WgIconHolderRes::Deserialize(xmlNode, s, widget);

	widget->SetSource(s.ResDb()->GetBlockSet(xmlNode["blockset"]));
//	widget->SetText(xmlNode["text"].c_str());

	bool b1 = true, b2 = false, b3 = false, bo = false;
	WgUtil::FromString(xmlNode["pressanim"], b1, b2, b3, bo);
	widget->SetPressAnim(b1, b2, b3, bo);

	widget->SetIcon(s.ResDb()->GetBlockSet(xmlNode["icon"]));
	widget->SetTooltipString( WgCharSeq(ReadLocalizedString(xmlNode["tooltip"], s).c_str()));


	widget->SetRefreshAnimation( (WgGfxAnim*)s.ResDb()->GetAnim( xmlNode["refresh_anim"] ) );

	std::string mode = xmlNode["refresh_mode"];
	if( mode.size() > 0 )
	{
		if( mode == "spinning" )
			widget->SetRefreshMode( WgGizmoRefreshButton::SPINNING );
		else if( mode == "progress" )
			widget->SetRefreshMode( WgGizmoRefreshButton::PROGRESS );
	}

	std::string target = xmlNode["anim_target"];
	if( target.size() > 0 )
	{
		if( target == "icon" )
			widget->SetAnimTarget( WgGizmoRefreshButton::ICON );
		else if( target == "button_centered" )
			widget->SetAnimTarget( WgGizmoRefreshButton::BUTTON_CENTERED );
		else if( target == "button_stretched" )
			widget->SetAnimTarget( WgGizmoRefreshButton::BUTTON_STRETCHED );
	}

	widget->SetRestartable(WgUtil::ToBool(xmlNode["restartable"]));

	widget->SetRefreshText(  WgCharSeq(ReadLocalizedString(xmlNode["refresh_text"], s).c_str()));

	WgTextPropPtr prop = s.ResDb()->GetTextProp(xmlNode["refresh_prop"]);
	if(prop)
		widget->SetRefreshTextProperties(prop);

}

WgCharBuffer* Wdg_RefreshButton_Res::GetCharBuffer()
{
	if(m_pCharBuffer == 0)
		m_pCharBuffer = new WgCharBuffer();

	return m_pCharBuffer;
}




//////////////////////////////////////////////////////////////////////////
/// Wdg_CheckBox2_Res ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <checkbox [widget-attribs]
//		blockset_checked=[name]
//		blockset_unchecked=[name]
//		icon_checked=[name]
//		icon_unchecked=[name]
//		text=[string]
//		checked=[bool]
//		flip_on_release=[bool]
//
//
//	/>
Wdg_CheckBox2_Res::Wdg_CheckBox2_Res(WgResourceXML* parent, Wdg_CheckBox2* widget) :
	WgWidgetRes(parent, widget),
	m_pCharBuffer(0)
{
}

Wdg_CheckBox2_Res::~Wdg_CheckBox2_Res()
{
	delete m_pCharBuffer;
}

void Wdg_CheckBox2_Res::Serialize(WgResourceSerializerXML& s)
{
	const WgXmlNode& xmlNode = XmlNode();
	Wdg_CheckBox2* widget = GetWidget();

	s.BeginTag(TagName(), XmlNode());

	WgWidgetRes::Serialize(s);
	WgTextHolderRes::Serialize(this, xmlNode, s, widget);
	WgIconHolderRes::Serialize(this, xmlNode, s, widget);

	WriteDiffAttr(s, xmlNode, "checked", widget->IsChecked(), false);
	WriteDiffAttr(s, xmlNode, "flip_on_release", widget->FlipOnRelease(), false);
	WriteBlockSetAttr(s, widget->GetCheckedSource(), "blockset_checked");
	WriteBlockSetAttr(s, widget->GetUncheckedSource(), "blockset_unchecked");
	WriteTextAttrib(s, widget->GetRealTooltipString().Chars(), "tooltip");

	WriteBlockSetAttr(s, widget->GetCheckedIcon(), "icon_checked");
	WriteBlockSetAttr(s, widget->GetUncheckedIcon(), "icon_unchecked");

	s.EndTag();
}

void Wdg_CheckBox2_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_CheckBox2* widget = new Wdg_CheckBox2();
	m_Widget = widget;

	WgWidgetRes::Deserialize(xmlNode, s);
	WgTextHolderRes::Deserialize(xmlNode, s, widget);
	WgIconHolderRes::Deserialize(xmlNode, s, widget);

	WgBlockSetPtr checked = s.ResDb()->GetBlockSet(xmlNode["blockset_checked"]);
	WgBlockSetPtr unchecked = s.ResDb()->GetBlockSet(xmlNode["blockset_unchecked"]);
	widget->SetSource(unchecked, checked);
	widget->SetState(WgUtil::ToBool(xmlNode["checked"]));
	widget->SetFlipOnRelease(WgUtil::ToBool(xmlNode["flip_on_release"]));
	widget->SetTooltipString(ReadLocalizedString(xmlNode["tooltip"], s).c_str());

	WgBlockSetPtr icon_checked	= s.ResDb()->GetBlockSet(xmlNode["icon_checked"]);
	WgBlockSetPtr icon_unchecked = s.ResDb()->GetBlockSet(xmlNode["icon_unchecked"]);

	widget->SetIcons(icon_unchecked, icon_checked);
}

WgCharBuffer* Wdg_CheckBox2_Res::GetCharBuffer()
{
	if(m_pCharBuffer == 0)
		m_pCharBuffer = new WgCharBuffer();

	return m_pCharBuffer;
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_ComboBox_Res /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <combobox [widget-attribs]
//		blockset=[name]
//		textformat=[string]
//		placeholder=[string]
//
//			<menu ... />
//
// </combobox>
Wdg_ComboBox_Res::Wdg_ComboBox_Res(WgResourceXML* parent, Wdg_ComboBox* widget) :
	WgWidgetRes(parent, widget),
	m_pCharBuffer(0)
{
}

Wdg_ComboBox_Res::~Wdg_ComboBox_Res()
{
	delete m_pCharBuffer;
}

void Wdg_ComboBox_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	const WgXmlNode& xmlNode = XmlNode();
	Wdg_ComboBox* widget = GetWidget();
	WgWidgetRes::Serialize(s);
	WgEditTextRes::Serialize(this, xmlNode, s, widget);

	WriteBlockSetAttr(s, widget->GetSource(), "blockset");

	WriteTextAttrib(s, widget->GetTextFormat().Chars(), "textformat");
	WriteTextAttrib(s, widget->GetPlaceholderText().Chars(), "placeholder");

	// Write menu as a subtag only if it doesn't have a parent.
	Wdg_Menu* menu = widget->GetMenu();
	if(menu && WgXmlRoot::IsTopLevelWidget(menu, s))
	{
		Wdg_Menu_Res* menuRes = new Wdg_Menu_Res(this, menu);
		WgResDB::WidgetRes* menuDb = s.ResDb()->FindResWidget(menu);
		if(menuDb)
			menuRes->SetMetaData(menuDb->meta);
		menuRes->Serialize(s);
		delete menuRes;
	}

	s.EndTag();
}

void Wdg_ComboBox_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_ComboBox* widget = new Wdg_ComboBox();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);
	WgEditTextRes::Deserialize(xmlNode, s, widget);

	widget->SetSource(s.ResDb()->GetBlockSet(xmlNode["blockset"]));
	widget->SetPlaceholderText(ReadLocalizedString(xmlNode["placeholder"], s));
	widget->SetTextFormat(ReadLocalizedString(xmlNode["textformat"], s));
}

WgCharBuffer* Wdg_ComboBox_Res::GetCharBuffer()
{
	if(m_pCharBuffer == 0)
		m_pCharBuffer = new WgCharBuffer();

	return m_pCharBuffer;
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_Container_Res ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <container [widget-attribs] />
Wdg_Container_Res::Wdg_Container_Res(WgResourceXML* parent, Wdg_Container* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_Container_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());
	WgWidgetRes::Serialize(s);
	s.EndTag();
}

void Wdg_Container_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_Container* widget = new Wdg_Container();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_HDrag_Res ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Wdg_HDrag_Res::Wdg_HDrag_Res(WgResourceXML* parent, Wdg_HDrag* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_HDrag_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	const WgXmlNode& xmlNode = XmlNode();
	Wdg_HDrag* widget = GetWidget();
	WgWidgetRes::Serialize(s);

	WriteDiffAttr(s, xmlNode, "pos", widget->GetSliderPos(), 0.f);
	WriteDiffAttr(s, xmlNode, "size", widget->GetSliderSize(), 0.f);
	WriteBlockSetAttr(s, widget->GetBgSource(), "source_bg");
	WriteBlockSetAttr(s, widget->GetBarSource(), "source_bar");
	WriteBlockSetAttr(s, widget->GetBwdSource(), "source_bwd");
	WriteBlockSetAttr(s, widget->GetFwdSource(), "source_fwd");
	WgButtonLayoutRes::Serialize(s, xmlNode, "layout", widget->GetButtonLayout());
	WriteTextAttrib(s, widget->GetRealTooltipString().Chars(), "tooltip");

	s.EndTag();
}

void Wdg_HDrag_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_HDrag* widget = new Wdg_HDrag();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);

	widget->SetSliderPos(WgUtil::ToFloat(xmlNode["pos"]));
	widget->SetSliderSize(WgUtil::ToFloat(xmlNode["size"]));
	WgBlockSetPtr bg = s.ResDb()->GetBlockSet(xmlNode["source_bg"]);
	WgBlockSetPtr bar = s.ResDb()->GetBlockSet(xmlNode["source_bar"]);
	WgBlockSetPtr bwd = s.ResDb()->GetBlockSet(xmlNode["source_bwd"]);
	WgBlockSetPtr fwd = s.ResDb()->GetBlockSet(xmlNode["source_fwd"]);
	widget->SetButtonLayout(WgButtonLayoutRes::Deserialize(xmlNode, "layout"));
	widget->SetTooltipString(ReadLocalizedString(xmlNode["tooltip"], s).c_str());

	ASSERT(bg && bar, "missing [bg] and/or [bar] on <hdrag>");
	widget->SetSource( bg, bar, bwd, fwd );

	WgBaseViewRes* parentView = WgResourceXML::Cast<WgBaseViewRes>(Parent());
	if(parentView)
	{
		parentView->GetView()->SetScrollbarX(widget);
	}
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_VDrag_Res ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Wdg_VDrag_Res::Wdg_VDrag_Res(WgResourceXML* parent, Wdg_VDrag* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_VDrag_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	const WgXmlNode& xmlNode = XmlNode();
	Wdg_VDrag* widget = GetWidget();
	WgWidgetRes::Serialize(s);

	WriteDiffAttr(s, xmlNode, "pos", widget->GetSliderPos(), 0.f);
	WriteDiffAttr(s, xmlNode, "size", widget->GetSliderSize(), 0.f);
	WriteBlockSetAttr(s, widget->GetBgSource(), "source_bg");
	WriteBlockSetAttr(s, widget->GetBarSource(), "source_bar");
	WriteBlockSetAttr(s, widget->GetBwdSource(), "source_bwd");
	WriteBlockSetAttr(s, widget->GetFwdSource(), "source_fwd");
	WgButtonLayoutRes::Serialize(s, xmlNode, "layout", widget->GetButtonLayout());
	WriteTextAttrib(s, widget->GetRealTooltipString().Chars(), "tooltip");

	s.EndTag();
}

void Wdg_VDrag_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_VDrag* widget = new Wdg_VDrag();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);

	widget->SetSliderPos(WgUtil::ToFloat(xmlNode["pos"]));
	widget->SetSliderSize(WgUtil::ToFloat(xmlNode["size"]));
	WgBlockSetPtr bg = s.ResDb()->GetBlockSet(xmlNode["source_bg"]);
	WgBlockSetPtr bar = s.ResDb()->GetBlockSet(xmlNode["source_bar"]);
	WgBlockSetPtr bwd = s.ResDb()->GetBlockSet(xmlNode["source_bwd"]);
	WgBlockSetPtr fwd = s.ResDb()->GetBlockSet(xmlNode["source_fwd"]);
	widget->SetButtonLayout(WgButtonLayoutRes::Deserialize(xmlNode, "layout"));
	widget->SetTooltipString(ReadLocalizedString(xmlNode["tooltip"], s).c_str());

	ASSERT(bg && bar, "missing [bg] and/or [bar] on <vdrag>");
	widget->SetSource( bg, bar, bwd, fwd );

	WgBaseViewRes* parentView = WgResourceXML::Cast<WgBaseViewRes>(Parent());
	if(parentView)
	{
		parentView->GetView()->SetScrollbarY(widget);
	}
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_EditValue_Res ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Wdg_EditValue_Res::Wdg_EditValue_Res(WgResourceXML* parent, Wdg_EditValue* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_EditValue_Res::Serialize(WgResourceSerializerXML& s)
{
	const WgXmlNode& xmlNode = XmlNode();
	Wdg_EditValue* widget = GetWidget();

	s.BeginTag(TagName(), XmlNode());

	WgWidgetRes::Serialize(s);
	WgValueHolderRes::Serialize(this, xmlNode, s, widget);

	WriteDiffAttr(s, xmlNode, "textalign", widget->GetTextAlignment(), WgOrigo::topLeft());
	WriteTextPropAttr(s, widget->GetTextProp(), "prop");
	WriteTextManager(widget, s);

	WriteDiffAttr(s, xmlNode, "max_chars", widget->MaxInputChars(), 256);

	s.EndTag();
}

void Wdg_EditValue_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_EditValue* widget = new Wdg_EditValue();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);
	WgValueHolderRes::Deserialize(xmlNode, s, widget);

	widget->SetTextAlignment(WgUtil::ToOrigo(xmlNode["textalign"]));

	WgTextPropPtr prop = s.ResDb()->GetTextProp(xmlNode["prop"]);
	if(prop)
		widget->SetTextProp(prop);

	if( xmlNode.HasAttribute("max_chars") )
		widget->SetMaxInputChars(WgUtil::ToSint32(xmlNode["max_chars"]));

	ReadTextManager(widget, xmlNode, s);
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_Fill_Res /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <fill [widget-attribs]
//		col_enabled=[color]
//		col_disabled=[color]
// />
Wdg_Fill_Res::Wdg_Fill_Res(WgResourceXML* parent, Wdg_Fill* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_Fill_Res::Serialize(WgResourceSerializerXML& s)
{
	const WgXmlNode& xmlNode = XmlNode();
	Wdg_Fill* wdgFill = GetWidget();

	s.BeginTag(TagName(), XmlNode());

	WgWidgetRes::Serialize(s);

	WgColorRes::Serialize(s, xmlNode, "col_enabled", wdgFill->GetEnabledColor());
	WgColorRes::Serialize(s, xmlNode, "col_disabled", wdgFill->GetDisabledColor());

	s.EndTag();
}

void Wdg_Fill_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_Fill* wdgFill = new Wdg_Fill();
	m_Widget = wdgFill;
	WgWidgetRes::Deserialize(xmlNode, s);

	WgColor colEnabled = WgColorRes::Deserialize(s, xmlNode["col_enabled"]);
	WgColor colDisabled = WgColorRes::Deserialize(s, xmlNode["col_disabled"]);

	wdgFill->SetColor(colEnabled, colDisabled);
}

//////////////////////////////////////////////////////////////////////////
/// WgBaseViewRes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgBaseViewRes::Serialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s, Wdg_Baseclass_View* view)
{
	WriteDiffAttr(s, xmlNode, "autoscroll", view->AutoScrollX(), view->AutoScrollY(), false, false);
	WriteDiffAttr(s, xmlNode, "stepsize", view->StepSizeX(), view->StepSizeY(), (Uint32)1, (Uint32)1);
	WriteDiffAttr(s, xmlNode, "jumpsize", view->JumpSizeX(), view->JumpSizeY(), 0.75f, 0.75f);
	WriteDiffAttr(s, xmlNode, "viewofs", view->ViewOfsX(), view->ViewOfsY(), 0.f, 0.f);
	WriteDiffAttr(s, xmlNode, "viewofs_pxl", view->ViewPixelOfsX(), view->ViewPixelOfsY(), (Uint32)0, (Uint32)0);
	WriteDiffAttr(s, xmlNode, "autohide", view->GetScrollbarAutoHideX(), view->GetScrollbarAutoHideY(), false, false);
	WriteDiffAttr(s, xmlNode, "scrollbarpos", view->GetScrollbarRight(), view->GetScrollbarBottom(), true, true);

	// Only reference external scrollbars
	// Child scroll bars are written as part of the widget tree
	if(view->GetScrollbarX() && view->GetScrollbarX()->Parent() != view)
		WriteWidgetRefAttr(s, view->GetScrollbarX(), "horr_scrollbar");
	else
		s.RemoveAttribute("horr_scrollbar");

	if(view->GetScrollbarY() && view->GetScrollbarY()->Parent() != view)
		WriteWidgetRefAttr(s, view->GetScrollbarY(), "vert_scrollbar");
	else
		s.RemoveAttribute("vert_scrollbar");
}

void WgBaseViewRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s, Wdg_Baseclass_View* view)
{
	for(Uint32 iAttrib = 0; iAttrib < xmlNode.GetAttributeCount(); iAttrib++)
	{
		const WgXmlAttribute& attr = xmlNode.GetAttribute(iAttrib);
		if(attr.GetName() == "autoscroll")
		{
			bool x, y;
			WgUtil::FromString(attr.GetValue(), x, y);
			view->SetAutoscroll(x, y);
		}
		else if(attr.GetName() == "stepsize")
		{
			Uint32 x, y;
			WgUtil::FromString(attr.GetValue(), x, y);
			view->SetStepSize(x, y);
		}
		else if(attr.GetName() == "jumpsize")
		{
			float x, y;
			WgUtil::FromString(attr.GetValue(), x, y);
			view->SetJumpSize(x, y);
		}
		else if(attr.GetName() == "viewofs")
		{
			float x, y;
			WgUtil::FromString(attr.GetValue(), x, y);
			view->SetViewOfs(x, y);
		}
		else if(attr.GetName() == "viewofs_pxl")
		{
			Uint32 x, y;
			WgUtil::FromString(attr.GetValue(), x, y);
			view->SetViewPixelOfs(x, y);
		}
		else if(attr.GetName() == "horr_scrollbar")
		{
			Wdg_HDrag* scroll = s.ResDb()->GetCastWidget<Wdg_HDrag>(attr.GetValue());
		//	ASSERT(scroll, "scrollbar does not exist: " + attr.GetValue());
			if(scroll)
				view->SetScrollbarX(scroll);
		}
		else if(attr.GetName() == "vert_scrollbar")
		{
			Wdg_VDrag* scroll = s.ResDb()->GetCastWidget<Wdg_VDrag>(attr.GetValue());
			ASSERT(scroll, "scrollbar does not exist: " + attr.GetValue());
			if(scroll)
				view->SetScrollbarY(scroll);
		}
		else if(attr.GetName() == "autohide")
		{
			bool x, y;
			WgUtil::FromString(attr.GetValue(), x, y);
			view->SetScrollbarAutoHide(x, y);
		}
		else if(attr.GetName() == "scrollbarpos")
		{
			bool x, y;
			WgUtil::FromString(attr.GetValue(), x, y);
			view->SetScrollbarPositions(x, y);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
/// WgItemHolderRes //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void WgItemHolderRes::Serialize(WgResourceXML* pThis, const WgXmlNode& xmlNode, WgResourceSerializerXML& s, Wg_Interface_ItemHolder* holder)
{

	WgColorRes::Serialize(s, xmlNode, "item_mark_color", holder->GetItemMarkColor(), WgColor(0, 0xff));

	WriteDiffAttr(s, xmlNode, "item_spacing", holder->ItemSpacing(), (Uint32)0);
	WriteDiffAttr(s, xmlNode, "items_selectable", holder->IsSelectable(), true);

	for(WgItem* item = holder->GetFirstItem(); item; item = item->GetNext())
	{
		WgResDB::ItemRes* itemDb = s.ResDb()->FindResItem(item);
		WgResourceXML* xmlRes = WgResourceFactoryXML::Create(item->Type(), pThis);
		WgItemRes* itemRes = WgResourceXML::Cast<WgItemRes>(xmlRes);
		ASSERT(itemRes, std::string("unsupported item type ") + item->Type());
		if(itemRes)
		{
			if(itemDb)
				itemRes->SetMetaData(itemDb->meta);
			itemRes->SetItem(item);
			itemRes->Serialize(s);
		}
		delete xmlRes;
	}
}

void WgItemHolderRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s, Wg_Interface_ItemHolder* holder)
{
	m_holder = holder;
	holder->SetItemMarkColor(WgColorRes::Deserialize(s, xmlNode["item_mark_color"]));
	holder->SetItemSpacing(WgUtil::ToUint32(xmlNode["item_spacing"]));
	holder->SetSelectable( WgUtil::ToBool(xmlNode["items_selectable"], true) );

}

//////////////////////////////////////////////////////////////////////////
/// Wdg_GridView_Res /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Wdg_GridView_Res::Wdg_GridView_Res(WgResourceXML* parent, Wdg_GridView* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_GridView_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	const WgXmlNode& xmlNode = XmlNode();
	Wdg_GridView* widget = GetWidget();
	WgWidgetRes::Serialize(s);
	WgItemHolderRes::Serialize(this, xmlNode, s, widget);
	WgBaseViewRes::Serialize(xmlNode, s, widget);

	WriteBlockSetAttr(s, widget->GetSelectionLasso(), "lasso");
	WriteBlockSetAttr(s, widget->GetCellOverlay(), "overlay");
	WriteBlockSetAttr(s, widget->GetCellUnderlay(), "underlay");
	WriteDiffAttr(s, xmlNode, "itemorigo", widget->GetItemOrigo(), WgOrigo::midCenter());
	WriteDiffAttr(s, xmlNode, "itemstretch", widget->GetItemStretch(), true);
	WriteDiffAttr(s, xmlNode, "itemlayout", widget->GetItemLayout() == Wdg_GridView::Vertical ? "vertical" : "horizontal", "vertical");
	WriteDiffAttr<Uint32>(s, xmlNode, "fixedsize", widget->GetFixedSize(), 0);
	WriteDiffAttr(s, xmlNode, "pack", widget->GetCellPacking(), false);
	WriteDiffAttr(s, xmlNode, "cellpointeropaque", widget->GetCellPointerOpacity(), false);
	WriteDiffAttr<Uint16>(s, xmlNode, "cellsize", widget->GetCellSize().minW, widget->GetCellSize().minH, widget->GetCellSize().maxW, widget->GetCellSize().maxH, 4,4, 4096,4096);
	WriteDiffAttr<Uint8>(s, xmlNode, "cellpad", widget->GetCellPaddingX(), widget->GetCellPaddingY(), 0, 0);

	s.EndTag();
}

void Wdg_GridView_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_GridView* widget = new Wdg_GridView();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);
	WgItemHolderRes::Deserialize(xmlNode, s, widget);
	WgBaseViewRes::Deserialize(xmlNode, s, widget);

	WgBlockSetPtr underlay = s.ResDb()->GetBlockSet(xmlNode["underlay"]);
	WgBlockSetPtr overlay = s.ResDb()->GetBlockSet(xmlNode["overlay"]);
	WgBlockSetPtr lasso = s.ResDb()->GetBlockSet(xmlNode["lasso"]);
	WgBlockSetPtr checked_radio = s.ResDb()->GetBlockSet(xmlNode["checked_radio"]);

	widget->SetSelectionLasso( lasso );
	widget->SetCellOverlay( overlay );
	widget->SetCellUnderlay( underlay );
	widget->SetItemOrigo( WgUtil::ToOrigo(xmlNode["itemorigo"], WgOrigo::midCenter()) );
	widget->SetItemStretch( WgUtil::ToBool(xmlNode["itemstretch"]) );

	std::string layout = xmlNode["itemlayout"];
	ASSERT(layout.empty() || layout == "vertical" || layout == "horizontal", "invald layout");
	if(layout == "vertical")
		widget->SetItemLayout(Wdg_GridView::Vertical);
	else if(layout == "horizontal")
		widget->SetItemLayout(Wdg_GridView::Horizontal);

	widget->SetFixedSize( WgUtil::ToUint32(xmlNode["fixedsize"]) );
	widget->SetCellPacking( WgUtil::ToBool(xmlNode["pack"]) );
	widget->SetCellPointerOpacity( WgUtil::ToBool(xmlNode["cellpointeropaque"]) );

	WgMinMax2D minMax(4,4, 4096,4096);
	WgUtil::FromString(xmlNode["cellsize"], minMax.minW, minMax.minH, minMax.maxW, minMax.maxH);
	widget->SetCellSize(minMax);

	Uint8 cellPadX = 0;
	Uint8 cellPadY = 0;
	WgUtil::FromString(xmlNode["cellpad"], cellPadX, cellPadY);
	widget->SetCellPadding(cellPadX, cellPadY);
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_ListView_Res /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <listview [widget-attribs]
// 		autoscroll=[x, y]  bool
// 		autoscroll_marked=[] bool
// 		stepsize=[x, y] pixels
// 		jumpSize=[fractionX, fractionY] float
// 		viewofs=[x, y] float
// 		viewofs_pxl=[x, y] pixels
// 		horr_scrollbar=[name] id of scrollbar widget
// 		vert_scrollbar=[name] id of scrollbar widget
// 		autohide=[hideX, hideY] bool
// 		scrollbarpos=[bottom, right] bool
// />
Wdg_ListView_Res::Wdg_ListView_Res(WgResourceXML* parent, Wdg_ListView* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_ListView_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	const WgXmlNode& xmlNode = XmlNode();
	Wdg_ListView* widget = GetWidget();
	WgWidgetRes::Serialize(s);
	WgBaseViewRes::Serialize(xmlNode, s, widget);

	WriteDiffAttr(s, xmlNode, "autoscroll_marked", widget->GetAutoScrollMarked(), false);

	WgItemHolderRes::Serialize(this, xmlNode, s, widget);

	s.EndTag();
}

// TODO: line mark source
void Wdg_ListView_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_ListView* widget = new Wdg_ListView();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);
	WgBaseViewRes::Deserialize(xmlNode, s, widget);
	WgItemHolderRes::Deserialize(xmlNode, s, widget);

	widget->SetAutoScrollMarked(WgUtil::ToBool(xmlNode["autoscroll_marked"]));
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_Menu_Res /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <menu [widget-attribs] [tileholder-attribs]
//	bkg=[blockset]
//	iconFieldWidth=[integer]
//	arrowFieldWidth=[integer]
//	separator=[blockset]
//	sepBorders=[WgBorders]
//	checked=[blockset]
//	unchecked=[blockset]
//	checked_radio=[blockset]
//	unchecked_radio=[blockset]
//	entry_prop=[textprop]
//	accel_prop=[textprop]
//	slider_bg=[blockset]
//	slider_bar=[blockset]
//	slider_bwd=[blockset]
//	slider_fwd=[blockset]
//	slider_layout=[layout]
//	entryheight=[integer]
// />

Wdg_Menu_Res::Wdg_Menu_Res(WgResourceXML* parent, Wdg_Menu* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_Menu_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	const WgXmlNode& xmlNode = XmlNode();
	Wdg_Menu* widget = GetWidget();
	WgWidgetRes::Serialize(s);
	WgTileHolderRes::Serialize(this, xmlNode, s, widget);

	WriteDiffAttr(s, xmlNode, "iconfieldwidth", widget->GetIconFieldWidth(), (Uint8)0);
	WriteDiffAttr(s, xmlNode, "arrowfieldwidth", widget->GetArrowFieldWidth(), (Uint8)0);
	WgBorderRes::Serialize(s, xmlNode, "sepBorders", widget->GetSeparatorBorders());
	WriteBlockSetAttr(s, widget->GetBgSource(), "bkg");
	WriteBlockSetAttr(s, widget->GetSeparatorSource(), "separator");
	WriteBlockSetAttr(s, widget->GetCheckedSource(), "checked");
	WriteBlockSetAttr(s, widget->GetUncheckedSource(), "unchecked");
	WriteBlockSetAttr(s, widget->GetRadioCheckedSource(), "checked_radio");
	WriteBlockSetAttr(s, widget->GetRadioUncheckedSource(), "unchecked_radio");
	WriteTextPropAttr(s, widget->GetTextEntryProperties(), "entry_prop");
	WriteTextPropAttr(s, widget->GetKeyAccelProperties(), "accel_prop");
	WriteBlockSetAttr(s, widget->GetSliderBgSource(), "slider_bg");
	WriteBlockSetAttr(s, widget->GetSliderBarSource(), "slider_bar");
	WriteBlockSetAttr(s, widget->GetSliderBwdSource(), "slider_bwd");
	WriteBlockSetAttr(s, widget->GetSliderFwdSource(), "slider_fwd");
	WgButtonLayoutRes::Serialize(s, xmlNode, "slider_layout", widget->GetSliderButtonLayout());
	WriteDiffAttr(s, xmlNode, "entryheight", widget->GetEntryHeight(), 0);

	//
	for(WgMenuItem* item = widget->GetFirstItem(); item; item = item->Next())
	{
		WgMenuItemRes* itemRes = 0;

		switch(item->GetType())
		{
		case ENTRY: itemRes = new WgMenuEntryRes(this, (WgMenuEntry*)item); break;
		case CHECKBOX: itemRes = new WgMenuCheckBoxRes(this, (WgMenuCheckBox*)item); break;
		case RADIOBUTTON: itemRes = new WgMenuRadioButtonRes(this, (WgMenuRadioButton*)item); break;
		case SUBMENU: itemRes = new WgMenuSubMenuRes(this, (WgMenuSubMenu*)item); break;
		case SEPARATOR: itemRes = new WgMenuSeparatorRes(this, (WgMenuSeparator*)item); break;
		}
		ASSERT(itemRes, "invalid menu item type");

		if(itemRes)
		{
			WgResDB::MenuItemRes* itemDb = s.ResDb()->FindResMenuItem(item);
			if(itemDb)
				itemRes->SetMetaData(itemDb->meta);
			itemRes->Serialize(s);
		}

		delete itemRes;
	}

	s.EndTag();
}

void Wdg_Menu_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_Menu* widget  = new Wdg_Menu();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);
	WgTileHolderRes::Deserialize(xmlNode, s, widget);

	WgBlockSetPtr bkg = s.ResDb()->GetBlockSet(xmlNode["bkg"]);
	Sint32 iconfieldwidth = WgUtil::ToSint32(xmlNode["iconfieldwidth"]);
	Sint32 arrowfieldwidth = WgUtil::ToSint32(xmlNode["arrowfieldwidth"]);
	WgBlockSetPtr separator = s.ResDb()->GetBlockSet(xmlNode["separator"]);
	WgBorders sepBorders = WgBorderRes::Deserialize(s, xmlNode["sepBorders"]);
	WgBlockSetPtr checked = s.ResDb()->GetBlockSet(xmlNode["checked"]);
	WgBlockSetPtr unchecked = s.ResDb()->GetBlockSet(xmlNode["unchecked"]);
	WgBlockSetPtr checked_radio = s.ResDb()->GetBlockSet(xmlNode["checked_radio"]);
	WgBlockSetPtr unchecked_radio = s.ResDb()->GetBlockSet(xmlNode["unchecked_radio"]);
	WgTextPropPtr entry_prop = s.ResDb()->GetTextProp(xmlNode["entry_prop"]);
	WgTextPropPtr accel_prop = s.ResDb()->GetTextProp(xmlNode["accel_prop"]);
	WgBlockSetPtr slider_bg = s.ResDb()->GetBlockSet(xmlNode["slider_bg"]);
	WgBlockSetPtr slider_bar = s.ResDb()->GetBlockSet(xmlNode["slider_bar"]);
	WgBlockSetPtr slider_bwd = s.ResDb()->GetBlockSet(xmlNode["slider_bwd"]);
	WgBlockSetPtr slider_fwd = s.ResDb()->GetBlockSet(xmlNode["slider_fwd"]);
	WgGizmoSlider::ButtonLayout slider_layout = WgButtonLayoutRes::Deserialize(xmlNode, "slider_layout");
	Uint32 entryheight = WgUtil::ToUint32(xmlNode["entryheight"]);

	VERIFY(entry_prop && accel_prop, "required text property not found");

	widget->SetBgSource(bkg, iconfieldwidth, arrowfieldwidth );
	widget->SetSeparatorSource(separator, sepBorders);
	widget->SetCheckBoxSource(unchecked, checked);
	widget->SetRadioButtonSource(unchecked_radio, checked_radio);
	// widget->SetArrowSource( WgGfxAnim * pAnim );
	widget->SetTextProperties(entry_prop, accel_prop);
	widget->SetSliderSource(slider_bg, slider_bar, slider_bwd, slider_fwd);
	widget->SetSliderButtonLayout(slider_layout);
	widget->SetEntryHeight(entryheight);

	Wdg_ComboBox_Res* combo = WgResourceXML::Cast<Wdg_ComboBox_Res>(Parent());
	if(combo)
	{
		combo->GetWidget()->SetMenu(widget);
		widget->SetParent(0);
	}
	WgMenuSubMenuRes* subMenuRes = WgResourceXML::Cast<WgMenuSubMenuRes>(Parent());
	if(subMenuRes)
	{
		subMenuRes->GetItem()->SetSubMenu(widget);
		widget->SetParent(0);
	}
	WgMenuBarItemRes* menuBarItemRes = WgResourceXML::Cast<WgMenuBarItemRes>(Parent());
	if(menuBarItemRes)
	{
		menuBarItemRes->SetMenu(widget);
		widget->SetParent(0);
	}
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_MenuBar_Res //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <menubar [widget-attribs]
//	bkg=[blockset]
//	entry=[blockset]
//  entry_prop=[textprop]
//  textmargin=[integer]
//  textofsy=[integer]
// />
Wdg_MenuBar_Res::Wdg_MenuBar_Res(WgResourceXML* parent, Wdg_MenuBar* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_MenuBar_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());
	WgWidgetRes::Serialize(s);

	Wdg_MenuBar* widget = GetWidget();
//	const WgXmlNode& xmlNode = XmlNode();

	WriteBlockSetAttr(s, widget->GetBgSource(), "bkg");
	WriteBlockSetAttr(s, widget->GetEntrySource(), "entry_bkg");
	WriteTextPropAttr(s, widget->GetTextProp(), "entry_prop");

	WgMenuBarItem* barItem = widget->GetFirstMenuBarItem();
	for(; barItem; barItem = barItem->Next())
	{
		WgMenuBarItemRes(this, barItem).Serialize(s);
	}

	s.EndTag();
}

void Wdg_MenuBar_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_MenuBar* widget = new Wdg_MenuBar();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);

	widget->SetBgSource(
		s.ResDb()->GetBlockSet(xmlNode["bkg"])
	);

	widget->SetEntrySource(
		s.ResDb()->GetBlockSet(xmlNode["entry_bkg"]),
		s.ResDb()->GetTextProp(xmlNode["entry_prop"])
	);
}

void WgMenuBarItemRes::Serialize(WgResourceSerializerXML& s)
{
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());

	WriteTextAttrib(s, m_pBarItem->m_pText, "title");
	WriteDiffAttr(s, xmlNode, "navkey", m_pBarItem->m_navKey, (Uint16)0);
	// Only write submenu if it doesn't have a parent
	if(m_pBarItem->m_pMenu && WgXmlRoot::IsTopLevelWidget(m_pBarItem->m_pMenu, s))
	{
		Wdg_Menu_Res* menuRes = new Wdg_Menu_Res(this, m_pBarItem->m_pMenu);
		WgResDB::WidgetRes* menuDb = s.ResDb()->FindResWidget(m_pBarItem->m_pMenu);
		if(menuDb)
			menuRes->SetMetaData(menuDb->meta);
		menuRes->Serialize(s);
		delete menuRes;
	}

	s.EndTag();
}

void WgMenuBarItemRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_MenuBar_Res* menuBarRes = WgResourceXML::Cast<Wdg_MenuBar_Res>(Parent());
	VERIFY(menuBarRes, "Invalid parent for <menubaritem>. Should be <menubar>");

	m_Title = ReadLocalizedString(xmlNode["title"], s);
	m_navKey = WgUtil::ToUint16(xmlNode["navkey"]);
}

void WgMenuBarItemRes::Deserialized(WgResourceSerializerXML& s)
{
	Wdg_MenuBar_Res* menuBarRes = WgResourceXML::Cast<Wdg_MenuBar_Res>(Parent());
	if(menuBarRes)
	{
		menuBarRes->GetWidget()->AddMenu(m_Title.c_str(), m_pMenu, m_navKey);
	}
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_Pixmap_Res ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <pixmap [widget-attribs]
//		blockset=[blockset]
// />
Wdg_Pixmap_Res::Wdg_Pixmap_Res(WgResourceXML* parent, Wdg_Pixmap* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_Pixmap_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());
	WgWidgetRes::Serialize(s);

	Wdg_Pixmap* widget = GetWidget();
//	const WgXmlNode& xmlNode = XmlNode();

	WriteBlockSetAttr(s, widget->GetSource(), "blockset");
	WriteTextAttrib(s, widget->GetRealTooltipString().Chars(), "tooltip");

	s.EndTag();
}

void Wdg_Pixmap_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_Pixmap* widget = new Wdg_Pixmap();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);

	widget->SetSource(s.ResDb()->GetBlockSet(xmlNode["blockset"]));
	widget->SetTooltipString(ReadLocalizedString(xmlNode["tooltip"], s).c_str());

}

//////////////////////////////////////////////////////////////////////////
/// Wdg_Lodder_Res ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <lodder [widget-attribs]>
//	 ...
// </lodder>
Wdg_Lodder_Res::Wdg_Lodder_Res(WgResourceXML* parent, Wdg_Lodder* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_Lodder_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	// Detach LODs and write them out separately
	for(Uint32 i = 0; i < GetWidget()->GetLODCount(); i++)
	{
		const Wdg_Lodder::Lod* pLod = GetWidget()->GetLOD(i);
		pLod->pWidget->SetParent(0);
	}

	// Write widget params and all non-LOD-children
	WgWidgetRes::Serialize(s);

	// Write LODs
	for(Uint32 i = 0; i < GetWidget()->GetLODCount(); i++)
	{
		const Wdg_Lodder::Lod* pLod = GetWidget()->GetLOD(i);

		// restore original minsize
		pLod->pWidget->MinSize(pLod->minSize);

		WgResDB::WidgetRes* widgetDb = s.ResDb()->FindResWidget(pLod->pWidget);
		WgResourceXML* xmlRes = WgResourceFactoryXML::Create(pLod->pWidget->Type(), this);
		WgWidgetRes* widgetRes = WgResourceXML::Cast<WgWidgetRes>(xmlRes);
		ASSERT(widgetRes, std::string("invalid widget type: ") + pLod->pWidget->Type());
		if(widgetRes)
		{
			if(widgetDb)
				widgetRes->SetMetaData(widgetDb->meta);
			widgetRes->SetWidget(pLod->pWidget);
			widgetRes->Serialize(s);
		}
		delete xmlRes;

		// Reattach LOD to lodder
		pLod->pWidget->MinSize(0, 0);
		pLod->pWidget->SetParent(GetWidget());
	}

	s.EndTag();
}

void Wdg_Lodder_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_Lodder* widget = new Wdg_Lodder();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);
}

void Wdg_Lodder_Res::Deserialized(WgResourceSerializerXML& s)
{
	GetWidget()->AddChildrenAsLODs();
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_RadioButton2_Res /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <radiobutton [widget-attribs]
//		checked=[true/false]
//		allowuncheck=[true/false]
//		flip_on_release=[true/false]
//		blockset_checked=[name]
//		blockset_unchecked=[name]
//		icon_checked=[name]
//		icon_unchecked=[name]
//		text=[string]
// />
Wdg_RadioButton2_Res::Wdg_RadioButton2_Res(WgResourceXML* parent, Wdg_RadioButton2* widget) :
	WgWidgetRes(parent, widget),
	m_pCharBuffer(0)
{
}

Wdg_RadioButton2_Res::~Wdg_RadioButton2_Res()
{
	delete m_pCharBuffer;
}

WgCharBuffer* Wdg_RadioButton2_Res::GetCharBuffer()
{
	if(m_pCharBuffer == 0)
		m_pCharBuffer = new WgCharBuffer();

	return m_pCharBuffer;
}

void Wdg_RadioButton2_Res::Serialize(WgResourceSerializerXML& s)
{
	const WgXmlNode& xmlNode = XmlNode();
	Wdg_RadioButton2* widget = GetWidget();

	s.BeginTag(TagName(), XmlNode());

	WgWidgetRes::Serialize(s);
	WgTextHolderRes::Serialize(this, xmlNode, s, widget);
	WgIconHolderRes::Serialize(this, xmlNode, s, widget);

	WriteDiffAttr(s, xmlNode, "checked", widget->IsChecked(), false);
	WriteDiffAttr(s, xmlNode, "allowuncheck", widget->AllowUnchecking(), false);
	WriteDiffAttr(s, xmlNode, "flip_on_release", widget->FlipOnRelease(), false);
	WriteBlockSetAttr(s, widget->GetCheckedSource(), "blockset_checked");
	WriteBlockSetAttr(s, widget->GetUncheckedSource(), "blockset_unchecked");
	WriteTextAttrib(s, widget->GetRealTooltipString().Chars(), "tooltip");

	WriteBlockSetAttr(s, widget->GetCheckedIcon(), "icon_checked");
	WriteBlockSetAttr(s, widget->GetUncheckedIcon(), "icon_unchecked");

	s.EndTag();
}

void Wdg_RadioButton2_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_RadioButton2* widget = new Wdg_RadioButton2();
	m_Widget = widget;

	WgWidgetRes::Deserialize(xmlNode, s);
	WgTextHolderRes::Deserialize(xmlNode, s, widget);
	WgIconHolderRes::Deserialize(xmlNode, s, widget);

	WgBlockSetPtr checked = s.ResDb()->GetBlockSet(xmlNode["blockset_checked"]);
	WgBlockSetPtr unchecked = s.ResDb()->GetBlockSet(xmlNode["blockset_unchecked"]);
	widget->SetSource(unchecked, checked);
	widget->AllowUnchecking(WgUtil::ToBool(xmlNode["allowuncheck"]));
	widget->SetState(WgUtil::ToBool(xmlNode["checked"]));
	widget->SetFlipOnRelease(WgUtil::ToBool(xmlNode["flip_on_release"]));
	widget->SetTooltipString(ReadLocalizedString(xmlNode["tooltip"], s).c_str());

	WgBlockSetPtr icon_checked	= s.ResDb()->GetBlockSet(xmlNode["icon_checked"]);
	WgBlockSetPtr icon_unchecked = s.ResDb()->GetBlockSet(xmlNode["icon_unchecked"]);

	widget->SetIcons(icon_unchecked, icon_checked);
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_Root_Res /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <root [widget-attribs] />
Wdg_Root_Res::Wdg_Root_Res(WgResourceXML* parent, Wdg_Root* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_Root_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());
	WgWidgetRes::Serialize(s);
	s.EndTag();
}

void Wdg_Root_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_Root* widget = new Wdg_Root();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_Shader_Res ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <shader [widget-attribs]
//		color=[color]
//		tint=[opaque | mul]
//		blend=[blend | opaque | add | mul]
// />
Wdg_Shader_Res::Wdg_Shader_Res(WgResourceXML* parent, Wdg_Shader* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_Shader_Res::Serialize(WgResourceSerializerXML& s)
{
	Wdg_Shader* shader = GetWidget();
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());

	WgWidgetRes::Serialize(s);

	WgColorRes::Serialize(s, xmlNode, "color", shader->Color(), WgColor(255,255,255));

	if(xmlNode.HasAttribute("tint") || shader->TintMode() != WG_TINTMODE_OPAQUE)
	{
		if(shader->TintMode() == WG_TINTMODE_MULTIPLY)
			s.AddAttribute("tint", "mul");
		else
			s.AddAttribute("tint", "opaque");
	}

	if(xmlNode.HasAttribute("blend") || shader->BlendMode() != WG_BLENDMODE_BLEND)
	{
		switch(shader->BlendMode())
		{
			case WG_BLENDMODE_OPAQUE: s.AddAttribute("blend", "opaque"); break;
			case WG_BLENDMODE_BLEND: s.AddAttribute("blend", "blend"); break;
			case WG_BLENDMODE_ADD: s.AddAttribute("blend", "add"); break;
			case WG_BLENDMODE_MULTIPLY: s.AddAttribute("blend", "mul"); break;
			default:assert(0);
		}
	}

	s.EndTag();
}

void Wdg_Shader_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_Shader* widget = new Wdg_Shader();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);

	if(xmlNode.HasAttribute("color"))
	{
		widget->SetColor(WgColorRes::Deserialize(s, xmlNode["color"]));
	}
	WgTintMode tint = WG_TINTMODE_OPAQUE;
	if(xmlNode["tint"] == "mul")
		tint = WG_TINTMODE_MULTIPLY;
	widget->SetTintMode(tint);

	std::string blend = xmlNode["blend"];
	if(blend == "opaque") widget->SetBlendMode(WG_BLENDMODE_OPAQUE);
	else if(blend.empty() || blend == "blend") widget->SetBlendMode(WG_BLENDMODE_BLEND);
	else if(blend == "add")
		widget->SetBlendMode(WG_BLENDMODE_ADD);
	else if(blend == "mul") widget->SetBlendMode(WG_BLENDMODE_MULTIPLY);
}

//////////////////////////////////////////////////////////////////////////
/// WgTableColumnRes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <column [textholder-attribs]
//
// />

WgTableColumnRes::~WgTableColumnRes()
{
	delete m_pCharBuffer;
}

void WgTableColumnRes::Serialize(WgResourceSerializerXML& s)
{
	Wdg_TableView_Res* tableViewRes = WgResourceXML::Cast<Wdg_TableView_Res>(Parent());
	VERIFY(tableViewRes, "Invalid parent for <column>. Should be <table>");
//	Wdg_TableView* table = tableViewRes->GetWidget();
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());

	WgTextHolderRes::Serialize(this, xmlNode, s, m_column);

	WriteDiffAttr(s, xmlNode, "pixelwidth", m_column->GetWidth(), (Uint32)0);
	WriteDiffAttr(s, xmlNode, "enabled", !m_column->IsDisabled(), true);
	WriteDiffAttr(s, xmlNode, "ascend", m_column->IsInitialAscend(), true);
	WriteDiffAttr(s, xmlNode, "uid", m_column->GetID(), (Uint32)0);
	WriteDiffAttr(s, xmlNode, "scale_weight", m_column->GetScaleWeight(), 1.f);

	s.EndTag();
}

void WgTableColumnRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_TableView_Res* tableViewRes = WgResourceXML::Cast<Wdg_TableView_Res>(Parent());
	VERIFY(tableViewRes, "Invalid parent for <column>. Should be <tableview>");

//	WgTableColumn temp;
//	WgSimpleText tempText;
//	temp.SetTextObj(&tempText);
//	WgTextHolderRes::Deserialize(xmlNode, s, &temp);

	bool ascend = WgUtil::ToBool(xmlNode["ascend"], true);
	bool enabled = WgUtil::ToBool(xmlNode["enabled"], true);
	Uint16 pixelw = WgUtil::ToUint16(xmlNode["pixelwidth"]);
	WgOrigo origo = WgUtil::ToOrigo(xmlNode["origo"]);
	float weight = WgUtil::ToFloat(xmlNode["scale_weight"],1.f);
	int id = WgUtil::ToSint32(xmlNode["uid"]);

//	WgCharSeq charSeq(m_pCharBuffer);
	Uint32 column = tableViewRes->GetWidget()->AddColumn(ReadLocalizedString(xmlNode["text"], s).c_str(), pixelw, origo, 0, ascend, enabled, id);
	tableViewRes->GetWidget()->GetColumn(column)->SetScaleWeight(weight);
}

WgCharBuffer* WgTableColumnRes::GetCharBuffer()
{
	if(m_pCharBuffer == 0)
		m_pCharBuffer = new WgCharBuffer();

	return m_pCharBuffer;
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_TableView_Res ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <table [widget-attribs] [listview-attribs] [tileholder-attribs]
//		header=[true/false]
//		header_textprop=[name]
//		header_blockset=[name]
//		header_textborder=[left, right, top, bottom]
//		scaleheader=[true/false]
//		arrow_asc=[name]
//		arrow_dsc=[name]
//		arrow_origo=[]
//		arrow_pos=[x,y]
//		cellpad=[x,y]
//		sortprio=[prio] integer
//		empty_row_height=[height]
// />
Wdg_TableView_Res::Wdg_TableView_Res(WgResourceXML* parent, Wdg_TableView* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_TableView_Res::Serialize(WgResourceSerializerXML& s)
{
	Wdg_TableView* widget = GetWidget();
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());

	WgWidgetRes::Serialize(s);

	WgBaseViewRes::Serialize(xmlNode, s, widget);

	WriteDiffAttr(s, xmlNode, "autoscroll_marked", widget->GetAutoScrollMarked(), false);
	WriteDiffAttr(s, xmlNode, "header", widget->ShowHeader(), true);
	WriteDiffAttr(s, xmlNode, "scaleheader", widget->GetAutoScaleHeaders(), false);

	WriteTextPropAttr(s, widget->GetHeaderTextProp(), "header_textprop");
	WriteBlockSetAttr(s, widget->GetHeaderSourceNormal(), "header_blockset_normal");
	WriteBlockSetAttr(s, widget->GetHeaderSourceSelected(), "header_blockset_selected");

	WriteBlockSetAttr(s, widget->GetArrowAscend(), "arrow_asc");
	WriteBlockSetAttr(s, widget->GetArrowDescend(), "arrow_dsc");

	WriteDiffAttr(s, xmlNode, "empty_row_height", (Sint32) widget->GetEmptyRowHeight(), 0);
	WriteDiffAttr(s, xmlNode, "arrow_origo", widget->GetArrowOrigo(), WgOrigo::topLeft());
	WriteDiffAttr(s, xmlNode, "arrow_pos", widget->GetArrowPosX(), widget->GetArrowPosY(), 0, 0);
	WriteDiffAttr(s, xmlNode, "cellpad", widget->GetCellPaddingX(), widget->GetCellPaddingY(), (Uint8)0, (Uint8)0);
	WriteDiffAttr(s, xmlNode, "sortprio", widget->GetClickSortPrio(), (Uint8)0);

	// columns
	for(Uint32 iColumn = 0; iColumn < widget->NbColumns(); iColumn++)
	{
		WgTableColumnRes(this, widget->GetColumn(iColumn)).Serialize(s);
	}

	WgItemHolderRes::Serialize(this, xmlNode, s, widget);
	WgTileHolderRes::Serialize(this, xmlNode, s, widget);

	s.EndTag();
}

void Wdg_TableView_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_TableView* widget = new Wdg_TableView();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);
	WgBaseViewRes::Deserialize(xmlNode, s, widget);
	WgItemHolderRes::Deserialize(xmlNode, s, widget);
	WgTileHolderRes::Deserialize(xmlNode, s, widget);

	Sint32 x,y;
	if(WgUtil::FromString(xmlNode["arrow_pos"], x, y))
		widget->SetArrowPos(x, y);

	if(WgUtil::FromString(xmlNode["cellpad"], x, y))
		widget->SetCellPadding(x, y);

	if(WgUtil::FromString(xmlNode["empty_row_height"], y))
		widget->SetEmptyRowHeight(y);

	widget->SetAutoScrollMarked(WgUtil::ToBool(xmlNode["autoscroll_marked"]));
	widget->ShowHeader(WgUtil::ToBool(xmlNode["header"], true));
	widget->SetAutoScaleHeaders(WgUtil::ToBool(xmlNode["scaleheader"], false));
	widget->SetHeaderTextProp(s.ResDb()->GetTextProp(xmlNode["header_textprop"]));

	if( xmlNode.HasAttribute("header_blockset") )
	{
		// Temporary code to ease the transition until we have header_blockset_normal
		// and header_blockset_selected in all XML-files.

		WgBlockSetPtr p = s.ResDb()->GetBlockSet(xmlNode["header_blockset"]);
		widget->SetHeaderSource( p, p );
	}
	else
	{
		widget->SetHeaderSource(s.ResDb()->GetBlockSet(xmlNode["header_blockset_normal"]), 
								s.ResDb()->GetBlockSet(xmlNode["header_blockset_selected"]));
	}

	widget->SetArrowOrigo(WgUtil::ToOrigo(xmlNode["arrow_origo"]));
	widget->SetClickSortPrio(WgUtil::ToUint8(xmlNode["sortprio"]));

	WgBlockSetPtr pAscend = s.ResDb()->GetBlockSet(xmlNode["arrow_asc"]);
	WgBlockSetPtr pDescend = s.ResDb()->GetBlockSet(xmlNode["arrow_dsc"]);
	widget->SetArrowSource(pAscend, pDescend);
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_TabList_Res //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Wdg_TabList_Res::Wdg_TabList_Res(WgResourceXML* parent, Wdg_TabList* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_TabList_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	const WgXmlNode& xmlNode = XmlNode();
	Wdg_TabList* widget = GetWidget();
	WgWidgetRes::Serialize(s);

	WriteTextPropAttr(s, widget->GetTextProperties(), "textprop");
	WriteBlockSetAttr(s, widget->GetSource(), "blockset");
	WriteDiffAttr<Sint32>(s, xmlNode, "overlap", widget->GetOverlap(), 0);
	WriteDiffAttr<Sint32>(s, xmlNode, "maxoverlap", widget->GetMaxOverlap(), 0);
	WriteDiffAttr<Sint32>(s, xmlNode, "minwidth", widget->GetMinTabWidth(), 0);
	WriteDiffAttr<Sint32>(s, xmlNode, "alertrate", widget->GetAlertRate(), 250);
	WriteDiffAttr(s, xmlNode, "textorigo", widget->GetTextOrigo(), WgOrigo::topLeft());
	WriteDiffAttr(s, xmlNode, "opaque_tabs", widget->getTabMouseOpacity(), false );

	if(widget->GetTabWidthMode() != Wdg_TabList::INDIVIDUAL_WIDTH || xmlNode.HasAttribute("tabwidth"))
	{
		Wdg_TabList::TabWidthMode mode = widget->GetTabWidthMode();
		std::string value = "individual";
		if(mode == Wdg_TabList::UNIFIED_WIDTH) value = "unified";

		s.AddAttribute("tabwidth", value);
	}

	if(widget->GetTabExpandMode() != Wdg_TabList::NO_EXPAND || xmlNode.HasAttribute("tabexpand"))
	{
		Wdg_TabList::TabExpandMode mode = widget->GetTabExpandMode();
		std::string value = "none";
		if(mode == Wdg_TabList::GROW_TABS) value = "grow";
		else if(mode == Wdg_TabList::GROW_TABS) value = "spread";
		else if(mode == Wdg_TabList::UNIFY_TABS) value = "unify";
		
		s.AddAttribute("tabexpand", value);
	}

	if(widget->GetTabCompressMode() != Wdg_TabList::NO_COMPRESS || xmlNode.HasAttribute("tabcompress"))
	{
		Wdg_TabList::TabCompressMode mode = widget->GetTabCompressMode();
		std::string value = "none";
		if(mode == Wdg_TabList::SHRINK_TABS) value = "shrink";
		else if(mode == Wdg_TabList::OVERLAP_TABS) value = "overlap";
		
		s.AddAttribute("tabcompress", value);
	}

	for(WgTab*tab = widget->GetFirstTab(); tab; tab = tab->Next())
	{
		WgTabRes* tabRes = new WgTabRes(this, tab);
		WgResDB::TabRes* tabDb = s.ResDb()->FindResTab(tab);
		if(tabDb)
			tabRes->SetMetaData(tabDb->meta);
		tabRes->Serialize(s);
		delete tabRes;
	}

	s.EndTag();
}

void Wdg_TabList_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_TabList* widget = new Wdg_TabList();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);
	widget->SetTextProperties(s.ResDb()->GetTextProp(xmlNode["textprop"]));
	WgBlockSetPtr blockset = s.ResDb()->GetBlockSet(xmlNode["blockset"]);
	widget->SetSource( blockset, Wdg_TabList::SourceTypeAll);
	widget->SetOverlap(WgUtil::ToSint32(xmlNode["overlap"]));
	widget->SetMaxOverlap(WgUtil::ToSint32(xmlNode["maxoverlap"]));
	widget->SetMinTabWidth(WgUtil::ToSint32(xmlNode["minwidth"]));
	widget->SetAlertRate(WgUtil::ToSint32(xmlNode["alertrate"]));
	widget->SetTextOrigo(WgUtil::ToOrigo(xmlNode["textorigo"]));
	widget->setTabMouseOpacity(WgUtil::ToBool(xmlNode["opaque_tabs"]));

	std::string mode = xmlNode["tabwidth"];
	if(mode.empty() || mode == "individual")widget->SetTabWidthMode(Wdg_TabList::INDIVIDUAL_WIDTH);
	else if(mode == "unified")			widget->SetTabWidthMode(Wdg_TabList::UNIFIED_WIDTH);
	else assert(0);

	mode = xmlNode["tabexpand"];
	if(mode.empty() || mode == "none")widget->SetTabExpandMode(Wdg_TabList::NO_EXPAND);
	else if(mode == "grow")			widget->SetTabExpandMode(Wdg_TabList::GROW_TABS);
	else if(mode == "spread")			widget->SetTabExpandMode(Wdg_TabList::SPREAD_TABS);
	else if(mode == "unify")			widget->SetTabExpandMode(Wdg_TabList::UNIFY_TABS);
	else assert(0);

	mode = xmlNode["tabcompress"];
	if(mode.empty() || mode == "none")widget->SetTabCompressMode(Wdg_TabList::NO_COMPRESS);
	else if(mode == "shrink")			widget->SetTabCompressMode(Wdg_TabList::SHRINK_TABS);
	else if(mode == "overlap")			widget->SetTabCompressMode(Wdg_TabList::OVERLAP_TABS);
	else assert(0);

}

//////////////////////////////////////////////////////////////////////////
/// WgTabRes /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <tab	[text-attribs]
//		uid=[integer]
//		alert=[true/false]
//		selected=[true/false]
//		blockset=[name]
// />
WgTabRes::WgTabRes(WgResourceXML* parent, WgTab* tab) :
	WgResourceXML(parent),
	m_tab(tab),
	m_pCharBuffer(0)
{
}

WgTabRes::~WgTabRes()
{
	delete m_pCharBuffer;
}

void WgTabRes::Serialize(WgResourceSerializerXML& s)
{
	Wdg_TabList_Res* tabListRes = WgResourceXML::Cast<Wdg_TabList_Res>(Parent());
	ASSERT(tabListRes, "invalid parent for <tab>. should be <tablist>");
	if(tabListRes == 0)
		return;

	s.BeginTag(TagName(), XmlNode());

	WriteTextAttrib(s, m_tab->getText(), "text");
	//WgTextHolderRes::Serialize(this, XmlNode(), s, 0);

	const WgXmlNode& xmlNode = XmlNode();

	WriteDiffAttr(s, xmlNode, "uid", m_tab->GetId(), 0);
	WriteDiffAttr(s, xmlNode, "alert", m_tab->GetAlert(), false);
	WriteDiffAttr(s, xmlNode, "selected", tabListRes->GetWidget()->GetSelectedTabId() == m_tab->GetId(), false);

	if(xmlNode.HasAttribute("icon"))
	{
		WgItemRow *pItems = tabListRes->GetWidget()->LockTabContent(m_tab->GetId());
		if(pItems)
		{
			WgItem* pItem = pItems->GetFirstItem();
			if(pItem && pItem->Type() == WgItemPixmap::GetMyType())
			{
				WriteBlockSetAttr(s, ((WgItemPixmap*)pItem)->GetSource(), "icon");
			}
			tabListRes->GetWidget()->UnlockTabContent(m_tab->GetId());
		}
	}

	if( m_tab->GetSource() )
		WriteBlockSetAttr(s, m_tab->GetSource(), "blockset");

	s.EndTag();
}

void WgTabRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_TabList_Res* tabListRes = WgResourceXML::Cast<Wdg_TabList_Res>(Parent());
	ASSERT(tabListRes, "invalid parent for <tab>. should be <tablist>");
	if(tabListRes == 0)
		return;

	//WgTextHolderRes::Deserialize(xmlNode, s, 0);
	std::string text = ReadLocalizedString(xmlNode["text"], s);
	Wdg_TabList* tabList = tabListRes->GetWidget();
	Sint32 uid = WgUtil::ToSint32(xmlNode["uid"], 1 + tabList->GetTabCount());
	WgBlockSetPtr blockset = s.ResDb()->GetBlockSet(xmlNode["blockset"]);
	tabList->AddTab(uid, text.c_str(), INT_MAX, blockset);
	tabList->SetAlert(uid, WgUtil::ToBool(xmlNode["alert"]) );
	if(xmlNode.HasAttribute("icon"))
	{
		WgBlockSetPtr block = s.ResDb()->GetBlockSet(xmlNode["icon"]);
		if(block)
		{
			WgItemRow *pItems = tabList->LockTabContent(uid);
			pItems->AddItem(new WgItemPixmap(uid, WgBorders(0), block));
			tabList->UnlockTabContent(uid);
		}
	}

	if(WgUtil::ToBool(xmlNode["selected"]))
		tabList->SelectTab(uid);

	WgTab* pTab = tabList->GetLastTab();
	s.ResDb()->AddTab(xmlNode["id"], pTab, new WgXMLMetaData(XmlNode()));
}

WgCharBuffer* WgTabRes::GetCharBuffer()
{
	if(m_pCharBuffer == 0)
		m_pCharBuffer = new WgCharBuffer();

	return m_pCharBuffer;
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_Text_Res /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <text [widget-attribs] [text-attribs]
//		tooltip=[text]
// />
Wdg_Text_Res::Wdg_Text_Res(WgResourceXML* parent, Wdg_Text* widget) :
	WgWidgetRes(parent, widget),
	m_pCharBuffer(0)
{
}

Wdg_Text_Res::~Wdg_Text_Res()
{
	delete m_pCharBuffer;
}

void Wdg_Text_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	const WgXmlNode& xmlNode = XmlNode();
	Wdg_Text* widget = GetWidget();
	WgWidgetRes::Serialize(s);
	WgEditTextRes::Serialize(this, xmlNode, s, widget);
	WriteTextAttrib(s, widget->GetRealTooltipString().Chars(), "tooltip");
	WriteDiffAttr(s, xmlNode, "max_rows", widget->MaxLines(), 0);
	s.EndTag();
}

void Wdg_Text_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_Text* widget = new Wdg_Text();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);
	WgEditTextRes::Deserialize(xmlNode, s, widget);
	widget->SetTooltipString(WgCharSeq(ReadLocalizedString(xmlNode["tooltip"], s).c_str()));
	widget->SetMaxLines(WgUtil::ToUint32(xmlNode["max_rows"]));
}

WgCharBuffer* Wdg_Text_Res::GetCharBuffer()
{
	if(m_pCharBuffer == 0)
		m_pCharBuffer = new WgCharBuffer();

	return m_pCharBuffer;
}


//////////////////////////////////////////////////////////////////////////
/// Wdg_EditLine_Res /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <editline [widget-attribs] [text-attribs]
//		password=[true | false]
//		pwd_glyph=[integer]		// TODO: support unicode/utf8 character
//		max_length=[integer]
// />
Wdg_EditLine_Res::Wdg_EditLine_Res(WgResourceXML* parent, Wdg_Text* widget) :
	WgWidgetRes(parent, widget),
	m_pCharBuffer(0)
{
}

Wdg_EditLine_Res::~Wdg_EditLine_Res()
{
	delete m_pCharBuffer;
}

void Wdg_EditLine_Res::Serialize(WgResourceSerializerXML& s)
{
	s.BeginTag(TagName(), XmlNode());

	const WgXmlNode& xmlNode = XmlNode();
	Wdg_EditLine* widget = GetWidget();
	WgWidgetRes::Serialize(s);
	WgEditTextRes::Serialize(this, xmlNode, s, widget);

	WriteDiffAttr(s, xmlNode, "password", widget->PasswordMode(), false);
	WriteDiffAttr(s, xmlNode, "pwd_glyph", widget->PasswordGlyph(), (Uint16)'*');

	s.EndTag();
}

void Wdg_EditLine_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_EditLine* widget = new Wdg_EditLine();
	m_Widget = widget;
	WgWidgetRes::Deserialize(xmlNode, s);
	WgEditTextRes::Deserialize(xmlNode, s, widget);

	widget->SetPasswordMode(WgUtil::ToBool(xmlNode["password"]));
	widget->SetPasswordGlyph(WgUtil::ToUint16(xmlNode["pwd_glyph"], (Uint16)'*'));
}

WgCharBuffer* Wdg_EditLine_Res::GetCharBuffer()
{
	if(m_pCharBuffer == 0)
		m_pCharBuffer = new WgCharBuffer();

	return m_pCharBuffer;
}

//////////////////////////////////////////////////////////////////////////
/// Wdg_TextView_Res /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Wdg_TextView_Res::Wdg_TextView_Res(WgResourceXML* parent, class Wdg_TextView* widget) :
	WgWidgetRes(parent, widget),
	m_pCharBuffer(0)
{
}

Wdg_TextView_Res::~Wdg_TextView_Res()
{
}

WgCharBuffer* Wdg_TextView_Res::GetCharBuffer()
{
	if(m_pCharBuffer == 0)
		m_pCharBuffer = new WgCharBuffer();

	return m_pCharBuffer;
}

void Wdg_TextView_Res::Serialize(WgResourceSerializerXML& s)
{
	Wdg_TextView* widget = GetWidget();
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());

	WgWidgetRes::Serialize(s);
	WgBaseViewRes::Serialize(xmlNode, s, widget);
	WgEditTextRes::Serialize(this, xmlNode, s, widget);

	s.EndTag();
}

void Wdg_TextView_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_TextView* widget = new Wdg_TextView();
	m_Widget = widget;

	WgWidgetRes::Deserialize(xmlNode, s);
	WgBaseViewRes::Deserialize(xmlNode, s, widget);
	WgEditTextRes::Deserialize(xmlNode, s, widget);

}

//////////////////////////////////////////////////////////////////////////
/// Wdg_YSplitter_Res ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <ysplitter [widget-attribs] 
//		top_pane=
//		bottom_pane=
//		handle=
//		split_fraction=
// />
Wdg_YSplitter_Res::Wdg_YSplitter_Res(WgResourceXML* parent, Wdg_YSplitter* widget) :
	WgWidgetRes(parent, widget)
{
}

void Wdg_YSplitter_Res::Serialize(WgResourceSerializerXML& s)
{
	const WgXmlNode& xmlNode = XmlNode();
	Wdg_YSplitter* widget = GetWidget();

	s.BeginTag(TagName(), XmlNode());

	WgWidgetRes::Serialize(s);

	if( widget->GetTopPane() )
		WriteWidgetRefAttr(s, widget->GetTopPane(), "top_pane");

	if( widget->GetBottomPane() )
		WriteWidgetRefAttr(s, widget->GetBottomPane(), "bottom_pane");

	if( widget->GetHandle() )
		WriteWidgetRefAttr(s, widget->GetHandle(), "handle");

	WriteDiffAttr(s, xmlNode, "split_fraction", widget->GetTopFraction(), 0.5f);

	s.EndTag();
}

void Wdg_YSplitter_Res::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	Wdg_YSplitter* widget = new Wdg_YSplitter();
	m_Widget = widget;

	WgWidgetRes::Deserialize(xmlNode, s);

	WgWidget* topPane = s.ResDb()->GetWidget( xmlNode["top_pane"] );
	WgWidget* bottomPane = s.ResDb()->GetWidget( xmlNode["bottom_pane"] );
	WgWidget* handle = s.ResDb()->GetWidget( xmlNode["handle"] );
	float fraction = WgUtil::ToFloat( xmlNode["split_fraction"], 0.5f );

	widget->Setup( topPane, handle, bottomPane, fraction );
}

//////////////////////////////////////////////////////////////////////////
/// WgItemRes ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <...
//		selected=[true | false]
//		enabled=[true | false]
// .../>
void WgItemRes::Serialize(WgResourceSerializerXML& s)
{
	VERIFY(m_item, "item not set");
	const WgXmlNode& xmlNode = XmlNode();

	WriteDiffAttr(s, xmlNode, "uid", m_item->Id64(), (Sint64)0);
	WriteDiffAttr(s, xmlNode, "selected", m_item->IsSelected(), false);
	WriteDiffAttr(s, xmlNode, "enabled", !m_item->IsDisabled(), true);
}

void WgItemRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	VERIFY(m_item, "item not set");

	WgItemHolderRes* parentRes = WgResourceXML::Cast<WgItemHolderRes>(Parent());
	VERIFY(parentRes, "invalid parent for item. must be an item holder");

	m_item->SetId(WgUtil::ToSint32(xmlNode["uid"]));

	if(WgUtil::ToBool(xmlNode["selected"]))
		m_item->Select();
	else
		m_item->Unselect();

	if(WgUtil::ToBool(xmlNode["enabled"], true))
		m_item->Enable();
	else
		m_item->Disable();

	s.ResDb()->AddItem(xmlNode["id"], m_item, new WgXMLMetaData(XmlNode()));
}

void WgItemRes::Deserialized(WgResourceSerializerXML& s)
{
	WgItemHolderRes* holderRes = WgResourceXML::Cast<WgItemHolderRes>(Parent());
	if(holderRes)
		holderRes->GetHolder()->AddItem(m_item);
}


//////////////////////////////////////////////////////////////////////////
/// WgItemPixmapRes //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <itempixmap [item-attribs]
//		blockset=[blockset]
//		margin=[integer]
//		forcesize=[w, h]
//		tooltip=[text]
// />
WgItemPixmapRes::WgItemPixmapRes(WgResourceXML* parent, WgItemPixmap* item) :
	WgItemRes(parent, item)
{
}

void WgItemPixmapRes::Serialize(WgResourceSerializerXML& s)
{
	WgItemPixmap* item = GetItem();
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());
	WriteBlockSetAttr(s, item->GetSource(), "blockset");
	WgBorderRes::Serialize(s, xmlNode, "margin", item->Margin());
	WgSizeRes::Serialize(s, xmlNode, "forcesize", item->GetForceSize(), WgSize(0, 0) );
	WriteTextAttrib(s, item->GetRealTooltipString().Chars(), "tooltip");
	s.EndTag();
}

void WgItemPixmapRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgItemPixmap* item = new WgItemPixmap();
	m_item = item;
	WgItemRes::Deserialize(xmlNode, s);

	item->SetSource( s.ResDb()->GetBlockSet(xmlNode["blockset"]) );
	item->SetMargin( WgBorderRes::Deserialize(s, xmlNode["margin"]) );
	item->ForceSize( WgSizeRes::Deserialize(xmlNode, "forcesize") );
	item->SetTooltipString(ReadLocalizedString(xmlNode["tooltip"], s).c_str());
}

//////////////////////////////////////////////////////////////////////////
/// WgItemRowRes /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <itemrow [item-attribs]
//		tooltip=[text]
// />
WgItemRowRes::WgItemRowRes(WgResourceXML* parent, WgItemRow* item) :
	WgItemRes(parent, item)
{
}

void WgItemRowRes::Serialize(WgResourceSerializerXML& s)
{
	WgItemRow* item = GetItem();
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());
	WgItemRes::Serialize(s);
	WgItemHolderRes::Serialize(this, xmlNode, s, item);
//	WriteTextAttrib(s, item->GetRealTooltipString().Chars(), "tooltip");
	s.EndTag();
}

void WgItemRowRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgItemRow* item = new WgItemRow();
	m_item = item;
	WgItemRes::Deserialize(xmlNode, s);
	WgItemHolderRes::Deserialize(xmlNode, s, item);
//	item->SetTooltipString(ReadLocalizedString(xmlNode["tooltip"], s).c_str());
}

//////////////////////////////////////////////////////////////////////////
/// WgItemStackRes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <itemstack [item-attribs]
//		tooltip=[text]
// />
WgItemStackRes::WgItemStackRes(WgResourceXML* parent, WgItemStack* item) :
	WgItemRes(parent, item)
{
}

void WgItemStackRes::Serialize(WgResourceSerializerXML& s)
{
	const WgXmlNode& xmlNode = XmlNode();
	WgItemStack* item = GetItem();

	s.BeginTag(TagName(), XmlNode());
	WgItemRes::Serialize(s);
	WgItemHolderRes::Serialize(this, xmlNode, s, item->GetItemHolder());
	WriteTextAttrib(s, item->GetRealTooltipString().Chars(), "tooltip");
	s.EndTag();
}

void WgItemStackRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgItemStack* item = new WgItemStack();
	m_item = item;
	WgItemRes::Deserialize(xmlNode, s);
	WgItemHolderRes::Deserialize(xmlNode, s, item->GetItemHolder());
	item->SetTooltipString(ReadLocalizedString(xmlNode["tooltip"], s).c_str());
}

//////////////////////////////////////////////////////////////////////////
/// WgItemTextRes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <itemtext [item-attribs] [text-attribs]
// />
WgItemTextRes::WgItemTextRes(WgResourceXML* parent, WgItemText* item) :
	WgItemRes(parent, item),
	m_pCharBuffer(0)
{
}

WgItemTextRes::~WgItemTextRes()
{
	delete m_pCharBuffer;
}

void WgItemTextRes::Serialize(WgResourceSerializerXML& s)
{
	WgItemText* item = GetItem();
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());
	WgItemRes::Serialize(s);
	WgTextHolderRes::Serialize(this, xmlNode, s, item);
	s.EndTag();
}

void WgItemTextRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgItemText* item = new WgItemText();
	m_item = item;
	WgTextHolderRes::Deserialize(xmlNode, s, item);
	WgItemRes::Deserialize(xmlNode, s);
}

WgCharBuffer* WgItemTextRes::GetCharBuffer()
{
	if(m_pCharBuffer == 0)
		m_pCharBuffer = new WgCharBuffer();

	return m_pCharBuffer;
}

//////////////////////////////////////////////////////////////////////////
/// WgItemWrapTextRes ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <itemwraptext [item-attribs] [text-attribs]
// />
WgItemWrapTextRes::WgItemWrapTextRes(WgResourceXML* parent, WgItemWrapText* item) :
	WgItemRes(parent, item),
	m_pCharBuffer(0)
{
}

WgItemWrapTextRes::~WgItemWrapTextRes()
{
	delete m_pCharBuffer;
}

void WgItemWrapTextRes::Serialize(WgResourceSerializerXML& s)
{
	WgItemWrapText* item = GetItem();
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());
	WgItemRes::Serialize(s);
	WgTextHolderRes::Serialize(this, xmlNode, s, item);
	s.EndTag();
}

void WgItemWrapTextRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgItemWrapText* item = new WgItemWrapText();
	m_item = item;
	WgTextHolderRes::Deserialize(xmlNode, s, item);
	WgItemRes::Deserialize(xmlNode, s);
}

WgCharBuffer* WgItemWrapTextRes::GetCharBuffer()
{
	if(m_pCharBuffer == 0)
		m_pCharBuffer = new WgCharBuffer();

	return m_pCharBuffer;
}

//////////////////////////////////////////////////////////////////////////
/// WgMenuItemRes ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <...
//		id=name
//		uid=[integer]
//		visible=[true | false]
// />
void WgMenuItemRes::Serialize(WgResourceSerializerXML& s)
{
	WgMenuItem* item = GetItem();
	const WgXmlNode& xmlNode = XmlNode();

	WriteIDAttrib(s, xmlNode, s.ResDb()->FindMenuItemId(m_item));
	WriteDiffAttr(s, xmlNode, "uid", item->GetId(), 0);
	WriteDiffAttr(s, xmlNode, "visible", item->IsSetToVisible(), true);
}

void WgMenuItemRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	VERIFY(m_item, "item not set");

	m_item->SetId(WgUtil::ToSint32(xmlNode["uid"]));
	m_item->SetVisible(WgUtil::ToBool(xmlNode["visible"], true));
	s.ResDb()->AddMenuItem(xmlNode["id"], m_item, new WgXMLMetaData(XmlNode()));

	Wdg_Menu_Res* menuRes = WgResourceXML::Cast<Wdg_Menu_Res>(Parent());
	if(menuRes)
	{
		menuRes->GetWidget()->AddItem(m_item);
	}
}

//////////////////////////////////////////////////////////////////////////
/// WgMenuSeparatorRes ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <menuseparator [menuitem-attribs] />
WgMenuSeparatorRes::WgMenuSeparatorRes(WgResourceXML* parent, WgMenuSeparator* item) :
	WgMenuItemRes(parent, item)
{
}

void WgMenuSeparatorRes::Serialize(WgResourceSerializerXML& s)
{
//	WgMenuSeparator* item = GetItem();
//	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());
	WgMenuItemRes::Serialize(s);
	s.EndTag();
}

void WgMenuSeparatorRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgMenuSeparator* item = new WgMenuSeparator();
	m_item = item;
	WgMenuItemRes::Deserialize(xmlNode, s);
}

//////////////////////////////////////////////////////////////////////////
/// WgMenuEntryRes ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <menuentry [menuitem-attribs]
//		enabled=[true | false]
//		text=[text]
//		helptext=[text]
//		acceltext=[text]
//		accelmod=[none | shift | ctrl | alt | ctrl+alt]
//		accelkey=[integer]
//		navkey=[integer]
// />
WgMenuEntryRes::WgMenuEntryRes(WgResourceXML* parent, WgMenuEntry* item) :
	WgMenuItemRes(parent, item)
{
}

void WgMenuEntryRes::Serialize(WgResourceSerializerXML& s, bool bOpenTag)
{
	WgMenuEntry* item = GetItem();
	const WgXmlNode& xmlNode = XmlNode();

	if(bOpenTag)
		s.BeginTag(TagName(), XmlNode());

	WgMenuItemRes::Serialize(s);
	WriteDiffAttr(s, xmlNode, "enabled", item->IsEnabled(), true);
	WriteTextAttrib(s, item->GetText().Chars(), "text");

	if( !item->GetHelpText().IsEmpty() )
		WriteTextAttrib(s, item->GetHelpText().Chars(), "helptext");
	else
		s.RemoveAttribute("helptext");

	WriteTextAttrib(s, item->GetAccelText().Chars(), "acceltext");
	WriteDiffAttr(s, xmlNode, "navkey", item->GetNavKey(), (Uint16)0);
	if(xmlNode.HasAttribute("accelmod") || item->GetAccelModif() != WG_MODKEY_NONE)
	{
		std::string value;
		switch(item->GetAccelModif())
		{
		case WG_MODKEY_NONE:				value = "none"; break;
		case WG_MODKEY_SHIFT:				value = "shift"; break;
		case WG_MODKEY_CTRL:				value = "ctrl"; break;
		case WG_MODKEY_ALT:					value = "alt"; break;
		case WG_MODKEY_ALT_SHIFT:			value = "alt+shift"; break;
		case WG_MODKEY_CTRL_SHIFT:			value = "ctrl+shift"; break;
		case WG_MODKEY_CTRL_ALT:			value = "ctrl+alt"; break;
		case WG_MODKEY_CTRL_ALT_SHIFT:		value = "ctrl+alt+shift"; break;
		case WG_MODKEY_SUPER:				value = "super"; break;
		case WG_MODKEY_SUPER_SHIFT:			value = "super+shift"; break;
		case WG_MODKEY_SUPER_CTRL:			value = "super+ctrl"; break;
		case WG_MODKEY_SUPER_ALT:			value = "super+alt"; break;
		case WG_MODKEY_SUPER_ALT_SHIFT:		value = "super+alt+shift"; break;
		case WG_MODKEY_SUPER_CTRL_SHIFT:	value = "super+ctrl+shift"; break;
		case WG_MODKEY_SUPER_CTRL_ALT:		value = "super+ctrl+alt"; break;
		case WG_MODKEY_SUPER_CTRL_ALT_SHIFT:value = "super+ctrl+alt+shift"; break;
		default:	s.Warning( "Unknown modifier key in menu entry skipped" ); break;
		}
		s.AddAttribute("accelmod", value);
	}
	WriteDiffAttr(s, xmlNode, "accelkey", item->GetAccelKey(), (Uint16)0);

	if(bOpenTag)
		s.EndTag();
}

void WgMenuEntryRes::Serialize(WgResourceSerializerXML& s)
{
	Serialize(s, true);
}

void WgMenuEntryRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgMenuEntry* item;
	if(m_item)
	{
		item = (WgMenuEntry*)m_item;
	}
	else
	{
		item = new WgMenuEntry();
		m_item = item;
	}
	item->SetText( WgCharSeq(ReadLocalizedString(xmlNode["text"], s)) );	// HACK: set text before adding entry to menu to get correct width
	WgMenuItemRes::Deserialize(xmlNode, s);	//		 this will add the entry to the menu
	item->SetHelpText(WgCharSeq(ReadLocalizedString(xmlNode["helptext"], s)));
	item->SetNavKey(WgUtil::ToUint16(xmlNode["navkey"]));

	std::string value = xmlNode["accelmod"];
	if(value.empty() || value == "none") item->SetAccelModifier(WG_MODKEY_NONE);
	else if(value == "shift") item->SetAccelModifier(WG_MODKEY_SHIFT);
	else if(value == "ctrl") item->SetAccelModifier(WG_MODKEY_CTRL);
	else if(value == "alt") item->SetAccelModifier(WG_MODKEY_ALT);
	else if(value == "alt+shift") item->SetAccelModifier(WG_MODKEY_ALT_SHIFT);
	else if(value == "ctrl+alt") item->SetAccelModifier(WG_MODKEY_CTRL_ALT);
	else if(value == "ctrl+shift") item->SetAccelModifier(WG_MODKEY_CTRL_SHIFT);
	else if(value == "ctrl+alt+shift") item->SetAccelModifier(WG_MODKEY_CTRL_ALT_SHIFT);
	else if(value == "super") item->SetAccelModifier(WG_MODKEY_SUPER);
	else if(value == "super+shift") item->SetAccelModifier(WG_MODKEY_SUPER_SHIFT);
	else if(value == "super+ctrl") item->SetAccelModifier(WG_MODKEY_SUPER_CTRL);
	else if(value == "super+alt") item->SetAccelModifier(WG_MODKEY_SUPER_ALT);
	else if(value == "super+alt+shift") item->SetAccelModifier(WG_MODKEY_SUPER_ALT_SHIFT);
	else if(value == "super+ctrl+alt") item->SetAccelModifier(WG_MODKEY_SUPER_CTRL_ALT);
	else if(value == "super+ctrl+shift") item->SetAccelModifier(WG_MODKEY_SUPER_CTRL_SHIFT);
	else if(value == "super+ctrl+alt+shift") item->SetAccelModifier(WG_MODKEY_SUPER_CTRL_ALT_SHIFT);
	else ASSERT(0, "invallid modifier key");

	item->SetAccelKey(WgUtil::ToUint16(xmlNode["accelkey"]));
	item->SetAccelText( WgCharSeq(ReadLocalizedString(xmlNode["acceltext"], s)) );

	if(WgUtil::ToBool(xmlNode["enabled"], true))
		item->Enable();
	else
		item->Disable();
}

//////////////////////////////////////////////////////////////////////////
/// WgMenuCheckBoxRes ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <menucheckbox [menuentry-attribs] [menuitem-attribs]
//		checked=[true | false]
// />
WgMenuCheckBoxRes::WgMenuCheckBoxRes(WgResourceXML* parent, WgMenuCheckBox*item) :
	WgMenuEntryRes(parent, item)
{
}

void WgMenuCheckBoxRes::Serialize(WgResourceSerializerXML& s)
{
	WgMenuCheckBox* item = GetItem();
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());
	WgMenuEntryRes::Serialize(s, false);
	WriteDiffAttr(s, xmlNode, "checked", item->IsChecked(), false);
	s.EndTag();
}

void WgMenuCheckBoxRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgMenuCheckBox* item = new WgMenuCheckBox();
	m_item = item;
	WgMenuEntryRes::Deserialize(xmlNode, s);
	if(WgUtil::ToBool(xmlNode["checked"]))
		item->Check();
}

//////////////////////////////////////////////////////////////////////////
/// WgMenuRadioButtonRes /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <menuradiobutton [menuentry-attribs] [menuitem-attribs]
//		selected=[true | false]
// />
WgMenuRadioButtonRes::WgMenuRadioButtonRes(WgResourceXML* parent, WgMenuRadioButton* item) :
	WgMenuEntryRes(parent, item)
{
}

void WgMenuRadioButtonRes::Serialize(WgResourceSerializerXML& s)
{
	WgMenuRadioButton* item = GetItem();
	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());
	WgMenuEntryRes::Serialize(s, false);
	WriteDiffAttr(s, xmlNode, "selected", item->IsSelected(), false);
	s.EndTag();
}

void WgMenuRadioButtonRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgMenuRadioButton* item = new WgMenuRadioButton();
	m_item = item;
	WgMenuEntryRes::Deserialize(xmlNode, s);
	if(WgUtil::ToBool(xmlNode["selected"]))
		item->Select();
}

//////////////////////////////////////////////////////////////////////////
/// WgMenuSubMenuRes /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <menusubmenu [menuentry-attribs] [menuitem-attribs] >
//		<menu ... />
// </menusubmenu>
WgMenuSubMenuRes::WgMenuSubMenuRes(WgResourceXML* parent, WgMenuSubMenu* item) :
	WgMenuEntryRes(parent, item)
{
}

void WgMenuSubMenuRes::Serialize(WgResourceSerializerXML& s)
{
	WgMenuSubMenu* item = GetItem();
//	const WgXmlNode& xmlNode = XmlNode();

	s.BeginTag(TagName(), XmlNode());
	WgMenuEntryRes::Serialize(s, false);

	Wdg_Menu* menu = item->GetSubMenu();
	if(menu && WgXmlRoot::IsTopLevelWidget(menu, s))
	{
		Wdg_Menu_Res* menuRes = new Wdg_Menu_Res(this, menu);
		WgResDB::WidgetRes* menuDb = s.ResDb()->FindResWidget(menu);
		if(menuDb)
			menuRes->SetMetaData(menuDb->meta);
		menuRes->Serialize(s);
		delete menuRes;
	}
	s.EndTag();
}

void WgMenuSubMenuRes::Deserialize(const WgXmlNode& xmlNode, WgResourceSerializerXML& s)
{
	WgMenuSubMenu* item = new WgMenuSubMenu();
	m_item = item;
	WgMenuEntryRes::Deserialize(xmlNode, s);
}

