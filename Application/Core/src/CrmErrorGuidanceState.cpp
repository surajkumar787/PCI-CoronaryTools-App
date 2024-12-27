// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmErrorGuidanceState.h"
#include "PciViewContext.h"
#include "ViewStateHelper.h"
#include "Translation.h"
#include "Resources.h"
#include "PciXrayViewer.h"
#include "pciGuidanceView.h"
#include "IUiStateController.h"
#include "CrmTsmPage.h"

using namespace Sense::ExperienceIdentity;
namespace Pci { namespace Core {

CrmErrorGuidanceState::CrmErrorGuidanceState(PciViewContext &viewContext, IUiStateController& uiStateController) :
	CrmGuidanceBaseState(viewContext, ViewState::CrmErrorGuidanceState, L"CrmErrorGuidanceState")
{
	_guidanceModel.showMessagePanel = true;
	_guidanceModel.showMessagePicturePanel = false;
	_guidanceModel.roadmapActive = false;
	_guidanceModel.showProcessingPanel = false;	
	_guidanceModel.messageColor = getInfoColour(pciViewContext.crmGuidance.getViewType());
	_guidanceModel.messageHeaderText = Localization::Translation::IDS_RoadmapFailed;
	_guidanceModel.messageHeaderColor = getInfoColour(pciViewContext.crmGuidance.getViewType());
	_guidanceModel.messageHeaderIcon = &Icons::GeneralWarning;

	getTsmPage().roadmapOptionsButton->visible = true;

	getTsmPage().eventButtonReleased = [this, &uiStateController](unsigned int button)
	{
		if (button == CrmTsmPage::RoadmapSelectionButton)
		{
			uiStateController.switchState(ViewState::CrmRoadmapActive);
		}
	};
}

void CrmErrorGuidanceState::onEnter()
{
    CrmGuidanceBaseState::onEnter();

	getTsmPage().roadmapOptionsButton->enabled = pciViewContext.crmAdminStatus == CrmAdministration::Status::Active;
	getTsmPage().requestUpdate();

	setRoadMapState(pciViewContext.crmAdminStatus);
	setErrorText();	
	pciViewContext.crmGuidance.setGuidanceModel(_guidanceModel);
	show(&pciViewContext.crmGuidance);
}

ViewState CrmErrorGuidanceState::roadmapStatusChanged()
{
	if ((pciViewContext.currentEpx.getType() == PciEpx::Type::NonPci) &&
		(pciViewContext.crmAdminStatus == CrmAdministration::Status::NoRoadmap))
	{
		return ViewState::NonPciEpxState;
	}

	_guidanceModel.roadMapImage = pciViewContext.currentRoadmap;
	_guidanceModel.roadmapActive = false;

	if (pciViewContext.crmAdminStatus == CrmAdministration::Status::Active)
	{
		pciViewContext.roadmapErrorState = PciSuite::CrmErrors::Error::OK;
	}

	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
}


ViewState CrmErrorGuidanceState::studyChanged()
{
	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
}


void CrmErrorGuidanceState::setRoadMapState( CrmAdministration::Status crmAdminStatus)
{
	_guidanceModel.showPositionPanel = crmAdminStatus == CrmAdministration::Status::MoveArc; 

	if (crmAdminStatus == CrmAdministration::Status::Active )
	{
		_guidanceModel.roadMapImage		= pciViewContext.currentRoadmap;
		_guidanceModel.roadmapActive	= true;
		_guidanceModel.statusIcon		= nullptr;
		_guidanceModel.status	= Localization::Translation::IDS_PreviousRoadmap;
	}
	else if ( crmAdminStatus == CrmAdministration::Status::MoveArc )
	{
		_guidanceModel.roadMapImage             = pciViewContext.currentRoadmap;
		_guidanceModel.roadmapActive            = false;
		_guidanceModel.statusIcon               = nullptr;
		_guidanceModel.currentRoadmapAngulation = pciViewContext.currentRoadmap->geometry.angulation;
		_guidanceModel.currentRoadmapRotation   = pciViewContext.currentRoadmap->geometry.rotation;
		_guidanceModel.status            = Localization::Translation::IDS_NearestRoadmap;
	}
	else 
	{
		_guidanceModel.roadMapImage		= nullptr;
		_guidanceModel.statusIcon		= &PciSuite::Resources::ThumbPreview;
		_guidanceModel.status	= Localization::Translation::IDS_ReadyToReceiveRoadmap;
	}
}

void CrmErrorGuidanceState::setErrorText()
{
	if ( pciViewContext.crmAdminStatus == CrmAdministration::Status::Active ||
		 pciViewContext.crmAdminStatus == CrmAdministration::Status::MoveArc )
	{
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_NewRoadmapFailed;
	}
	else
	{
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_RoadmapFailed;
	}
	_guidanceModel.messageHeaderColor = getInfoColour(pciViewContext.crmGuidance.getViewType());
	_guidanceModel.messageHeaderIcon = &Icons::GeneralWarning;

	switch (pciViewContext.roadmapErrorState)
	{
	case PciSuite::CrmErrors::Error::SaveFailure:
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_UnknownRoadmapFailure;
		_guidanceModel.messageText = Localization::Translation::IDS_FailedToSaveRoadmap;
		_guidanceModel.guidanceText = L"";
		break;
	case PciSuite::CrmErrors::Error::Unknown:
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_UnknownRoadmapFailure;
		_guidanceModel.messageText  = L"";
		_guidanceModel.guidanceText = L"";
		break;
	case PciSuite::CrmErrors::Error::NoInjection:
		_guidanceModel.messageText = Localization::Translation::IDS_NoContrast;
		_guidanceModel.guidanceText = Localization::Translation::IDS_ContrastFillAllVessels;
		break;
	case PciSuite::CrmErrors::Error::InjectionTooShort:
	case PciSuite::CrmErrors::Error::NoHeartCycle:
		_guidanceModel.messageText = Localization::Translation::IDS_NotEnoughContrast;
		_guidanceModel.guidanceText =  Localization::Translation::IDS_CaptureThreeHeartbeatsWithContrast;
		break;
	case PciSuite::CrmErrors::Error::InjectionNotWellDefined:
		_guidanceModel.messageText = Localization::Translation::IDS_TooManyContrastFluctuations;
		_guidanceModel.guidanceText =  Localization::Translation::IDS_SteadyContrastInjection;
		break;
	case PciSuite::CrmErrors::Error::PatientTableMoved:
		_guidanceModel.messageText = Localization::Translation::IDS_TabledMovedDuringAngio;
		_guidanceModel.guidanceText =  Localization::Translation::IDS_CreateAngioWithoutMovingTable;
		break;
	case PciSuite::CrmErrors::Error::CarmMoved:
		_guidanceModel.messageText = Localization::Translation::IDS_CArcMovedDuringAngio;
		_guidanceModel.guidanceText =  Localization::Translation::IDS_CreateAngioWithoutMovingCArc;
		break;
	case PciSuite::CrmErrors::Error::CarmAndTableMoved:
		_guidanceModel.messageText = Localization::Translation::IDS_CArcAndTableMovedDuringAngio;
		_guidanceModel.guidanceText =  Localization::Translation::IDS_CreateAngioWithoutMovingCArcAndTable;
		break;
	case PciSuite::CrmErrors::Error::AngioTooShort:
		_guidanceModel.messageText = Localization::Translation::IDS_ExposureTooShort;
		_guidanceModel.guidanceText =  Localization::Translation::IDS_CaptureThreeHeartbeats;
		break;
	}

}

}}

