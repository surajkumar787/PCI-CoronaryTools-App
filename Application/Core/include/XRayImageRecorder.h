// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <vector>
#include <memory>

namespace CommonPlatform { namespace Xray
{
	struct XrayImage;
}}

namespace Pci {	namespace Core {

	class XRayImageRecording;

	class XRayImageRecorder
	{
	public:
		typedef std::shared_ptr<CommonPlatform::Xray::XrayImage> XrayImagePtr;

		XRayImageRecorder() = default;
		~XRayImageRecorder() = default;

		void addImage(const CommonPlatform::Xray::XrayImage& image);

		const std::shared_ptr<XRayImageRecording> getLastRecording() const;

	private:
		std::shared_ptr<XRayImageRecording> recording;
	};

}}