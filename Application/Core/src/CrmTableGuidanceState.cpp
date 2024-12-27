// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmTableGuidanceState.h"
#include "PciViewContext.h"
#include "ViewStateHelper.h"
#include "Translation.h"
#include "Resources.h"
#include "PciXrayViewer.h"
#include "pciGuidanceView.h"

using namespace Sense::ExperienceIdentity;
namespace Pci { namespace Core {

CrmTableGuidanceState::CrmTableGuidanceState(PciViewContext &viewContext) :
	CrmGuidanceBaseState(viewContext, ViewState::CrmTableGuidanceState, L"CrmTableGuidanceState")
{
	_guidanceModel.showMessagePanel= true;
	_guidanceModel.showMessagePicturePanel = false;	
	_guidanceModel.showPositionPanel = false;
	_guidanceModel.showProcessingPanel = false;
	_guidanceModel.statusIcon = nullptr;
	_guidanceModel.messageHeaderText = Localization::Translation::IDS_TableMoved;
	_guidanceModel.messageHeaderColor = getInfoColour(pciViewContext.crmGuidance.getViewType());
	_guidanceModel.messageHeaderIcon = &Icons::Information;
	_guidanceModel.roadmapActive = false;
	_guidanceModel.status =  Localization::Translation::IDS_RoadmapNotReady;
	_guidanceModel.guidanceText = Localization::Translation::IDS_ManuallyMoveTable;
	_guidanceModel.messageText = L"";

}


void CrmTableGuidanceState::onEnter()
{

    CrmGuidanceBaseState::onEnter();

	_guidanceModel.roadMapImage = pciViewContext.currentRoadmap;
	pciViewContext.crmGuidance.setGuidanceModel(_guidanceModel);
	show(&pciViewContext.crmGuidance);
}

ViewState  CrmTableGuidanceState::roadmapStatusChanged()
{
	if ((pciViewContext.currentEpx.getType() == PciEpx::Type::NonPci) &&
		(pciViewContext.crmAdminStatus == CrmAdministration::Status::NoRoadmap))
	{
		return ViewState::NonPciEpxState;
	}
	_guidanceModel.roadMapImage = pciViewContext.currentRoadmap;
	_guidanceModel.roadmapActive = false;

	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
}

}}