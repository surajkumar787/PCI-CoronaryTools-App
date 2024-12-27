// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PciProgressBar.h"
#include "Sense/Shapes/Rectangle.h"
#include "ExperienceIdentity/Resource/Palette.h"

using namespace Sense;
using namespace ExperienceIdentity;
namespace Pci{ namespace  Core{

static const double MaxRange = 100.0;

PciProgressBar::PciProgressBar(Sense::Control &parent) :
	Control(parent),
	value	(MaxRange, [&] { invalidate(); }),
	min	   (0.0  , [&]{ invalidate(); }),	
	max	   (MaxRange, [&]{ invalidate(); }),
	color  (Color(1.0f,1.0f,0.0f)),
	drawing(gpu)
{
}



void PciProgressBar::onResize()
{
}

void PciProgressBar::render(Sense::IRenderer &renderer) const 
{
	Space space = getClientSpace();

	if(isInvalidated())
	{
		drawing.open(space /*, L"progress"*/);

		Rect rect(space.bounds);
		Rect bar(space.bounds);
		double stepSize = space.bounds.width() / max;

		if(value > 0)
		{
			 rect.left += stepSize * (max - value);
		}
		else
		{
			rect.left += stepSize * max ;
		}

		bar.right = rect.left;
		Rectangle::draw(drawing, rect, Palette::Black);
		Rectangle::draw(drawing, bar, Gradient(color));

		drawing.close();
	}

	drawing.render(space, renderer);
}
}}



