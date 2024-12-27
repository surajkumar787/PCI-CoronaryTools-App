// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbAlgoLive.h"

#include "XrayImage.h"
#include "StbParameters.h"
#include "SystemType.h"

using namespace Simplicity;

namespace Pci { namespace Core
{
	using Simplicity::OmpMR;
	using Simplicity::Metadata;
	using Simplicity::NodeMultires;
	using CommonPlatform::Xray::XrayImage;

	StbAlgoLive::StbAlgoLive(ThreadPool& pool, const CommonPlatform::Xray::SystemType systemType) :
		_pool			(pool),
		_unique			(pool),
		_multi			(pool),
		_nodeLut		(pool),
		_basic			(pool),
		_width			(0),
		_height			(0),
		_systemType		(systemType)
	{
	}

	StbAlgoLive::~StbAlgoLive()
	{
	}

	bool StbAlgoLive::process(const StbParameters& params, const std::shared_ptr<const XrayImage>& src, const std::shared_ptr<XrayImage>& live, bool skip)
	{
		if (!src)	return false;
		if (!live)	return false;
		if (skip)	return false;

		if (_isnan(src->mmPerPixel.x) || _isnan(src->mmPerPixel.y))
		{
			return false;
		}

		Metadata meta = src->getMeta();
		OmpMR<short> gaussian;
		OmpMR<short> laplacian;
		gaussian.resize(6, *src);
		laplacian.resize(6, *src);
		live->resize(*src);
	
		if(params.liveLut && params.liveUnique)
		{
			_multi.decompose(meta, *src, gaussian, laplacian);
			_unique.process(meta, *params.liveUnique, *params.liveLut, gaussian, laplacian, laplacian);
			_multi.compose(meta, NodeMultires::Parameters(), laplacian, *live);
		}
		else
		{
			_basic.copy(*src, *live);
		}

		// Apply perception lut to make images output DICOM compatible for allura systems
		// For SmartSuite systems the Perception lut is already applied on processed image. 
		// These fluoro image are assumed to be processed.
		if ( _systemType == CommonPlatform::Xray::SystemType::Allura )
		{
			_nodeLut.process(*params.perceptionLut, *live, *live);
		}

		live->runIndex			= src->runIndex;
		live->imageIndex		= src->imageIndex;
		live->type				= src->type;
		live->validRect			= src->validRect;
		live->shutters			= src->shutters;
		live->frustumToPixels	= src->frustumToPixels;
		live->mmPerPixel		= src->mmPerPixel;
		live->frameTime			= src->frameTime;
		live->geometry			= src->geometry;
		live->epx				= src->epx;
        live->seriesNumber      = src->seriesNumber;
        live->imageNumber       = src->imageNumber;

		return true;
	}

	bool StbAlgoLive::end()
	{
		return true;
	}

}}
