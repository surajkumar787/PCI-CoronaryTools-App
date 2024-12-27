// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbContrastRoiDefinitionState.h"
#include "StbRoiTsmPage.h"
#include <assert.h>

using namespace CommonPlatform;

namespace Pci {	namespace Core {

	StbContrastRoiDefinitionState::StbContrastRoiDefinitionState(
		PciViewContext &viewContext,
		IUiStateController& uiController,
		Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
		const IArchivingSettings& archivingSettings)
			: StbRoiDefinitionState(viewContext, 
				ViewState::StbContrastRoiDefinitionState, 
				L"StbContrastRoiDefinitionState", 
				uiController, 
				ViewState::StbContrastProcessingState, 
				ViewState::StbContrastReviewState, 
				copyToRefClient, 
				archivingSettings)
	{
        getTsmPage().contrastFrameSelectButton->visible = true;
        getTsmPage().boostFrameSelectButton->visible = true;
        getTsmPage().invertBoostButton->visible = true;
	}

	StbContrastRoiDefinitionState::~StbContrastRoiDefinitionState()
	{
	}

	std::wstring StbContrastRoiDefinitionState::getCopyToRefTargetName() const
	{
		return m_archivingSettings.getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract);
	}
}}

