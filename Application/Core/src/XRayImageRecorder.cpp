// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "XRayImageRecorder.h"
#include "XRayImageRecording.h"
#include "XrayImage.h"

namespace Pci {	namespace Core {

	void XRayImageRecorder::addImage(const CommonPlatform::Xray::XrayImage& image)
	{
		if ((recording == nullptr) || (recording->getRunIndex() != image.runIndex))
		{
			recording.reset(new XRayImageRecording(image.runIndex));
		}

		CommonPlatform::Xray::XrayImage recordedImage(image);
		recordedImage.type = CommonPlatform::Xray::XrayImage::Type::ExposureReplay;

		recording->addImage(recordedImage);
	}

	const std::shared_ptr<XRayImageRecording> XRayImageRecorder::getLastRecording() const
	{
		return recording;
	}

} }