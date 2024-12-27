// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "MenuButton.h"
#include "ViewType.h"

namespace Pci { namespace Core
{
using namespace Sense;
using namespace Sense::ExperienceIdentity;

MenuButton::MenuButton(Control &parent)
:
	ButtonFlat 	(parent),
	menu		(*this),
    root        ()
{
	border = 0.0;
	edge   = 0.0;
	radius = 0.0;

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//												Up       			UpCheck     		Down     			DownCheck  			Hover    			HoverCheck 				Disabled			DisabledCheck		
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	static const Color defaultTop			[] = {	Color(),			Color(),			Palette::Gray160,	Color(),			Palette::Gray160,	Color(),				Color(),			Color()	};
	static const Color defaultBottom		[] = {	Color(),			Color(),			Palette::Gray160,	Color(),			Palette::Gray160,	Color(),				Color(),			Color()	};
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	colorTop	= &defaultTop;
	colorBottom	= &defaultBottom;
    
	eventClicked = [this]
	{ 
        menu.setItems(root.submenu);
		if (!root.submenu.empty()) menu.expand(Point(0, size->height));
	}; 
}

MenuButton::~MenuButton()
{
}

void MenuButton::clearMenuItems()
{
	collapse();
	root.submenu.clear();
}

void MenuButton::addMenuItem( const std::wstring& itemText, const std::function<void()> &event )
{
    ContextMenu::Item item;

    item.checked		= false;
    item.checkable	    = false;
    item.text			= itemText;
    item.eventClicked	= event;

    root.submenu.push_back(item);
}

void MenuButton::collapse()
{
    menu.collapse();
}

void MenuButton::disable()
{
    menu.enabled = false;
}

void MenuButton::enable()
{
    menu.enabled = true;
}

}}