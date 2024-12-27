// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbPostDeployGuidanceState.h"
#include "StbGuidanceState.h"
#include "PciViewContext.h"
#include "StbAdministration.h"
#include "StbTsmPage.h"
#include "Translation.h"
#include "Resources.h"
#include "pciGuidanceView.h"
#include "ViewType.h"
#include "StbBoostViewer.h"
#include "StbMarkersViewer.h"

namespace Pci{ namespace Core{
using namespace Sense::ExperienceIdentity;

StbPostDeployGuidanceState::StbPostDeployGuidanceState(
	PciViewContext &viewContext, 
	Pci::Platform::iControl::ICopyToRefClient& copyToRefClient, 
	const IArchivingSettings& archivingSettings)
		: StbGuidanceState(viewContext, 
			ViewState::StbPostDeployGuidanceState, 
			L"StbPostDeployGuidanceState", 
			LicenseFeature::StbPostDeploy, 
			copyToRefClient,
			archivingSettings)
{
    getTsmPage().defineRoiButton->visible = true;
}

void StbPostDeployGuidanceState::onEnter()
{
	_guidanceModel.status = Localization::Translation::IDS_ReadyToPostDeployStentboost;
	_guidanceModel.messagePicture = &PciSuite::Resources::Icon_StbPostDeployment;


	if (pciViewContext.stbAdministration.lastImageIndex() == -1)
	{
		_guidanceModel.messageHeaderIcon = nullptr;
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_UseCinePedalToCreateSBLPostDeployment;
		_guidanceModel.messageText = Localization::Translation::IDS_DoNotMoveStentMarkers;
		_guidanceModel.guidanceText = Localization::Translation::IDS_MakeSureMarkersAreVisible;
	}
	else
	{
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_RunTooShort;
		_guidanceModel.messageHeaderIcon = &Icons::Information;
		_guidanceModel.messageText = Localization::Translation::IDS_MinimumImagesForEnhancement;
		_guidanceModel.guidanceText = L"";
	}

	StbGuidanceState::onEnter();
}

std::wstring StbPostDeployGuidanceState::getCopyToRefTargetName() const
{
	return m_archivingSettings.getCopyToRefViewer(IArchivingSettings::Feature::StentBoostPostDeploy);
}

}}