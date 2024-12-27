// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbReviewState.h"
#include "Log.h"
#include "UtilizationLogEvent.h"
#include "IArchivingSettings.h"
#include "PciViewContext.h"
#include "StbBoostViewer.h"
#include "ViewStateHelper.h"
#include "StbTsmPage.h"
#include "StbAdministration.h"
#include "MovieGrabber.h"
#include "IArchivingStatus.h"
#include "StbRecording.h"
#include "StbMarkersViewer.h"
#include "FrameGrabber.h"
#include "PACSArchiver.h"
#include "ICopyToRefClient.h"
#include "Snapshot.h"
#include "Bitmap.h"
#include "Translation.h"
#include "stlstring.h"
using namespace Pci::Core::PACS;

namespace Pci { namespace Core {

StbReviewState::StbReviewState(
	PciViewContext &viewContext,
	ViewState state,
	const std::wstring &name,
	Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
	const IArchivingSettings& archivingSettings,
	const LicenseFeature license)
		: UiState(viewContext, state, name, license),
		m_log(CommonPlatform::Logger(CommonPlatform::LoggerType::StentboostLive)),
		m_archivingSettings(archivingSettings),
		m_copyToRefClient(copyToRefClient),
		m_player(viewContext.stbViewer.getCanvas().getScheduler(), viewContext.stbReviewParameters)
{
	tsmPage.reset(new StbTsmPage());

	tsmPage->eventButtonReleased = [this](unsigned int button)
	{
		if (button == PciTsmPage::SnapshotButton)
		{
			sendSnapshot();
		}
		if (button == PciTsmPage::MovieButton)
		{
			if (m_movie)
			{
				sendMovie();
			}
			else
			{
				recordMovie();
			}
			getTsmPage().storeMovieButton->enabled = false;
		}
		if (button == StbTsmPage::CopyToRefButton)
		{
			sendSnapshotToRefViewer();
		}
	};

	m_copyToRefClient.subscribeConnectionEvents(stateName, [this](bool connected) 
														   { 
																getTsmPage().copyToRefButton->enabled = connected; 
																getTsmPage().requestUpdate();
														   });
}

StbReviewState::~StbReviewState()
{
	m_copyToRefClient.unsubscribeConnectionEvents(stateName);
}

void StbReviewState::onEnter()
{
	// Reset m_movie because the user might have changed something in the result while in a different state, resulting in a different movie
	m_movie.reset();

	m_boostModel.showLastImageHold = true;

	pciViewContext.stbViewer.setXrayModel(m_boostModel);
	
	m_stbRecording = pciViewContext.stbAdministration.prepareReview();
	m_stbRecording->setIncludeImagesWithoutMarkers(false);

	prepareReview();

	if (hasBoostedImages())
	{
		getTsmPage().storeMovieButton->enabled = pciViewContext.moviesAvailable;
		getTsmPage().storeSnapshotButton->enabled = pciViewContext.snapshotsAvailable;

		getTsmPage().copyToRefButton->enabled = m_copyToRefClient.isConnected();
		m_log.DeveloperInfo(L"StbReviewState::onEnter : TsmPage Button enabled status, storeMovieButton = %d, storeSnapshotButton = %d, copyToRefButton = %d", 
			getTsmPage().storeMovieButton->enabled,
			getTsmPage().storeSnapshotButton->enabled,
			getTsmPage().copyToRefButton->enabled);

		if (!m_stbRecording->isAutoArchived())
		{
			archiveRun();
            m_stbRecording->setAutoArchived();
		}
	}
	else
	{
		m_log.DeveloperInfo(L"StbReviewState::onEnter : TsmPage Button enabled status for storeMovieButton, storeSnapshotButton, copyToRefButton are set to false");
		getTsmPage().storeMovieButton->enabled = false;
		getTsmPage().storeSnapshotButton->enabled = false;
		getTsmPage().copyToRefButton->enabled = false;
	}

	auto refTargetName = getCopyToRefTargetName();
	auto refTargetNr = m_archivingSettings.getCopyToRefTargetNumber(m_log, refTargetName);
	auto targetNumber = refTargetNr == -1 ? L"" : std::to_wstring(refTargetNr);
    getTsmPage().copyToRefButton->centerText = str_ext::replace(Localization::Translation::IDS_CopyToReference, L"{0}", targetNumber);
	getTsmPage().copyToRefButton->visible = m_copyToRefClient.isAvailable();
	m_log.DeveloperInfo(L"Current value of TsmPage copyToRefButton visible : %d", getTsmPage().copyToRefButton->visible);

	getTsmPage().requestUpdate();

	autoReactivate = false;
}

void StbReviewState::onExit()
{
}

ViewState StbReviewState::imageRunStarted(CommonPlatform::Xray::XrayImage::Type imageType)
{
	if (pciViewContext.currentEpx.getType() != PciEpx::Type::StentBoost)
	{
		return UiState::imageRunStarted(imageType);
	}

	if (imageType == XrayImage::Type::Exposure) 
	{
		return getProcessingState();
	}
	
	return viewState;
}

ViewState StbReviewState::getProcessingState() const
{
	ViewState newstate = ViewState::StbLiveProcessingState;
	if (pciViewContext.currentEpx.getSubType() == PciEpx::SubType::PostDeployment)
	{
		newstate = ViewState::StbPostDeployProcessingState;
	}
	else if(pciViewContext.currentEpx.getSubType() == PciEpx::SubType::Subtract)
	{
		newstate = ViewState::StbContrastProcessingState;
	}
	return newstate;
}

ViewState StbReviewState::coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState, const CoReg::CoRegistrationServerState& nextState)
{
	UiState::coRegistrationStateChanged(prevState, nextState);
	return viewState;
}

ViewState StbReviewState::coRegistrationConnectionChanged(bool connected)
{
	UiState::coRegistrationConnectionChanged(connected);
	return viewState;
}

ViewState StbReviewState::overlayImageAvailable()
{
	autoReactivate = true;
	if (pciViewContext.crmAdminStatus == CrmAdministration::Status::Active)
	{
		return ViewState::CrmOverlayState;
	}

	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
}

ViewState StbReviewState::epxChanged()
{
	return viewState;
}

PciXrayPositionModel& StbReviewState::getModel()
{
	return m_boostModel;
}

void StbReviewState::archiveRun()
{
	initFrameGrabber();

	if (isAutoPACSArchivingMovieEnabled() && pciViewContext.moviesAvailable)
	{
		recordMovie();
	}

	if (isAutoPACSArchivingSnapshotEnabled() && pciViewContext.snapshotsAvailable)
	{
		sendSnapshot();
	}

	if (isCopyToRefEnabled())
	{
		sendSnapshotToRefViewer();
	}
}

StbTsmPage& StbReviewState::getTsmPage() const
{
	return *static_cast<StbTsmPage*>(tsmPage.get());
}

bool StbReviewState::hasBoostedImages() const
{
	return (m_stbRecording != nullptr) && (m_stbRecording->getImageCount() != 0);
}

std::unique_ptr<PACS::MovieGrabber> StbReviewState::createMovieGrabber()
{
	std::unique_ptr<PACS::MovieGrabber> moviegrabber(nullptr);

	if (pciViewContext.pacsPush != nullptr && pciViewContext.msgBuilder != nullptr && pciViewContext.threadEncoding != nullptr)
	{
		moviegrabber = std::make_unique<PACS::MovieGrabber>(pciViewContext.stbViewer.getCanvas().getScheduler(), *pciViewContext.threadEncoding, *pciViewContext.pacsPush, *pciViewContext.msgBuilder, pciViewContext.stbViewer.getViewType());
	}

	return std::move(moviegrabber);
}

void StbReviewState::initMovieGrabber()
{
	if (m_movieGrabber) return;
	if (!pciViewContext.moviesAvailable) return;

	m_movieGrabber = createMovieGrabber();

	if (m_movieGrabber == nullptr) return;

	m_movieGrabber->onGrabbingDone = [this](std::shared_ptr<PACS::ImageSequence> movie)
	{
		m_movie = std::move(movie);
		if (m_movie != nullptr)
		{
			sendMovie();
		}
		else
		{
			m_log.DeveloperError(L"StbProcessingState::onGrabbingDone: movie grabbing failed, unable to export movie");
			if (m_archivingStatus)
			{
				m_archivingStatus->setMovieStatus(ArchivingStatus::Failure);
			}
		}
	};

	m_movieGrabber->onSent = [this](bool success, const std::shared_ptr<const PACS::ImageSequence>& movie)
	{
		onMovieGrabberFinished(success, movie);
	};
}

void StbReviewState::prepareReview()
{
	if (!m_stbRecording) return;

	auto& frames = m_stbRecording->getAllImages();

	auto& view = pciViewContext.stbViewer;
	auto& sideView = pciViewContext.stbAngioViewer;

	auto it = std::find_if(std::rbegin(frames), std::rend(frames), [](const StbResult& stbResult) { return stbResult.markers->valid; });
	if (it == std::rend(frames)) return;

	view.totalFrames = static_cast<int>(frames.size());
	view.showRunInfo = true;

	auto lastBoostedFrame = static_cast<int>(std::distance(std::begin(frames), it.base()) - 1);
	view.currentFrame = lastBoostedFrame;

	view.setImage(it->boost);
	sideView.setImage(it->live);
	sideView.setMarkers(it->markers);

	m_boostModel.showLastImageHold = true;

	show(&view);

	m_log.DeveloperInfo(L"StbProcessingState::prepareReview recording check [total frames: %llu, last boosted frame: %d, final frame showWarning: %d]",
		frames.size(), lastBoostedFrame, frames.back().boost ? frames.back().boost->showWarning : true);
}

void StbReviewState::sendSnapshot()
{
	getTsmPage().storeSnapshotButton->enabled = false;
	getTsmPage().requestUpdate();

	if (m_pacsArchiver == nullptr)
	{ 
		initPACSArchiving(pciViewContext.stbViewer.getCanvas().getScheduler());
	}	

	if (m_frameGrabber == nullptr) return;

	if (m_archivingStatus != nullptr)
	{
		m_archivingStatus->setSnapshotStatus(ArchivingStatus::Busy, CaptureArea::MainView);
	}

	auto frame = m_frameGrabber->grabSingleFrame();
	m_pacsArchiver->sendSnapshot(frame, m_stbRecording->getSeriesNumber(), RunType::SBL);

	CommonPlatform::UtilizationLogEvent::ArchiveSnapshotSbl(m_log);
}

void StbReviewState::sendSnapshotToRefViewer()
{
	const bool showLastImageHoldState = m_boostModel.showLastImageHold;

	pciViewContext.stbViewer.showRunInfo = false;
	m_boostModel.showLastImageHold = false;
	
	auto frame = m_frameGrabber->grabSingleFrame();
	
	pciViewContext.stbViewer.showRunInfo = true;
	m_boostModel.showLastImageHold = showLastImageHoldState;
	
	Pci::Platform::iControl::Snapshot snapshot;
	snapshot.acquisitionTime = std::chrono::system_clock::now();
	CommonPlatform::Utilities::Bitmap::fromRgb32ToBmp24(frame->getPixelData(), snapshot.imageData, frame->getWidth(), frame->getHeight(), 0);

	m_copyToRefClient.sendSnapshot(pciViewContext.currentPatient, getCopyToRefTargetName(), snapshot);
}

void StbReviewState::recordMovie()
{
	getTsmPage().storeMovieButton->enabled = false;
	getTsmPage().requestUpdate();

	initMovieGrabber();
	if (m_movieGrabber == nullptr) return;

	if (m_archivingStatus != nullptr)
	{
		m_archivingStatus->setMovieStatus(ArchivingStatus::Busy);
	}

	m_movieGrabber->grabMovie(m_stbRecording, 15, pciViewContext.stbViewer.size);
}

void StbReviewState::sendMovie()
{
	if (m_movieGrabber == nullptr) return;

	if (m_archivingStatus != nullptr)
	{
		m_archivingStatus->setMovieStatus(ArchivingStatus::Busy);
	}

	m_movieGrabber->sendMovie(m_movie);
	CommonPlatform::UtilizationLogEvent::ArchiveMovieSbl(m_log);
}

void StbReviewState::onFrameGrabberFinished(PACS::GrabType grabType, bool finishedSuccessfully)
{
	if (m_archivingStatus != nullptr)
	{
		auto status = finishedSuccessfully ? ArchivingStatus::Success : ArchivingStatus::Failure;

		if (grabType == PACS::GrabType::SingleFrame) m_archivingStatus->setSnapshotStatus(status, CaptureArea::MainView);
		if (grabType == PACS::GrabType::MultiFrame)  m_archivingStatus->setMovieStatus(status);
	}

	if (grabType == PACS::GrabType::SingleFrame)
	{
		getTsmPage().storeSnapshotButton->enabled = hasBoostedImages();
		getTsmPage().requestUpdate();
	}
}

void StbReviewState::onMovieGrabberFinished(bool finishedSuccessfully, const std::shared_ptr<const PACS::ImageSequence>& movie)
{
	if (m_archivingStatus != nullptr)
	{
		auto status = finishedSuccessfully ? ArchivingStatus::Success : ArchivingStatus::Failure;
		m_archivingStatus->setMovieStatus(status);
	}

	if (m_movie == movie)
	{
		m_movie = nullptr;
	}

	getTsmPage().storeMovieButton->enabled = hasBoostedImages();
	getTsmPage().requestUpdate();
}

}}