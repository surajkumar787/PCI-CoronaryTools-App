// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbPostDeployProcessingState.h"
#include "PciViewContext.h"
#include "StbBoostViewer.h"
#include "StbMarkersViewer.h"
#include "StbTsmPage.h"
#include "PciReviewParameters.h"
#include "StbAdministration.h"
#include "UtilizationLogEvent.h"
#include "IArchivingStatus.h"
#include "ViewStateHelper.h"
#include "ViewType.h"
#include "Log.h"
#include "StbMarkers.h"
#include "StbImage.h"
#include "IUiStateController.h"
#include "IArchivingSettings.h"

using namespace CommonPlatform;

namespace Pci {	namespace Core {

StbPostDeployProcessingState::StbPostDeployProcessingState(
	PciViewContext &viewContext,
	IUiStateController& controller,
	Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
	const IArchivingSettings& archivingSettings)
		: StbProcessingState(viewContext, ViewState::StbPostDeployProcessingState, L"StbPostDeployProcessingState", LicenseFeature::StbPostDeploy, controller, copyToRefClient, archivingSettings),
	      m_switch_away(true)
{
    getTsmPage().defineRoiButton->visible = true;

}

StbPostDeployProcessingState::~StbPostDeployProcessingState()
{
}

ViewState StbPostDeployProcessingState::getGuidanceState() const
{
	return ViewState::StbPostDeployGuidanceState;
}

ViewState StbPostDeployProcessingState::getReviewState() const
{
	return ViewState::StbPostDeployReviewState;
}

std::wstring StbPostDeployProcessingState::getCopyToRefTargetName() const
{
	return m_archivingSettings.getCopyToRefViewer(IArchivingSettings::Feature::StentBoostPostDeploy);
}

}}
