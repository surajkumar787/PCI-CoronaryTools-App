// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "MenuButtonSmartSuite.h"

namespace Pci { namespace Core
{
using namespace Sense;
using namespace Sense::ExperienceIdentity;

MenuButtonSmartSuite::MenuButtonSmartSuite(Control &parent)
:
	MenuButton(parent)
{
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//												Up       			UpCheck     		Down     			DownCheck  			Hover    			HoverCheck 				Disabled			DisabledCheck		
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	static const Color smartsuiteFont[] = { Palette::Gray050,	Palette::Gray100,   Palette::Gray030,	Palette::Gray100,   Palette::Gray070,	Palette::Gray100,		Palette::Gray100,	Palette::Gray100, };
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	colorFont	= &smartsuiteFont;
}

MenuButtonSmartSuite::~MenuButtonSmartSuite()
{
}

}}