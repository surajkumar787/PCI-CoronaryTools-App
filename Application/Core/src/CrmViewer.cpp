// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmViewer.h"

#include "XrayImage.h"
#include "CrmOverlay.h"
#include "AlluraColors.h"
#include "Translation.h"
#include "Sense/Common/Font.h"
#include "ExperienceIdentity.h"


namespace Pci { namespace Core
{
CrmViewer::CrmViewer(Control &parent, ViewType type) :
	PciXrayViewer	(parent, type, false),
	blendFactor		(0.40,  [this] { invalidate(); }),
	alpha			(0.01,  [this] { invalidate(); }),	//we can't set to zero, because alpha test will skip the pixel...
	overlayColor	(Color(1.0, 0.0, 0.0), [this] { invalidate(); }),
	warning			(gpu),
	textureOverlay	(gpu),	
	overlay			(nullptr)
{
	warning.font			= Font(L"Philips Healthcare Font", 48, false, false, false, false, Font::Antialias::None);
	warning.color			= Pci::Core::UI::ColorAlluraLightGreyFont;
	warning.outline         = Sense::ExperienceIdentity::Palette::Black;
	warning.text			= Pci::Core::Localization::Translation::IDS_NoRoadmapAvailable;
	warning.horizontalAlign	= HAlign::Center;
}

void CrmViewer::setOverlay(const std::shared_ptr<const CrmOverlay> &roadmapOverlay)
{
	overlay = roadmapOverlay;
	textureOverlay.invalidate();
	invalidate();
}

static const double TranspancyWarningBox = 0.4;
static const double WarningBoxHeight = 75;

void CrmViewer::renderOverlay(IRenderer &renderer) const
{
	Space space = getClientSpace();
	if ((overlay != nullptr) && overlay->pointer)
	{
		auto matrix = pixelsToClient();

		Color color(overlayColor);
		color = Color(color.r * blendFactor, color.g * blendFactor, color.b * blendFactor); color.a = alpha;
		if ((overlay != nullptr) && overlay->pointer)
		{
			textureOverlay.render(space, renderer, *overlay, color, xray->shutters, matrix, overlay->frustumToPixels * xray->frustumToPixels.inverse(), false);
		}
	}
	else
	{
		warning.bounds = Rect(space.bounds.left, space.bounds.bottom - WarningBoxHeight, space.bounds.right, space.bounds.bottom);
		renderer.render(space, warning.bounds, Color(0,0,0) * TranspancyWarningBox);
		warning.render(space,renderer);
	}
}

}}