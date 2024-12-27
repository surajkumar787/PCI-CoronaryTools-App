// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "StbViewer.h"

#include <iomanip>

#include "Translation.h"
#include "StbImage.h"
#include "AlluraColors.h"
#include "ViewType.h"
#include "StbMarkers.h"
#include "StbUIHelpers.h"

#include <Sense/Graphics/IRenderer.h>
#include <ExperienceIdentity/Resource/Palette.h>
#include <ExperienceIdentity/Resource/Icons.h>

#include <windows.h>

namespace UI = Pci::Core::UI;

namespace Pci {	namespace Core
{
	using Sense::ExperienceIdentity::Palette;

	const double StbViewer::ErrorImageSize = 64.0;
	const double StbViewer::ErrorImageSpacing = 25.0;


	StbViewer::StbViewer(Control &parent, ViewType type) :
		PciXrayViewer(parent, type, false),
		showRunInfo(false, [this] { invalidate(); }),
		showBoostedFrameCounter(false, [this] { invalidate(); }),
		currentFrame(0, [this] { invalidate(); }),
		boostedFrames(0, [this] { invalidate(); }),
		totalFrames(0, [this] { invalidate(); }),
		warningMain(gpu),
		warningSub(gpu),
		runInfo(gpu),
		runNumber(0)	
	{
		Sense::Color semiTransparent;
		semiTransparent.a = 0.5;

		const Sense::Color textColor = type == ViewType::Allura ? UI::ColorAlluraLightGreyFont : Palette::Yellow010;
		const Sense::Color outlineColor = type == ViewType::Allura ? Palette::Black : Palette::Black - semiTransparent;

		runInfo.color = textColor;
		runInfo.outline = outlineColor;
		runInfo.type = Sense::TextType::MultiLine;

		if (type == ViewType::Allura)
		{
			runInfo.font = Sense::Font(L"Philips Healthcare Font", 28, false, false, false, false, Sense::Font::Antialias::None);
			runInfo.verticalAlign = Sense::VAlign::Bottom;
			runInfo.horizontalAlign = Sense::HAlign::Right;
		}
		else
		{
			runInfo.font = Sense::Font(L"Philips Healthcare Font", 18, false, false, false, false, Sense::Font::Antialias::None);
			runInfo.verticalAlign = Sense::VAlign::Top;
			runInfo.horizontalAlign = Sense::HAlign::Left;
		}

		// No Anti-aliasing for outlined font
		warningMain.font = Sense::Font(L"Philips Healthcare Font", 48, false, false, false, false, Sense::Font::Antialias::None);
		warningMain.color = textColor;
		warningMain.outline = outlineColor;
		warningMain.text = Pci::Core::Localization::Translation::IDS_EnhancementFailed;
		warningMain.imageSize = ErrorImageSize;
		warningMain.image = Sense::ExperienceIdentity::Icons::GeneralWarning;
		warningMain.imageSpacing = ErrorImageSpacing;
		warningMain.horizontalAlign = Sense::HAlign::Center;

		// No Anti-aliasing for outlined font
		warningSub.font = Sense::Font(L"Philips Healthcare Font", 40, false, false, false, false, Sense::Font::Antialias::None);
		warningSub.color = textColor;
		warningSub.outline = outlineColor;
		warningSub.text = Pci::Core::Localization::Translation::IDS_BalloonMarkersLost;
		warningSub.imageSize = ErrorImageSize;
		warningSub.imageSpacing = ErrorImageSpacing;
		warningSub.imageHideEmpty = false;
		warningSub.horizontalAlign = Sense::HAlign::Center;
	}

	void StbViewer::setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image)
	{
		PciXrayViewer::setImage(image);
		runNumber = ( image)? image->seriesNumber: -1;
	}

	void StbViewer::renderOverlay(Sense::IRenderer &renderer) const
	{
		Sense::Space  space = getClientSpace();

		if (isInvalidated())
		{
			warningMain.bounds = Sense::Rect(space.bounds.left, space.bounds.bottom - 230, space.bounds.right, space.bounds.bottom);
			warningSub.bounds = Sense::Rect(space.bounds.left, space.bounds.bottom - 180, space.bounds.right, space.bounds.bottom);

			if (viewType == ViewType::Allura)
			{
				setRunInfoAllura();
			}
			else
			{
				setRunInfoAzurion();
			}
		}

		if (showRunInfo)
		{
			runInfo.render(space, renderer);
		}
	}

	void StbViewer::setRunInfoAllura() const
	{
		Sense::Space  space = getClientSpace();
		std::wstringstream runInfoText;

		if (showBoostedFrameCounter)
		{
			runInfoText << getBoostedFrameCount();
			runInfo.horizontalAlign = HAlign::Left;
		}
		else
		{
			runInfoText << std::setfill(L'0') << std::setw(3) << runNumber << std::endl << static_cast<int>(currentFrame) + 1;
			runInfo.horizontalAlign = HAlign::Right;
		}

		runInfo.text = runInfoText.str();
		runInfo.bounds = Sense::Rect(space.bounds.left, space.bounds.bottom - 160, space.bounds.right - 10, space.bounds.bottom - 30);
	}

	void StbViewer::setRunInfoAzurion() const
	{
		Sense::Space  space = getClientSpace();
		std::wstringstream runInfoText;
		
		runInfoText << Localization::Translation::IDS_Series << " " << runNumber << std::endl;
		if (showBoostedFrameCounter)
		{
			runInfoText << getBoostedFrameCount();
		}
		else
		{
			runInfoText << Localization::Translation::IDS_Image << " " << static_cast<int>(currentFrame) + 1 << L" / " << static_cast<int>(totalFrames);
		}

		runInfo.text = runInfoText.str();
		runInfo.bounds = Sense::Rect(Sense::Point(10, 40), runInfo.measureSize(space));
	}

	std::wstring StbViewer::getBoostedFrameCount() const
	{
		std::wstringstream frameCountText;
		frameCountText << Localization::Translation::IDS_Images << " " << static_cast<int>(totalFrames) << std::endl;
		frameCountText << Localization::Translation::IDS_Boosted << " " << static_cast<int>(boostedFrames);

		return frameCountText.str();
	}
}
}