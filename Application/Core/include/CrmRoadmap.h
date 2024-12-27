// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <IpSimplicity.h>
#include <Sense.h>

#include "XrayGeometry.h"
#include "XrayImage.h"
#include "CrmOverlay.h"
#include "CrmErrors.h"

namespace Pci { namespace Core {

using namespace Sense;
using namespace CommonPlatform::Xray;

class CrmRoadmap
{
public:
	CrmRoadmap();
	CrmRoadmap(unsigned runIndex, int seriesNumber, int width, int height, const Point &mmPerPixel, const XrayGeometry &geo, const Matrix &frustumToPixels, double quality);

	bool load(FILE *f);
	bool save(FILE *f) const;
	
	double getAngleDistance  (const XrayGeometry &geo) const;
	double getPanningDistance(const XrayGeometry &geo) const;

	int									width;
	int									height;
	int									runIndex; // todo what is definition of this variable?
	int									seriesNumber;

	bool								selected;
	bool								locked;
	
	Point								mmPerPixel;
	Matrix								frustumToPixels;
	XrayGeometry						geometry;
	std::shared_ptr<const CrmOverlay>	overlay;
	std::vector<char>					data;
    double								quality;
	PciSuite::CrmErrors::Error			warning;
};

}}