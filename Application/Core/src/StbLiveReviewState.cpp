// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbLiveReviewState.h"
#include "Log.h"
#include "IArchivingSettings.h"
#include "PciViewContext.h"
#include "StbBoostViewer.h"
#include "StbMarkersViewer.h"
#include "StbAdministration.h"
#include "StbTsmPage.h"

namespace Pci { namespace Core {


StbLiveReviewState::StbLiveReviewState(PciViewContext &viewContext, Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,const IArchivingSettings& archivingSettings) :
	StbReviewState(viewContext, ViewState::StbLiveReviewState, L"StbLiveReviewState", copyToRefClient, archivingSettings, LicenseFeature::StentboostLive)
{
	m_player.eventReviewImage = [this](int index, const StbResult& frame, bool /*lastFrameReached*/)
	{
		pciViewContext.stbViewer.currentFrame = index;
		pciViewContext.stbViewer.setImage(frame.boost);
		pciViewContext.stbAngioViewer.setImage(frame.live);
		pciViewContext.stbAngioViewer.setMarkers(frame.markers);
	};
}

StbLiveReviewState::~StbLiveReviewState()
{
}

void StbLiveReviewState::onEnter()
{
	show(&pciViewContext.stbViewer);

	StbReviewState::onEnter();

	if (hasBoostedImages())
	{
		pciViewContext.stbViewer.totalFrames = m_stbRecording->getImageCount();
		pciViewContext.stbViewer.showBoostedFrameCounter = false;
		pciViewContext.stbViewer.showRunInfo = true;

		m_player.setReview(m_stbRecording, m_stbRecording->getAllImages().front().live->frameTime);

		m_log.DeveloperInfo(L"Start review of StentBoost live images [Nr.Images=%d, RunIndex=%d, SeriesNumber=%d, FrameTime=%f]",
			m_stbRecording->getImageCount(),
			m_stbRecording->getAllImages().front().live->runIndex,
			m_stbRecording->getAllImages().front().live->seriesNumber,
			m_stbRecording->getAllImages().front().live->frameTime);

		allowGeometryUpdates = false;
		m_player.startReview();
	}
}

void StbLiveReviewState::onExit()
{
	StbReviewState::onExit();

	m_player.stopReview();
}

void StbLiveReviewState::initFrameGrabber()
{
	UiState::initFrameGrabber(pciViewContext.stbViewer.getCanvas(), pciViewContext.stbViewer);
}

bool StbLiveReviewState::isCopyToRefEnabled() const
{
	return m_archivingSettings.isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive);
}

std::wstring StbLiveReviewState::getCopyToRefTargetName() const
{
	return m_archivingSettings.getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive);
}

bool StbLiveReviewState::isAutoPACSArchivingMovieEnabled() const
{
	return m_archivingSettings.isAutoPACSArchiveMovieEnabled(IArchivingSettings::Feature::StentBoostLive);
}

bool StbLiveReviewState::isAutoPACSArchivingSnapshotEnabled() const
{
	return m_archivingSettings.isAutoPACSArchiveSnapshotEnabled(IArchivingSettings::Feature::StentBoostLive);
}

}}