// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbGuidanceState.h"
#include "PciViewContext.h"
#include "StbAdministration.h"
#include "ViewStateHelper.h"
#include "pciGuidanceView.h"
#include "ViewType.h"
#include "StbTsmPage.h"
#include "ICopyToRefClient.h"
#include "Translation.h"
#include "stlstring.h"

namespace Pci{ namespace Core{
using namespace Sense::ExperienceIdentity;

StbGuidanceState::StbGuidanceState(
	PciViewContext &viewContext,
	ViewState state,
	const std::wstring &name,
	const LicenseFeature license,
	Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
	const IArchivingSettings& archivingSettings) :
		UiState(viewContext, state, name, license),
		_guidanceModel(),
		m_archivingSettings(archivingSettings),
		m_log(CommonPlatform::Logger(CommonPlatform::LoggerType::StentboostLive))
{
	tsmPage.reset(new StbTsmPage());
	getTsmPage().copyToRefButton->visible = copyToRefClient.isAvailable();

    _guidanceModel.showProcessingPanel = false;
	_guidanceModel.showPositionPanel = false;
	_guidanceModel.showMessagePanel = true;
	_guidanceModel.showMessagePicturePanel = true;
	_guidanceModel.statusIcon = nullptr;
}

PciXrayPositionModel& StbGuidanceState::getModel()
{
	return _guidanceModel;
}

void StbGuidanceState::onEnter()
{
	Sense::Color color;
	if (pciViewContext.stbAdministration.lastImageIndex() == -1)
	{
		color = pciViewContext.stbGuidance.getViewType() == ViewType::Allura ? Palette::Black : Palette::Gray070;
	}
	else
	{
		color = pciViewContext.stbGuidance.getViewType() == ViewType::Allura ? Palette::White : Palette::Gray070;
	}
	_guidanceModel.messageHeaderColor = color;
	_guidanceModel.messageColor = color;

	pciViewContext.stbGuidance.setGuidanceModel(_guidanceModel);
	_guidanceModel.currentGeometry = pciViewContext.geometry;
	show(&pciViewContext.stbGuidance);

	auto refTargetName = getCopyToRefTargetName();
	auto refTargetNr = m_archivingSettings.getCopyToRefTargetNumber(m_log, refTargetName);
	auto targetNumber = refTargetNr == -1 ? L"" : std::to_wstring(refTargetNr);
	getTsmPage().copyToRefButton->centerText = str_ext::replace(Localization::Translation::IDS_CopyToReference, L"{0}", targetNumber);

	autoReactivate = false;
}

ViewState StbGuidanceState::overlayImageAvailable()
{
	autoReactivate = true;
	if (pciViewContext.crmAdminStatus == CrmAdministration::Status::Active)
	{
		return ViewState::CrmOverlayState;
	}

	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
}

ViewState StbGuidanceState::epxChanged()
{
	pciViewContext.stbAdministration.newRun();
	return UiState::epxChanged();
}

ViewState StbGuidanceState::coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState,
	const CoReg::CoRegistrationServerState& nextState)
{
	UiState::coRegistrationStateChanged(prevState, nextState);
	return viewState;
}

ViewState StbGuidanceState::coRegistrationConnectionChanged(bool connected)
{
	UiState::coRegistrationConnectionChanged(connected);
	return viewState;
}

StbTsmPage& StbGuidanceState::getTsmPage() const
{
	return *static_cast<StbTsmPage*>(tsmPage.get());
}

}}