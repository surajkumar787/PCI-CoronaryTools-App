// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <Sense/Common/Property.h>

#include "PciXrayPositionViewer.h"
#include "TextureSimplicity.h"

namespace CommonPlatform {namespace Xray
{
	struct XrayImage;
}}

namespace Pci { namespace Core
{
	class PciXrayModel;

	class PciXrayViewer : public PciXrayPositionViewer
	{
	public:
		PciXrayViewer(Sense::Control& parent, ViewType type, bool includeShutters, Sense::VAlign imgAlign = Sense::VAlign::Middle );

		void setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& image);
		const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& getImage() const;
		void setXrayModel(PciXrayModel& model);

	protected:
		virtual void render(Sense::IRenderer &renderer) const override;

		virtual void renderOverlay(Sense::IRenderer& renderer) const = 0;
		void renderLastImageHold(Sense::IRenderer& renderer) const;
		void renderXray(Sense::IRenderer& renderer) const;

		Sense::Matrix pixelsToClient() const;

		std::shared_ptr<const CommonPlatform::Xray::XrayImage> xray;

	private:
		PciXrayModel* model;
		bool includeShutters;
		Sense::VAlign imageAlignment;

		mutable Sense::TextureSimplicity textureXray;
		mutable Sense::TextureInput lastImageHoldTexture;
	};

	
}}