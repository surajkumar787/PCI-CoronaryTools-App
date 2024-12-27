// Copyright Koninklijke Philips N.V. 2018
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
#include "MockICrmAdministration.h"
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
#include "StbRecording.h"
#include "LicensesCollection.h"
#include "IArchivingStatus.h"
#include "MockIUserPrefs.h"
#include "MockIPACSPush.h"
#include "MockIMessageBuilder.h"
#include "Thread.h"
#include "ViewStateHelper.h"

using namespace Sense;
using namespace Pci::Core;

class TestUiState : public testing::Test, protected IArchivingStatus
{
public:
	Gpu									gpu;
	LicenseCollectionStub               licenseCollection;
	Scheduler							scheduler;
	ExperienceIdentity::Canvas			canvas;
	CrmViewer							viewer;
	CrmRoadmapSelectViewer				crmRoadmapSelectViewer;
	StbBoostViewer						stbBoostViewer;
	StbMarkersViewer					stbMarkersViewer;
	StbRoiViewer						stbRoiViewer;
	StbContrastViewer					stbContrastViewer;
    StbImageSelectViewer		        stbContrastFrameSelectViewer;
    StbImageSelectViewer        		stbBoostFrameSelectViewer;
    CoRegistrationControl				coRegControl;
	PciGuidanceView						crmGuidance;
	PciGuidanceView						stbGuidance;
	PciEpxSelectionView					epxSelection;
	PciReviewParameters					reviewParameters;
	StbAdministration					stbAdmin;
	PciReviewPlayer<PullbackRecording>	coRegPlayer;
	PciGuidanceView						coregGuidance;
	PciGuidanceView						noLicense;
	PciGuidanceView						noConnection;
	::testing::NiceMock<MockICrmAdministration>	crmAdministration;
	::testing::NiceMock<MockIPACSPush>	pacsPush;
	::testing::NiceMock<PACS::MockIMessageBuilder>	messageBuilder;
	CommonPlatform::Thread				encodingThread;
	PciViewContext						viewContext;
	::testing::NiceMock<MockIUserPrefs>	userPrefs;

	TestUiState() :
		gpu(0),
		scheduler(),
		canvas(gpu, scheduler),
		viewer(canvas, ViewType::Allura),
		crmRoadmapSelectViewer(canvas, ViewType::Allura),
		stbBoostViewer(canvas, ViewType::Allura),
		stbMarkersViewer(canvas, ViewType::Allura),
		stbRoiViewer(canvas, ViewType::Allura),
		stbContrastViewer(canvas, scheduler, ViewType::Allura),
        stbContrastFrameSelectViewer(canvas, ViewType::Allura, false),
        stbBoostFrameSelectViewer(canvas, ViewType::Allura, true),
        crmGuidance(viewer, ViewType::Allura),
		stbGuidance(viewer, ViewType::Allura),
		epxSelection(viewer, ViewType::Allura),
		noLicense(viewer, ViewType::Allura),
		noConnection(viewer, ViewType::Allura),
		coregGuidance(viewer, ViewType::Allura),
		coRegControl(viewer, ViewType::Allura),
		reviewParameters(1, 0.0, 0, 100),
		coRegPlayer(scheduler, reviewParameters),
		stbAdmin(reviewParameters),
		crmAdministration(),
		encodingThread(scheduler),
		viewContext(viewer, stbBoostViewer, stbMarkersViewer, stbRoiViewer, stbContrastViewer, stbContrastFrameSelectViewer, stbBoostFrameSelectViewer, viewer, crmGuidance, crmRoadmapSelectViewer, stbGuidance, coregGuidance, coRegControl, coRegPlayer, reviewParameters, stbAdmin, epxSelection, noConnection, noLicense,
			PciEpx(), licenseCollection, &pacsPush, &messageBuilder, &encodingThread, crmAdministration, true, true, SystemType::Allura)
	{
		Sense::Logging::eventAssert = [&](const wchar_t *t) { std::wcout << L"Sense assert: " << t << std::endl; FAIL(); };
		Sense::Logging::eventError = [&](const wchar_t *t) { std::wcout << L"Sense error: " << t << std::endl; FAIL(); };

		addImageToStentBoostAdministration();
	}

	~TestUiState()
	{
	}

	void addImageToStentBoostAdministration()
	{
		auto img = std::make_shared<CommonPlatform::Xray::XrayImage>();
		auto markers = std::make_shared<StbMarkers>();
		markers->valid = true;

		stbAdmin.setImage(img, markers, nullptr);
	}

	StbResult createStbResult(int runIndex, int imageIndex, int width, int height, bool validMarkers)
	{
		return createStbResult(runIndex, imageIndex, width, height, validMarkers, false);
	}

	StbResult createStbResult(int runIndex, int imageIndex, int width, int height, bool validMarkers, bool withContrast)
	{
		auto image = std::make_shared<CommonPlatform::Xray::XrayImage>();
		image->runIndex = runIndex;
		image->seriesNumber = runIndex;
		image->type = CommonPlatform::Xray::XrayImage::Type::Exposure;
		image->imageNumber = imageIndex;
		image->imageIndex = imageIndex;
		image->width = width;
		image->height = height;

        auto boostImage = std::make_shared<StbImage>();
        if (validMarkers)
        {
            boostImage->imageNumber = imageIndex;
			boostImage->imageIndex = imageIndex;
            boostImage->phase = withContrast ? StbImagePhase::contrastPhase : StbImagePhase::boostPhase;
            boostImage->resize(width, height);
        }

		auto registeredLive = std::make_shared<StbImage>();
		if (validMarkers)
		{
			registeredLive->imageNumber = imageIndex;
			registeredLive->imageIndex = imageIndex;
			registeredLive->phase = withContrast ? StbImagePhase::contrastPhase : StbImagePhase::boostPhase;
			registeredLive->resize(width, height);
		}

		auto markers = std::make_shared<Pci::Core::StbMarkers>();
		markers->valid = validMarkers;

		return StbResult{ image, markers, registeredLive, boostImage };
	}

	virtual void setSnapshotStatus(ArchivingStatus /*status*/, CaptureArea /*area*/) override {};
	virtual void setMovieStatus(ArchivingStatus /*status*/) {}

	virtual void clearStatus() override {}

    ViewState getGuidanceStateByEpxSelection() const
    {
        if (viewContext.currentEpx.getType() == PciEpx::Type::Roadmap)
        {
            return ViewStateHelper::DetermineCrmState(viewContext.crmAdminStatus, viewContext.roadmapErrorState);
        }
        else if (viewContext.currentEpx.getType() == PciEpx::Type::StentBoost)
        {
            if (viewContext.currentEpx.getSubType() == PciEpx::SubType::Live)
            {
                return ViewState::StbLiveGuidanceState;
            }
            else if (viewContext.currentEpx.getSubType() == PciEpx::SubType::Subtract)
            {
                return ViewState::StbContrastGuidanceState;
            }
            return ViewState::StbPostDeployGuidanceState;
        }
        else // Non-PCI EPX
        {
            if (viewContext.crmAdminStatus != CrmAdministration::Status::NoRoadmap)
            {
                // If a Roadmap has been created, allow fluoro overlay for each EPX.
                return ViewStateHelper::DetermineCrmState(viewContext.crmAdminStatus, viewContext.roadmapErrorState);
            }
            else
            {
                return ViewState::NonPciEpxState;
            }
        }
    }
};


