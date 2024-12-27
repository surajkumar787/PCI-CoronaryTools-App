// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <thread>

#include "StbAlgoMarkers.h"
#include "StbAlgoBoost.h"
#include "StbAlgoLive.h"
#include "StbAlgoContrast.h"
#include "StbAlgoTransform.h"
#include "StbParameters.h"
#include "PciEpx.h"
#include "Sense.h"
#include "Thread.h"
#include "ConcurrentQueue.h"

namespace Pci {	namespace Core {
struct StbImage;
enum class StbImagePhase;

class PciThreadStentBoost
{
public:
	PciThreadStentBoost( Sense::IScheduler &scheduler, StbParameters &stbParamsDeployment, CommonPlatform::Xray::SystemType systemType);
	~PciThreadStentBoost();

	void init();
	void processStentBoost(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image);
	void setEpxSubType(const PciEpx::SubType subType);

	void endLive();

	std::function<void(const std::vector<std::tuple<int, float>>)> onContrastScoreAvailable;
	std::function<void()>    onEnd;
	std::function<void(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &, const std::shared_ptr<const StbMarkers> &)> onLiveStent;
	std::function<void(const std::shared_ptr<const StbImage>, const std::shared_ptr<const StbImage>)> onStentBoost;

	const float MaxZoomFactor = 4.0f;
private:
	void markerWorkerthread();
	void boostWorkerthread();
	void correctForContrastDetect(std::shared_ptr<StbImage> boost);
	void newRun();

	struct StbProccesingImage
	{
		StbProccesingImage();
		
		std::shared_ptr<const CommonPlatform::Xray::XrayImage>  raw;
		std::shared_ptr<const CommonPlatform::Xray::XrayImage>  live;
		std::shared_ptr<StbMarkers>						     	markers;
		StbAlgoTransform::Transform								transform;
		StbImagePhase											phase;
		bool													endOfRun;
		bool													newRunStarted;
	};

	Simplicity::ThreadPool   m_pool;
	StbParameters            &m_stbParamsDeployment;
	StbAlgoTransform		 m_algoTransform;
	StbAlgoBoost		     m_algoBoost;
	StbAlgoContrast          m_algoContrast;
	CommonPlatform::Thread	 thread;

	CommonPlatform::Xray::SystemType m_systemType;
	CommonPlatform::Utilities::ConcurrentQueue<StbProccesingImage> m_markerDetectQueue;
	CommonPlatform::Utilities::ConcurrentQueue<StbProccesingImage> m_boostQueue;
	std::vector<std::shared_ptr<StbImage>>						   m_admin;

	PciEpx::SubType		m_stbSubType;
	bool				m_contrastDetected;
	bool				m_stopping;
	std::thread			m_markerThread;
	std::thread			m_boostThread;
	int					m_seriesNumber;
};

}}
