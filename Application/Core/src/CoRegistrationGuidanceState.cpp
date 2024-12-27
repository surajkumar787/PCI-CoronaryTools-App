// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegistrationGuidanceState.h"
#include "PciViewContext.h"
#include "CrmTsmPage.h"
#include "Translation.h"
#include "ViewStateHelper.h"
#include "Resources.h"
#include "pciGuidanceView.h"
#include "ViewType.h"


using namespace Sense::ExperienceIdentity;
using namespace CommonPlatform::CoRegistration;

namespace Pci {	namespace Core	{

const Sense::Image* getStatusIcon(CrmAdministration::Status status);
std::wstring getStatusText(CrmAdministration::Status status);

CoRegistrationGuidanceState::CoRegistrationGuidanceState(PciViewContext &viewContext) :
	UiState(viewContext, ViewState::CoRegistrationGuidanceState, L"CoRegistrationGuidanceState", LicenseFeature::Coregistration),
	_guidanceModel()
{
    CrmTsmPage* crmTsmPage = new CrmTsmPage();

    crmTsmPage->storeMovieButton->upperText = Localization::Translation::IDS_Start;
    crmTsmPage->storeMovieButton->lowerText = Localization::Translation::IDS_MovieRecord;

	tsmPage.reset(crmTsmPage);
	
	viewContext.coRegistrationGuidance.setGuidanceModel(_guidanceModel);
}

PciXrayPositionModel& CoRegistrationGuidanceState::getModel()
{
	return _guidanceModel;
}

void CoRegistrationGuidanceState::onEnter()
{
	Sense::Color info = pciViewContext.stbGuidance.getViewType() == ViewType::Allura ? Palette::White : Palette::Gray070;

	_guidanceModel.roadMapImage = pciViewContext.currentRoadmap;
	_guidanceModel.roadmapActive = pciViewContext.crmAdminStatus == CrmAdministration::Status::Active;
	enableTsmControls();

	_guidanceModel.showMessagePanel = true;
	_guidanceModel.showPositionPanel = false;
	_guidanceModel.showProcessingPanel = false;
	_guidanceModel.guidanceText = L"";
	_guidanceModel.messageColor = info;
	_guidanceModel.messageHeaderIcon = &Icons::Information;
	_guidanceModel.messageHeaderColor = info;
	_guidanceModel.messagePicture = &PciSuite::Resources::pedals_fluoro;
	_guidanceModel.statusIcon = getStatusIcon(pciViewContext.crmAdminStatus);
	roadmapStatusChanged();

	if ((pciViewContext.coRegState.fmState == CoRegistrationServerState::FmState::IFRSpot) ||
		(pciViewContext.coRegState.fmState == CoRegistrationServerState::FmState::IFRPullback))
	{
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_ReadyForiFRRoadmap;
		_guidanceModel.messageText = Localization::Translation::IDS_PressFluoroForWirePosition;
		_guidanceModel.showMessagePicturePanel = true;
	}

	if ((pciViewContext.coRegState.fmState == CoRegistrationServerState::FmState::FFR) && (!pciViewContext.fluoroAcquiredWhileRecording))
	{
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_ReadyForFFRRoadmap;
		_guidanceModel.messageText = Localization::Translation::IDS_PressFluoroForWirePosition;
		_guidanceModel.showMessagePicturePanel = true;
	}

	if ((pciViewContext.coRegState.systemState == CoRegistrationServerState::SystemState::SpotMeasurement) &&
		pciViewContext.fluoroAcquiredWhileRecording)
	{
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_WirePositionCaptured;
		_guidanceModel.messageText = Localization::Translation::IDS_KeepWirePosStable;
		_guidanceModel.showMessagePicturePanel = false;
	}

	show(&pciViewContext.coRegistrationGuidance);
}

ViewState CoRegistrationGuidanceState::angioProcessingStarted()
{
	return ViewState::CrmProcessingAngioState;
}

ViewState CoRegistrationGuidanceState::overlayImageAvailable()
{
	return ViewState::CoRegistrationState;
}

ViewState CoRegistrationGuidanceState::coRegistrationStateChanged(const CoReg::CoRegistrationServerState& /* prevState */,
	const CoReg::CoRegistrationServerState& nextState)
{
	if (!isPressureWireConnectedAndReady(nextState))
	{
		return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
	}

	enableTsmControls();

	switch (nextState.systemState)
	{
	case CoRegistrationServerState::SystemState::Live:
		pciViewContext.roadmapErrorState = PciSuite::CrmErrors::Error::OK;
		return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
		break;
	case CoRegistrationServerState::SystemState::Review:
		pciViewContext.roadmapErrorState = PciSuite::CrmErrors::Error::OK;

		if (pciViewContext.fluoroAcquiredWhileRecording && !pciViewContext.forceHideCoregControl && pciViewContext.coRegError != CoRegistrationError::UnreliableDataTransfer)
		{
			return ViewState::CoRegistrationState;
		}
		else
		{
			return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
		}
		break;
	default:
		break;
	}
	return viewState;
}

ViewState CoRegistrationGuidanceState::roadmapStatusChanged()
{
	_guidanceModel.roadMapImage = pciViewContext.currentRoadmap;
	_guidanceModel.roadmapActive = pciViewContext.crmAdminStatus == CrmAdministration::Status::Active;
	_guidanceModel.showPositionPanel = pciViewContext.crmAdminStatus == CrmAdministration::Status::MoveArc;
	if (pciViewContext.currentRoadmap)
	{
		_guidanceModel.currentRoadmapAngulation = pciViewContext.currentRoadmap->geometry.angulation;
		_guidanceModel.currentRoadmapRotation = pciViewContext.currentRoadmap->geometry.rotation;
	}
	_guidanceModel.status = getStatusText(pciViewContext.crmAdminStatus);
	_guidanceModel.statusIcon = getStatusIcon(pciViewContext.crmAdminStatus);

	return viewState;
}

const Sense::Image* getStatusIcon(CrmAdministration::Status status)
{
	if (status == CrmAdministration::Status::NoRoadmap)
	{
		return &PciSuite::Resources::ThumbPreview;;
	}
	return nullptr;
}

std::wstring getStatusText(CrmAdministration::Status status)
{
	std::wstring text(L"");
	if (status == CrmAdministration::Status::MoveArc ||
		status == CrmAdministration::Status::MoveTable)
	{
		text = Localization::Translation::IDS_NearestRoadmap;
	}
	else if (status == CrmAdministration::Status::NoRoadmap)
	{
		text = Localization::Translation::IDS_NoRoadmapAvailable;
	}
	else
	{
		text = Localization::Translation::IDS_RoadmapReady;
	}
	return text;
}


void Pci::Core::CoRegistrationGuidanceState::enableTsmControls()
{
	enableArchiveControlsOnTsm();
	getTsmPage().requestUpdate();
}

void Pci::Core::CoRegistrationGuidanceState::enableArchiveControlsOnTsm()
{
	bool visibilityMoviesButton = pciViewContext.coRegState.fmState == ::CommonPlatform::CoRegistration::CoRegistrationServerState::FmState::IFRPullback;
	getTsmPage().storeMovieButton->visible = visibilityMoviesButton;
}

ViewState CoRegistrationGuidanceState::geometryChanged()
{
	UiState::geometryChanged();
	pciViewContext.roadmapErrorState = PciSuite::CrmErrors::Error::OK;
	_guidanceModel.currentGeometry = pciViewContext.geometry;
	return viewState;
}

ViewState CoRegistrationGuidanceState::geometryEnd()
{
	UiState::geometryEnd();
	pciViewContext.roadmapErrorState = PciSuite::CrmErrors::Error::OK;
	return viewState;
}

PciTsmPage& CoRegistrationGuidanceState::getTsmPage() const
{
	return *static_cast<PciTsmPage*>(tsmPage.get());
}

}}