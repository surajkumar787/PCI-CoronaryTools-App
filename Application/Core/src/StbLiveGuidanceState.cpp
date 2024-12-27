// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbLiveGuidanceState.h"
#include "StbGuidanceState.h"
#include "PciViewContext.h"
#include "StbTsmPage.h"
#include "StbAdministration.h"
#include "ViewStateHelper.h"
#include "Translation.h"
#include "Resources.h"
#include "pciGuidanceView.h"
#include "ViewType.h"
#include "PciViewContext.h"
#include "StbBoostViewer.h"
#include "StbMarkersViewer.h"

namespace Pci{ namespace Core{
using namespace Sense::ExperienceIdentity;

StbLiveGuidanceState::StbLiveGuidanceState(
	PciViewContext &viewContext, 
	Pci::Platform::iControl::ICopyToRefClient& copyToRefClient, 
	const IArchivingSettings& archivingSettings)
		: StbGuidanceState(viewContext, ViewState::StbLiveGuidanceState, L"StbLiveGuidanceState", LicenseFeature::StentboostLive, copyToRefClient, archivingSettings)
{
}

void StbLiveGuidanceState::onEnter()
{
	_guidanceModel.status = Localization::Translation::IDS_ReadyToStentboost;
	_guidanceModel.messagePicture = &PciSuite::Resources::Icon_StbLive;

	if (pciViewContext.stbAdministration.lastImageIndex() == -1)
	{
		_guidanceModel.messageHeaderIcon = nullptr;
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_UseCinePedalToActivateSBL;
		_guidanceModel.messageText = Localization::Translation::IDS_ConfirmBalloonMarkersVisible;
	}
	else
	{
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_RunTooShort;
		_guidanceModel.messageHeaderIcon = &Icons::Information;
		_guidanceModel.messageText  = Localization::Translation::IDS_MinimumImagesForEnhancement;
	}

	StbGuidanceState::onEnter();
}

std::wstring StbLiveGuidanceState::getCopyToRefTargetName() const
{
	return m_archivingSettings.getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive);
}

}}