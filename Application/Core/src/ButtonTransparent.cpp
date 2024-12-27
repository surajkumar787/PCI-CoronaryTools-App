// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "ButtonTransparent.h"

namespace Pci { namespace Core
{

using namespace Sense;
using namespace Sense::ExperienceIdentity;

ButtonTransparent::ButtonTransparent(Control &parent)
: Button 	(parent)
{
    border = 0.0;
    edge   = 0.0;
    radius = 0.0;

    //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //												Up       			UpCheck     		Down     			DownCheck  			Hover    			HoverCheck 				Disabled			DisabledCheck		
    //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    static const Color defaultFont			[] = {	Palette::Gray180,	Palette::Gray100,   Palette::Gray100,	Palette::Gray100,   Palette::Gray070,	Palette::Gray100,		Palette::Gray100,	Palette::Gray100,	};
    static const Color defaultTop			[] = {	Color(),			Color(),			Color(),	        Color(),			Color(),	        Color(),				Color(),			Color()	};
    static const Color defaultBottom		[] = {	Color(),			Color(),			Color(),	        Color(),			Color(),	        Color(),				Color(),			Color()	};
    //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    colorFont	= &defaultFont;
    colorTop	= &defaultTop;
    colorBottom	= &defaultBottom;
}

ButtonTransparent::~ButtonTransparent()
{
}

void ButtonTransparent::setFontColor( Sense::Color normalColor, Sense::Color hoverColor  )
{
    //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //												Up       			UpCheck     		Down     			DownCheck  			Hover    			HoverCheck 				Disabled			DisabledCheck		
    //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    static const Color defaultFont			[] = {	normalColor,	    normalColor,        Palette::Gray100,	Palette::Gray100,   hoverColor,     	normalColor,            normalColor,        normalColor	        };
    colorFont	= &defaultFont;
}

}}