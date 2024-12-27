// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbPostDeployReviewState.h"
#include "Log.h"
#include "IArchivingSettings.h"
#include "PciViewContext.h"
#include "StbBoostViewer.h"
#include "StbMarkersViewer.h"
#include "StbAdministration.h"
#include "StbTsmPage.h"
#include "IUiStateController.h"

namespace Pci { namespace Core {


StbPostDeployReviewState::StbPostDeployReviewState(PciViewContext &viewContext, IUiStateController& stateController, Pci::Platform::iControl::ICopyToRefClient& copyToRefClient, const IArchivingSettings& archivingSettings) :
	StbReviewState(viewContext, ViewState::StbPostDeployReviewState, L"StbPostDeployReviewState", copyToRefClient, archivingSettings, LicenseFeature::StbPostDeploy),
	m_uiStateController(stateController)
{
    getTsmPage().defineRoiButton->visible = true;

	m_player.eventReviewImage = [this](int /*index*/, const StbResult& frame, bool /*lastFrameReached*/)
	{
		pciViewContext.stbAngioViewer.setImage(frame.live);
		pciViewContext.stbAngioViewer.setMarkers(frame.markers);
	};

	auto currentTsmButtonHandler = tsmPage->eventButtonReleased;
	tsmPage->eventButtonReleased = [this, currentTsmButtonHandler](unsigned int button)
	{
		if (currentTsmButtonHandler) currentTsmButtonHandler(button);

		if (button == StbTsmPage::DefineROIButton)
		{
			m_uiStateController.switchState(ViewState::StbPostDeployRoiDefinitionState);
		}
	};
}

StbPostDeployReviewState::~StbPostDeployReviewState()
{
}

void StbPostDeployReviewState::onEnter()
{
	StbReviewState::onEnter();

	if (hasBoostedImages())
	{
		m_player.setReview(m_stbRecording, m_stbRecording->getAllImages().front().live->frameTime);

		m_log.DeveloperInfo(L"Start review of StentBoost post deployment images [Nr.Images=%d, RunIndex=%d, SeriesNumber=%d, FrameTime=%f]",
			m_stbRecording->getImageCount(),
			m_stbRecording->getAllImages().front().live->runIndex,
			m_stbRecording->getAllImages().front().live->seriesNumber,
			m_stbRecording->getAllImages().front().live->frameTime);

		allowGeometryUpdates = false;
		m_player.startReview();
	}

	getTsmPage().defineRoiButton->enabled = true;
	getTsmPage().requestUpdate();

	show(&pciViewContext.stbViewer);
}

void StbPostDeployReviewState::onExit()
{
	StbReviewState::onExit();

	m_player.stopReview();
}

void StbPostDeployReviewState::prepareReview()
{
	StbReviewState::prepareReview();

	m_stbRecording->setIncludeImagesWithoutMarkers(true);

	pciViewContext.stbViewer.totalFrames = m_stbRecording->getImageCount();
	pciViewContext.stbViewer.boostedFrames = m_stbRecording->getBoostedImageCount();
	pciViewContext.stbViewer.showRunInfo = true;
	pciViewContext.stbViewer.showBoostedFrameCounter = true;

	m_stbRecording->setIncludeImagesWithoutMarkers(false);

	auto &boost = m_stbRecording->getLastBoostedImage();
	pciViewContext.stbViewer.setImage(boost.boost);
}

void StbPostDeployReviewState::initFrameGrabber()
{
	UiState::initFrameGrabber(pciViewContext.stbViewer.getCanvas(), pciViewContext.stbViewer);
}

void StbPostDeployReviewState::sendSnapshot()
{
	StbReviewState::sendSnapshot();
}

bool StbPostDeployReviewState::isCopyToRefEnabled() const
{
	return m_archivingSettings.isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostPostDeploy);
}

std::wstring StbPostDeployReviewState::getCopyToRefTargetName() const
{
	return m_archivingSettings.getCopyToRefViewer(IArchivingSettings::Feature::StentBoostPostDeploy);
}

bool StbPostDeployReviewState::isAutoPACSArchivingMovieEnabled() const
{
	return m_archivingSettings.isAutoPACSArchiveMovieEnabled(IArchivingSettings::Feature::StentBoostPostDeploy);
}

bool StbPostDeployReviewState::isAutoPACSArchivingSnapshotEnabled() const
{
	return m_archivingSettings.isAutoPACSArchiveSnapshotEnabled(IArchivingSettings::Feature::StentBoostPostDeploy);
}

}}