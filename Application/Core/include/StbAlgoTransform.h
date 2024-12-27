// Copyright Koninklijke Philips N.V. 2020
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


namespace CommonPlatform {
	namespace Xray
	{
		struct XrayImage;
	}
}

namespace Pci {
	namespace StentBoostAlgo {
		struct StentMarkers;
	}
}

namespace Pci {	namespace Core{

using StentBoostAlgo::StentMarkers;
using Simplicity::Image;
using Simplicity::NodeAffineTransform;
struct StbImage;
class StbParameters;

class StbAlgoTransform
{
	enum class State
	{
		Idle,
		NoLock,
		Locked,
		Unlocked,
		LostLock
	};
public:

	explicit StbAlgoTransform(float maxZoomFactor);
	~StbAlgoTransform();

	struct Transform
	{
		NodeAffineTransform::Matrix		trans;
		StbMarkers						refMarkers;
		bool							hasMarkers;
		bool							showWarning;
	};

	bool process(const StbParameters &params, const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image,const std::shared_ptr<StbMarkers>	&markers, Transform &trans, bool skip);
	bool end();

protected:
	void setRefMarkers(const StentMarkers& markers);
	bool processLock(const StbParameters&		params, const StbMarkers &markers, Transform &trans);
	bool processNoLock(const StbParameters&		params, Transform &trans);
	bool processLostLock(const StbParameters&	params, Transform &trans);

private:
	StbMarkers							_refMarkers;
	int									_width;
	int									_height;
	State								_state;
	int									_lostMarkerCount;
	float								_maxZoomFactor;
};

}}

