// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbPostDeployRoiDefinitionState.h"
#include "StbRoiTsmPage.h"
#include <assert.h>

using namespace CommonPlatform;

namespace Pci {	namespace Core {

	StbPostDeployRoiDefinitionState::StbPostDeployRoiDefinitionState(
		PciViewContext &viewContext,
		IUiStateController& controller,
		Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
		const IArchivingSettings& archivingSettings)
			: StbRoiDefinitionState(viewContext, ViewState::StbPostDeployRoiDefinitionState, L"StbPostDeployRoiDefinitionState", controller, ViewState::StbPostDeployProcessingState, ViewState::StbPostDeployReviewState, copyToRefClient, archivingSettings)	{
	}

	StbPostDeployRoiDefinitionState::~StbPostDeployRoiDefinitionState()
	{
	}

	std::wstring StbPostDeployRoiDefinitionState::getCopyToRefTargetName() const
	{
		return m_archivingSettings.getCopyToRefViewer(IArchivingSettings::Feature::StentBoostPostDeploy);
	}
}}

