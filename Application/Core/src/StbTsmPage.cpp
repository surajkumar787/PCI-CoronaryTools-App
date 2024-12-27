// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbTsmPage.h"
#include "TsmButtonDefinitions.h"
#include "Translation.h"
#include "stlstring.h"

namespace Pci { namespace Core
{
	StbTsmPage::StbTsmPage() :
		PciTsmPage(),
		defineRoiButton(buttonProperties[DefineROIButton - 1]), // numbering in the array is tsm numbering - 1
		copyToRefButton(buttonProperties[CopyToRefButton - 1]),
        contrastFrameSelectButton(buttonProperties[ContrastFrameSelectButton - 1]),
        boostFrameSelectButton(buttonProperties[BoostFrameSelectButton - 1]),
        invertBoostButton(buttonProperties[InvertBoostButton - 1]),
		toBoostButton(buttonProperties[ToBoostButton - 1]),
		toContrastButton(buttonProperties[ToContrastButton - 1]),
		stopStartButton(buttonProperties[StopStartButton - 1])
	{
		storeSnapshotButton->visible = true;
		storeMovieButton->visible = true;

		defineRoiButton->visible = false;
		defineRoiButton->enabled = false;
		defineRoiButton->centerText = Localization::Translation::IDS_AdjustROI;
		defineRoiButton->buttonType = TSM::ButtonType::MainLevelFunction;

		copyToRefButton->visible = false;
		copyToRefButton->enabled = false;
		copyToRefButton->centerText = str_ext::replace(Localization::Translation::IDS_CopyToReference, L"{0}", L"");
		copyToRefButton->buttonType = TSM::ButtonType::MainLevelFunction;

        contrastFrameSelectButton->visible = false;
        contrastFrameSelectButton->enabled = false;
        contrastFrameSelectButton->centerText = Localization::Translation::IDS_ChangeContrastImage;
        contrastFrameSelectButton->buttonType = TSM::ButtonType::MainLevelFunction;

        boostFrameSelectButton->visible = false;
        boostFrameSelectButton->enabled = false;
        boostFrameSelectButton->centerText = Localization::Translation::IDS_ChangeBoostImage;
        boostFrameSelectButton->buttonType = TSM::ButtonType::MainLevelFunction;

        invertBoostButton->visible = false;
		invertBoostButton->enabled = false;
		invertBoostButton->icon = TSM::ButtonIcon::VideoInvert;
		invertBoostButton->buttonType = TSM::ButtonType::MainLevelFunction;
        
		toBoostButton->visible = false;
		toBoostButton->enabled = false;
		toBoostButton->centerText = Localization::Translation::IDS_BoostedImage;
		toBoostButton->buttonType = TSM::ButtonType::SubLevelFunction;

		toContrastButton->visible = false;
		toContrastButton->enabled = false;
		toContrastButton->centerText = Localization::Translation::IDS_ContrastImage;
		toContrastButton->buttonType = TSM::ButtonType::SubLevelFunction;

		stopStartButton->visible = false;
		stopStartButton->enabled = true;
		stopStartButton->icon = TSM::ButtonIcon::MovieStop;
		stopStartButton->buttonType = TSM::ButtonType::SubLevelFunction;
	}
}}