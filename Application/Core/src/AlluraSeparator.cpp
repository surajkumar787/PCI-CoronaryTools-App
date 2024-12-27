// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "AlluraSeparator.h"
#include "AlluraColors.h"

using namespace Pci::Core;
using namespace Sense;

AlluraSeparator::AlluraSeparator(Control &parent, SeparatorStyle separatorStyle) : 
    Control(parent),
	style(separatorStyle)
{
}

AlluraSeparator::~AlluraSeparator()
{
}


void AlluraSeparator::render(Sense::IRenderer &renderer) const
{
	Control::render(renderer);

	auto space = getSpace();

	Point center = space.bounds.center();

	Vertex barTop[] =
	{
		Vertex(space.bounds.topLeft(),				    Color::fromRgb(90,  90,  90)),
		Vertex(space.bounds.topRight(),				    Color::fromRgb(90,  90,  90)),
		Vertex(Point(space.bounds.left,  center.y),     Color::fromRgb(0,   0,   0)),
		Vertex(Point(space.bounds.right, center.y),     Color::fromRgb(0,   0,   0)),
	};

	Vertex barBottom[] =
	{
		Vertex(Point(space.bounds.left,  center.y),     Color::fromRgb(156, 156, 156)),
		Vertex(Point(space.bounds.right, center.y),     Color::fromRgb(156, 156, 156)),
		Vertex(space.bounds.bottomLeft(),               UI::AlluraGreyPanelsColor),
		Vertex(space.bounds.bottomRight(),              UI::AlluraGreyPanelsColor)
	};

	if ((style == SeparatorStyle::Full) || (style == AlluraSeparator::SeparatorStyle::Top))
	{
		renderer.render(space, barTop, 4);
	}

	if ((style == SeparatorStyle::Full) || (style == AlluraSeparator::SeparatorStyle::Bottom))
	{
		renderer.render(space, barBottom, 4);
	}
}


