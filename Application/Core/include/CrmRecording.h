// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <deque>
#include <memory>

namespace CommonPlatform { namespace Xray { 
	struct XrayImage;
}}

namespace Pci { namespace Core {

struct CrmOverlay;

class CrmRecording
{
public:
	typedef std::pair<std::shared_ptr<const CommonPlatform::Xray::XrayImage>, std::shared_ptr<const CrmOverlay>> XrayImageAndOverlay;

	explicit CrmRecording(int maxImagesInBuffer);
	virtual ~CrmRecording() = default;

	void  addImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& image, const std::shared_ptr<const CrmOverlay> &roadmap);

	void  setRunIndex(int runIndex);
	int	  getRunIndex() const;

	int   getSeriesNumber() const;

	const std::deque<XrayImageAndOverlay>& getAllImages()const;

	void  clear();

	int	  getImageCount()const;
	void  removeImagesBefore(double timeStamp);

private:
	std::deque<XrayImageAndOverlay> images;

	int  maxImages;
	int  runIndex;
};
}}

