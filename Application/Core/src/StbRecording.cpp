// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <limits>
#include "StbRecording.h"
#include "StbImage.h"
#include "StbMarkers.h"
#include <algorithm> 

namespace Pci { namespace Core
{

StbRecording::StbRecording(size_t loopMaxNrFrames) :
	m_images(),
	m_maxFrames(loopMaxNrFrames),
	m_includeAllImagesWithoutMarkers(false),
	m_currentContrastFrameIndex(-1),
	m_autoSelectedContrastFrameIndex(-1),
	m_autoSelectedLastBoostFrameIndex(-1),
    m_isAutoArchived(false)
{
}

StbRecording::~StbRecording()
{
	m_images.clear();
}

const std::deque<StbResult> StbRecording::getAllImages() const
{
	std::deque<StbResult> allImages;
	auto firstBoostFrame = std::find_if(m_images.begin(), m_images.end(), [this](const StbResult& stbResult) { return m_includeAllImagesWithoutMarkers || ((stbResult.markers != nullptr) && (stbResult.markers->valid)); });
	if (firstBoostFrame != m_images.end())
	{
		allImages = std::deque<StbResult>(firstBoostFrame, m_images.end());
	}

	return allImages;
}

const std::deque<StbResult> StbRecording::getAllContrastImages() const
{
    std::deque<StbResult> allImages;
    auto firstContrastFrame = std::find_if(m_images.begin(), m_images.end(), [this](const StbResult& stbResult) { return stbResult.boost && stbResult.boost->phase == StbImagePhase::contrastPhase; });
    if (firstContrastFrame != m_images.end())
    {
        allImages = std::deque<StbResult>(firstContrastFrame, m_images.end());
    }

    return allImages;
}

    const std::deque<StbResult> StbRecording::getAllReferenceImages() const
{
        std::deque<StbResult> allRegisteredImages;
        for (auto image: m_images)
    {
            if (image.registeredLive)
            {
                allRegisteredImages.push_back(image);
            }
    }

        return allRegisteredImages;
}

int StbRecording::getSeriesNumber() const
{
	if (!m_images.empty() && m_images.back().live)
	{
		return m_images.back().live->seriesNumber;
	}
	return -1;
}

int StbRecording::getImageCount() const
{
	return static_cast<int>(getAllImages().size());
}

int StbRecording::getRunIndex() const
{
	if (!m_images.empty() && m_images.back().live)
	{
		return m_images.back().live->runIndex;
	}

	return -1;
}

int StbRecording::getBoostedImageCount() const
{
	auto count = std::count_if(m_images.begin(), m_images.end(), [](const StbResult& result) { return  (result.boost && result.boost->isValid() && result.boost->phase == StbImagePhase::boostPhase) && (result.markers->valid); });
	return static_cast<int>(count);
}

bool StbRecording::isEmpty() const
{
	return m_images.empty();
}

const StbResult StbRecording::getContrastImage() const
{
	StbResult contrastImage{ nullptr, nullptr, nullptr };
    int index = m_currentContrastFrameIndex;
    auto contrastFrame = std::find_if(m_images.rbegin(), m_images.rend(), [index](const StbResult& result) { return  (result.registeredLive && result.registeredLive->isValid() && result.registeredLive->imageNumber == index); });
    if (contrastFrame != m_images.rend())
    {
        contrastImage = *contrastFrame;
	}
	else
	{
		contrastFrame = std::find_if(m_images.rbegin(), m_images.rend(), [](const StbResult& result) { return  (result.registeredLive && result.registeredLive->isValid() && result.boost &&result.boost->phase == StbImagePhase::contrastPhase) && (result.markers->valid); });
		if (contrastFrame != m_images.rend())
		{
			contrastImage = *contrastFrame;
		}
	}
	return contrastImage;
}

void StbRecording::setContrastScore(const std::vector<std::tuple<int, float>>& score)
{
	for (auto item : score)
	{
		int imageNumber = std::get<0>(item);
		auto result = std::find_if(m_images.rbegin(), m_images.rend(), [imageNumber](const StbResult& result) { return  (result.live->imageNumber == imageNumber); });
		if (result != m_images.rend())
		{
			result->contrastScore.isValid = true;
			result->contrastScore.score = std::get<1>(item);
		}

	}
}


void StbRecording::setAutoDetectedFrame(int index)
{
	m_autoSelectedContrastFrameIndex = index;
	m_currentContrastFrameIndex = index;
}

void StbRecording::setManualContrastFrame(int index)
{
	m_currentContrastFrameIndex = index;
}

int StbRecording::getAutoDetectedContrastFrameIndex() const
{
	return m_autoSelectedContrastFrameIndex;
}

int  StbRecording::getFirstLastIIndexWithMarkers()
{
	auto firstLastvalidIndex = 0;
	bool fountLastValid = false;
	for (int i = static_cast<int>(m_images.size()) - 1; i > 0; i--)
	{
		if (m_images[i].markers->valid)
		{
			firstLastvalidIndex = m_images[i].live->imageNumber;
			if (fountLastValid)
			{
				break;
			}
			fountLastValid = true;
		}
	}
	return firstLastvalidIndex;
}

void StbRecording::setBoostPhaseEnd(int index)
{
	if (m_autoSelectedLastBoostFrameIndex == -1)
	{
		m_autoSelectedLastBoostFrameIndex = getLastBoostedImage().boost->imageNumber;
	}

	auto firstLastvalidIndex = getFirstLastIIndexWithMarkers();

	index = std::min(index, firstLastvalidIndex);

	for (auto& frame : m_images)
	{
		if (frame.live->imageNumber <= index)
		{
			const_cast<StbImage&>(*frame.boost).phase = StbImagePhase::boostPhase;
			const_cast<StbImage&>(*frame.registeredLive).phase = StbImagePhase::boostPhase;
		}
		else
		{
			const_cast<StbImage&>(*frame.boost).phase = StbImagePhase::contrastPhase;
			const_cast<StbImage&>(*frame.registeredLive).phase = StbImagePhase::contrastPhase;
		}
	}
}

const StbResult StbRecording::getLastBoostedImage() const
{
	StbResult lastBoostedImage{ nullptr, nullptr, nullptr };
	if (!m_images.empty())
	{
		auto boostedFrame = std::find_if(m_images.rbegin(), m_images.rend(), [](const StbResult& result) { return (result.boost && result.boost->isValid() && result.boost->phase == StbImagePhase::boostPhase && result.markers->valid); });
		if (boostedFrame != m_images.rend())
		{
			lastBoostedImage = *boostedFrame;
		}
	}

	if (lastBoostedImage.boost == nullptr && !m_images.empty())
	{
		auto boostedImage = std::make_shared<StbImage>();
		boostedImage->runIndex = getRunIndex();
		boostedImage->seriesNumber = getSeriesNumber();

		lastBoostedImage.boost = boostedImage;
	}

	return lastBoostedImage;
}

void StbRecording::addImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image,
	const std::shared_ptr<const StbMarkers> &markers,
	const std::shared_ptr<const StbImage> &reference,
	const std::shared_ptr<const StbImage>& boost)
{
	if (image)
	{
		if (m_images.size() >= m_maxFrames)
		{
			auto data = m_images.front();
			m_images.pop_front();
		}

		m_images.push_back(StbResult{ image,  markers, reference, boost });
	}
}

void StbRecording::setBoostImage(const std::shared_ptr<const StbImage>& boost)
{
	for (auto& image : m_images)
	{
		if (image.live->imageNumber == boost->imageNumber)
		{
			image.boost = boost;
			break;
		}
	}
}

void StbRecording::setRegisteredLiveImage(const std::shared_ptr<const StbImage>& registeredLive)
{
	if (registeredLive)
	{
		for (auto& image : m_images)
		{
			if (image.live->imageNumber == registeredLive->imageNumber)
			{
				image.registeredLive = registeredLive;
				break;
			}
		}
	}
}

Sense::Rect StbRecording::getMarkerRegion() const
{
	Sense::Point leftTop(std::numeric_limits<double>::infinity());
	Sense::Point rightBottom(-1.0);

	Sense::Rect markerRegion;

	for (auto& image : m_images)
	{
		const auto& markers = image.markers;
		if (markers != nullptr && markers->valid)
		{
			Sense::Point p1 = Sense::Point(static_cast<double>(markers->p1.x), static_cast<double>(markers->p1.y));
			Sense::Point p2 = Sense::Point(static_cast<double>(markers->p2.x), static_cast<double>(markers->p2.y));

			leftTop = Sense::Point::minimum(leftTop, p1);
			leftTop = Sense::Point::minimum(leftTop, p2);

			rightBottom = Sense::Point::maximum(rightBottom, p1);
			rightBottom = Sense::Point::maximum(rightBottom, p2);

			markerRegion = Sense::Rect(leftTop, rightBottom);
		}
	}

	return markerRegion;
}

Sense::Rect StbRecording::getImageBoundries() const
{
	Sense::Rect validRect;

	if (!m_images.empty() && m_images.front().live)
	{
		validRect = m_images.front().live->validRect;
	}

	return validRect;
}

void StbRecording::setIncludeImagesWithoutMarkers(bool include) const
{
	m_includeAllImagesWithoutMarkers = include;
}

void StbRecording::restoreDefaultBoostedPhase()
{
	if (m_autoSelectedLastBoostFrameIndex != -1)
	{
		setBoostPhaseEnd(m_autoSelectedLastBoostFrameIndex);
	}
}

bool StbRecording::isAutoArchived() const
{
    return m_isAutoArchived;
}

void StbRecording::setAutoArchived()
{
    m_isAutoArchived = true;
}

}}