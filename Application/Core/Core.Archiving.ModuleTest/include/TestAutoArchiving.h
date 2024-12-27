// Copyright Koninklijke Philips N.V. 2018
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
#include "XraySourceFxd.h"
#include "TsmSourceStub.h"
#include "MainModel.h"
#include "UiStateController.h"
#include "StbRecording.h"
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
#include "MockIUserPrefs.h"

#include <memory>

#include <locale>					// std::codecvt_utf8
#include <codecvt>					// std::wstring_convert

namespace ModuleTest {

	using namespace Pci::Core;
	using namespace CommonPlatform::CoRegistration;

	namespace Merge = ::CommonPlatform::Dicom::Merge;

	// This class tests the Archiving of snapshots and movies during several workflows
	class TestAutoArchiving : public ::testing::Test
	{
	public:
		TestAutoArchiving(const std::wstring& epxName) : ::testing::Test(),
			m_epxName(epxName)
		{
		}

		static std::unique_ptr<Merge::Library> mergecomLib;
		static std::unique_ptr<Merge::Service> snapshots;
		static std::unique_ptr<Merge::Service> movies;

		static void SetUpTestCase()
		{
			TRANSFER_SYNTAX snapshotSyntax = EXPLICIT_LITTLE_ENDIAN;
			TRANSFER_SYNTAX movieSyntax = JPEG_LOSSLESS_HIER_14;

			mergecomLib.reset(new Merge::Library("Merge.ini"));

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

		Pci::Core::PciViewContext& getViewCtx()
		{
			return controller->getViewCtx();
		}

		Pci::Core::UiStateController& getUiStateController()
		{
			return controller->getUiStateController();
		}

	protected:

		void SetUp() override
		{
			Test::SetUp();

			std::unique_ptr<CommonPlatform::Dicom::CharsetManager> charsetManager(new CommonPlatform::Dicom::CharsetManager());
			std::vector<std::string> characterSets = { CommonPlatform::Dicom::Constants::ISO_IR_100 };
			charsetManager->setPossibleCharacterset(characterSets);

			// Connect stubbed source to viewer and controller (plus setup supporting objects)
			scheduler.reset(new TestScheduler());
			gpu.reset(new Gpu(0));
			canvas.reset(new Canvas(*gpu, *scheduler));
			offscreen.reset(new OutputOffscreen(*canvas));
			tsm.reset(new TsmSourceStub());
			msgBuilder.reset(new PACS::MessageBuilder(*snapshots, *movies, nullptr, std::move(charsetManager)));
			pacs.reset(new PACSPushSpy());
			storCommit.reset(new StorageCommitStub());
			xraySource.reset(new XraySourceRtoMock());
			xrayApc.reset(new XrayGeometryControllerMock());
			mainModel.reset(new MainModel(m_licenseCollection, SystemType::Allura));
			mainView.reset(new MainViewerUnderTest(*canvas, *mainModel, ViewType::SmartSuiteFlexVision));
			copyToRefClient.reset(new Pci::Platform::iControl::NullCopyToRefClient);
			userPrefs.reset(new ::testing::NiceMock<MockIUserPrefs>);
			controller.reset(new MainControllerUnderTest(*mainView, *mainModel, *xraySource, *xrayApc, *scheduler, *tsm, storCommit.get(), pacs.get(), msgBuilder.get(), *copyToRefClient, *userPrefs));

			controller->getViewCtx().moviesAvailable = true;
			controller->getViewCtx().snapshotsAvailable = true;

			msgBuilder->setSeriesNumber(0);

			controller->connectionChanged(true, IXraySourceRto::CwisStatus::Connected, IXraySourceRto::DvlpStatus::Connected, L"", L"");
			setStudy(L"patient1");
			setEpx(*controller, m_epxName);

			gpu->show();
		}

		void setEpx(MainControllerUnderTest& control, const std::wstring& activityType)
		{
			XrayEpx epx;
			epx.activityType = activityType;
			control.setEpx(epx);
		}

		void setStudy(const std::wstring& patientName, const std::string& studyDate = "20180101")
		{
			XrayStudy study;
			study.patientName = patientName;
			study.studyDate = studyDate;
			controller->setStudy(study);
		}

		ViewState getViewState(MainControllerUnderTest& control)
		{
			return control.getUiStateController().getSelectedView();
		}

		void waitForEncoder()
		{
			controller->waitForEncoder();
		}

		void processMessagesWithTimeOut(unsigned int time)
		{
			scheduler->processMessagesWithTimeOut(time);
		}

		PACSPushSpy* getPacs()
		{
			return pacs.get();
		}

		int getSnapshotsPerRun(const std::wstring& epx)
		{
			int numberOfSnapshots = 1;

			if (epx == L"SBLiveSubtract")
			{
				numberOfSnapshots = 2;
			}

			return numberOfSnapshots;
		}

		void createSBLRun(int seriesNumber)
		{
			const int FrameUnderTest = 6;

			Sense::Size size(720, 720);
			auto markers = std::make_shared<Pci::Core::StbMarkers>();

			auto boost = std::make_shared<StbImage>();
			boost->resize(static_cast<int>(size.width), static_cast<int>(size.height));
			boost->refMarkers.valid = true;
			auto img = std::make_shared<XrayImage>();
			img->imageIndex = 0;
			img->seriesNumber = seriesNumber;

			markers->valid = true;

			getUiStateController().imageRunStarted(XrayImage::Type::Exposure);
			getUiStateController().stentBoostImageAvailable(img, markers, boost);

			ASSERT_NE(epxToViewStateMap.end(), epxToViewStateMap.find(m_epxName));
			ASSERT_EQ(getViewState(*controller), epxToViewStateMap.at(m_epxName));

			for (int i = 1; i <= FrameUnderTest; i++)
			{
				boost = std::make_shared<StbImage>();
				img = std::make_shared<XrayImage>();
				auto registeredLive = std::make_shared<StbImage>();

				boost->resize(static_cast<int>(size.width), static_cast<int>(size.height));
				registeredLive->resize(static_cast<int>(size.width), static_cast<int>(size.height));

				img->seriesNumber = seriesNumber;
				img->imageIndex = i;
				img->imageNumber = i;
				boost->seriesNumber = seriesNumber;
				boost->imageIndex = i;
				boost->imageNumber = i;
				registeredLive->seriesNumber = seriesNumber;
				registeredLive->imageIndex = i;
				registeredLive->imageNumber = i;
				markers->valid = true;
				boost->phase = (i > (FrameUnderTest / 2)) ? StbImagePhase::contrastPhase : StbImagePhase::boostPhase;
				registeredLive->phase = boost->phase;

				getUiStateController().stentBoostImageAvailable(img, markers, boost);
				getUiStateController().boostImageAvailable(boost, registeredLive);
			}

			getUiStateController().endLive();
			getUiStateController().lastImageHold();
		}

		std::unique_ptr<::testing::NiceMock<MockIUserPrefs>> userPrefs;

		static const std::map<std::wstring, ViewState>	epxToViewStateMap;

	private:
		std::wstring m_epxName;

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
		std::unique_ptr<MainControllerUnderTest>        controller;

	};

	const std::map<std::wstring, ViewState> TestAutoArchiving::epxToViewStateMap =
	{
		{ L"SBLive", ViewState::StbLiveProcessingState },
		{ L"SBLivePost", ViewState::StbPostDeployProcessingState },
		{ L"SBLiveSubtract", ViewState::StbContrastProcessingState }
	};

	class TestAutoArchivingVariations : public TestAutoArchiving, public testing::WithParamInterface<std::tuple<std::wstring, bool, bool>>
	{
	public:
		TestAutoArchivingVariations() : TestAutoArchiving(std::get<0>(GetParam()))
		{}

		void SetUp() override
		{
			TestAutoArchiving::SetUp();

			IArchivingSettings::Feature archiveSettingFeature(IArchivingSettings::Feature::StentBoostLive);
			if (std::get<0>(GetParam()) == L"SBLivePost")
			{
				archiveSettingFeature = IArchivingSettings::Feature::StentBoostPostDeploy;
			}
			else if (std::get<0>(GetParam()) == L"SBLiveSubtract")
			{
				archiveSettingFeature = IArchivingSettings::Feature::StentBoostSubtract;
			}

			EXPECT_CALL(userPrefs->getArchivingSettings(), isAutoPACSArchiveMovieEnabled(archiveSettingFeature)).WillRepeatedly(testing::Return(std::get<1>(GetParam())));
			EXPECT_CALL(userPrefs->getArchivingSettings(), isAutoPACSArchiveSnapshotEnabled(archiveSettingFeature)).WillRepeatedly(testing::Return(std::get<2>(GetParam())));
		}
	};

	class TestAutoSnapshot : public TestAutoArchiving, public testing::WithParamInterface<std::tuple<std::wstring, std::vector<bool>>>
	{
	public:
		TestAutoSnapshot() : TestAutoArchiving(std::get<0>(GetParam()))
		{}

		void SetUp() override
		{
			TestAutoArchiving::SetUp();

			IArchivingSettings::Feature archiveSettingFeature(IArchivingSettings::Feature::StentBoostLive);
			if (std::get<0>(GetParam()) == L"SBLivePost")
			{
				archiveSettingFeature = IArchivingSettings::Feature::StentBoostPostDeploy;
			}
			else if (std::get<0>(GetParam()) == L"SBLiveSubtract")
			{
				archiveSettingFeature = IArchivingSettings::Feature::StentBoostSubtract;
			}

			EXPECT_CALL(userPrefs->getArchivingSettings(), isAutoPACSArchiveMovieEnabled(archiveSettingFeature)).WillRepeatedly(testing::Return(false));
			EXPECT_CALL(userPrefs->getArchivingSettings(), isAutoPACSArchiveSnapshotEnabled(archiveSettingFeature)).WillRepeatedly(testing::Return(true));
		}
	};

	std::unique_ptr<Merge::Library> TestAutoArchiving::mergecomLib;
	std::unique_ptr<Merge::Service> TestAutoArchiving::snapshots;
	std::unique_ptr<Merge::Service> TestAutoArchiving::movies;

	TEST_P(TestAutoSnapshot, GivenAnSblRunWithAutoSnapshotsEnabled_WhenTheRunHasValidMarkers_TheLastBoostedImageIsArchived)
	{
		auto& view = getViewCtx().stbViewer;

		auto boost = std::make_shared<StbImage>();
		Sense::Size size = view.size;
		boost->resize(static_cast<int>(size.width), static_cast<int>(size.height));
		auto img = std::make_shared<XrayImage>();
		img->imageIndex = 1;
		img->seriesNumber = 1;
		boost->imageIndex = 1;
		boost->seriesNumber = 1;

		auto markers = std::make_shared<Pci::Core::StbMarkers>();
		markers->valid = true;

		getUiStateController().imageRunStarted(XrayImage::Type::Exposure);
		getUiStateController().stentBoostImageAvailable(img, markers, boost);

		auto& frames = std::get<1>(GetParam());

		for (size_t i = 0; i < frames.size(); i++)
		{
			markers->valid = frames[i];
			getUiStateController().stentBoostImageAvailable(img, markers, boost);

			img->imageIndex++;
			boost->imageIndex++;
		}

		int lastValidFrame = -1;
		auto recording = getViewCtx().stbAdministration.prepareReview();
		if (recording)
		{
			auto& images = recording->getAllImages();
			auto it = std::find_if(std::rbegin(images), std::rend(images), [](const StbResult& stbResult) { return stbResult.markers->valid; });
			if (it != std::rend(images)) lastValidFrame = static_cast<int>(std::distance(std::begin(images), it.base()) - 1);
		}

		getUiStateController().endLive();
		getUiStateController().lastImageHold();

		waitForEncoder();

		const size_t expectedSnapshots = static_cast<size_t>(lastValidFrame == -1 ? 0 : getSnapshotsPerRun(std::get<0>(GetParam())));
		ASSERT_EQ(getPacs()->getSentImages().size(), expectedSnapshots);

		if (expectedSnapshots) ASSERT_EQ(view.currentFrame, lastValidFrame);
	}

	static std::vector<bool> SblRunMarkersValid[] =
	{
		{ true, true, true, true, true, true },
		{ true, true, false, true, true, false },
		{ true, false, true, true, false, true },
		{ false, true, false, false, true, false },
		{ true, false, true, false, true, false, true, false },
		{ false, false, false, false, false, false }
	};

	INSTANTIATE_TEST_SUITE_P(SblRuns, TestAutoSnapshot,
		::testing::Combine(
			::testing::Values(L"SBLive", L"SBLivePost"),
			::testing::ValuesIn(SblRunMarkersValid)),
		[](const testing::TestParamInfo<std::tuple<std::wstring, std::vector<bool> > >& arg)
		{
			auto& frames = std::get<1>(arg.param);
	auto it = std::find(std::rbegin(frames), std::rend(frames), true);
	if (it == std::rend(frames)) return std::to_string(arg.index) + "_NoValidMarkers";

	auto lastBoostedFrame = static_cast<int>(std::distance(std::begin(frames), it.base()) - 1);
	return std::to_string(arg.index) + "_LastValidMarkersAt_" + std::to_string(lastBoostedFrame);
		}
	);

	TEST_P(TestAutoArchivingVariations, GivenAValidSblRunWithAutoArchivingEnabled_WhenTheRunEnds_AutoArchivingIsPerformed)
	{
		createSBLRun(10);

		waitForEncoder();
		processMessagesWithTimeOut(100);

		const size_t expectedMovies = static_cast<size_t>(std::get<1>(GetParam()) ? 1 : 0);
		const size_t expectedSnapshots = static_cast<size_t>(std::get<2>(GetParam()) ? getSnapshotsPerRun(std::get<0>(GetParam())) : 0);

		ASSERT_EQ(getPacs()->getSentImages().size(), expectedSnapshots);
		if (expectedSnapshots > 0) ASSERT_EQ(getPacs()->getSentImages()[0].study.patientName, L"patient1");

		ASSERT_EQ(getPacs()->getSentSequences().size(), expectedMovies);
		if (expectedMovies > 0) ASSERT_EQ(getPacs()->getSentSequences()[0].study.patientName, L"patient1");
	}

	TEST_P(TestAutoArchivingVariations, GivenAValidSblRunWithAutoArchivingEnabled_WhenTheEpxSwitches_AutoArchivingIsPerformed)
	{
		createSBLRun(10);
		getUiStateController().epxChanged();

		waitForEncoder();
		processMessagesWithTimeOut(100);

		const size_t expectedMovies = static_cast<size_t>(std::get<1>(GetParam()) ? 1 : 0);
		const size_t expectedSnapshots = static_cast<size_t>(std::get<2>(GetParam()) ? getSnapshotsPerRun(std::get<0>(GetParam())) : 0);

		ASSERT_EQ(getPacs()->getSentImages().size(), expectedSnapshots);
		if (expectedSnapshots > 0) ASSERT_EQ(getPacs()->getSentImages()[0].study.patientName, L"patient1");

		ASSERT_EQ(getPacs()->getSentSequences().size(), expectedMovies);
		if (expectedMovies > 0) ASSERT_EQ(getPacs()->getSentSequences()[0].study.patientName, L"patient1");
	}

	TEST_P(TestAutoArchivingVariations, GivenAValidSblRunWithAutoArchivingEnabled_WhenStateReEntered_AutoArchivingIsNotPerformed)
	{
		createSBLRun(10);
		waitForEncoder();

		getUiStateController().overlayImageAvailable();
		getUiStateController().lastImageHold();

		processMessagesWithTimeOut(100);

		// Wait for switch-back
		std::this_thread::sleep_for(std::chrono::seconds(2));

		processMessagesWithTimeOut(100);

		waitForEncoder();

		const size_t expectedMovies = static_cast<size_t>(std::get<1>(GetParam()) ? 1 : 0);
		const size_t expectedSnapshots = static_cast<size_t>(std::get<2>(GetParam()) ? getSnapshotsPerRun(std::get<0>(GetParam())) : 0);

		ASSERT_EQ(getPacs()->getSentImages().size(), expectedSnapshots);
		if (expectedSnapshots > 0) ASSERT_EQ(getPacs()->getSentImages()[0].study.patientName, L"patient1");

		ASSERT_EQ(getPacs()->getSentSequences().size(), expectedMovies);
		if (expectedMovies > 0) ASSERT_EQ(getPacs()->getSentSequences()[0].study.patientName, L"patient1");
	}

	TEST_P(TestAutoArchivingVariations, GivenAValidSblRunWithAutoArchivingEnabled_WhenStateEnteredAfterPatientSwitchWithSameSeriesNumberAsPreviousSBL_AutoArchivingIsPerformed)
	{
		const int SERIES_NUMBER = 12;

		setStudy(L"PatientA", "20200101");

		createSBLRun(SERIES_NUMBER);

		waitForEncoder();
		processMessagesWithTimeOut(100);

		setStudy(L"PatientB", "20200101");

		createSBLRun(SERIES_NUMBER);
		waitForEncoder();

		processMessagesWithTimeOut(100);

		const auto NR_OF_RUNS_CREATED = 2;

		const auto expectedMovies = static_cast<size_t>(std::get<1>(GetParam()) ? NR_OF_RUNS_CREATED : 0);
		const auto expectedSnapshotsPerRun = getSnapshotsPerRun(std::get<0>(GetParam()));
		const auto expectedSnapshots = static_cast<size_t>(std::get<2>(GetParam()) ? expectedSnapshotsPerRun * NR_OF_RUNS_CREATED : 0);

		ASSERT_EQ(getPacs()->getSentImages().size(), expectedSnapshots);

		if (expectedSnapshots > 0)
		{
			for (auto i = 0; i < expectedSnapshotsPerRun; i++)
			{
				ASSERT_EQ(getPacs()->getSentImages()[i].study.patientName, L"PatientA");
				ASSERT_EQ(getPacs()->getSentImages()[i + expectedSnapshotsPerRun].study.patientName, L"PatientB");
			}
		}

		ASSERT_EQ(getPacs()->getSentSequences().size(), expectedMovies);
		if (expectedMovies > 0) ASSERT_EQ(getPacs()->getSentSequences()[0].study.patientName, L"PatientA");
		if (expectedMovies > 0) ASSERT_EQ(getPacs()->getSentSequences()[1].study.patientName, L"PatientB");
	}

	INSTANTIATE_TEST_SUITE_P(ManualAutoCombinations, TestAutoArchivingVariations,
		::testing::Combine(
			::testing::Values(L"SBLive", L"SBLivePost", L"SBLiveSubtract"),
			::testing::Values(false, true),
			::testing::Values(false, true)),
		[](const testing::TestParamInfo<std::tuple<std::wstring, bool, bool>>& arg)
		{
			return (std::get<1>(arg.param) ? "Auto" : "Manual") + std::string("Movie_") +
			(std::get<2>(arg.param) ? "Auto" : "Manual") + std::string("Snapshot_") + std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(std::get<0>(arg.param).c_str());
		});
}