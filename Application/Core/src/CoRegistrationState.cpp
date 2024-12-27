// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegistrationState.h"
#include "PciViewContext.h"
#include "PciXrayViewer.h"
#include "PciTsmPage.h"
#include "TsmButtonDefinitions.h"
#include "PciReviewPlayer.h"
#include "IArchivingStatus.h"
#include "FrameGrabber.h"
#include "PACSArchiver.h"
#include "Translation.h"
#include "UtilizationLogEvent.h"
#include "ViewStateHelper.h"
#include "PullbackRecording.h"
#include "CrmTsmPage.h"

#include <assert.h>

using namespace Pci::Core::PACS;
using namespace CommonPlatform;
using namespace CommonPlatform::CoRegistration;
namespace Pci { namespace Core {

static const uint8_t MovieFrameRate = 15;

CoRegistrationState::CoRegistrationState(PciViewContext &viewContext) :
	UiState(viewContext, ViewState::CoRegistrationState, L"CoRegistrationState", LicenseFeature::Coregistration),
	movieRecordingBusy(false),
	log(::CommonPlatform::Logger(::CommonPlatform::LoggerType::CoronaryRoadmap))
{
	pciViewContext.coRegViewer.setXrayModel(model);
	
	CrmTsmPage* crmTsmPage = new CrmTsmPage();
	
	crmTsmPage->storeMovieButton->upperText = Localization::Translation::IDS_Start;
	crmTsmPage->storeMovieButton->lowerText = Localization::Translation::IDS_MovieRecord;

	crmTsmPage->eventButtonPressed = [this](unsigned int button, bool /*repeated*/)
	{
		if (button == CrmTsmPage::FrameForwardButton)
		{
			if (pciViewContext.coRegistrationPlayer.inReview())
			{
				stopReview();
			}

			pciViewContext.coRegistrationPlayer.nextFrame();
			::UtilizationLogEvent::NextFrame(log);
		}

		if (button == CrmTsmPage::FrameBackButton)
		{
			if (pciViewContext.coRegistrationPlayer.inReview())
			{
				stopReview();
			}

			pciViewContext.coRegistrationPlayer.previousFrame();
			::UtilizationLogEvent::PreviousFrame(log);
		}
	};

	crmTsmPage->eventButtonReleased = [this](unsigned int button)
	{
		if (button == PciTsmPage::SnapshotButton)
		{
			if (!m_frameGrabber)
			{
				initFrameGrabber(pciViewContext.coRegViewer.getCanvas(), pciViewContext.coRegViewer, pciViewContext.coRegistrationControl);
			}

			if (!m_pacsArchiver)
			{
				initPACSArchiving(pciViewContext.coRegViewer.getCanvas().getScheduler());
			}

			if (m_frameGrabber)
			{
				if (m_archivingStatus)
				{
					m_archivingStatus->setSnapshotStatus(ArchivingStatus::None, CaptureArea::MainAndSideView);
				}
				
				auto frame = m_frameGrabber->grabSingleFrame();
				if (m_archivingStatus)
				{
					m_archivingStatus->setSnapshotStatus(ArchivingStatus::Busy, CaptureArea::MainAndSideView);
				}
				
				m_pacsArchiver->sendSnapshot(frame, pciViewContext.coRegViewer.getImage()->seriesNumber,RunType::DCR);
				::UtilizationLogEvent::ArchiveSnapshotCrm(log);
			}
		}
		if (button == PciTsmPage::MovieButton)
		{
			if (!m_frameGrabber)
			{
				initFrameGrabber(pciViewContext.coRegViewer.getCanvas(), pciViewContext.coRegViewer, pciViewContext.coRegistrationControl);
			}

			if (!m_pacsArchiver)
			{
				initPACSArchiving(pciViewContext.coRegViewer.getCanvas().getScheduler());
			}

			if (m_frameGrabber)
			{
				if (movieRecordingBusy)
				{
					// Stop Movie recording
					stopRecordingMovie();
				}
				else
				{
					// Start movie recording
					startRecordingMovie();
				}
			}
		}

		if (button == CrmTsmPage::PlayPauseButton)
		{
			if (pciViewContext.coRegistrationPlayer.inReview())
			{
				stopReview();
			}
			else
			{
				startReview();
			}

			::UtilizationLogEvent::PlayStop(log);
		}
	};
	
	tsmPage.reset(crmTsmPage);
}

void CoRegistrationState::startRecordingMovie()
{
	movieRecordingBusy = true;
	getTsmPage().storeMovieButton->upperText = Localization::Translation::IDS_Stop;
	getTsmPage().storeMovieButton->blinking = true;
	getTsmPage().requestUpdate(); // do explicitly here because startMultiFrame() would delay the button update

	if (m_archivingStatus)
	{
		m_archivingStatus->setMovieStatus(ArchivingStatus::None);
	}

	m_frameGrabber->onMultiFrameStopped = [this]
	{
		stopRecordingMovie();
		getTsmPage().requestUpdate();
	};
	m_frameGrabber->startMultiFrame(MovieFrameRate);

	::UtilizationLogEvent::MovieRecordStart(log);
}

void CoRegistrationState::stopRecordingMovie()
{
	if (movieRecordingBusy)
	{
		movieRecordingBusy = false;
		getTsmPage().storeMovieButton->enabled = false;
		getTsmPage().storeMovieButton->upperText = Localization::Translation::IDS_Start;
		getTsmPage().storeMovieButton->blinking = false;
		getTsmPage().requestUpdate();

		if (m_archivingStatus)
		{
			m_archivingStatus->setMovieStatus(ArchivingStatus::Busy);
		}

		m_frameGrabber->onMultiFrameStopped = nullptr;
		m_frameGrabber->stopMultiFrame();
		auto frames = m_frameGrabber->getFrames();
		m_pacsArchiver->sendMovie(frames, MovieFrameRate, pciViewContext.coRegViewer.getImage()->seriesNumber, RunType::DCR);

		::UtilizationLogEvent::MovieRecordEnd(log);
	}
}

void CoRegistrationState::onFrameGrabberFinished(PACS::GrabType grabType, bool finishedSuccessfully)
{
	if (m_archivingStatus)
	{
		auto status = finishedSuccessfully ? ArchivingStatus::Success : ArchivingStatus::Failure;

		if (grabType == PACS::GrabType::SingleFrame) m_archivingStatus->setSnapshotStatus(status, CaptureArea::MainAndSideView);
		if (grabType == PACS::GrabType::MultiFrame)  m_archivingStatus->setMovieStatus(status);
	}

	if (grabType == PACS::GrabType::MultiFrame)
	{
		m_frameGrabber->flushMultiFrame();
		getTsmPage().storeMovieButton->enabled = true;
		getTsmPage().requestUpdate();
	}
}

void CoRegistrationState::onExit()
{
	if (m_frameGrabber)
	{
		m_frameGrabber->flushMultiFrame();
	}
}

PciXrayPositionModel& CoRegistrationState::getModel()
{
	return model;
}

void CoRegistrationState::stopReview()
{
	pciViewContext.coRegistrationPlayer.stopReview();
	getTsmPage().playPauseButton->icon = TSM::ButtonIcon::MoviePlay;
	getTsmPage().requestUpdate();
}

void CoRegistrationState::startReview()
{
	pciViewContext.coRegistrationPlayer.startReview();
	getTsmPage().playPauseButton->icon = TSM::ButtonIcon::MovieStop;
	getTsmPage().requestUpdate();
}

void CoRegistrationState::onEnter()
{
	enableTsmControls();

	if (!isPressureWireConnectedAndReady(pciViewContext.coRegState))
	{
		stopReview();
	}

    // only reset the last image hold when we are about to be shown
	if (!pciViewContext.coRegViewer.visible && 
			(pciViewContext.coRegState.systemState != CoRegistrationServerState::SystemState::Review || pciViewContext.coRegistrationPlayer.inReview()))
    {
		allowGeometryUpdates = true;
    }

    // When showLastImageHold has been set during measurement than do not show the icon because guidance is shown
    if (pciViewContext.coRegState.systemState != CoRegistrationServerState::SystemState::Review &&
        model.showLastImageHold)
    {
        model.showLastImageHold = false;
    }

    if (!pciViewContext.coRegViewer.visible)
	{
		if(pciViewContext.coRegState.systemState == CoRegistrationServerState::SystemState::Review)
		{
			updateArchivingStatus();
		}
	}

    show(&pciViewContext.coRegViewer);
}

void CoRegistrationState::updateArchivingStatus()
{
	if(pciViewContext.pacsPush == nullptr || m_archivingStatus == nullptr ) return;

	if (pciViewContext.snapshotsAvailable)
	{
		m_archivingStatus->setSnapshotStatus(ArchivingStatus::Ready, CaptureArea::MainAndSideView);
	}

	if(pciViewContext.coRegState.fmState == CoRegistrationServerState::FmState::IFRPullback)
	{
		if (pciViewContext.moviesAvailable)
		{
			m_archivingStatus->setMovieStatus(ArchivingStatus::Ready);
		}
	}
}

ViewState CoRegistrationState::roadmapStatusChanged()
{
	//always show the CoRegistration state, even if there is no roadmap.
    return viewState;
}

ViewState CoRegistrationState::angioProcessingStarted()
{
    return ViewState::CrmProcessingAngioState;
}

ViewState CoRegistrationState::coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState,
	                                                      const CoReg::CoRegistrationServerState& nextState)

{
	UiState::coRegistrationStateChanged(prevState, nextState);
	
	if (!isPressureWireConnectedAndReady(nextState))
	{
		log.DeveloperInfo(L"Connection with FFR/iFR system or pressure wire not ready for use during CoRegistration. Switch to Roadmap state without CoRegistration");
		stopReview();
		return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
	}

	if (nextState.systemState != ::CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState::Review ||
	    pciViewContext.coRegError == CoRegistrationError::UnreliableDataTransfer)
	{
		return ViewStateHelper:: DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
	}

	enableTsmControls();

	return viewState;
}

ViewState CoRegistrationState::epxChanged()
{
	// Stop review player if CoReg is in review mode
	if (pciViewContext.coRegState.systemState == CoRegistrationServerState::SystemState::Review)
	{
		stopReview();
	}
	return UiState::epxChanged();
}

ViewState CoRegistrationState::lastImageHold()
{
	model.showLastImageHold = true;
	allowGeometryUpdates = false;

	if(pciViewContext.coRegState.systemState == CoRegistrationServerState::SystemState::Review)
	{
		updateArchivingStatus();
	}

	if(pciViewContext.coRegState.systemState != CoRegistrationServerState::SystemState::Review)
	{
		return ViewState::CoRegistrationGuidanceState;
	}

	return viewState;
}

ViewState CoRegistrationState::imageRunStarted(CommonPlatform::Xray::XrayImage::Type type)
{
	UiState::imageRunStarted(type);

	stopReview();

	if(type == CommonPlatform::Xray::XrayImage::Type::Exposure)
	{
		if(pciViewContext.currentEpx.getType() == PciEpx::Type::Roadmap)
		{
			return ViewState::CrmProcessingAngioState;
		}

		if(pciViewContext.currentEpx.getType() == PciEpx::Type::StentBoost)
		{
			return ViewState::StbLiveProcessingState;
		}
	}
	else if (pciViewContext.coRegState.systemState != CoRegistrationServerState::SystemState::Pullback &&
	         pciViewContext.coRegState.systemState != CoRegistrationServerState::SystemState::SpotMeasurement)
	{
		if(pciViewContext.crmAdminStatus == CrmAdministration::Status::Active)
		{
			return ViewState::CrmOverlayState;
		}
		else
		{
			return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
		}
	}
	
	return viewState;
}

ViewState CoRegistrationState::coRegistrationRunChanged(int runIndex)
{
	UiState::coRegistrationRunChanged(runIndex);

	if(pciViewContext.coRegState.systemState == CoRegistrationServerState::SystemState::Review)
	{
		log.DeveloperInfo(L"Review of previous run selected on FFR/iFR system. Leave Co-Registration state. [RunIndex=%d]", runIndex);
		stopReview();
		return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
	}

	return viewState;
}

ViewState CoRegistrationState::coRegistrationConnectionChanged(bool connected)
{
	stopReview();
	return UiState::coRegistrationConnectionChanged(connected);
}

ViewState CoRegistrationState::studyChanged()
{
	stopReview();
	return UiState::studyChanged();
}

ViewState CoRegistrationState::overlayImageAvailable()
{
	if(model.showLastImageHold)
	{
		model.showLastImageHold = false;
	}
	allowGeometryUpdates = true;
    model.showLastImageHold = false;
	return viewState;
}

void CoRegistrationState::enableTsmControls()
{
	enablePullbackControlsOnTsm();
	enableArchiveControlsOnTsm();

	getTsmPage().requestUpdate();
}

void CoRegistrationState::enablePullbackControlsOnTsm()
{
	bool enable = pciViewContext.coRegState.systemState == ::CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState::Review &&
				  pciViewContext.coRegState.fmState == ::CommonPlatform::CoRegistration::CoRegistrationServerState::FmState::IFRPullback &&
				  isPressureWireConnectedAndReady(pciViewContext.coRegState);

	//set the initial review button state;
	if(pciViewContext.coRegistrationPlayer.inReview())
	{
		getTsmPage().playPauseButton->icon = TSM::ButtonIcon::MovieStop;
	}
	else
	{
		getTsmPage().playPauseButton->icon = TSM::ButtonIcon::MoviePlay;
	}

	getTsmPage().frameForwardButton->visible  = enable;
	getTsmPage().frameForwardButton->enabled  = enable;

	getTsmPage().frameBackwardButton->visible = enable;
	getTsmPage().frameBackwardButton->enabled = enable;

	getTsmPage().playPauseButton->visible     = enable;
	getTsmPage().playPauseButton->enabled     = enable;
}

void CoRegistrationState::enableArchiveControlsOnTsm()
{
    bool visibilityMoviesButton = pciViewContext.coRegState.fmState == ::CommonPlatform::CoRegistration::CoRegistrationServerState::FmState::IFRPullback;

	bool enableSnapshotButton = pciViewContext.pacsPush && pciViewContext.snapshotsAvailable && 
								pciViewContext.coRegState.systemState == ::CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState::Review &&
								isPressureWireConnectedAndReady(pciViewContext.coRegState);

	bool enableMoviesButton	  =	pciViewContext.pacsPush && pciViewContext.moviesAvailable && 
								pciViewContext.coRegState.systemState == ::CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState::Review &&
								pciViewContext.coRegState.fmState == ::CommonPlatform::CoRegistration::CoRegistrationServerState::FmState::IFRPullback &&
								isPressureWireConnectedAndReady(pciViewContext.coRegState);

	getTsmPage().storeSnapshotButton->visible = true;
	getTsmPage().storeSnapshotButton->enabled = enableSnapshotButton;
	getTsmPage().storeMovieButton->visible = visibilityMoviesButton;
	getTsmPage().storeMovieButton->enabled = enableMoviesButton;
}

CrmTsmPage& CoRegistrationState::getTsmPage() const
{
	return *static_cast<CrmTsmPage*>(tsmPage.get());
}

}}
