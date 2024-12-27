// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbContrastProcessingState.h"
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
#include "PciGuidanceView.h"
#include "Translation.h"

using namespace CommonPlatform;

namespace Pci {	namespace Core {

const double BoostPhaseTime(3.0);

StbContrastProcessingState::StbContrastProcessingState(
	PciViewContext &viewContext,
	IUiStateController& controller,
	Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
	const IArchivingSettings& archivingSettings)
		: StbProcessingState(viewContext, ViewState::StbContrastProcessingState, L"StbContrastProcessingState", LicenseFeature::StbSubtract, controller, copyToRefClient, archivingSettings),
	      boostPhaseTimer(viewContext.stbGuidance.getCanvas().getScheduler(), [&] { timeUpdate();  })
{
    getTsmPage().contrastFrameSelectButton->visible = true;
    getTsmPage().boostFrameSelectButton->visible = true;
    getTsmPage().defineRoiButton->visible = true;
    getTsmPage().invertBoostButton->visible = true;
}

StbContrastProcessingState::~StbContrastProcessingState()
{
}

void StbContrastProcessingState::onEnter()
{
	StbProcessingState::onEnter();

	Sense::Color color;
	if (pciViewContext.stbAdministration.lastImageIndex() == -1)
	{
		color = pciViewContext.stbGuidance.getViewType() == ViewType::Allura ? ExperienceIdentity::Palette::Black : ExperienceIdentity::Palette::Gray070;
	}
	else
	{
		color = pciViewContext.stbGuidance.getViewType() == ViewType::Allura ? ExperienceIdentity::Palette::White : ExperienceIdentity::Palette::Gray070;
	}

	if (!m_uiStateController.isReplayActive())
	{
		_guidanceModel.showMessagePanel = true;
		_guidanceModel.showSBSGuidance = true;

		_guidanceModel.messageHeaderColor = color;
		_guidanceModel.messageColor = color;

		_guidanceModel.messageHeaderIcon = nullptr;
		_guidanceModel.messageHeaderText = L"";

		_guidanceModel.stentBoostPhaseDuration = BoostPhaseTime;
		_guidanceModel.stentBoostTimeRemaining = BoostPhaseTime;
		_guidanceModel.stentBoostPhaseActive = true;
		_guidanceModel.contrastPhaseActive = false;

		_guidanceModel.currentGeometry = pciViewContext.geometry;

		pciViewContext.stbGuidance.setGuidanceModel(_guidanceModel);

		show(&pciViewContext.stbGuidance);

		boostPhaseTimer.start(0.05);

		getTsmPage().defineRoiButton->visible = true;
		getTsmPage().defineRoiButton->enabled = false;
		getTsmPage().requestUpdate();
	}
	else
	{
		_guidanceModel.showMessagePanel = false;
		_guidanceModel.showSBSGuidance = false;
	}
}

void StbContrastProcessingState::onExit()
{
	StbProcessingState::onExit();

	boostPhaseTimer.stop();

	getTsmPage().defineRoiButton->visible = true;
	getTsmPage().defineRoiButton->enabled = false;
	getTsmPage().requestUpdate();
}

void StbContrastProcessingState::timeUpdate()
{
	_guidanceModel.stentBoostTimeRemaining = std::max(0.0, BoostPhaseTime - boostPhaseTimer.getElapsed());
	_guidanceModel.contrastPhaseActive = boostPhaseTimer.getElapsed() >= BoostPhaseTime;

	if (boostPhaseTimer.getElapsed() >= BoostPhaseTime)
	{
		boostPhaseTimer.stop();

		_guidanceModel.messageHeaderIcon = &Sense::ExperienceIdentity::Icons::Information;
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_HoldCinePedalUntillVesselsFilledWithContrast;
	}
}

PciXrayPositionModel& StbContrastProcessingState::getModel()
{
	return _guidanceModel;
}

ViewState StbContrastProcessingState::getGuidanceState() const
{
	return ViewState::StbContrastGuidanceState;
}

ViewState StbContrastProcessingState::getReviewState() const
{
	return ViewState::StbContrastReviewState;

}

std::wstring StbContrastProcessingState::getCopyToRefTargetName() const
{
	return m_archivingSettings.getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract);
}

}}
