// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbContrastGuidanceState.h"
#include "StbGuidanceState.h"
#include "PciViewContext.h"
#include "StbAdministration.h"
#include "StbTsmPage.h"
#include "Translation.h"
#include "Resources.h"
#include "PciGuidanceView.h"
#include "ViewType.h"

namespace Pci { namespace Core {
	using namespace Sense::ExperienceIdentity;

	StbContrastGuidanceState::StbContrastGuidanceState(
		PciViewContext &pciViewContext,
		Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
		const IArchivingSettings& archivingSettings)
			: StbGuidanceState(pciViewContext, ViewState::StbContrastGuidanceState, L"StbContrastGuidanceState", LicenseFeature::StbSubtract, copyToRefClient, archivingSettings)
	{
        getTsmPage().contrastFrameSelectButton->visible = true;
        getTsmPage().boostFrameSelectButton->visible = true;
        getTsmPage().defineRoiButton->visible = true;
        getTsmPage().invertBoostButton->visible = true;
	}


	void StbContrastGuidanceState::onEnter()
	{
		_guidanceModel.status = Localization::Translation::IDS_ReadyToReceiveStbSubtract;
		_guidanceModel.messagePicture = &PciSuite::Resources::Icon_StbSubtract;

		_guidanceModel.showSBSGuidance = true;
		_guidanceModel.stentBoostPhaseDuration = 3;
		_guidanceModel.stentBoostTimeRemaining = 3;


		if (pciViewContext.stbAdministration.lastImageIndex() == -1)
		{
			_guidanceModel.messageHeaderIcon = nullptr;
			_guidanceModel.messageHeaderText = Localization::Translation::IDS_UseCinePedalToCreateSbSubtract;
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

	std::wstring StbContrastGuidanceState::getCopyToRefTargetName() const
	{
		return m_archivingSettings.getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract);
	}
}
}