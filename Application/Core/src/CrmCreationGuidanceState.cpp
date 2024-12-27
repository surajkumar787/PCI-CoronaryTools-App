// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmCreationGuidanceState.h"
#include "PciViewContext.h"
#include "PciTsmPage.h"
#include "ViewStateHelper.h"
#include "PciGuidanceModel.h"
#include "Translation.h"
#include "Resources.h"
#include "pciGuidanceView.h"

using namespace Sense::ExperienceIdentity;
namespace Pci { namespace Core {

CrmCreationGuidanceState::CrmCreationGuidanceState(PciViewContext &viewContext) :
	CrmGuidanceBaseState(viewContext, ViewState::CrmCreationGuidanceState, L"CrmCreationGuidanceState")
{
	_guidanceModel.messageHeaderText = Localization::Translation::IDS_CreateAngioWithoutMoving;
	_guidanceModel.messageHeaderIcon = nullptr;
	_guidanceModel.messagePicture = &PciSuite::Resources::crmstartscreen_guidance_catheter;
	_guidanceModel.messagePictureText = L"";
	_guidanceModel.status = Localization::Translation::IDS_ReadyToReceiveRoadmap;
	_guidanceModel.messageText = Localization::Translation::IDS_DoNotMoveTableCarc;
	_guidanceModel.guidanceText = Localization::Translation::IDS_CaptureThreeHeartbeatsWithContrast;
	_guidanceModel.messageHeaderColor = getNormalColour(pciViewContext.crmGuidance.getViewType());
	_guidanceModel.messageColor = getNormalColour(pciViewContext.crmGuidance.getViewType());
	_guidanceModel.showMessagePanel = true;
	_guidanceModel.showMessagePicturePanel = true;
	_guidanceModel.showPositionPanel = false;
	_guidanceModel.showProcessingPanel = false;
	_guidanceModel.statusIcon =  &PciSuite::Resources::ThumbPreview;;
	_guidanceModel.roadMapImage = nullptr;
}

void CrmCreationGuidanceState::onEnter()
{

    CrmGuidanceBaseState::onEnter();

	pciViewContext.crmGuidance.setGuidanceModel(_guidanceModel);
	show(&pciViewContext.crmGuidance);
}


ViewState CrmCreationGuidanceState::roadmapStatusChanged()
{
	if ((pciViewContext.currentEpx.getType() == PciEpx::Type::NonPci) &&
		(pciViewContext.crmAdminStatus == CrmAdministration::Status::NoRoadmap))
	{
		return ViewState::NonPciEpxState;
	}

	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
}
}}

