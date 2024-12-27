// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbAdministration.h"
#include "XrayImage.h"
#include "StbMarkers.h"
#include "StbImage.h"
#include "PciEpx.h"
#include "StbRecording.h"
#include "PciReviewParameters.h"

namespace Pci { namespace Core
{

StbAdministration::StbAdministration(const PciReviewParameters &reviewParams) :
	m_reviewParams(reviewParams),
	m_reviewReady(false)
{
	clear();
}

void StbAdministration::setImage(const ::std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image,
                                 const ::std::shared_ptr<const StbMarkers> &markers,
								 const ::std::shared_ptr<const StbImage>& registeredLive,
                                 const ::std::shared_ptr<const StbImage>& boost)
{
	if (m_reviewReady)
	{
		clear();
	}

	if (( !isEmpty() && (image != nullptr)) && 
		((m_stbImages->getRunIndex() != image->runIndex) ||	(m_lastImageIndex >= image->imageIndex)))
	{
		clear();
	}

	if (image != nullptr)
	{
		m_lastImageIndex = image->imageIndex;
	}

	m_stbImages->addImage(image, markers, registeredLive, boost);
}


void StbAdministration::setImage(const ::std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image,
	const ::std::shared_ptr<const StbMarkers> &markers,
	const ::std::shared_ptr<const StbImage> &boost)
{
	setImage(image, markers, std::make_shared<StbImage>(), boost);
}

void StbAdministration::setBoostImage(const ::std::shared_ptr<const StbImage>& boost, const ::std::shared_ptr<const StbImage>& registeredLive)
{
	m_stbImages->setBoostImage(boost);
	m_stbImages->setRegisteredLiveImage(registeredLive);
}

void StbAdministration::studyChanged()
{
	clear();
}

bool StbAdministration::isEmpty() const
{
	return m_stbImages->isEmpty();
}

void StbAdministration::newRun()
{
	clear();
}

int StbAdministration::lastImageIndex() const
{
	return m_lastImageIndex;
}

int StbAdministration::numberOfValidImages() const
{
	return m_stbImages->getImageCount();
}

void StbAdministration::setBoostPhaseEnd(int index)
{
	m_stbImages->setBoostPhaseEnd(index);
	int bestIndex = getBestContrastImage();
	setAutoSelectedContrastFrame(bestIndex);
}

int StbAdministration::getBoostImageIndex()
{
	return m_stbImages->getLastBoostedImage().boost->imageNumber;
}

void StbAdministration::restoreDefaultBoostedPhase()
{
	m_stbImages->restoreDefaultBoostedPhase();
}

void StbAdministration::setContrastScore(const std::vector<std::tuple<int, float>>& score)
{
	m_stbImages->setContrastScore(score);
	auto index = getBestContrastImage();
	setAutoSelectedContrastFrame(index);
}

int StbAdministration::getBestContrastImage()
{
	int index = -1;
	float bestScore = 0.0f;
	auto results = m_stbImages->getAllContrastImages();
	for (auto item : results)
	{
		if (item.contrastScore.isValid &&
		   ( bestScore < item.contrastScore.score || index == -1))
		{
			bestScore = item.contrastScore.score;
			index = item.live->imageNumber;
		}
	}

	return index;
}

void StbAdministration::setAutoSelectedContrastFrame(int index)
{
	m_stbImages->setAutoDetectedFrame(index);
}

::std::shared_ptr<StbRecording> StbAdministration::prepareReview()
{
	return prepareReview(true);
}

::std::shared_ptr<StbRecording> StbAdministration::prepareReview(bool includeFramesWithoutMarkers)
{
	m_stbImages->setIncludeImagesWithoutMarkers(includeFramesWithoutMarkers);
	if (m_stbImages->getImageCount() >= std::max(1, static_cast<int>(m_reviewParams.LoopMinNrFrames)))
	{
		m_reviewReady = true;
		return m_stbImages;
	}
	return std::make_shared<StbRecording>(m_reviewParams.LoopMinNrFrames);
}

void StbAdministration::clear()
{
	m_stbImages = std::make_shared<StbRecording>(m_reviewParams.LoopMaxNrFrames);
	m_reviewReady = false;
	m_lastImageIndex = -1;
}

}}