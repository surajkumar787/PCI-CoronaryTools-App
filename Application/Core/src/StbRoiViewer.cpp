// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "StbRoiViewer.h"

#include <iomanip>

#include "Translation.h"
#include "StbImage.h"
#include "AlluraColors.h"
#include "ViewType.h"
#include "StbMarkers.h"
#include "StbUIHelpers.h"

#include <Sense/Graphics/IRenderer.h>
#include <ExperienceIdentity/Resource/Palette.h>
#include <ExperienceIdentity/Resource/Icons.h>


namespace UI = Pci::Core::UI;

namespace Pci {	namespace Core
{
	using Sense::ExperienceIdentity::Palette;

	StbRoiViewer::StbRoiViewer(Control &parent, ViewType type) :
		StbViewer(parent, type),
		regionOfInterest(Sense::Rect(Sense::Point(100, 100), Sense::Size(100, 100)), [this] { invalidate(); })
	{
	}

	void StbRoiViewer::setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image, const std::shared_ptr<const StbMarkers>& marker)
	{
		StbViewer::setImage(image);
		this->markers = marker;
	}

	void StbRoiViewer::renderOverlay(Sense::IRenderer &renderer) const
	{
		StbViewer::renderOverlay(renderer);

		Sense::Space  space = getClientSpace();
		Sense::Drawing drawing(gpu);

		drawRegionOfInterest(drawing);
		drawing.render(space, renderer);

		if (markers && markers->valid)
		{
			drawMarkers(drawing);
			drawing.render(space, renderer);
		}
	}

	void StbRoiViewer::drawRegionOfInterest(Sense::Drawing& drawing) const
	{
		Sense::Matrix matrix = pixelsToClient();
		drawing.open(getClientSpace());

		const double lineWidth(1.0);
		const std::vector<Sense::Point> roiPoints =
		{
			matrix * regionOfInterest->bottomLeft(),
			matrix * regionOfInterest->bottomRight(),
			matrix * regionOfInterest->topRight(),
			matrix * regionOfInterest->topLeft(),
			matrix * regionOfInterest->bottomLeft()
		};

		std::vector<Sense::Point> shadowPoints;
		for (auto point : roiPoints)
		{
			shadowPoints.emplace_back(point + lineWidth);
		}

		const std::vector<Sense::Polyline::Band> roiBand = { Sense::Polyline::Band(lineWidth, Sense::ExperienceIdentity::Palette::Blue050) };
		const std::vector<Sense::Polyline::Band> shadowBand = { Sense::Polyline::Band(lineWidth, Sense::ExperienceIdentity::Palette::Black) };

		// Draw the shadow first so that it doesn't overwrite the blue ROI indicator.
		Sense::Polyline::draw(drawing, shadowPoints.data(), static_cast<int>(shadowPoints.size()),	shadowBand.data(),	static_cast<int>(shadowBand.size()),	Sense::Polyline::Type::Segmented);
		Sense::Polyline::draw(drawing, roiPoints.data(),	static_cast<int>(roiPoints.size()),		roiBand.data(),		static_cast<int>(roiBand.size()),		Sense::Polyline::Type::Segmented);
		drawing.close();
	}

	void StbRoiViewer::drawMarkers(Sense::Drawing& drawing) const
	{
		Sense::Matrix matrix = pixelsToClient();

		std::vector<Sense::Point> markerPoints;
		markerPoints.emplace_back(markers->p1.x, markers->p1.y);
		markerPoints.emplace_back(markers->p2.x, markers->p2.y);

		drawing.open(getClientSpace());

		StbUIHelpers::drawMarkersPlus(drawing, matrix, markerPoints);

		drawing.close();
	}
}}