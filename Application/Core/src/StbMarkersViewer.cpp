// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "StbMarkersViewer.h"

#include "XrayImage.h"
#include "StbMarkers.h"
#include "ViewType.h"
#include "StbUIHelpers.h"

namespace Pci { namespace Core
{

StbMarkersViewer::StbMarkersViewer(Control &parent, ViewType type) :
	PciXrayViewer(parent, type, true, type == ViewType::SmartSuiteFullHD ? Sense::VAlign::Middle : Sense::VAlign::Top)
{
	showXrayPosition = false;
}

void StbMarkersViewer::setMarkers(const std::shared_ptr<const StbMarkers> &Markers)
{
	markers = Markers;
	invalidate();
}

void StbMarkersViewer::renderOverlay(Sense::IRenderer &renderer) const
{
	using Sense::Point;

	Sense::Space  space  = getClientSpace();
	Sense::Matrix matrix = pixelsToClient();

	if (markers && markers->valid)
	{
		Sense::Drawing drawing(gpu);

		drawing.open(space);

		Point marker1(markers->p1.x, markers->p1.y);
		Point marker2(markers->p2.x, markers->p2.y);

		StbUIHelpers::drawMarkersEllipse(drawing, matrix, marker1, marker2);

		drawing.close();

		drawing.render(space, renderer);
	}
}

}}