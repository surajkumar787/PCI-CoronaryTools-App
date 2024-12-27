// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "StbViewer.h"

namespace Pci {	namespace Core
{
	struct StbMarkers;
	class StbRoiViewer : public StbViewer
	{
	public:
		StbRoiViewer(Control &parent, ViewType type);

		void setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image, const std::shared_ptr<const StbMarkers>& markers);

		Sense::Property<Sense::Rect> regionOfInterest;

		StbRoiViewer(const StbRoiViewer&) = delete;
		StbRoiViewer& operator=(const StbRoiViewer&) = delete;

	private:
		void renderOverlay(Sense::IRenderer &renderer) const override;

		void drawRegionOfInterest(Sense::Drawing& drawing) const;
		void drawMarkers(Sense::Drawing& drawing) const;

		std::shared_ptr<const StbMarkers> markers;
	};
}}