// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbUIHelpers.h"

#include <Sense.h>
#include <ExperienceIdentity.h>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace Sense;

namespace Pci { namespace Core {

	void StbUIHelpers::drawMarkersEllipse(Sense::Drawing& drawing, const Sense::Matrix& matrix, const Sense::Point& marker1, const Sense::Point& marker2)
	{
		const int NumberOfSegments = 128;
		const double ElipticMarkerXFraction = 0.8;
		const double ElipticMarkerYFraction = 0.4;
		const double MarkerLineBandWidth = 2.0;

		Point pos[NumberOfSegments];

		auto marker1Normalized = matrix * marker1;
		auto marker2Normalized = matrix * marker2;

		Point x = marker2Normalized - marker1Normalized;
		Point y = Point(x.y, -x.x);

		Point center = (marker1Normalized + marker2Normalized) / 2.0;

		for (int i = 0; i < NumberOfSegments; i++)
		{
			double angle = static_cast<double>(i) / NumberOfSegments*2.0*M_PI;

			Point p(sin(angle) * ElipticMarkerXFraction, cos(angle) * ElipticMarkerYFraction);
			pos[i] = Point(p.dot(x), p.dot(y)) + center;
		}

		Sense::Polygon::Band b[] =
		{
			Sense::Polygon::Band(0.0, Sense::Color(0.0)),
			Sense::Polygon::Band(MarkerLineBandWidth, Sense::Color(1,1,1))
		};

		Sense::Polygon::draw(drawing, center, pos, b);
	}

	void StbUIHelpers::drawMarkersPlus(Sense::Drawing& drawing, const Sense::Matrix& matrix, const std::vector<Sense::Point>& markers)
	{
		const double LineWidth = 1.0;
		const int plusWidth = 12;
		const int plusHeight = plusWidth;

		const Sense::Polyline::Band bandMarker[] = { Sense::Polyline::Band(LineWidth, Sense::ExperienceIdentity::Palette::Blue050) };
		const Sense::Polyline::Band bandShadow[] = { Sense::Polyline::Band(LineWidth,  Sense::ExperienceIdentity::Palette::Black) };

		for (const auto& marker : markers)
		{
			auto markerNormalized = matrix * marker;

			Point lineMarkerTop[] = { Point(0, -(plusHeight)) + markerNormalized, Point(0, -3) + markerNormalized };
			Point lineMarkerTopShadow[] = { lineMarkerTop[0] + LineWidth, lineMarkerTop[1] + LineWidth };
			Point lineMarkerBottom[] = { Point(0, 3) + markerNormalized, Point(0, plusHeight) + markerNormalized };
			Point lineMarkerBottomShadow[] = { lineMarkerBottom[0] + LineWidth, lineMarkerBottom[1] + LineWidth };

			Point lineMarkerLeft[] = { Point(-(plusWidth), 0) + markerNormalized, Point(-3, 0) + markerNormalized };
			Point lineMarkerLeftShadow[] = { lineMarkerLeft[0] + LineWidth, lineMarkerLeft[1] + LineWidth };
			Point lineMarkerRight[] = { Point(3, 0) + markerNormalized, Point(plusWidth, 0) + markerNormalized };
			Point lineMarkerRightShadow[] = { lineMarkerRight[0] + LineWidth, lineMarkerRight[1] + LineWidth };

			Sense::Polyline::draw(drawing, lineMarkerTopShadow, bandShadow);
			Sense::Polyline::draw(drawing, lineMarkerBottomShadow, bandShadow);
			Sense::Polyline::draw(drawing, lineMarkerLeftShadow, bandShadow);
			Sense::Polyline::draw(drawing, lineMarkerRightShadow, bandShadow);
			
			Sense::Polyline::draw(drawing, lineMarkerTop, bandMarker);
			Sense::Polyline::draw(drawing, lineMarkerBottom, bandMarker);
			Sense::Polyline::draw(drawing, lineMarkerLeft, bandMarker);
			Sense::Polyline::draw(drawing, lineMarkerRight, bandMarker);
		}
	}
}}

