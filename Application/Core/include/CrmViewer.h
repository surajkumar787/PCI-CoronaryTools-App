// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "Sense/Shapes/ImageText.h"

#include "TextureSimplicity.h"
#include "PciXrayViewer.h"

namespace CommonPlatform { namespace Xray
{
	struct XrayImage;
}}

namespace Pci { namespace Core
{	
	struct CrmOverlay;
	
	class CrmViewer : public Pci::Core::PciXrayViewer
	{
	public:
		CrmViewer(Sense::Control &parent, ViewType type);

		void setOverlay(const std::shared_ptr<const CrmOverlay> &roadmapOverlay);

		Sense::Property<double>			blendFactor;
		Sense::Property<double>			alpha;
		Sense::Property<Sense::Color>	overlayColor;

		CrmViewer(const CrmViewer&) = delete;
		CrmViewer& operator=(const CrmViewer&) = delete;

	protected:

		void renderOverlay(Sense::IRenderer &renderer) const override;

		mutable Sense::ImageText			warning;
		mutable Sense::TextureSimplicity	textureOverlay;
		std::shared_ptr<const CrmOverlay>	overlay;
	};

}}