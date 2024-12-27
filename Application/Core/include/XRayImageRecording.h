// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <vector>
#include <memory>

namespace CommonPlatform {
	namespace Xray
	{
		struct XrayImage;
	}
}

namespace Pci {	namespace Core {
	class XRayImageRecording
	{
	public:
		typedef std::shared_ptr<CommonPlatform::Xray::XrayImage> XrayImagePtr;

		explicit XRayImageRecording(int runIndex);
		~XRayImageRecording() = default;

		void addImage(const CommonPlatform::Xray::XrayImage& image);
		double getAvgFPS() const;
		int getImageCount() const;
		int getRunIndex() const;

		const std::vector<XrayImagePtr> getAllImages() const;

	private:
		std::vector<XrayImagePtr> images;
		int runIndex;
	};

}}
