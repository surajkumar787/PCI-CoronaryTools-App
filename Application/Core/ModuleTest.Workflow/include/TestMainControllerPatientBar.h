// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "gtest/gtest.h"
#include "MainViewerUnderTest.h"
#include "MainController.h"
#include "XraySourceRtoMock.h"
#include "XrayGeometryControllerMock.h"
#include "TsmSourceStub.h"
#include "MainModel.h"
#include "ViewType.h"
#include "SystemType.h"
#include "MockIArchivingSettings.h"
#include "MockIUserPrefs.h"
#include "LicensesCollectionStub.h"

#include <memory>

using namespace Pci::Core;

// This class tests the behavior of the patient bar in the GUI when new study/exam information is available
class TestMainControllerPatientBar : public ::testing::Test
{
public:
	Canvas& getCanvas()
	{
		return *canvas;
	}

	MainModel& getMainModel()
	{
		return *mainModel;
	}

	MainViewerUnderTest& getMainView()
	{
		return *mainView;
	}

	XraySourceRtoMock& getXraySource()
	{
		return *xraySource;
	}

	Scheduler& getScheduler()
	{
		return *scheduler;
	}

	MainController& getController()
	{
		return *controller;
	}

	TSM::ITsmSource& getTSM()
	{
		return *tsm;
	}

	XrayGeometryControllerMock& getXrayApc()
	{
		return *xrayApc;
	}

	void setMainView( std::unique_ptr<MainViewerUnderTest> newmainView)
	{
		mainView.swap(newmainView);
	}

	void setController( std::unique_ptr<MainController> mainController)
	{
		controller.swap(mainController);
	}

protected:

	void SetUp() override
	{
		Sense::Logging::eventAssert = [&](const wchar_t *t) { std::wcout << L"Sense assert: " << t << std::endl; FAIL(); };
		Sense::Logging::eventError = [&](const wchar_t *t) { std::wcout << L"Sense error: " << t << std::endl; FAIL(); };

		// Connect stubbed source to mainController (plus setup supporting objects)
		scheduler .reset(new Scheduler());
		gpu       .reset(new Gpu(0));
		canvas    .reset(new Canvas(*gpu, *scheduler));
		xraySource.reset(new XraySourceRtoMock());
		xrayApc   .reset(new XrayGeometryControllerMock());
		tsm       .reset(new TsmSourceStub());
		mainModel .reset(new MainModel(m_licenseCollection, SystemType::Allura));
		copyToRefClient.reset(new Pci::Platform::iControl::NullCopyToRefClient);
		userPrefs.reset(new ::testing::NiceMock<MockIUserPrefs>);
		
	}

	std::unique_ptr<Pci::Platform::iControl::ICopyToRefClient> copyToRefClient;
	std::unique_ptr<MockIUserPrefs>			userPrefs;
	LicenseCollectionStub					m_licenseCollection;

private:
	std::unique_ptr<Scheduler>                      scheduler;
	std::unique_ptr<Gpu>                            gpu;
	std::unique_ptr<Canvas>                         canvas;
	std::unique_ptr<XraySourceRtoMock>              xraySource;
	std::unique_ptr<XrayGeometryControllerMock>     xrayApc;
	std::unique_ptr<TSM::ITsmSource>                tsm;
	std::unique_ptr<MainModel>                      mainModel;
	std::unique_ptr<MainViewerUnderTest>            mainView;
	std::unique_ptr<MainController>                 controller;
};

TEST_F(TestMainControllerPatientBar, WhenNewStudyIsAvailable_Then_PatientNameIsDisplayedInAlluraGUI)
{
	setMainView( std::make_unique<MainViewerUnderTest>(getCanvas(), getMainModel(), ViewType::Allura));

	setController(std::make_unique< MainController>(getMainView(), getMainModel(), getXraySource(), getXrayApc(), L"", L"", getScheduler(), getTSM(), nullptr, nullptr, nullptr,
	                                    false, false, CommonPlatform::Xray::SystemType::Allura, *copyToRefClient, *userPrefs));
	getXraySource().eventStudy = [this] (const XrayStudy &study) { getController().setStudy(study); };
	std::wstring rawPatient(L"Bergkamp^Dennis");
    std::wstring patient(L"Bergkamp, Dennis");
    ASSERT_NE(patient, getMainView().getPatientName());

	getXraySource().setNewPatient(rawPatient);
    ASSERT_EQ(patient, getMainView().getPatientName());
}

TEST_F(TestMainControllerPatientBar, WhenNewStudyIsAvailable_Then_PatientNameIsDisplayedInSmSFullHDGUI)
{
	setMainView(std::make_unique<MainViewerUnderTest>(getCanvas(), getMainModel(), ViewType::SmartSuiteFullHD));

	setController(std::make_unique< MainController>(getMainView(), getMainModel(), getXraySource(), getXrayApc(), L"", L"", getScheduler(), getTSM(), nullptr, nullptr, nullptr,
	                                    false, false, CommonPlatform::Xray::SystemType::SmartSuite, *copyToRefClient, *userPrefs));
	getXraySource().eventStudy = [this] (const XrayStudy &study) { getController().setStudy(study); };

	std::wstring rawPatient(L"Bergkamp^Dennis");
	std::wstring patient(L"BERGKAMP, Dennis");
	ASSERT_NE(patient, getMainView().getPatientName());

	getXraySource().setNewPatient(rawPatient);
	ASSERT_EQ(patient, getMainView().getPatientName());
}

TEST_F(TestMainControllerPatientBar, WhenNewStudyIsAvailable_Then_PatientNameIsDisplayedInSmSFlexvisionGUI)
{
	setMainView(std::make_unique<MainViewerUnderTest>(getCanvas(), getMainModel(), ViewType::SmartSuiteFlexVision));

	setController(std::make_unique< MainController>(getMainView(), getMainModel(), getXraySource(), getXrayApc(), L"", L"", getScheduler(), getTSM(), nullptr, nullptr, nullptr,
	                                    false, false, CommonPlatform::Xray::SystemType::SmartSuite, *copyToRefClient, *userPrefs));
	getXraySource().eventStudy = [this] (const XrayStudy &study) { getController().setStudy(study); };

	std::wstring rawPatient(L"Bergkamp^Dennis");
	std::wstring patient(L"BERGKAMP, Dennis");
	ASSERT_NE(patient, getMainView().getPatientName());

	getXraySource().setNewPatient(rawPatient);
	ASSERT_EQ(patient, getMainView().getPatientName());
}

TEST_F(TestMainControllerPatientBar, WhenNewStudyIsAvailableAndPatientNameDoNotContainTrailingCarate_Then_PatientNameIsDisplayedCorrectlyOnSmartSuite)
{
	setMainView(std::make_unique<MainViewerUnderTest>(getCanvas(), getMainModel(), ViewType::SmartSuiteFlexVision));

	setController(std::make_unique< MainController>(getMainView(), getMainModel(), getXraySource(), getXrayApc(), L"", L"", getScheduler(), getTSM(), nullptr, nullptr, nullptr,
		false, false, CommonPlatform::Xray::SystemType::SmartSuite, *copyToRefClient, *userPrefs));
	getXraySource().eventStudy = [this](const XrayStudy& study) { getController().setStudy(study); };

	//Only LAST Name
	std::wstring rawPatient(L"Bergkamp");
	std::wstring patient(L"BERGKAMP");

	getXraySource().setNewPatient(rawPatient);
	ASSERT_EQ(patient, getMainView().getPatientName());

	//Only FIRST Name
	rawPatient= L"^Dennis";
	patient = L"Dennis";
	getXraySource().setNewPatient(rawPatient);
	ASSERT_EQ(patient, getMainView().getPatientName());

	//Only MIDDLE Name
	rawPatient = L"^^Nicolaas";
	patient = L", Nicolaas";
	getXraySource().setNewPatient(rawPatient);
	ASSERT_EQ(patient, getMainView().getPatientName());
}

TEST_F(TestMainControllerPatientBar, WhenNewStudyIsAvailableAndPatientNameContainTrailingCarate_Then_PatientNameIsDisplayedCorrectlyOnSmartSuite)
{
	setMainView(std::make_unique<MainViewerUnderTest>(getCanvas(), getMainModel(), ViewType::SmartSuiteFlexVision));

	setController(std::make_unique< MainController>(getMainView(), getMainModel(), getXraySource(), getXrayApc(), L"", L"", getScheduler(), getTSM(), nullptr, nullptr, nullptr,
		false, false, CommonPlatform::Xray::SystemType::SmartSuite, *copyToRefClient, *userPrefs));
	getXraySource().eventStudy = [this](const XrayStudy& study) { getController().setStudy(study); };

	//Only LAST Name
	std::wstring rawPatient(L"Bergkamp^^^");
	std::wstring patient(L"BERGKAMP");

	getXraySource().setNewPatient(rawPatient);
	ASSERT_EQ(patient, getMainView().getPatientName());

	//Only FIRST Name
	rawPatient = L"^Dennis^^";
	patient = L"Dennis";
	getXraySource().setNewPatient(rawPatient);
	ASSERT_EQ(patient, getMainView().getPatientName());

	//Only MIDDLE Name
	rawPatient = L"^^Nicolaas^";
	patient = L", Nicolaas";
	getXraySource().setNewPatient(rawPatient);
	ASSERT_EQ(patient, getMainView().getPatientName());
}