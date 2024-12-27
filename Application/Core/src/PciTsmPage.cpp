// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PciTsmPage.h"
#include "Translation.h"
#include "TsmButtonDefinitions.h"

namespace Pci { namespace Core
{

	PciTsmPage::PciTsmPage() :
		log(::CommonPlatform::Logger(::CommonPlatform::LoggerType::Platform)),
		storeSnapshotButton(buttonProperties[SnapshotButton      - 1]), // numbering in the array is tsm numbering - 1
		storeMovieButton   (buttonProperties[MovieButton         - 1])
	{
		storeSnapshotButton->visible = false;
		storeSnapshotButton->enabled = false;
		storeSnapshotButton->icon = TSM::ButtonIcon::Snapshot;
		storeSnapshotButton->buttonType = TSM::ButtonType::MainLevelFunction;

		storeMovieButton->visible = false;
		storeMovieButton->enabled = false;
		storeMovieButton->buttonType = TSM::ButtonType::MainLevelFunction;
		storeMovieButton->upperText = Localization::Translation::IDS_Store;
		storeMovieButton->lowerText = Localization::Translation::IDS_Movie;
	}

	void PciTsmPage::onButtonPress(unsigned int button, bool repeated)
	{
		auto buttonProp = buttonProperties[button - 1];

		auto identifier = buttonProp.icon;
		if (identifier.empty()) identifier = buttonProp.upperText + L" " + buttonProp.lowerText;
		log.DeveloperInfo(L"PciTsmPage: received button press for button %s (repeat: %d)", identifier.c_str(), repeated);
		TsmPage::onButtonPress(button, repeated);
	}

}}