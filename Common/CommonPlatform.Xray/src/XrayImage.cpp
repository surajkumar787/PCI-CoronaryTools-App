// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "XrayImage.h"
#include "Timing.h"

namespace CommonPlatform {	namespace Xray {

static const double DefaultMmPerPixel = 0.184;
static const double NumberOfFramesPerSecond = 15.0;

XrayImage::XrayImage()
:
	runIndex		(0),
	imageIndex		(0),
    imageNumber     (0),
    seriesNumber    (0),
	bitdepth		(0),
	type			(Type::None),
	validRect		(),
	shutters		(),
	frustumToPixels	(Matrix::identity()),
	mmPerPixel		(DefaultMmPerPixel),
	frameTime		(1.0/ NumberOfFramesPerSecond),
	timestamp		(::CommonPlatform::Timing::getTimeStamp())
{
}

XrayImage::XrayImage(const XrayImage &rhs)
{
	copyFrom(rhs);

	runIndex = rhs.runIndex;
	imageIndex = rhs.imageIndex;
	imageNumber = rhs.imageNumber; 
	seriesNumber = rhs.seriesNumber;
	bitdepth = rhs.bitdepth;

	type = rhs.type;
	validRect = rhs.validRect;
	shutters = rhs.shutters;
	frustumToPixels = rhs.frustumToPixels;
	mmPerPixel = rhs.mmPerPixel;
	frameTime = rhs.frameTime;
	geometry = rhs.geometry;
	epx = rhs.epx;

	timestamp = rhs.timestamp;
}

void XrayImage::resize(int newWidth, int newHeight)
{
	OmpImage<short>::resize(newWidth, newHeight);

	validRect = Rect(0, 0, newWidth, newHeight);
	shutters = Rect(0, 0, newWidth, newHeight);
}

void XrayImage::resize(const XrayImage& image)
{
	OmpImage<short>::resize(image);

	validRect = Rect(0, 0, width, height);
	shutters = Rect(0, 0, width, height);
}

bool XrayImage::operator==(const XrayImage &rhs) const
{
	bool equal(true);
	
	equal = equal && (crc32() == rhs.crc32());

	equal = equal && (runIndex == rhs.runIndex);
	equal = equal && (imageIndex == rhs.imageIndex);
	equal = equal && (imageNumber == rhs.imageNumber);
	equal = equal && (seriesNumber == rhs.seriesNumber);
	equal = equal && (bitdepth == rhs.bitdepth);

	equal = equal && (type == rhs.type);
	equal = equal && (validRect == rhs.validRect);
	equal = equal && (shutters == rhs.shutters);
	equal = equal && (frustumToPixels == rhs.frustumToPixels);
	equal = equal && (mmPerPixel == rhs.mmPerPixel);
	equal = equal && (frameTime == rhs.frameTime);
	equal = equal && (!geometry.changed(rhs.geometry));
	equal = equal && (!epx.changed(rhs.epx));

	equal = equal && (timestamp == rhs.timestamp);

	return equal;
}

bool XrayImage::isFromExposureRun() const
{
	return (type == Type::Exposure) || (type == Type::ExposureReplay);
}

bool XrayImage::isFromRecording() const
{
	return (type == Type::ExposureReplay);
}

bool XrayImage::isValid() const
{
	return (width > 0 && height > 0);
}

Simplicity::Metadata XrayImage::getMeta() const
{
	Simplicity::Metadata meta;
	
	meta.shutters.left	    = static_cast<int>(shutters.left    + 0.5);
	meta.shutters.top	    = static_cast<int>(shutters.top	    + 0.5);
	meta.shutters.right     = static_cast<int>(shutters.right   + 0.5);
	meta.shutters.bottom    = static_cast<int>(shutters.bottom  + 0.5);
												   
	meta.validrect.left		= static_cast<int>(validRect.left   + 0.5);
	meta.validrect.top		= static_cast<int>(validRect.top	+ 0.5);
	meta.validrect.right	= static_cast<int>(validRect.right  + 0.5);
	meta.validrect.bottom	= static_cast<int>(validRect.bottom + 0.5);
	
	return meta;
}

struct XrayImageHeader
{
	static const int Version = 2;

	int				    version;
	int				    width;
	int				    height;
	int					bitdepth;
	int				    runIndex;
	int				    imageIndex;
	XrayImage::Type	    type;
	Rect			    validRect;
	Rect			    shutters;
	Matrix			    frustumToPixels;
	Point			    mmPerPixel;
	double			    frameTime;
	XrayGeometry	    geometry;
	XrayEpx				epx;
};

bool XrayImage::load(FILE *f)
{
	XrayImageHeader header;

	if (fread(&header, sizeof(header), 1, f) != 1)  return false;
	if (header.version != XrayImageHeader::Version) return false;
	if (header.width  < 0 || header.width  > 4096)  return false;
	if (header.height < 0 || header.height > 4096)  return false;
	
	bitdepth		= header.bitdepth;
	runIndex		= header.runIndex;
	imageIndex		= header.imageIndex;
	type			= header.type;
	validRect		= header.validRect;
	shutters		= header.shutters;
	frustumToPixels	= header.frustumToPixels;
	mmPerPixel		= header.mmPerPixel;
	frameTime		= header.frameTime;
	geometry		= header.geometry;
	epx				= header.epx;
	
	resize(header.width, header.height);
	for (int y = 0; y < height; y++)
	{
		if (fread(pointer + y * stride, width * sizeof(short), 1, f) != 1)
		{
			return false;
		}
	}
	
	return true;
}
	
bool XrayImage::save(FILE *f) const
{
	XrayImageHeader header;

	header.version			= XrayImageHeader::Version;
	header.width			= width;
	header.height			= height;
	header.bitdepth			= bitdepth;
	header.runIndex			= runIndex;
	header.imageIndex		= imageIndex;
	header.type				= type;
	header.validRect		= validRect;
	header.shutters			= shutters;
	header.frustumToPixels	= frustumToPixels;
	header.mmPerPixel		= mmPerPixel;
	header.frameTime		= frameTime;
	header.geometry			= geometry;
	header.epx				= epx;

	if (fwrite(&header, sizeof(header), 1, f) != 1)
	{
		return false;
	}
	
	for (int y = 0; y < height; y++)
	{
		if (fwrite(pointer + y * stride, width * sizeof(short), 1, f) != 1)
		{
			return false;
		}
	}
	return true;
}
}}