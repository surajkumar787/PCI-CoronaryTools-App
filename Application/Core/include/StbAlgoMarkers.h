// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <IpXres3.h>
#include <NodeMarkerExtraction.h>
#include <memory>
#include <Sense/Common/Rect.h>
#include <Log.h>

namespace CommonPlatform {
	namespace Xray
	{
		struct XrayImage;
	}
}

namespace Simplicity
{
	class ThreadPool;
}

namespace Pci {	namespace StentBoostAlgo {	struct StentMarkers; } }

namespace Pci { namespace Core
{
	class StbParameters;

	class StbAlgoMarkers
	{
	public:
		explicit StbAlgoMarkers(Simplicity::ThreadPool& pool);
		virtual ~StbAlgoMarkers();

		bool process(const StbParameters& params, const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& src, const std::shared_ptr<Pci::StentBoostAlgo::StentMarkers>& markers,
					 bool skip, bool useHistory, const Sense::Rect& roi = Sense::Rect());
		bool end();

		StbAlgoMarkers(const StbAlgoMarkers&) = delete;
		StbAlgoMarkers& operator=(const StbAlgoMarkers&) = delete;

	protected:
		Simplicity::NodeUnique			 _unique;
		Simplicity::NodeBasic			 _basic;
		Simplicity::NodeMultires		 _multi;
		StentBoostAlgo::NodeMarkerExtraction _mkx;
		Simplicity::NodeLut				 _nodeLut;

	private:
        CommonPlatform::Log&            _log;
	};


}}