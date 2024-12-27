// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <Sense/Shapes/ImageText.h>

#include "PciXrayViewer.h"

namespace Pci {	namespace Core
{
	class StbViewer : public PciXrayViewer
	{
	public:
		StbViewer(Control &parent, ViewType type);

		void setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image);

		Sense::Property<bool> showRunInfo;
		Sense::Property<bool> showBoostedFrameCounter;
		Sense::Property<int> currentFrame;
		Sense::Property<int> boostedFrames;
		Sense::Property<int> totalFrames;

		StbViewer(const StbViewer&) = delete;
		StbViewer& operator=(const StbViewer&) = delete;

	protected:
		void renderOverlay(Sense::IRenderer &renderer) const override;

		mutable Sense::ImageText	warningMain;
		mutable Sense::ImageText	warningSub;

		static const double ErrorImageSize;
		static const double ErrorImageSpacing;

	private:
		void setRunInfoAllura() const;
		void setRunInfoAzurion() const;
		std::wstring getBoostedFrameCount() const;

		mutable Sense::Text runInfo;
		int runNumber;
	};

}}