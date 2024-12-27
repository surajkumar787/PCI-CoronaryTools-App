// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <IpXres3.h>
#include <IpSimplicity.h>
#include <memory>

namespace CommonPlatform {
	namespace Xray
	{
		struct XrayImage;
		enum class SystemType;
	}
}

namespace Pci { namespace Core
{
	class StbParameters;
	struct StbMarkers;
	struct StbImage;


	// This class implements Allura-like image processing.
	class StbAlgoLive
	{	
	public:
		StbAlgoLive(Simplicity::ThreadPool& pool, CommonPlatform::Xray::SystemType systemType);
		~StbAlgoLive();
	
		bool process(const StbParameters& params, const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& src, const std::shared_ptr<CommonPlatform::Xray::XrayImage>& live, bool skip);
		bool end();

		StbAlgoLive(const StbAlgoLive&) = delete;
		StbAlgoLive& operator=(const StbAlgoLive&) = delete;
	protected:
		Simplicity::ThreadPool&					_pool;
		Simplicity::NodeUnique					_unique;
		Simplicity::NodeMultires				_multi;
		Simplicity::NodeLut						_nodeLut;
		Simplicity::NodeBasic					_basic;
		int										_width;
		int										_height;
		const CommonPlatform::Xray::SystemType	_systemType;
	};

}}