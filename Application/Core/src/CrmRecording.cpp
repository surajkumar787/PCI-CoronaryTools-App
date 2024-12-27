// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmRecording.h"
#include "XrayImage.h"
#include "CrmOverlay.h"

using namespace Pci::Core;

CrmRecording::CrmRecording(int maxImagesInBuffer) :
	maxImages(maxImagesInBuffer),
	runIndex(-1)
{
}

void CrmRecording::setRunIndex(int id)
{
	if(id != runIndex)
	{
		runIndex = id;
	}
}

const std::deque<CrmRecording::XrayImageAndOverlay>& CrmRecording::getAllImages() const
{
	return images;
}

int CrmRecording::getImageCount() const
{
	return static_cast<int>(images.size());
}

void CrmRecording::removeImagesBefore(double timeStamp)
{
	auto firstRecordedImageToKeep = std::find_if(images.begin(), images.end(), [&, timeStamp](const XrayImageAndOverlay &imagePair)
	{
		return imagePair.first && imagePair.first->timestamp >= timeStamp;
	});
	images.erase(images.begin(), firstRecordedImageToKeep);
}

void CrmRecording::addImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& image, const std::shared_ptr<const CrmOverlay>& roadmap)
{
	images.emplace_back(image, roadmap);

	if(static_cast<int>(images.size()) > maxImages)
	{
		images.pop_front();
	}
}

void CrmRecording::clear()
{
	images.clear();
}

int CrmRecording::getRunIndex() const
{
	return runIndex;
}

int CrmRecording::getSeriesNumber() const
{
	int seriesNumber = -1;
	
	if (!images.empty())
	{
		seriesNumber = images.front().first->seriesNumber;
	}

	return seriesNumber;
}
