// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmProcessingAngioState.h"
#include "PciViewContext.h"
#include "CrmTsmPage.h"
#include "CoRegistrationServerState.h"
#include "ViewStateHelper.h"
#include "Translation.h"
#include "pciGuidanceView.h"

namespace Pci{ namespace Core {
using namespace CommonPlatform::CoRegistration;

CrmProcessingAngioState::CrmProcessingAngioState(PciViewContext &viewContext)
	: UiState(viewContext, ViewState::CrmProcessingAngioState, L"CrmProcessingAngioState",LicenseFeature::Roadmap),
	_guidanceModel()
{
	_guidanceModel.showMessagePanel = false;
	_guidanceModel.showMessagePicturePanel = false;
	_guidanceModel.showPositionPanel = false;
	_guidanceModel.status = Localization::Translation::IDS_ProcessingAngio;
	_guidanceModel.showProcessingPanel = true;
	_guidanceModel.roadMapImage = nullptr;
	_guidanceModel.statusIcon = nullptr;

	tsmPage.reset(new CrmTsmPage());
}

PciXrayPositionModel& CrmProcessingAngioState::getModel()
{
	return _guidanceModel;
}

void CrmProcessingAngioState::onEnter()
{
	_guidanceModel.currentGeometry = pciViewContext.geometry;
	pciViewContext.crmGuidance.setGuidanceModel(_guidanceModel);
	show(&pciViewContext.crmGuidance);
}

// The only way to leave this state is by calling 'angioProcessingStopped'. 
ViewState CrmProcessingAngioState::angioProcessingStopped()
{
    if((pciViewContext.coRegState.systemState == CoRegistrationServerState::SystemState::Pullback) || 
	   (pciViewContext.coRegState.systemState == CoRegistrationServerState::SystemState::SpotMeasurement))
	{
		return ViewState::CoRegistrationGuidanceState;
	}
	else
	{
		return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
	}
}

ViewState CrmProcessingAngioState::roadmapStatusChanged()
{
	return viewState;
}

}}