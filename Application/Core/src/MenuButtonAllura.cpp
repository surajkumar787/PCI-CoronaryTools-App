// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "MenuButtonAllura.h"

namespace Pci { namespace Core
{
using namespace Sense;
using namespace Sense::ExperienceIdentity;

MenuButtonAllura::MenuButtonAllura(Control &parent)
:
	MenuButton(parent)
{
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//												Up       			UpCheck     		Down     			DownCheck  			Hover    			HoverCheck 				Disabled			DisabledCheck		
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	static const Color alluraFont			[] = {	Palette::Gray180,	Palette::Gray100,   Palette::Gray100,	Palette::Gray100,   Palette::Gray070,	Palette::Gray100,		Palette::Gray100,	Palette::Gray100,	};
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	colorFont	= &alluraFont;
}

MenuButtonAllura::~MenuButtonAllura()
{
}

}}