// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PciThreadLive.h"
#include "StbParameters.h"
#include "StbImage.h"
#include "StbMarkers.h"
#include "XrayImage.h"
#include "PciEpx.h"

namespace Pci{ namespace Core{
using namespace PciSuite;

PciThreadLive::PciThreadLive(IScheduler &scheduler, const CrmParameters &crmParams, const StbParameters &stbParamsLive, CommonPlatform::Xray::SystemType systemType, double latencyCompensationInSec) :
	pool				(false, "live"),
	crmParams			(crmParams),
	stbParamsLive		(stbParamsLive),
	algoRoadmap			(pool, crmParams, systemType),
	algoMarkers			(pool),
	algoTransform		(MaxZoomFactor),
	algoBoost			(pool),
	algoLive			(pool, systemType),
	queueRoadmaps		(0),
	queueImages			(0),
	latencyCompensation	(latencyCompensationInSec),
	thread				(scheduler),
	m_stbSubType		(PciEpx::SubType::Live),
	m_runProcessing(false)
{
}

PciThreadLive::~PciThreadLive()
{
}

void PciThreadLive::init()
{
	m_runProcessing = false;
}

void PciThreadLive::processRoadmap(const std::shared_ptr<const XrayImage> &image, bool skip)
{
	_InterlockedIncrement(&queueImages);
	m_runProcessing = true;
	thread.executeAsync([this, image, skip]
	{
		int count = _InterlockedDecrement(&queueImages);
		bool skipped = skip || (count != 0);
		
		auto overlay = std::make_shared<CrmOverlay>();
		auto live    = std::make_shared<XrayImage>();

		// The live image (output) will contain a corrected time stamp based on the time stamp of the acquired image (input)
		live->timestamp = image->timestamp - latencyCompensation;

		algoRoadmap.process(image, overlay,live, skipped);

		if (!skipped) thread.executeUI([this, live, overlay] { if (onRoadmap) onRoadmap(live, overlay);	});
	});
}

void PciThreadLive::setRoadmap(const std::shared_ptr<const CrmRoadmap> &roadmap)
{
	_InterlockedIncrement(&queueRoadmaps);

	thread.executeAsync([this, roadmap]
	{
		int count = _InterlockedDecrement(&queueRoadmaps);
		if (count == 0) algoRoadmap.set(roadmap);
	});
}

void PciThreadLive::processStentBoost(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& image, bool skip )
{
	_InterlockedIncrement(&queueImages);
	m_runProcessing = true;
	thread.executeAsync([this, image, skip]
	{
		int count = _InterlockedDecrement(&queueImages);
		bool skipped = skip || (count != 0);


		StbAlgoTransform::Transform transform;
		auto markers = std::make_shared<StbMarkers>();
		auto boost = std::make_shared<StbImage>();
		auto live = std::make_shared<XrayImage>();
		boost->imageNumber = live->imageNumber =  image->imageNumber;
	
		algoMarkers.process(stbParamsLive, image, markers, skipped, false, stbParamsLive.regionOfInterest);
		algoLive.process(stbParamsLive, image, live, skipped);
		algoTransform.process(stbParamsLive, image, markers, transform, skipped);
		algoBoost.boost(stbParamsLive,transform, image, boost, skipped);

		if (!skipped)
		{
			thread.executeUI([this, live, markers, boost]
			{
				if (onStentBoost)
				{

					onStentBoost(live, markers, boost);
				}
			});
		}
	});
}

void PciThreadLive::endLive()
{
	bool runProcessing = m_runProcessing;
	thread.executeAsync([this, runProcessing]()
	{
		algoRoadmap.end();
		algoMarkers.end();
		algoTransform.end();
		algoBoost.end();

		if (runProcessing)
		{
			thread.executeUI([this] { if (onEnd) onEnd(); });
		}
	});

	m_runProcessing = false;
}

void PciThreadLive::setEpxSubType(const PciEpx::SubType subType)
{
	m_stbSubType = subType;
}

}}