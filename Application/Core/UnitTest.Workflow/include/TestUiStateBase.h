// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <gtest/gtest.h>
#include <Sense.h>
#include <ExperienceIdentity/Controls/Canvas.h>
#include "UiStateController.h"
#include "PciViewContext.h"
#include "PciGuidanceView.h"
#include "CoRegistrationControl.h"
#include "CrmViewer.h"
#include "CrmRoadmapSelectViewer.h"
#include "CrmRepository.h"
#include "StbBoostViewer.h"
#include "StbMarkersViewer.h"
#include "StbRoiViewer.h"
#include "StbContrastViewer.h"
#include "StbImageSelectViewer.h"
#include "StbAdministration.h"
#include "PciReviewParameters.h"
#include "PciReviewPlayer.h"
#include "PullbackRecording.h"
#include "PciEpxSelectionView.h"
#include "TsmSourceStub.h"
#include "MockIReplayControl.h"
#include "CoRegistrationServerState.h"
#include "NullCopyToRefClient.h"
#include "MockIArchivingSettings.h"
#include "MockIUserPrefs.h"
#include "LicensesCollectionStub.h"
#include "IArchivingStatus.h"
#include "MockICrmAdministration.h"

using namespace Sense;
using namespace Pci::Core;
using namespace CommonPlatform::CoRegistration;
using namespace Pci::Platform::iControl;

class TestUiStateBase : public testing::Test, private IArchivingStatus
{


public:
	LicenseCollectionStub				m_licenseCollection;
	Gpu									gpu;
	Scheduler							scheduler;
	ExperienceIdentity::Canvas			canvas;
	CrmViewer							viewer;
	StbBoostViewer						stbBoostViewer;
	StbMarkersViewer					stbMarkersViewer;
	StbRoiViewer						stbRoiViewer;
	StbContrastViewer					stbContrastViewer;
    StbImageSelectViewer	        	stbContrastFrameSelectViewer;
    StbImageSelectViewer                stbBoostFrameSelectViewer;
    CoRegistrationControl				coRegControl;
	PciGuidanceView						crmGuidance;
	CrmRoadmapSelectViewer				crmRoadmapSelectViewer;
	PciGuidanceView						stbGuidance;
	PciEpxSelectionView					epxSelection;
	PciReviewParameters					reviewParameters;
	StbAdministration					stbAdmin;
	PciReviewPlayer<PullbackRecording>	coRegPlayer;
	PciGuidanceView						coregGuidance;
	PciGuidanceView						noLicense;
	PciGuidanceView						noConnection;
	::testing::NiceMock<MockICrmAdministration>	crmAdmin;
	PciViewContext						viewContext;
	TsmSourceStub						tsm;
	::testing::NiceMock<MockIReplayControl>		replayControl;
	NullCopyToRefClient					copyTorefClient;
	::testing::NiceMock<MockIUserPrefs>	userPrefs;
	UiStateController					controller;

	std::shared_ptr<XrayImage> liveImage;
	std::shared_ptr<Pci::Core::StbMarkers> markers;
	std::shared_ptr<StbImage> boostImage;

    TestUiStateBase():
		m_licenseCollection(),
		gpu(0),
        scheduler(),
        canvas(gpu, scheduler),
        viewer (canvas, ViewType::Allura),
		stbBoostViewer(canvas, ViewType::Allura),
		stbMarkersViewer(canvas, ViewType::Allura),
		stbRoiViewer(canvas, ViewType::Allura),
		stbContrastViewer(canvas,scheduler,ViewType::Allura),
        stbContrastFrameSelectViewer(canvas, ViewType::Allura, false),
        stbBoostFrameSelectViewer(canvas, ViewType::Allura, true),
        crmGuidance(viewer, ViewType::Allura),
		crmRoadmapSelectViewer(viewer, ViewType::Allura),
        stbGuidance(viewer, ViewType::Allura),
		epxSelection(viewer, ViewType::Allura),
        noConnection(viewer, ViewType::Allura),
		noLicense(viewer, ViewType::Allura),
		crmAdmin(),
        coregGuidance(viewer, ViewType::Allura),
		coRegControl(viewer, ViewType::Allura),
		reviewParameters(1, 0.0, 0, 100),
		coRegPlayer(scheduler, reviewParameters),
		stbAdmin(reviewParameters),
		viewContext(viewer, stbBoostViewer, stbMarkersViewer, stbRoiViewer, stbContrastViewer, stbContrastFrameSelectViewer, stbBoostFrameSelectViewer, viewer, crmGuidance, crmRoadmapSelectViewer, stbGuidance, coregGuidance, coRegControl, coRegPlayer, reviewParameters, stbAdmin, epxSelection, noConnection, noLicense,
		            PciEpx(), m_licenseCollection, nullptr, nullptr, nullptr, crmAdmin, false, false, SystemType::Allura),
		tsm(),
		replayControl(),
        controller(viewContext, scheduler, tsm, replayControl, copyTorefClient, userPrefs, this),
		liveImage(std::make_shared<XrayImage>()),
		markers(std::make_shared<Pci::Core::StbMarkers>()),
		boostImage(std::make_shared<StbImage>())
    {
		Sense::Logging::eventAssert = [&](const wchar_t *t) { std::wcout << L"Sense assert: " << t << std::endl; FAIL(); };
		Sense::Logging::eventError = [&](const wchar_t *t) { std::wcout << L"Sense error: " << t << std::endl; FAIL(); };
	}

    ~TestUiStateBase()
    {
    }


protected:
	void switchToEpx(PciEpx::SubType subType)
	{
		PciEpx epx(subType);
		viewContext.currentEpx = epx;
		
		controller.epxChanged();
	}

	void switchToEpx(PciEpx::Type type)
	{
		PciEpx epx(type);
		viewContext.currentEpx = epx;

		controller.epxChanged();
	}


	void studyChanged()
	{
		viewContext.roadmapErrorState = PciSuite::CrmErrors::Error::OK;
		viewContext.crmAdminStatus = CrmAdministration::Status::NoRoadmap;

		controller.studyChanged();
	}

    void switchRoadmapStatus(CrmAdministration::Status status)
    {
        viewContext.crmAdminStatus = status;
		viewContext.roadmapErrorState = PciSuite::CrmErrors::Error::OK;
        controller.roadmapStatusChanged();
    }

    void switchToEpxAndRoadmapStatus(PciEpx::Type type, CrmAdministration::Status status)
    {
        PciEpx epx(type);
        viewContext.currentEpx = epx;

        switchRoadmapStatus(status);
    }

	void switchCoRegistrationState(::CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState systemState, ::CommonPlatform::CoRegistration::CoRegistrationServerState::FmState fmState)
	{
		::CommonPlatform::CoRegistration::CoRegistrationServerState nextState;

		nextState.systemState	= systemState;
		nextState.fmState		= fmState;

		nextState.normalized			= true;
		nextState.wireConnected		    = true; 
		nextState.pressurePlugConnected = true; 
		nextState.pimConnected			= true;

		auto prevState = viewContext.coRegState;

		if ((nextState.systemState == CoRegistrationServerState::SystemState::Pullback ||
			 nextState.systemState == CoRegistrationServerState::SystemState::SpotMeasurement) &&
			 prevState.systemState == CoRegistrationServerState::SystemState::Live)
		{
			// New recording started
			viewContext.fluoroAcquiredWhileRecording = false;
		}

		viewContext.coRegState = nextState;

		controller.coRegistrationStateChanged(prevState, nextState);
	}

	void sendExposure()
	{
		controller.imageRunStarted(CommonPlatform::Xray::XrayImage::Type::Exposure);
	}

    void sendExposureWithEventsFromAlgorithm()
    {
        controller.imageRunStarted(CommonPlatform::Xray::XrayImage::Type::Exposure);
        controller.stentBoostImageAvailable(liveImage, markers, boostImage);
        controller.endLive();
        controller.lastImageHold();
    }

    void sendFluoro()
    {
        controller.imageRunStarted(CommonPlatform::Xray::XrayImage::Type::Fluoro);
        controller.endLive();
    }
			
	void angioProcessingStoppedRoadmapActive()
	{
		viewContext.crmAdminStatus = CrmAdministration::Status::Active ;
	    viewContext.roadmapErrorState = PciSuite::CrmErrors::Error::OK;
		controller.angioProcessingStopped();
	}

	void angioProcessingStopped(CrmAdministration::Status status, PciSuite::CrmErrors::Error error)
	{
		viewContext.crmAdminStatus = status;
		viewContext.roadmapErrorState = error;
		controller.angioProcessingStopped();
	}

	void angioProcessingStartedClearError()
	{
		viewContext.roadmapErrorState = PciSuite::CrmErrors::Error::OK;
		controller.angioProcessingStarted();
	}

	void disconnectWireAndCheckCoregControlIsHidden()
	{
		disconnectWire();
		checkCoregControlIsHidden();
	}

	void switchToLive()
	{
		switchCoRegistrationState(CoRegistrationServerState::SystemState::Live, CoRegistrationServerState::FmState::FFR);
	}

	void switchToIfrPullback()
	{
		switchCoRegistrationState(CoRegistrationServerState::SystemState::Pullback, CoRegistrationServerState::FmState::IFRPullback);
	}

	void switchToIfrSpotMeasurement()
	{
		switchCoRegistrationState(CoRegistrationServerState::SystemState::SpotMeasurement, CoRegistrationServerState::FmState::IFRSpot);
	}

	void switchToFfrSpotMeasurement()
	{
		switchCoRegistrationState(CoRegistrationServerState::SystemState::SpotMeasurement, CoRegistrationServerState::FmState::FFR);
	}

	void switchToIfrReview()
	{
		switchCoRegistrationState(CoRegistrationServerState::SystemState::Review, CoRegistrationServerState::FmState::IFRSpot);
	}

	void switchToFfrReview()
	{
		switchCoRegistrationState(CoRegistrationServerState::SystemState::Review, CoRegistrationServerState::FmState::FFR);
	}

	void setCoRegConnected(bool connected)
	{
		controller.coRegistrationConnectionChanged(connected);
	}

	void disconnectWire()
	{
		CoRegistrationServerState nextState;

		nextState.fmState				= CoRegistrationServerState::FmState::IFRPullback;
		nextState.systemState			= CoRegistrationServerState::SystemState::Review;
		nextState.normalized			= true;
		nextState.pressurePlugConnected = true;
		nextState.wireConnected			= false;
		nextState.wireReady				= true;
		nextState.wireShorted			= true;
		nextState.pimConnected			= true;

		auto prevState = viewContext.coRegState;
		viewContext.coRegState = nextState;
		controller.coRegistrationStateChanged(prevState, nextState);
	}

	void checkCoregControlIsHidden()
	{
		ASSERT_TRUE(viewContext.forceHideCoregControl || !viewContext.coRegistrationControl.visible);
	}

	virtual void setSnapshotStatus(ArchivingStatus /*status*/, CaptureArea /*area*/) override {}
	virtual void setMovieStatus(ArchivingStatus /*status*/) override {}
	virtual void clearStatus() override {};
};

