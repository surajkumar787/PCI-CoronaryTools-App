// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "gtest/gtest.h"
#include "MainViewerUnderTest.h"
#include "MainControllerUnderTest.h"
#include "XraySourceRtoMock.h"
#include "XrayGeometryControllerMock.h"
#include "TsmSourceStub.h"
#include "MainModel.h"
#include "UiStateController.h"
#include "LicensesCollectionStub.h"

#include "CoRegistrationSource.h"
#include "TestScheduler.h"
#include "PciTsmPage.h"

#include "PACSPushSpy.h"
#include "StorageCommitStub.h"

#include "MessageBuilder.h"
#include "Merge/library.h"
#include "DicomConstants.h"

#include "NullCopyToRefClient.h"
#include "MockIArchivingSettings.h"
#include "MockIUserPrefs.h"
#include "charsetmanager.h"

#include <memory>

namespace ModuleTest{

using namespace Pci::Core;
using namespace CommonPlatform::CoRegistration;

namespace Merge = ::CommonPlatform::Dicom::Merge;

static std::unique_ptr<Merge::Library> mergecomLib;
static std::unique_ptr<Merge::Service> snapshots;
static std::unique_ptr<Merge::Service> movies;

// This class tests the Archiving of snapshots and movies during several workflows
class TestArchiving : public ::testing::Test
{
public:

	MainControllerUnderTest* getControler()
	{
		return controller.get();
	}

	void processMessagesWithTimeOut(unsigned int timeout)
	{
		scheduler->processMessagesWithTimeOut(timeout);
	}

	PACSPushSpy* getPacs()
	{
		return pacs.get();
	}

protected:

	static void SetUpTestCase()
	{
		TRANSFER_SYNTAX snapshotSyntax = EXPLICIT_LITTLE_ENDIAN;
		TRANSFER_SYNTAX movieSyntax = JPEG_LOSSLESS_HIER_14;

		mergecomLib.reset(new Merge::Library("..\\..\\Configuration\\Merge.ini"));

		snapshots.reset(new Merge::Service(Merge::sopClassUidToserviceName(CommonPlatform::Dicom::Constants::SopClass::SecondaryCaptureImageStorage),
			Merge::SyntaxList(std::vector<TRANSFER_SYNTAX>(1, snapshotSyntax)),
			Merge::Service::Role::SCU));
		movies.reset(new Merge::Service(Merge::sopClassUidToserviceName(CommonPlatform::Dicom::Constants::SopClass::MultiFrameSecondaryCaptureTrueColor),
			Merge::SyntaxList(std::vector<TRANSFER_SYNTAX>(1, movieSyntax)),
			Merge::Service::Role::SCU));
	}

	static void TearDownTestCase()
	{
		mergecomLib.reset();
		snapshots.reset();
		movies.reset();
	}

	void SetUp() override
	{
		Test::SetUp();

		std::unique_ptr<CommonPlatform::Dicom::CharsetManager> charsetManager(new CommonPlatform::Dicom::CharsetManager());
		std::vector<std::string> characterSets = { CommonPlatform::Dicom::Constants::ISO_IR_100 };
		charsetManager->setPossibleCharacterset(characterSets);

		// Connect stubbed source to viewer and controller (plus setup supporting objects)
		scheduler .reset(new TestScheduler());
		gpu       .reset(new Gpu(0));
		canvas    .reset(new Canvas(*gpu, *scheduler));
		offscreen .reset(new OutputOffscreen(*canvas));
		tsm       .reset(new TsmSourceStub());
		msgBuilder.reset(new PACS::MessageBuilder(*snapshots, *movies, nullptr, std::move(charsetManager)));
		pacs      .reset(new PACSPushSpy());
		storCommit.reset(new StorageCommitStub());
		xraySource.reset(new XraySourceRtoMock());
		xrayApc   .reset(new XrayGeometryControllerMock());
		mainModel .reset(new MainModel(m_licenseCollection, SystemType::Allura));
		mainView  .reset(new MainViewerUnderTest(*canvas, *mainModel, ViewType::SmartSuiteFlexVision));
		copyToRefClient.reset(new Pci::Platform::iControl::NullCopyToRefClient);
		userPrefs.reset(new ::testing::NiceMock<MockIUserPrefs>());
		controller.reset(new MainControllerUnderTest(*mainView, *mainModel, *xraySource, *xrayApc, *scheduler, *tsm, storCommit.get(), pacs.get(), msgBuilder.get(), *copyToRefClient, *userPrefs));

		gpu->show();
	}

	void sendFluoroImages(MainControllerUnderTest& control)
	{
		const double imageFrameTime = 1.0 / 15;
		const int nrOfImages = 10;

		for (int imageNumber = 0; imageNumber < nrOfImages; ++imageNumber)
		{
			auto image = std::make_shared<CommonPlatform::Xray::XrayImage>();

			image->type = CommonPlatform::Xray::XrayImage::Type::Fluoro;
			image->frameTime = imageFrameTime;
			image->imageNumber = imageNumber;
			control.setImage(image, false);
		}
	}

	void sendStentBoostLiveImages(MainControllerUnderTest& control, int seriesNumber)
	{
		const int nrOfImages = 15;

		auto markers = std::make_shared<Pci::Core::StbMarkers>();
		auto boost = std::make_shared<StbImage>();
		boost->resize(720, 720);

		control.getUiStateController().imageRunStarted(XrayImage::Type::Exposure);

		for (int i = 0; i < nrOfImages; i++)
		{
			auto img = std::make_shared<XrayImage>();
			img->imageIndex = i;
			img->seriesNumber = seriesNumber;
			markers->valid = true;
			control.getUiStateController().stentBoostImageAvailable(img, markers, boost);
		}

		control.getUiStateController().endLive();
	}

	void switchCoRegistrationState(MainControllerUnderTest& control, CoRegistrationServerState::SystemState systemState, CoRegistrationServerState::FmState fmState)
	{
		CoRegistrationServerState nextState;

		nextState.systemState = systemState;
		nextState.fmState = fmState;

		nextState.normalized = true;
		nextState.wireConnected = true;
		nextState.pressurePlugConnected = true;
		nextState.pimConnected = true;

		control.setCoRegState(nextState);
	}

	std::shared_ptr<const CrmRoadmap> createRoadmap(int runIndex)
	{
		auto roadmap = std::make_shared<CrmRoadmap>(runIndex, runIndex, 1024, 1024, 0.1, XrayGeometry(), Sense::Matrix(), 0.0);
		roadmap->overlay = createOverlay();
		roadmap->data.push_back('A');
		return roadmap;
	}

	std::shared_ptr<const CrmRoadmap> createRoadmap(int runIndex, const XrayGeometry& geometry)
	{
		auto roadmap = std::make_shared<CrmRoadmap>(runIndex, runIndex, 1024, 1024, 0.1, geometry, Sense::Matrix(), 0.0);
		roadmap->overlay = createOverlay();
		roadmap->data.push_back('A');
		return roadmap;
	}

	std::shared_ptr<const CrmOverlay> createOverlay()
	{
		auto overlay = std::make_shared<CrmOverlay>();
		overlay->resize(256, 256);
		return overlay;
	}

	void startPullback(MainControllerUnderTest& control)
	{
		switchCoRegistrationState(control, CoRegistrationServerState::SystemState::Live, CoRegistrationServerState::FmState::IFRSpot);
		switchCoRegistrationState(control, CoRegistrationServerState::SystemState::Pullback, CoRegistrationServerState::FmState::IFRPullback);
	}

	void switchToReview(MainControllerUnderTest& control)
	{
		switchCoRegistrationState(control, CoRegistrationServerState::SystemState::Review, CoRegistrationServerState::FmState::IFRPullback);
	}

	void setEpx(MainControllerUnderTest& control, const std::wstring& activityType)
	{
		XrayEpx epx;
		epx.activityType = activityType;
		control.setEpx(epx);
	}

	void setStudy(MainControllerUnderTest& control, const std::wstring& patientName)
	{
		XrayStudy study;
		study.patientName = patientName;
		control.setStudy(study);
	}

	void setGeometry(MainControllerUnderTest& control, const XrayGeometry& geometry)
	{
		control.setGeometry(geometry);
		control.endGeometry();
	}

	void setSeriesNumber(int seriesNumber)
	{
		msgBuilder->setSeriesNumber(seriesNumber);
	}

	ViewState getViewState(MainControllerUnderTest& control)
	{
		return control.getUiStateController().getSelectedView();
	}

	void pressTsmButton(MainControllerUnderTest& control, unsigned int button)
	{
		control.getUiStateController().getCurrentState()->getTsmPage()->onButtonRelease(button);
	}

private:
	LicenseCollectionStub							m_licenseCollection;
	std::unique_ptr<TestScheduler>                  scheduler;
	std::unique_ptr<Gpu>                            gpu;
	std::unique_ptr<Canvas>                         canvas;
	std::unique_ptr<OutputOffscreen>                offscreen;
	std::unique_ptr<TSM::ITsmSource>                tsm;
	std::unique_ptr<PACS::MessageBuilder>           msgBuilder;
	std::unique_ptr<PACSPushSpy>                    pacs;
	std::unique_ptr<PACS::IStorageCommitListener>   storCommit;
	std::unique_ptr<MainModel>                      mainModel;
	std::unique_ptr<XraySourceRtoMock>              xraySource;
	std::unique_ptr<XrayGeometryControllerMock>     xrayApc;
	std::unique_ptr<MainViewerUnderTest>            mainView;
	std::unique_ptr<Pci::Platform::iControl::ICopyToRefClient> copyToRefClient;
	std::unique_ptr<IUserPrefs>						userPrefs;
	std::unique_ptr<MainControllerUnderTest>        controller;

};

static const int TsmMoveStartStopButtonIndex = 2;

TEST_F(TestArchiving, GivenInIfrReview_WhenTakingSnapshotWhileRecordingMovie_BothAreArchived)
{
	auto crmActivityType1 = L"LCA";

	// Connect X-ray
	getControler()->connectionChanged(true, IXraySourceRto::CwisStatus::Connected, IXraySourceRto::DvlpStatus::Connected, L"", L"");

	// Connect CoReg
	getControler()->setCoRegConnection(CoRegistrationSource::ConnectionStatus::ConnectedWithTimeSync);

	// Switch to Roadmap state
	setEpx(*getControler(), crmActivityType1);
	ASSERT_EQ(PciEpx::Type::Roadmap, getControler()->getEpx().getType());

	// Set metadata
	setStudy(*getControler(), L"patient1");

	startPullback(*getControler());

	sendFluoroImages(*getControler());
	processMessagesWithTimeOut(100);

	switchToReview(*getControler());

	pressTsmButton(*getControler(), TsmMoveStartStopButtonIndex); // Start recording movie

	pressTsmButton(*getControler(), 1); // Take snapshot

	// Grab frames on the UI thread
	processMessagesWithTimeOut(100);

	pressTsmButton(*getControler(), TsmMoveStartStopButtonIndex); // End recording movie

	getControler()->waitForEncoder();

	// Process recorded movie in background thread
	processMessagesWithTimeOut(100);

	ASSERT_EQ(getPacs()->getSentImages().size(), 1);
	ASSERT_EQ(getPacs()->getSentImages()[0].study.patientName, L"patient1");

	ASSERT_EQ(getPacs()->getSentSequences().size(), 1);
	ASSERT_EQ(getPacs()->getSentSequences()[0].study.patientName, L"patient1");
}

TEST_F(TestArchiving, GivenCrmLastImageHold_WhenRecordingMovie_WholeRecordingIsArchived)
{
	auto crmActivityType1 = L"LCA";
	getControler()->getViewCtx().moviesAvailable = true;

	// Connect X-ray
	getControler()->connectionChanged(true, IXraySourceRto::CwisStatus::Connected, IXraySourceRto::DvlpStatus::Connected, L"", L"");

	// Switch to Roadmap state
	setEpx(*getControler(), crmActivityType1);
	ASSERT_EQ(PciEpx::Type::Roadmap, getControler()->getEpx().getType());

	// Set metadata
	setStudy(*getControler(), L"patient1");

	setGeometry(*getControler(), CommonPlatform::Xray::XrayGeometry());

	getControler()->getCrmAdmin().onRoadmapStatusChanged = [&]() { getControler()->getViewCtx().crmAdminStatus = getControler()->getCrmAdmin().getStatus(); };
	getControler()->getCrmAdmin().addRoadmap(createRoadmap(1));
	getControler()->getUiStateController().roadmapStatusChanged();

	sendFluoroImages(*getControler());
	processMessagesWithTimeOut(100);
	getControler()->getUiStateController().lastImageHold();

	pressTsmButton(*getControler(), TsmMoveStartStopButtonIndex); // Start recording movie

	getControler()->waitForEncoder();

	// Process recorded movie in background thread
	processMessagesWithTimeOut(100);

	ASSERT_EQ(getPacs()->getSentSequences().size(), 1);
	ASSERT_EQ(getPacs()->getSentSequences()[0].study.patientName, L"patient1");
}

TEST_F(TestArchiving, GivenSblLastImageHold_WhenSendingSnapshot_TheCorrectSeriesNumberIsSent)
{
	const int SERIES_NUMBER(100);
	const int WRONG_SERIES_NUMBER(345);

	auto SBLActivityType = L"SBLive";
	getControler()->getViewCtx().snapshotsAvailable = true;

	// Connect X-ray
	getControler()->connectionChanged(true, IXraySourceRto::CwisStatus::Connected, IXraySourceRto::DvlpStatus::Connected, L"", L"");

	setEpx(*getControler(), SBLActivityType);
	ASSERT_EQ(PciEpx::Type::StentBoost, getControler()->getEpx().getType());

	// Set metadata
	setStudy(*getControler(), L"patient1");
	setGeometry(*getControler(), CommonPlatform::Xray::XrayGeometry());

	sendStentBoostLiveImages(*getControler(), SERIES_NUMBER);
	getControler()->getUiStateController().lastImageHold();

	setSeriesNumber(WRONG_SERIES_NUMBER);

	pressTsmButton(*getControler(), PciTsmPage::SnapshotButton);

	getControler()->waitForEncoder();

	ASSERT_EQ(getPacs()->getSentImages().size(), 1);
	ASSERT_EQ(getPacs()->getSentImages()[0].seriesNumber, SERIES_NUMBER);
}

}