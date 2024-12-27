// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbAlgoMarkers.h"

#include "XrayImage.h"
#include "StentMarkers.h"
#include "StbParameters.h"

#include <IpSimplicity.h>
#include <Mkx.h>

namespace Pci { namespace Core
{

using Simplicity::OmpMR;
using Simplicity::OmpImage;
using Simplicity::Metadata;
using Simplicity::NodeMultires;
using Simplicity::Region;
using Pci::StentBoostAlgo::StentMarkers;
using CommonPlatform::Xray::XrayImage;

StbAlgoMarkers::StbAlgoMarkers(ThreadPool& pool) :
	_unique	(pool),
	_basic	(pool),	
	_multi	(pool),
	_mkx	(pool),
	_nodeLut(pool),
    _log    (::CommonPlatform::Logger(::CommonPlatform::LoggerType::StentboostLive))
{
    int major=0,minor=0,patch=0;
    MkxGetVersionNumbers(&major, &minor, &patch);
    _log.DeveloperInfo(L"StentBoost processing library version=%d.%d.%d", major, minor, patch );
}

StbAlgoMarkers::~StbAlgoMarkers()
{
}

float getFramesPerSecond(const std::shared_ptr<const XrayImage>& src);
float getMmPerPixelInIsoCenter(const std::shared_ptr<const XrayImage>& src);

bool StbAlgoMarkers::process(const StbParameters& params, const std::shared_ptr<const XrayImage>& src, const std::shared_ptr<StentMarkers>& markers,
								bool skip, bool useHistory, const Sense::Rect& roi)
{
	if (!src)		return false;
	if (!markers)	return false;
	if (skip)		return false;

	if (isnan(src->mmPerPixel.x) || isnan(src->mmPerPixel.y))
	{
		return false;
	}

	OmpMR<short>	gaussian;
	OmpMR<short>	laplacian;
	OmpImage<short> temp;

	Metadata meta = src->getMeta();
	temp.resize(src->width, src->height);

	//perform unique
	gaussian .resize(6, temp);
	laplacian.resize(6, temp);

	if (params.markersLut && params.markersUnique)
	{
		_multi .decompose(meta, *src, gaussian, laplacian);
		_unique.process  (meta, *params.markersUnique, *params.markersLut, gaussian, laplacian, laplacian);
		_multi .compose  (meta, NodeMultires::Parameters(), laplacian, temp);
	}

	Region rect;
	rect.left   = static_cast<int>(roi.left);
	rect.top    = static_cast<int>(roi.top);
	rect.right  = static_cast<int>(roi.right);
	rect.bottom = static_cast<int>(roi.bottom);
	
	float fps		 = getFramesPerSecond(src);
	float mmPerPixel = getMmPerPixelInIsoCenter(src);

	//perform marker detection
	_mkx.process(meta, *params.markerParams, rect, fps, mmPerPixel, temp, *markers, useHistory);

	return true;
}

bool StbAlgoMarkers::end()
{
	_mkx.reset();
	return true;
}

static const float FallBackFamesPerSeconds = 15.0f;

float getFramesPerSecond(const std::shared_ptr<const XrayImage>& src)
{
	if(src->frameTime > 0)
	{
		return static_cast<float>(static_cast<int>(1.0/src->frameTime));
	}

	return FallBackFamesPerSeconds;
}

static const float FallBackPixelSize = 0.11f;

float getMmPerPixelInIsoCenter(const std::shared_ptr<const XrayImage>& src)
{
	if(src->geometry.sourceImageDistance > 0)
	{
		return static_cast<float>(src->mmPerPixel.x * src->geometry.sourceObjectDistance / src->geometry.sourceImageDistance);
	}
	else
	{
		return FallBackPixelSize;
	}
}

}}