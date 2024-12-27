// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmTsmPage.h"
#include "TsmButtonDefinitions.h"
#include "Translation.h"

namespace Pci { namespace Core
{
	CrmTsmPage::CrmTsmPage() :
		PciTsmPage(),
		roadmapOptionsButton(buttonProperties[RoadmapSelectionButton - 1]),
		frameBackwardButton(buttonProperties[FrameBackButton - 1]), // numbering in the array is tsm numbering - 1
		frameForwardButton(buttonProperties[FrameForwardButton - 1]),
		playPauseButton(buttonProperties[PlayPauseButton - 1]),
		navigateLeftButton(buttonProperties[NavigateLeftButton - 1]),
		navigateRightButton(buttonProperties[NavigateRightButton - 1]),
		lockRoadmapButton(buttonProperties[LockRoadmapButton - 1])
	{
        storeSnapshotButton->visible = true;
        storeMovieButton->visible = true;

		roadmapOptionsButton->visible = true;
		roadmapOptionsButton->enabled = false;
		roadmapOptionsButton->centerText = Localization::Translation::IDS_RoadmapOptions;
		roadmapOptionsButton->buttonType = TSM::ButtonType::MainLevelFunction;

		frameBackwardButton->visible = false;
		frameBackwardButton->enabled = true;
		frameBackwardButton->icon = TSM::ButtonIcon::PreviousImage;
		frameBackwardButton->buttonType = TSM::ButtonType::SubLevelFunction;

		frameForwardButton->visible = false;
		frameForwardButton->enabled = true;
		frameForwardButton->icon = TSM::ButtonIcon::NextImage;
		frameForwardButton->buttonType = TSM::ButtonType::SubLevelFunction;

		playPauseButton->visible = false;
		playPauseButton->enabled = true;
		playPauseButton->icon = TSM::ButtonIcon::MoviePlay;
		playPauseButton->buttonType = TSM::ButtonType::SubLevelFunction;

		navigateLeftButton->visible = false;
		navigateLeftButton->enabled = true;
		navigateLeftButton->buttonType = TSM::ButtonType::Left;

		navigateRightButton->visible = false;
		navigateRightButton->enabled = true;
		navigateRightButton->buttonType = TSM::ButtonType::Right;

		lockRoadmapButton->visible = false;
		lockRoadmapButton->enabled = true;
		lockRoadmapButton->buttonType = TSM::ButtonType::SubLevelFunction;
		lockRoadmapButton->centerText = Localization::Translation::IDS_LockRoadmap;
	}


}}