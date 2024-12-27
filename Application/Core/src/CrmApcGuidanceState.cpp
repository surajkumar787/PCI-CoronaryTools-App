// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmApcGuidanceState.h"
#include "PciViewContext.h"
#include "CrmGuidanceBaseState.h"
#include "ViewStateHelper.h"
#include "Translation.h"
#include "Resources.h"
#include "PciXrayViewer.h"
#include "pciGuidanceView.h"
#include "ViewType.h"

using namespace Sense::ExperienceIdentity;
namespace Pci { namespace Core {
	
CrmApcGuidanceState::CrmApcGuidanceState(PciViewContext &viewContext) :
	CrmGuidanceBaseState(viewContext, ViewState::CrmApcGuidanceState, L"CrmApcGuidanceState")
{
	_guidanceModel.showMessagePanel = true;
	_guidanceModel.showMessagePicturePanel = true;
	_guidanceModel.showProcessingPanel = false;
	_guidanceModel.showPositionPanel = true;
	_guidanceModel.statusIcon = nullptr;
	_guidanceModel.messageHeaderColor = getInfoColour(pciViewContext.crmGuidance.getViewType());
	_guidanceModel.messageHeaderIcon = &Icons::Information;
	_guidanceModel.status = Localization::Translation::IDS_NearestRoadmap;
	_guidanceModel.messageText = L"";
	_guidanceModel.guidanceText = L"";
	_guidanceModel.roadmapActive = false;
}

void CrmApcGuidanceState::onEnter()
{

    CrmGuidanceBaseState::onEnter();

	setApcGuidance();   
	roadmapStatusChanged();
	pciViewContext.crmGuidance.setGuidanceModel(_guidanceModel);
	show(&pciViewContext.crmGuidance);
}

ViewState CrmApcGuidanceState::roadmapStatusChanged()
{
	if ((pciViewContext.currentEpx.getType() == PciEpx::Type::NonPci) &&
		(pciViewContext.crmAdminStatus == CrmAdministration::Status::NoRoadmap))
	{
		return ViewState::NonPciEpxState;
	}

	_guidanceModel.roadMapImage = pciViewContext.currentRoadmap;
	_guidanceModel.roadmapActive = false;

	if (pciViewContext.currentRoadmap)
	{
		_guidanceModel.currentRoadmapAngulation = pciViewContext.currentRoadmap->geometry.angulation;
		_guidanceModel.currentRoadmapRotation   = pciViewContext.currentRoadmap->geometry.rotation;
	}
	else
	{
		_guidanceModel.currentRoadmapAngulation = 0;
		_guidanceModel.currentRoadmapRotation   = 0;
	}

	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
}

ViewState CrmApcGuidanceState::geometryEnd()
{
	auto state = CrmGuidanceBaseState::geometryEnd();

	setApcGuidance();
	return state;
}

ViewState CrmApcGuidanceState::geometryChanged()
{
	auto state = CrmGuidanceBaseState::geometryChanged();

	if (  pciViewContext.geometryIsMoving && 
		( pciViewContext.crmApcStatus == CrmApcController::ApcModeStatus::TargetSetWithoutActivaton) &&
		(!pciViewContext.crmApcAutomaticActivation))
	{
		_guidanceModel.messageHeaderText = L"";
		_guidanceModel.messagePicture = nullptr;
	}

	return state;
}

ViewState CrmApcGuidanceState::apcStatusChanged()
{
	setApcGuidance();
	return viewState;
}

void CrmApcGuidanceState::setApcGuidance()
{
	bool show = true;

	if(pciViewContext.crmApcStatus == CrmApcController::ApcModeStatus::AutomaticActivation )
	{
		_guidanceModel.messagePictureText = L"";
		if (pciViewContext.crmGuidance.getViewType() == ViewType::Allura)
		{
			_guidanceModel.messagePicture = &PciSuite::Resources::tso_acc;
		}
		else
		{
			_guidanceModel.messagePicture = &PciSuite::Resources::tso_acc_SmS;
		}
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_HoldACCToMoveToRoadmap;
	}
	else if(pciViewContext.crmApcStatus == CrmApcController::ApcModeStatus::TargetSetWithoutActivaton)
	{
		if ( pciViewContext.crmApcAutomaticActivation )
        {
            // show '3D-APC' when we are in automatic activation mode after we detected a status conflict and guide user to press
            // the TSM '3D-APC' button to enable automatic activation again.

			if (pciViewContext.crmGuidance.getViewType() == ViewType::Allura)
			{
				_guidanceModel.messagePictureText = Localization::Translation::IDS_Apc;
				_guidanceModel.messagePicture = &PciSuite::Resources::apc_3dapc;
				_guidanceModel.messageHeaderText = Localization::Translation::IDS_EnableAPCForAutoPositiong;
			}
			else
			{
				_guidanceModel.messagePictureText = L"";
				_guidanceModel.messagePicture = &PciSuite::Resources::XrayRotational140x140;

				if (pciViewContext.systemType == SystemType::SmartSuite_1x)
				{
					_guidanceModel.messageHeaderText = Localization::Translation::IDS_EnableAPCForAutoPositiongSmS;
				}
				else
				{
					_guidanceModel.messageHeaderText = Localization::Translation::IDS_EnableAPCForAutoPositiongAzurion21;
				}
				
			}

        }
        else
        {
            // show 'nothing' because we expect to have status conflicts when user prefers use of APC sequence.
			_guidanceModel.messagePictureText = L"";
			_guidanceModel.messagePicture = nullptr;
			_guidanceModel.messageHeaderText = L"";
			show = false;
        }
	}
	else if (pciViewContext.crmApcStatus == CrmApcController::ApcModeStatus::DisallowBecauseOfProgrammingConflict)
	{
		// let the user know this app does not use APC anymore, and user can manually move c-arc.
		_guidanceModel.messagePictureText = L"";
		_guidanceModel.messagePicture = nullptr;
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_ManuallyMoveCarc;
	}
	
	_guidanceModel.showMessagePanel = show;

}

}}