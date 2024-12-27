// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbRoiTsmPage.h"
#include "Translation.h"
#include "TsmButtonDefinitions.h"

namespace Pci {	namespace Core
{
	StbRoiTsmPage::StbRoiTsmPage() :
		StbTsmPage(),
		sizeUpButton(buttonProperties[SizeUpButton - 1]), // numbering in the array is tsm numbering - 1
		sizeDownButton(buttonProperties[SizeDownButton - 1]),
		panLeftButton(buttonProperties[PanLeftButton - 1]),
		panRightButton(buttonProperties[PanRightButton - 1]),
		panUpButton(buttonProperties[PanUpButton - 1]),
		panDownButton(buttonProperties[PanDownButton - 1]),
		applyButton(buttonProperties[ApplyButton - 1])
	{
		sizeUpButton->visible = true;
		sizeUpButton->enabled = true;
		sizeUpButton->buttonType = TSM::ButtonType::Increase;

		sizeDownButton->visible = true;
		sizeDownButton->enabled = true;
		sizeDownButton->label = Localization::Translation::IDS_Size;
		sizeDownButton->buttonType = TSM::ButtonType::Decrease;

		panLeftButton->visible = true;
		panLeftButton->enabled = true;
		panLeftButton->buttonType = TSM::ButtonType::Left;

		panRightButton->visible = true;
		panRightButton->enabled = true;
		panRightButton->buttonType = TSM::ButtonType::Right;

		panUpButton->visible = true;
		panUpButton->enabled = true;
		panUpButton->buttonType = TSM::ButtonType::Up;

		panDownButton->visible = true;
		panDownButton->enabled = true;
		panDownButton->buttonType = TSM::ButtonType::Down;

		applyButton->visible = true;
		applyButton->enabled = true;
		applyButton->centerText = Localization::Translation::IDS_EnhanceStent;
		applyButton->buttonType = TSM::ButtonType::SubLevelFunction;

		defineRoiButton->selected = true;
		defineRoiButton->enabled = true;
	}
}}