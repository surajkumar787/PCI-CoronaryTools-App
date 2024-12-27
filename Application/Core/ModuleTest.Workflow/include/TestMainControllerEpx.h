// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "gtest/gtest.h"
#include "XraySourceRtoMock.h"
#include "TsmSourceStub.h"
#include "MainControllerUnderTest.h"
#include "MainModel.h"
#include "ViewType.h"
#include "mockIUserPrefs.h"

#include <memory>

using namespace Pci::Core;

// This class tests the behavior of the patient bar in the GUI when new study/exam information is available
class TestMainControllerEpx : public ::testing::Test
{
public:

	void setNewEPX(const std::wstring& epx)
	{
		xraySource->setNewEPX(epx);
	}

	Pci::Core::PciEpx &getEpx()
	{
		return viewer->getEpx();
	}

protected:

	std::wstring crmActivityType1;
	std::wstring crmActivityType2;
	std::wstring stbActivityType;

	void SetUp() override
	{
		crmActivityType1 = L"LCA";
		crmActivityType2 = L"RCA";
		stbActivityType  = L"StbPCISuite";
	
		// Connect stubbed source to mainController (plus setup supporting objects)
		scheduler .reset(new Scheduler());
		gpu       .reset(new Gpu(0));
		canvas    .reset(new Canvas(*gpu, *scheduler));
		xraySource.reset(new XraySourceRtoMock());
		xrayApc   .reset(new XrayGeometryControllerMock());
		tsm       .reset(new TsmSourceStub());
		mainModel .reset(new MainModel(m_licenseCollection, SystemType::Allura));
		mainView  .reset(new MainViewer(*canvas, *mainModel, ViewType::Allura));
		copyToRefClient.reset(new Pci::Platform::iControl::NullCopyToRefClient);
		userPrefs.reset(new ::testing::NiceMock<MockIUserPrefs>);

		viewer    .reset(new MainControllerUnderTest(*mainView, *mainModel, *xraySource, *xrayApc, *scheduler, *tsm, nullptr, nullptr, nullptr, *copyToRefClient, *userPrefs));
		xraySource->eventEpx = [this] (const XrayEpx &xrayEpx) { viewer->setEpx(xrayEpx); };
	}

private:
	LicenseCollectionStub							m_licenseCollection;
	std::unique_ptr<Scheduler>                      scheduler;
	std::unique_ptr<Gpu>                            gpu;
	std::unique_ptr<Canvas>                         canvas;
	std::unique_ptr<XraySourceRtoMock>              xraySource;
	std::unique_ptr<XrayGeometryControllerMock>     xrayApc;
	std::unique_ptr<TSM::ITsmSource>                tsm;
	std::unique_ptr<MainModel>                      mainModel;
	std::unique_ptr<MainViewer>                     mainView;
	std::unique_ptr<Pci::Platform::iControl::ICopyToRefClient> copyToRefClient;
	std::unique_ptr<MockIUserPrefs>			        userPrefs;
	std::unique_ptr<MainControllerUnderTest>        viewer;
};

TEST_F(TestMainControllerEpx, When_ActivityType_Is_For_CoronaryRoadmap_Then_EpxType_Is_Roadmap)
{
	setNewEPX(crmActivityType1);
	ASSERT_EQ ( PciEpx::Type::Roadmap, getEpx().getType() );

	setNewEPX(crmActivityType2);
	ASSERT_EQ ( PciEpx::Type::Roadmap, getEpx().getType());
}

TEST_F(TestMainControllerEpx, When_ActivityType_In_LowerCase_Is_For_CoronaryRoadmap_Then_EpxType_Is_Roadmap)
{
	// Test lower case
	setNewEPX(L"lca");
	ASSERT_EQ ( PciEpx::Type::Roadmap, getEpx().getType());

	setNewEPX(L"rca");
	ASSERT_EQ ( PciEpx::Type::Roadmap, getEpx().getType());
}

TEST_F(TestMainControllerEpx, When_ActivityType_In_CombinedString_Is_For_CoronaryRoadmap_Then_EpxType_Is_Roadmap)
{
	setNewEPX(L"rca,hallo");
	ASSERT_EQ ( PciEpx::Type::Roadmap, getEpx().getType());

	setNewEPX(L"rca,CrmPCISuite");
	ASSERT_EQ ( PciEpx::Type::Roadmap, getEpx().getType());
}

TEST_F(TestMainControllerEpx, When_ActivityType_Is_For_StentBoostLive_Then_EpxType_Is_StentBoost)
{
	setNewEPX(stbActivityType);
	ASSERT_EQ ( PciEpx::Type::StentBoost, getEpx().getType());
}

TEST_F(TestMainControllerEpx, When_ActivityType_Is_For_Other_Application_Then_EpxType_Is_NonPci)
{
	setNewEPX(L"3DRAanypos,3DRAstentboost");
	ASSERT_EQ ( PciEpx::Type::NonPci, getEpx().getType());

	setNewEPX(L"3DRAside,3DRA3dca");
	ASSERT_EQ ( PciEpx::Type::NonPci, getEpx().getType());

	setNewEPX(L"3DRAhead,3DRA3dca");
	ASSERT_EQ ( PciEpx::Type::NonPci, getEpx().getType());

	setNewEPX(L"3DRAanypos,3DRAsbsubtract");
	ASSERT_EQ ( PciEpx::Type::NonPci, getEpx().getType());

	setNewEPX(L"3DRAanypos,3DRAvascularsb,3DRAabdomen");
	ASSERT_EQ ( PciEpx::Type::NonPci, getEpx().getType());

	// Include string 'lca' and 'rca' as part of other activity types
	setNewEPX(L"3draLCA");
	ASSERT_EQ ( PciEpx::Type::NonPci, getEpx().getType());

	setNewEPX(L"3dralca");
	ASSERT_EQ ( PciEpx::Type::NonPci, getEpx().getType());

	setNewEPX(L"3drarca");
	ASSERT_EQ ( PciEpx::Type::NonPci, getEpx().getType());

	setNewEPX(L"3draRCA");
	ASSERT_EQ ( PciEpx::Type::NonPci, getEpx().getType());

}

TEST_F(TestMainControllerEpx, When_ActivityType_Is_Empty_Then_EpxType_Is_NonPci)
{
	setNewEPX(L"");
	ASSERT_EQ ( PciEpx::Type::NonPci, getEpx().getType());
}

TEST_F(TestMainControllerEpx, Given_RoadmapEpx_When_ActivityType_Is_For_StentBoostLive_Then_EpxType_Is_Roadmap)
{
	setNewEPX(crmActivityType1);
	ASSERT_EQ ( PciEpx::Type::Roadmap, getEpx().getType());

	setNewEPX(stbActivityType);
	ASSERT_EQ ( PciEpx::Type::StentBoost, getEpx().getType());
}

TEST_F(TestMainControllerEpx, Given_RoadmapEpx_When_ActivityType_Is_For_Other_Application_Then_EpxType_Is_NonPci)
{
	setNewEPX(crmActivityType1);
	ASSERT_EQ ( PciEpx::Type::Roadmap, getEpx().getType());

	setNewEPX(L"3drahead, 3draXperct");
	ASSERT_EQ ( PciEpx::Type::NonPci, getEpx().getType());
}

TEST_F(TestMainControllerEpx, Given_RoadmapEpx_When_ActivityType_Is_Empty_Then_EpxType_Is_NonPci)
{
	setNewEPX(crmActivityType1);
	ASSERT_EQ ( PciEpx::Type::Roadmap, getEpx().getType());

	setNewEPX(L"");
	ASSERT_EQ ( PciEpx::Type::NonPci, getEpx().getType());
}


