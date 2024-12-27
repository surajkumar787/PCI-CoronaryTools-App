// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "XRayImageRecording.h"
#include "XrayImage.h"

namespace Pci {
	namespace Core {

		XRayImageRecording::XRayImageRecording(int runIndex) :
			images(),
			runIndex(runIndex)
		{
		}

		void XRayImageRecording::addImage(const CommonPlatform::Xray::XrayImage& image)
		{
			images.emplace_back(new CommonPlatform::Xray::XrayImage(image));
		}

		double XRayImageRecording::getAvgFPS() const
		{
			double avgFPS(0.0);
			if (images.size() > 2)
			{
				avgFPS = images.size() / (images.back()->timestamp - images.front()->timestamp);
			}

			return avgFPS;
		}

		int XRayImageRecording::getRunIndex() const
		{
			return runIndex;
		}

		int XRayImageRecording::getImageCount() const
		{
			return static_cast<int>(images.size());
		}

		const std::vector<XRayImageRecording::XrayImagePtr> XRayImageRecording::getAllImages() const
		{
			return images;
		}

	}
}