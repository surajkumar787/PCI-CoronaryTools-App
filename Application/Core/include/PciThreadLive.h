// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <deque>
#include "Thread.h"
#include "StbAlgoMarkers.h"
#include "StbAlgoBoost.h"
#include "StbAlgoLive.h"
#include "CrmAlgoFluoro.h"
#include "StbAlgoContrast.h"
#include "StbParameters.h"
#include "PciEpx.h"

namespace Pci{ namespace Core{

class PciThreadLive
{
public:
	PciThreadLive(IScheduler &scheduler, const CrmParameters &crmPparams, const StbParameters &stbParamsLive, CommonPlatform::Xray::SystemType systemType, double latencyCompensationInSec);
	~PciThreadLive();
		
	void init();
	void processRoadmap   (const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image, bool skip);
	void processStentBoost(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image, bool skip);
	void endLive();
	
	void setRoadmap(const std::shared_ptr<const CrmRoadmap> &roadmap);
	void setEpxSubType(const PciEpx::SubType subType);	

	std::function<void(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &, const std::shared_ptr<const CrmOverlay> &)>											onRoadmap;
	std::function<void(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &, const std::shared_ptr<const StbMarkers> &, const std::shared_ptr<const StbImage> &)> onStentBoost;
	std::function<void()>    onEnd;

	PciThreadLive(const PciThreadLive&) = delete;
	PciThreadLive& operator=(const PciThreadLive&) = delete;

	const float MaxZoomFactor = 2.0f;
protected:
	
	Simplicity::ThreadPool pool;
	
	const CrmParameters &crmParams;
	const StbParameters &stbParamsLive;

	CrmAlgoFluoro		algoRoadmap;
	StbAlgoMarkers		algoMarkers;
	StbAlgoTransform	algoTransform;
	StbAlgoBoost		algoBoost;
	StbAlgoLive         algoLive;

	volatile long queueRoadmaps;
	volatile long queueImages;
	
	CommonPlatform::Thread	thread;

	const double		latencyCompensation;

private:
	PciEpx::SubType		m_stbSubType;
	bool 				m_runProcessing;

};

}}