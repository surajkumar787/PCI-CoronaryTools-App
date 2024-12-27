// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <IpSimplicity.h>
#include <Sense.h>

#include "XrayGeometry.h"
#include "XrayEpx.h"
#include "XrayStudy.h"

namespace CommonPlatform {	namespace Xray {

using namespace Sense;

struct XrayImage : public Simplicity::OmpImage<short>
{
	enum class Type	{ None, Fluoro, Exposure, SingleShot, ExposureReplay };

	XrayImage();
	XrayImage(const XrayImage &);

	void resize(int width, int height);
	void resize(const XrayImage&);

	bool operator ==(const XrayImage &) const;

	bool isFromExposureRun() const;
	bool isFromRecording() const;

	bool isValid() const;

	bool load(FILE *f);
	bool save(FILE *f) const;

	Simplicity::Metadata getMeta() const;

	int				   runIndex;
	int				   imageIndex;
	int                imageNumber; // As on Allura
	int                seriesNumber; // As on Allura
	int				   bitdepth;

	Type			   type;
	Rect			   validRect;
	Rect			   shutters;
	Matrix			   frustumToPixels;
	Point			   mmPerPixel;
	double			   frameTime;
	XrayGeometry	   geometry;
    XrayEpx			   epx;

	double			   timestamp;
};

}}