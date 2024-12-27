// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <IpSimplicity.h>
#include <IpXres3.h>
#include <NodeRunningAvg.h>
#include <memory>
#include "StbMarkers.h"
#include "StbAlgoTransform.h"


namespace CommonPlatform {
	namespace Xray
	{
		struct XrayImage;
	}
}

namespace Simplicity
{
	class ThreadPool;
	class Philips::Healthcare::Medisys::Metadata;
}

namespace Pci {	namespace StentBoostAlgo {
		struct StentMarkers;
}}

namespace Pci { namespace Core
{
	using Simplicity::NodeUnique;
	using Simplicity::NodeAtr2Ex;
	using Simplicity::Metadata;
	using StentBoostAlgo::StentMarkers;
	using Simplicity::Image;
	using Simplicity::NodeAffineTransform;
	struct StbImage;
	class StbParameters;
	class StbAlgoBoost
	{
	public:

		explicit StbAlgoBoost(Simplicity::ThreadPool& pool);
		~StbAlgoBoost();

		bool boost(const StbParameters &params, const StbAlgoTransform::Transform &trans, const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &src,
			const std::shared_ptr<StbImage> &boost, bool skip);
	
		bool end();
		void transformImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> input, const std::shared_ptr<StbImage> output, const StbAlgoTransform::Transform  transform, short background);

		StbAlgoBoost(const StbAlgoBoost&) = delete;
		StbAlgoBoost& operator=(const StbAlgoBoost&) = delete;

	protected:
		void processUnique	(const Metadata& meta, const NodeUnique::Parameters&	 params, const NodeUnique::ParametersLut& lut,				const Image<short>& input, Image<short>& output);
		void processUnique	(const Metadata& meta, const NodeUnique::ParametersCBEH& params, const NodeUnique::ParametersLut& lut,				const Image<short>& input, Image<short>& output);
		void processAtr		(const Metadata& meta, const NodeAtr2Ex::Parameters&	 params,													const Image<short>& input, Image<short>& output);
	private:
		int									_time;
		StentMarkers						_refMarkers;
		Simplicity::ThreadPool&				_pool;
		Simplicity::NodeUnique				_unique;
		Simplicity::NodeMultires			_multi;
		Simplicity::NodeAffineTransform		_transform;
		StentBoostAlgo::NodeRunningAvg		_avg;
		Simplicity::NodeBasic				_basic;
		Simplicity::NodeAtr2Ex				_atr;
		Simplicity::NodeLut					_nodeLut;
		Simplicity::OmpImage<short>			_lastFrame;
		Simplicity::NodeUnique::Parameters	_localBoostUqParams;

		bool hasReferenceMarkes();
		bool isReferenceMarkersEqual(const StentMarkers &refmarkers);
		void copyMetaData(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> & src, const std::shared_ptr<StbImage> dst);
	};
}}
