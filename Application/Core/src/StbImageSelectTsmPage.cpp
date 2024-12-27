// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbImageSelectTsmPage.h"
#include "Translation.h"
#include "TsmButtonDefinitions.h"

namespace Pci {	namespace Core
{
	StbImageSelectTsmPage::StbImageSelectTsmPage() :
		StbTsmPage(),
		navigateLeftButton(buttonProperties[NavigateLeftButton - 1]), // numbering in the array is tsm numbering - 1
		navigateRightButton(buttonProperties[NavigateRightButton - 1]),
		resetDefaultButton(buttonProperties[ResetDefaultButton - 1]),
		applyButton(buttonProperties[ApplyButton - 1])
	{
		navigateLeftButton->visible = true;
		navigateLeftButton->enabled = true;
		navigateLeftButton->buttonType = TSM::ButtonType::Left;

		navigateRightButton->visible = true;
		navigateRightButton->enabled = true;
		navigateRightButton->buttonType = TSM::ButtonType::Right;

		resetDefaultButton->visible = true;
		resetDefaultButton->enabled = true;
		resetDefaultButton->centerText = Localization::Translation::IDS_ResetToDefault;
		resetDefaultButton->buttonType = TSM::ButtonType::SubLevelFunction;

		applyButton->visible = true;
		applyButton->enabled = true;
		applyButton->centerText = Localization::Translation::IDS_Apply;
		applyButton->buttonType = TSM::ButtonType::SubLevelFunction;
	}
}}