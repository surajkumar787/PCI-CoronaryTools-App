// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PciThumbnail.h"
#include <algorithm>

namespace Pci{ namespace Core{

#define _USE_MATH_DEFINES
#include <math.h>

using namespace Sense;
using namespace Sense::ExperienceIdentity;

void calculate(Vertex *vertices, const Rect &dst, const Color &color, double radius, double scale, int count)
{
	for (int i = 0; i < count; i++)
	{
		double factor = static_cast<double>(i)/ static_cast<double>(count-1);
		double s = sin(0.5*M_PI*factor);
		double c = cos(0.5*M_PI*factor);

		Point tl = dst.topLeft()     + Point(-c, -s) * radius;	 vertices[(count * 0 + i) * 2] = Vertex(tl, color, tl * scale);
		Point tr = dst.topRight()    + Point( s, -c) * radius;	 vertices[(count * 1 + i) * 2] = Vertex(tr, color, tr * scale);
		Point br = dst.bottomRight() + Point( c,  s) * radius;	 vertices[(count * 2 + i) * 2] = Vertex(br, color, br * scale);
		Point bl = dst.bottomLeft()  + Point(-s,  c) * radius;	 vertices[(count * 3 + i) * 2] = Vertex(bl, color, bl * scale);
	}

	vertices[count * 4 * 2] = vertices[0];
}


PciThumbnail::PciThumbnail(Control &parent)
:
	Control			(parent),
	image			(Image(),			[this] { imageValid = false;	invalidate();						}),
	blink			(Image(),			[this] { blinkValid = false;	invalidate();	onVisibleChanged(); }),
	speed			(2.0,				[this] {						invalidate();						}),
	border			(Palette::Gray140,	[this] {						invalidate();						}),
	tickness		(3.0,				[this] {						invalidate();						}),
	radius			(24.0,				[this] {						invalidate();						}),
	blinkTimer		(scheduler,			[this] {						invalidate();						}),
	imageTexture	(gpu),
	imageValid		(false),
	blinkTexture	(gpu),
	blinkValid		(false)
{
}

PciThumbnail::~PciThumbnail()
{
}

void PciThumbnail::onVisibleChanged()
{
	Control::onVisibleChanged();

	if (isVisible() && (!blink->empty()))
	{
		blinkTimer.start(Timer::Priority::Fast);
	}
	else
	{
		blinkTimer.stop();
	}
}

void PciThumbnail::render(IRenderer &renderer) const
{
	Space space = getSpace();

	if (!imageValid || imageTexture.isValid()) imageTexture.set(image->data());
	if (!blinkValid || blinkTexture.isValid()) blinkTexture.set(blink->data());
	
	imageValid = true;
	blinkValid = true;
	
	Rect dst = space.bounds;
	
	if (!dst.empty())
	{
		Rect src = Rect(0,0, imageTexture.getWidth(), imageTexture.getHeight());
		double scaling = std::max(src.width() / dst.width(), src.height() / dst.height());
		int    count = 6;
	
		Vertex vertices[128];

		Rect rect = space.bounds - Margin(radius);

		calculate(vertices,   Rect(rect.center()),	Color(1.0), 0.0,			scaling, count);
		calculate(vertices+1, rect,					Color(1.0), radius - 0.5,	scaling, count);

		renderer.render(space, vertices, count * 4 * 2 + 2, imageTexture);

		if (!blink->empty())
		{
			double a = 0.5 + cos(blinkTimer.getElapsed() * speed) / 2.0;
		
			calculate(vertices,   Rect(rect.center()),	Color(1.0) * a, 0.0,			scaling, count);
			calculate(vertices+1, rect,					Color(1.0) * a, radius - 0.5,	scaling, count);

			renderer.render(space, vertices, count * 4 * 2 + 2, blinkTexture);
		}

		double width = std::max(0.0, tickness - 1.0);

		calculate(vertices,   rect, 0.0,    radius - 1.5 - width, scaling, count);
		calculate(vertices+1, rect, border, radius - 0.5 - width, scaling, count);

		renderer.render(space, vertices, count * 4 * 2 + 2);

		calculate(vertices,   rect, border, radius - 0.5 - width, scaling, count);
		calculate(vertices+1, rect, border, radius - 0.5,		 scaling, count);

		renderer.render(space, vertices, count * 4 * 2 + 2);

		calculate(vertices,   rect, border, radius - 0.5,		 scaling, count);
		calculate(vertices+1, rect, 0.0,    radius + 0.5,		 scaling, count);

		renderer.render(space, vertices, count * 4 * 2 + 2);
	}


	Control::render(renderer);
}
}}