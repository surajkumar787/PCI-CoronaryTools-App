// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbAlgoBoost.h"

#include "XrayImage.h"
#include "StbImage.h"
#include "StbParameters.h"
#include "StbAlgoTransform.h"
#include <fstream>

using namespace Simplicity;

namespace Pci { namespace Core
{
	using Simplicity::OmpMR;
	using Simplicity::OmpImage;
	using Simplicity::NodeMultires;
	using Simplicity::Region;
	using StentBoostAlgo::NodeMarkerExtraction;
	using CommonPlatform::Xray::XrayImage;
	using Sense::Rect;


	StbAlgoBoost::StbAlgoBoost(Simplicity::ThreadPool &pool) :
		_pool				(pool),
		_unique				(pool),
		_multi				(pool),
		_transform			(pool),
		_avg				(pool),	
		_basic				(pool),
		_atr				(pool),
		_nodeLut			(pool),
		_lastFrame			(),
		_localBoostUqParams	()
	{
	}

	StbAlgoBoost::~StbAlgoBoost()
	{
	}

	bool StbAlgoBoost::boost(const StbParameters &params, const StbAlgoTransform::Transform &trans, const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &src,
		const std::shared_ptr<StbImage> &boost, bool skip)
	{
		if (!src)		return false;
		if (!boost)		return false;
		if (skip)		return false;			
		
		if (isnan(src->mmPerPixel.x) || isnan(src->mmPerPixel.y)||
			(!(_refMarkers.valid || trans.refMarkers.valid)))
		{
			boost->showWarning = trans.showWarning;
			return false;
		}	

		OmpImage<short> temp;
		temp.resize(*src);

		if(_lastFrame.height != src->height || _lastFrame.width != src->width)
		{
			_lastFrame.resize(*src);
		}

		if (( !hasReferenceMarkes() || !isReferenceMarkersEqual(trans.refMarkers))&&
			  trans.refMarkers.valid)
		{
			_avg.reset(std::max(1, params.boostIntegrationCount));
			_atr.reset();
			_localBoostUqParams = *params.boostUnique;
			_time = 0;
			_refMarkers = trans.refMarkers;		 
		}

		Metadata meta = src->getMeta();
		const int margin = params.boostShutterMargin;
		meta.shutters = Region(meta.shutters.left + margin, meta.shutters.top + margin, meta.shutters.right - margin, meta.shutters.bottom - margin);
		meta.validrect = meta.shutters;
		boost->resize(*src);
		boost->validRect = Rect(0, 0, boost->width, boost->height);
		boost->shutters = Rect(0, 0, boost->width, boost->height);
		copyMetaData(src, boost);
		boost->showWarning = trans.showWarning;
		boost->refMarkers = _refMarkers;

		if (!trans.hasMarkers)
		{
			_basic.copy(_lastFrame,*boost );
			return true;
		}
		
		short background = static_cast<short>(abs(params.boostBackground));

		if (params.boostLut && params.boostUnique)
		{
			if (params.boostEdgeRampupTime > 0)
			{
				float boostTimeEdgeDelta = static_cast<float>(_time)* ((params.boostUnique->band[0].mulFactor - params.boostStartEdgeValue) / static_cast<float>(params.boostEdgeRampupTime));
				_localBoostUqParams.band[0].mulFactor = std::min(params.boostUnique->band[0].mulFactor, params.boostStartEdgeValue + boostTimeEdgeDelta);
			}
			processUnique(meta, _localBoostUqParams, *params.boostLut, *src, temp);
		}

		_transform.process(meta, NodeAffineTransform::Interpol::Bilinear, trans.trans, temp, *boost, background);

		if ((params.boostAtr != nullptr) && params.boostUseAtr)
		{
			processAtr(meta, *params.boostAtr, *boost, temp);
		}
		else if (params.boostIntegrationCount > 0)
		{
			_avg.process(*boost, temp);
		}
		else
		{
			_basic.copy(*boost, temp);
		}

		processUnique(meta, *params.liveUniqueStentboost, *params.liveLutStentboost, temp, *boost);

		_nodeLut.process(*params.perceptionLut, *boost, *boost);

		_basic.copy(*boost, _lastFrame);
		_time++;

		return true;
	}


	bool  StbAlgoBoost::end()
	{
		_avg.reset(1);
		_atr.reset();
		_refMarkers.valid = false;
		_lastFrame.resize(0, 0);
		return true;
	}

	static const int NumberOfUniqueParms = 6;

	void StbAlgoBoost::processUnique(const Metadata& meta, const NodeUnique::Parameters& params, const NodeUnique::ParametersLut& lut, const Simplicity::Image<short>& input, Simplicity::Image<short>& output)
	{
		OmpMR<short>	gaussian;
		OmpMR<short>	laplacian;
		gaussian.resize(NumberOfUniqueParms, input);
		laplacian.resize(NumberOfUniqueParms, input);
		_multi.decompose(meta, input, gaussian, laplacian);
		_unique.process(meta, params, lut, gaussian, laplacian, laplacian);
		_multi.compose(meta, NodeMultires::Parameters(), laplacian, output);
	}

	void StbAlgoBoost::processUnique(const Metadata& meta, const NodeUnique::ParametersCBEH& params, const NodeUnique::ParametersLut& lut, const Simplicity::Image<short>& input, Simplicity::Image<short>& output)
	{
		OmpMR<short>	gaussian;
		OmpMR<short>	laplacian;
		gaussian.resize(NumberOfUniqueParms, input);
		laplacian.resize(NumberOfUniqueParms, input);
		_multi.decompose(meta, input, gaussian, laplacian);
		_unique.process(meta, params, lut, gaussian, laplacian, laplacian);
		_multi.compose(meta, NodeMultires::Parameters(), laplacian, output);
	}

	static const int NumberOfAtrBands = 3;

	void StbAlgoBoost::processAtr(const Metadata& meta, const NodeAtr2Ex::Parameters& params, const Simplicity::Image<short>& input, Simplicity::Image<short>& output)
	{
		OmpImage<short> temp;
		temp.resize(input);
		OmpMR<short>	gaussian;
		OmpMR<short>	laplacian;
		gaussian.resize(NumberOfAtrBands, input);
		laplacian.resize(NumberOfAtrBands, input);
		_multi.decompose(meta, input, gaussian, laplacian);
		_atr.process(meta, params, gaussian, laplacian, laplacian);
		_multi.compose(meta, NodeMultires::Parameters(), laplacian, output);
	}
	
	void StbAlgoBoost::copyMetaData(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> & src, const std::shared_ptr<StbImage> dst)
	{
		dst->seriesNumber = src->seriesNumber;
		dst->runIndex = src->runIndex;
		dst->imageIndex = src->imageIndex;
		dst->type = src->type;
		dst->frustumToPixels = src->frustumToPixels;
		dst->mmPerPixel = src->mmPerPixel;
		dst->frameTime = src->frameTime;
		dst->geometry = src->geometry;
		dst->epx = src->epx;
	}

	void StbAlgoBoost::transformImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> input , const std::shared_ptr<StbImage> output, const StbAlgoTransform::Transform transform, short background)
	{
		if (transform.refMarkers.valid && transform.hasMarkers)
		{
			auto meta = input->getMeta();
			output->resize(*input);

			_transform.process(meta, NodeAffineTransform::Interpol::Bilinear, transform.trans, *input, *output, background);
			copyMetaData(input, output);

			output->refMarkers = transform.refMarkers;
		}
	}

	bool StbAlgoBoost::hasReferenceMarkes()
	{
		return _refMarkers.valid;
	}

	bool StbAlgoBoost::isReferenceMarkersEqual(const StentMarkers &refmarkers)
	{
		return (_refMarkers.valid &&
			_refMarkers.p1.x == refmarkers.p1.x &&
			_refMarkers.p1.y == refmarkers.p1.y &&
			_refMarkers.p2.x == refmarkers.p2.x &&
			_refMarkers.p2.y == refmarkers.p2.y);
	}
}}