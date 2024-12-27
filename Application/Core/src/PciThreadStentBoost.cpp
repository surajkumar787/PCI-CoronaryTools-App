// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PciThreadStentBoost.h"
#include "StbParameters.h"
#include "StbImage.h"
#include "StbMarkers.h"
#include "XrayImage.h"
#include "PciEpx.h"
#include <windows.h>
#include <processthreadsapi.h>

namespace Pci {	namespace Core {

static const int maxNumberOfImages = 300;

PciThreadStentBoost::PciThreadStentBoost(IScheduler &scheduler, StbParameters &stbParamsDeployment, CommonPlatform::Xray::SystemType systemType):
	m_pool(false, "stentboostPost"),
	m_stbParamsDeployment(stbParamsDeployment),
	m_algoTransform(MaxZoomFactor),
	m_algoBoost(m_pool),
	m_algoContrast(),
	thread(scheduler),
	m_systemType(systemType),
	m_markerDetectQueue(),
	m_boostQueue(),
	m_admin(),
	m_stbSubType(PciEpx::SubType::Live),
	m_contrastDetected(false),
	m_stopping(false),
	m_markerThread(),
	m_boostThread(),
	m_seriesNumber(-1)
{
	m_markerThread = std::thread(&PciThreadStentBoost::markerWorkerthread, this);
	m_boostThread = std::thread(&PciThreadStentBoost::boostWorkerthread, this);
}

PciThreadStentBoost::~PciThreadStentBoost()
{
	m_stopping = true;

	m_boostThread.join();
	m_markerThread.join();	
}

void PciThreadStentBoost::init()
{
	m_contrastDetected = false;
}

void PciThreadStentBoost::processStentBoost(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& image)
{
	if (image->seriesNumber != m_seriesNumber)
	{
		newRun();
		m_seriesNumber = image->seriesNumber;
	}

	StbProccesingImage stbProcessingImage;
	stbProcessingImage.raw = image;
	stbProcessingImage.markers = std::make_shared<StbMarkers>();

	if (m_markerDetectQueue.size() > maxNumberOfImages) m_markerDetectQueue.pop();
	m_markerDetectQueue.push(stbProcessingImage);
}

void PciThreadStentBoost::endLive()
{
	StbProccesingImage stbProcessingImage;
	stbProcessingImage.endOfRun = true;

	if (m_markerDetectQueue.size() > maxNumberOfImages) m_markerDetectQueue.pop();
	m_markerDetectQueue.push(stbProcessingImage);
}

void PciThreadStentBoost::newRun()
{
	m_markerDetectQueue.clear();
	m_boostQueue.clear();

	StbProccesingImage stbProcessingImage;
	stbProcessingImage.newRunStarted = true;

	if (m_markerDetectQueue.size() > maxNumberOfImages) m_markerDetectQueue.pop();
	m_markerDetectQueue.push(stbProcessingImage);
}

void PciThreadStentBoost::markerWorkerthread()
{
	ThreadPool      threadPool(false,"marker extraction thread-pool");
	StbAlgoLive     algoLive(threadPool,m_systemType);
	StbAlgoMarkers  algoMarkers(threadPool);

	while (!m_stopping)
	{
		StbProccesingImage imageData;
		if (m_markerDetectQueue.pop(imageData, std::chrono::milliseconds(500)))
		{
			if (!imageData.endOfRun && !imageData.newRunStarted)
			{
				//proccess
				algoMarkers.process(m_stbParamsDeployment, imageData.raw, imageData.markers, false, m_contrastDetected, m_stbParamsDeployment.regionOfInterest);

				//find contrast 
				if (m_stbSubType == PciEpx::SubType::Subtract)
				{
					m_algoContrast.process(imageData.raw, *imageData.markers, imageData.phase);
				}
				m_contrastDetected |= imageData.phase == StbImagePhase::contrastPhase;

				auto live = std::make_shared<CommonPlatform::Xray::XrayImage>();
				live->imageNumber = imageData.raw->imageNumber;
				imageData.live = live;

				algoLive.process(m_stbParamsDeployment, imageData.raw, live, false);
				m_algoTransform.process(m_stbParamsDeployment, imageData.raw, imageData.markers, imageData.transform, false);

				thread.executeUI([this, live, markers = imageData.markers]
				{
					if (onLiveStent)
					{
						onLiveStent(live, markers);
					}
				});
			}
			else
			{
				algoMarkers.end();
				algoLive.end();
				m_algoTransform.end();
			}

			if (m_boostQueue.size() > maxNumberOfImages) m_boostQueue.pop();
			m_boostQueue.push(std::move(imageData));
		}
	}
}

static const float MaxZoomFactor = 4.0f;

void PciThreadStentBoost::boostWorkerthread()
{
	while (!m_stopping)
	{
		StbProccesingImage imageData;
		if (m_boostQueue.pop(imageData, std::chrono::milliseconds(500)))
		{
			if(!imageData.endOfRun && !imageData.newRunStarted)
			{
				std::shared_ptr<StbImage> registeredLiveImage;
				if (m_stbSubType == PciEpx::SubType::Subtract && imageData.markers->valid)
				{
					registeredLiveImage = std::make_shared<StbImage>();
					m_algoBoost.transformImage(imageData.live, registeredLiveImage, imageData.transform, static_cast<short>(m_stbParamsDeployment.boostBackground));
					registeredLiveImage->phase = imageData.phase;
					registeredLiveImage->imageNumber = imageData.raw->imageNumber;
				}

				m_stbParamsDeployment.boostIntegrationCount = 0;		
				m_stbParamsDeployment.boostUseAtr = true;

				auto boost = std::make_shared<StbImage>();
				m_algoBoost.boost(m_stbParamsDeployment,imageData.transform, imageData.raw, boost, false);
				boost->phase = imageData.phase;
				boost->imageNumber = imageData.raw->imageNumber;

				correctForContrastDetect(boost);

				thread.executeUI([this, boost, registeredLiveImage]
				{
					if (onStentBoost)
					{
						onStentBoost(boost, registeredLiveImage);
					}
				});
			}
			else if (imageData.endOfRun)
			{
				if (m_stbSubType == PciEpx::SubType::Subtract)
				{
					auto contrastscore = m_algoContrast.getContrastScore();
					if (onContrastScoreAvailable)
					{
						thread.executeUI([this, contrastscore]
						{
							onContrastScoreAvailable(contrastscore);
						});
					}
				}

				m_algoContrast.end();
				m_algoBoost.end();

				thread.executeAsync([this]
				{
					thread.executeUI([this] { if (onEnd) onEnd(); });
				});
			}
			else if (imageData.newRunStarted)
			{
				m_algoContrast.end();
				m_algoBoost.end();
			}
		}
	}
}

void PciThreadStentBoost::correctForContrastDetect(std::shared_ptr<StbImage> boost)
{
	if (boost->phase == StbImagePhase::boostPhase)
	{
		m_admin.clear();
	}
	else if (boost->phase == StbImagePhase::transitionPhase)
	{
		m_admin.push_back(boost);
	}
	else  if (boost->phase == StbImagePhase::contrastPhase && m_admin.size() > 0)
	{
		while( m_admin.size() > 0)
		{
			m_admin.back()->phase = StbImagePhase::contrastPhase;
			m_admin.pop_back();
		}
	}
}

void PciThreadStentBoost::setEpxSubType(const PciEpx::SubType subType)
{
	m_stbSubType = subType;
}

PciThreadStentBoost::StbProccesingImage::StbProccesingImage()
{
	endOfRun = false; 
	newRunStarted = false;
	phase = StbImagePhase::boostPhase;
}

}}

