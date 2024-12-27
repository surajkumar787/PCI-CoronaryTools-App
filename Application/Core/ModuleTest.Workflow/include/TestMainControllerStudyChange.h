// Copyright Koninklijke Philips N.V. 2021
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

#include "LicensesCollectionStub.h"

using namespace Pci::Core;

class TestMainControllerStudyChange : public ::testing::Test
{
public:
    void SetUp() override
    {
        scheduler.reset(new Scheduler());
        gpu.reset(new Gpu(0));
        canvas.reset(new Canvas(*gpu, *scheduler));
        xraySource.reset(new XraySourceRtoMock());
        xrayApc.reset(new XrayGeometryControllerMock());
        tsm.reset(new TsmSourceStub());
        mainModel.reset(new MainModel(m_licenseCollection, SystemType::Allura));
        mainView.reset(new MainViewer(*canvas, *mainModel, ViewType::Allura));
        copyToRefClient.reset(new Pci::Platform::iControl::NullCopyToRefClient);
        userPrefs.reset(new ::testing::NiceMock<MockIUserPrefs>);

        mainController.reset(new MainControllerUnderTest(*mainView, *mainModel, *xraySource, *xrayApc, *scheduler, *tsm, nullptr, nullptr, nullptr, *copyToRefClient, *userPrefs));
    }

    void addStbImagesToStbAdministration(int images)
    {
        auto& stbAdmin = mainController->getViewCtx().stbAdministration;
        for (int i = 0; i < images; i++)
        {
            auto xrayImage = std::make_shared<XrayImage>();
            xrayImage->imageIndex = i;
            stbAdmin.setImage(xrayImage, std::make_shared<const StbMarkers>(), std::make_shared<const StbImage>());
        }
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

protected:
    std::unique_ptr<MainControllerUnderTest> mainController;
};

TEST_F(TestMainControllerStudyChange, When_StudyChanged_StbAdminIsCleared)
{
    XrayStudy oldStudy;
    oldStudy.patientId = L"oldPatientID";

    mainController->setStudy(oldStudy);
    mainController->getViewCtx().stbAdministration.setImage(std::make_shared<const XrayImage>(), std::make_shared<const StbMarkers>(), std::make_shared<const StbImage>());

    addStbImagesToStbAdministration(10);

    ASSERT_FALSE(mainController->getViewCtx().stbAdministration.isEmpty());

    XrayStudy newStudy;
    newStudy.patientId = L"newPatientID";

    mainController->setStudy(newStudy);
    ASSERT_TRUE(mainController->getViewCtx().stbAdministration.isEmpty());
}

TEST_F(TestMainControllerStudyChange, When_StudyNotChanged_StbAdminIsNotCleared)
{
    XrayStudy study;
    study.patientId = L"patientID";

    mainController->setStudy(study);
    mainController->getViewCtx().stbAdministration.setImage(std::make_shared<const XrayImage>(), std::make_shared<const StbMarkers>(), std::make_shared<const StbImage>());

    addStbImagesToStbAdministration(10);

    ASSERT_FALSE(mainController->getViewCtx().stbAdministration.isEmpty());

    mainController->setStudy(study);
    ASSERT_FALSE(mainController->getViewCtx().stbAdministration.isEmpty());
}