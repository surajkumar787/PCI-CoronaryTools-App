// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "StbBoostViewer.h"

#include "Translation.h"
#include "StbImage.h"
#include "AlluraColors.h"
#include "ViewType.h"

#include <Sense/Graphics/IRenderer.h>
#include <ExperienceIdentity/Resource/Palette.h>
#include <ExperienceIdentity/Resource/Icons.h>

namespace UI = Pci::Core::UI;

namespace Pci { namespace Core
{
	using Sense::ExperienceIdentity::Palette;

StbBoostViewer::StbBoostViewer(Control &parent, ViewType type) :
	StbViewer			(parent, type),
	showUpdateDelayed	(false, [this] { invalidate(); }),
	_hasMarkers			(true),
	fadeCounter			(0),
	m_stbMask			()
{
}

void StbBoostViewer::setImage(const std::shared_ptr<const StbImage> &image)
{
	_hasMarkers = image ? !image->showWarning : false;

	if (image)
	{
		std::shared_ptr<StbImage> maskBoost = std::make_shared<StbImage>(*image);
		m_stbMask.mask(image->refMarkers, *image, *maskBoost);
		StbViewer::setImage(maskBoost);
	}
	else
	{
		StbViewer::setImage(std::make_shared<CommonPlatform::Xray::XrayImage>());
	}
}

void StbBoostViewer::renderOverlay(Sense::IRenderer &renderer) const
{
	StbViewer::renderOverlay(renderer);

	Sense::Space  space  = getClientSpace();
	Sense::Matrix matrix = pixelsToClient();

	if (!_hasMarkers || showUpdateDelayed)
	{
		const uint32_t FadeCounterMax = 5;
		const double FadeValueMax = 0.4;
			
		if (fadeCounter < FadeCounterMax)
		{
			fadeCounter++;
		}

		auto fadeValue = (FadeValueMax / FadeCounterMax) * static_cast<double>(fadeCounter);
		renderer.render(space, space.bounds, Sense::Color(0,0,0) * fadeValue);
	}

	if (!_hasMarkers)
	{
		warningMain.text = Localization::Translation::IDS_EnhancementFailed;
		warningSub.text = Localization::Translation::IDS_BalloonMarkersLost;

		warningMain.render(space, renderer);
		warningSub.render(space, renderer);
	}
	else if (showUpdateDelayed)
	{
		warningMain.text = Localization::Translation::IDS_EnhancementFailed;
		warningMain.render(space, renderer);
	}
	else
	{
		fadeCounter = 0;
	}
}

}}