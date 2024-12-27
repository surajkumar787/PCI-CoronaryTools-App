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
#include "ViewType.h"
#include "NullCopyToRefClient.h"
#include "MockIArchivingSettings.h"
#include "MockIUserPrefs.h"
#include "LicensesCollectionStub.h"

#include <memory>

namespace ModuleTest{
using namespace Pci::Core;

const double RoadmapAngulation = 7.0;

// This class tests the behavior of the APC programming when cwis connections become available
class TestMainControllerApcProgramming : public ::testing::Test
{
public:

	TestMainControllerApcProgramming():
		m_log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		m_licenseCollection(),
		controller(nullptr)
	{}

	void setApcConnectionChanged(bool connected)
	{
		controller->setApcConnectionChanged(connected);
	}

	void connectionChanged(bool status, IXraySourceRto::CwisStatus cwissStatus, IXraySourceRto::DvlpStatus dvlpStatus,const std::wstring cwisHost, const std::wstring dvlpAdapter)
	{
		controller->connectionChanged(status, cwissStatus, dvlpStatus, cwisHost, dvlpAdapter);
	}

	void setGeometry(const XrayGeometry &xrayGeo)
	{
		controller->setGeometry(xrayGeo);
	}

	void endGeometry()
	{
		controller->endGeometry();
	}

	Pci::Core::PciViewContext& getViewCtx()
	{
		return controller->getViewCtx();
	}

	XrayGeometryControllerMock* getXrayApc()
	{
		return xrayApc.get();
	}

protected:

	void SetUp() override
	{
		loadRoadmap();

		// Connect stubbed source to mainController (plus setup supporting objects)
		scheduler .reset(new Scheduler());
		gpu       .reset(new Gpu(0));
		canvas    .reset(new Canvas(*gpu, *scheduler));
		tsm       .reset(new TsmSourceStub());
		  
		mainModel .reset(new MainModel(m_licenseCollection, SystemType::Allura));

		xraySource.reset(new XraySourceRtoMock());
		xrayApc   .reset(new XrayGeometryControllerMock());
		mainView  .reset(new MainViewerUnderTest(*canvas, *mainModel, ViewType::SmartSuiteFlexVision));
		copyToRefClient.reset(new Pci::Platform::iControl::NullCopyToRefClient);
		userPrefs.reset(new ::testing::NiceMock<MockIUserPrefs>);
	}

	// clear the repository and add a dummy roadmap to it
	void loadRoadmap()
	{
		CrmParameters params(L"crm.ini");

		CrmRepository::deletePersistentData(m_log, params.storeRoadmapFolder);


		CrmRepository repo(params.storeRoadmapFolder);
		CrmAdministration admin(params, repo);

		auto attribs = GetFileAttributes(repo.getPath().c_str());
		if (attribs == INVALID_FILE_ATTRIBUTES)
		{
			std::wcerr << L"Target directory ('" << repo.getPath() << L"') does not exist. Did you forget to 'subst d: c:\\<something>'?\n";
			FAIL();
		}
		else if (attribs & ~FILE_ATTRIBUTE_DIRECTORY)
		{
			std::wcerr << L"Target directory ('" << repo.getPath() << L"') is not a directory\n";
			FAIL();
		}

		XrayGeometry geo;
		geo.angulation = RoadmapAngulation;
		auto roadmap = std::make_shared<CrmRoadmap>(25, 25, 512, 512, Point(0.15), geo, Matrix(), 0.9);
		roadmap->data.resize(512);
		auto overlay = std::make_shared<CrmOverlay>();
		overlay->resize(512,512);
		roadmap->overlay = overlay;
		repo.saveRoadmapToFile(roadmap);
		admin.addRoadmap(roadmap);
	}

	void createController(bool automaticActivation)
	{
		EXPECT_CALL(*userPrefs, getApcAutoActivation()).WillRepeatedly(testing::Return(automaticActivation));
		controller.reset(new MainControllerUnderTest(*mainView, *mainModel, *xraySource, *xrayApc, *scheduler, *tsm, nullptr, nullptr, nullptr, *copyToRefClient, *userPrefs));
		ASSERT_FALSE(controller->getCrmAdmin().isEmpty());

		xraySource->setNewEPX(L"LCA");
	}

private:
	CommonPlatform::Log&							m_log;
	LicenseCollectionStub  						    m_licenseCollection;
	std::unique_ptr<Scheduler>                      scheduler;
	std::unique_ptr<Gpu>                            gpu;
	std::unique_ptr<Canvas>                         canvas;
	std::unique_ptr<TSM::ITsmSource>                tsm;
	std::unique_ptr<MainModel>                      mainModel;

	std::unique_ptr<XraySourceRtoMock>              xraySource;
	std::unique_ptr<XrayGeometryControllerMock>     xrayApc;
	std::unique_ptr<MainViewerUnderTest>            mainView;
	std::unique_ptr<Pci::Platform::iControl::ICopyToRefClient> copyToRefClient;
	std::unique_ptr<MockIUserPrefs>					userPrefs;
	std::unique_ptr<MainControllerUnderTest>        controller;
};

TEST_F(TestMainControllerApcProgramming, GivenARoadmapNearbyWithAutomaticActivation_WhenCwisConnectsBeforeAPC_ThenApcIsActivated)
{
	createController(true);

	setApcConnectionChanged(false);
	connectionChanged(false, IXraySourceRto::CwisStatus::NotConnectedSinceNoActiveConnections, IXraySourceRto::DvlpStatus::Connected, L"", L"");
	setGeometry(XrayGeometry());
	endGeometry();
	connectionChanged(true, IXraySourceRto::CwisStatus::Connected, IXraySourceRto::DvlpStatus::Connected, L"", L"");

	setApcConnectionChanged(true);

	// check if the app and UI are in the correct state
	ASSERT_EQ(CrmAdministration::Status::MoveArc, getViewCtx().crmAdminStatus);
	ASSERT_EQ(CrmApcController::ApcModeStatus::AutomaticActivation, getViewCtx().crmApcStatus);
	ASSERT_TRUE(getViewCtx().crmApcAutomaticActivation);

	// check if apc is programmed
	XrayGeometry geo; geo.angulation = RoadmapAngulation;
	ASSERT_DOUBLE_EQ(geo.angulation, getXrayApc()->getLastAngulation());
	ASSERT_TRUE(getXrayApc()->getLastActivate());
}

TEST_F(TestMainControllerApcProgramming, GivenARoadmapNearbyWithAutomaticActivation_WhenAPCConnectsBeforeCWIS_ThenApcIsActivated)
{
	createController(true);

	setApcConnectionChanged(false);
	connectionChanged(false, IXraySourceRto::CwisStatus::NotConnectedSinceNoActiveConnections, IXraySourceRto::DvlpStatus::Connected, L"", L"");
	setApcConnectionChanged(true);
	setGeometry(XrayGeometry());
	endGeometry();
	connectionChanged(true, IXraySourceRto::CwisStatus::Connected, IXraySourceRto::DvlpStatus::Connected, L"", L"");

	// check if the app and UI are in the correct state
	ASSERT_EQ(CrmAdministration::Status::MoveArc, getViewCtx().crmAdminStatus);
	ASSERT_EQ(CrmApcController::ApcModeStatus::AutomaticActivation, getViewCtx().crmApcStatus);
	ASSERT_TRUE( getViewCtx().crmApcAutomaticActivation);

	// check if apc is programmed
	XrayGeometry geo; geo.angulation = RoadmapAngulation;
	ASSERT_DOUBLE_EQ(geo.angulation, getXrayApc()->getLastAngulation());
	ASSERT_TRUE(getXrayApc()->getLastActivate());
}

TEST_F(TestMainControllerApcProgramming, GivenARoadmapNearbyWithoutAutomaticActivation_WhenCwisConnectsBeforeAPC_ThenApcIsEnabled)
{
	createController(false);

	setApcConnectionChanged(false);
	connectionChanged(false, IXraySourceRto::CwisStatus::NotConnectedSinceNoActiveConnections, IXraySourceRto::DvlpStatus::Connected, L"", L"");
	setGeometry(XrayGeometry());
	endGeometry();
	connectionChanged(true, IXraySourceRto::CwisStatus::Connected, IXraySourceRto::DvlpStatus::Connected, L"", L"");

	setApcConnectionChanged(true);

	// check if the app and UI are in the correct state
	ASSERT_EQ(CrmAdministration::Status::MoveArc, getViewCtx().crmAdminStatus);
	ASSERT_EQ(CrmApcController::ApcModeStatus::TargetSetWithoutActivaton, getViewCtx().crmApcStatus);
	ASSERT_FALSE(getViewCtx().crmApcAutomaticActivation);

	// check if apc is programmed
	XrayGeometry geo; geo.angulation = RoadmapAngulation;
	ASSERT_DOUBLE_EQ(geo.angulation, getXrayApc()->getLastAngulation());
	ASSERT_FALSE(getXrayApc()->getLastActivate());
}

TEST_F(TestMainControllerApcProgramming, GivenARoadmapNearbyWithoutAutomaticActivation_WhenAPCConnectsBeforeCWIS_ThenApcIsEnabled)
{
	createController(false);

	setApcConnectionChanged(false);
	setApcConnectionChanged(true);

	connectionChanged(false, IXraySourceRto::CwisStatus::NotConnectedSinceNoActiveConnections, IXraySourceRto::DvlpStatus::Connected, L"", L"");
	setGeometry(XrayGeometry());
	endGeometry();
	connectionChanged(true, IXraySourceRto::CwisStatus::Connected, IXraySourceRto::DvlpStatus::Connected, L"", L"");

	// check if the app and UI are in the correct state
	ASSERT_EQ(CrmAdministration::Status::MoveArc, getViewCtx().crmAdminStatus);
	ASSERT_EQ(CrmApcController::ApcModeStatus::TargetSetWithoutActivaton, getViewCtx().crmApcStatus);
	ASSERT_FALSE(getViewCtx().crmApcAutomaticActivation);

	// check if apc is programmed
	XrayGeometry geo; geo.angulation = RoadmapAngulation;
	ASSERT_DOUBLE_EQ(geo.angulation, getXrayApc()->getLastAngulation());
	ASSERT_FALSE(getXrayApc()->getLastActivate());
}
}