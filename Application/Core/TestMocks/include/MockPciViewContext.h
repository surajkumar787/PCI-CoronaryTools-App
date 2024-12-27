// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gmock\gmock.h>
#include "Sense.h"
#include "PciViewContext.h"
#include "PciGuidanceView.h"
#include "CoRegistrationControl.h"
#include "CrmViewer.h"
#include "StbBoostViewer.h"
#include "StbMarkersViewer.h"
#include "StbRoiViewer.h"
#include "StbContrastViewer.h"
#include "StbContrastFrameSelectViewer.h"
#include "StbAdministration.h"
#include "PciReviewParameters.h"
#include "PciReviewPlayer.h"
#include "PciXrayViewer.h"
#include "PullbackRecording.h"
#include "PciEpxSelectionView.h"
#include "MockIReplayControl.h"
#include "CoRegistrationServerState.h"
#include "LicensesCollectionStub.h"

namespace Pci{ namespace Core{

class MockPciViewContext 
{
public:
	MockPciViewContext() 
	{
		gpu = std::make_unique<Gpu>(0);
			scheduler = std::make_unique<Scheduler>();
			canvas = std::make_unique< ExperienceIdentity::Canvas>(*gpu, *scheduler);
			xrayView = std::make_unique<CrmViewer>(*canvas, ViewType::Allura);
			stbView = std::make_unique<StbBoostViewer>(*canvas, ViewType::Allura);
			stbAngioView = std::make_unique<StbMarkersViewer>(*canvas, ViewType::Allura);
			stbRoiView = std::make_unique<  StbRoiViewer>(*canvas, ViewType::Allura);
			stbContrast = std::make_unique< StbContrastViewer>(*canvas, *scheduler, ViewType::Allura);
			contrastFrameSelectViewer = std::make_unique< StbContrastFrameSelectViewer>(*canvas, ViewType::Allura);
			guidanceView = std::make_unique<PciGuidanceView>(*canvas, ViewType::Allura);
			coRegControl = std::make_unique<Control>(*canvas);
			stbReviewParameters = std::make_unique<PciReviewParameters>(1, false, 0, 100);
			coRegPlayer = std::make_unique<PciReviewPlayer<PullbackRecording>>(*scheduler, *stbReviewParameters);
			stbAdmin = std::make_unique<StbAdministration>(*stbReviewParameters);
			pciEpxView = std::make_unique<PciEpxSelectionView>(*canvas, ViewType::Allura);
			currentEpx = std::make_unique<PciEpx>(Pci::Core::PciEpx::SubType::PostDeployment);
			licenseCollection = std::make_unique<LicenseCollectionStub>();

			viewContex = std::make_unique<PciViewContext>(*xrayView, *stbView, *stbAngioView, *stbRoiView, *stbContrast, *contrastFrameSelectViewer, *xrayView, *guidanceView, *guidanceView, *guidanceView, *coRegControl, *coRegPlayer, *stbReviewParameters, *stbAdmin,*pciEpxView, *guidanceView, *guidanceView, *currentEpx, *licenseCollection, nullptr, nullptr, nullptr, false, false);

	};


	~MockPciViewContext()
	{
		currentEpx = nullptr;
		licenseCollection = nullptr;
		xrayView = nullptr;
		stbView = nullptr;
		stbAngioView = nullptr;
		stbRoiView = nullptr;
		stbContrast = nullptr;
		contrastFrameSelectViewer = nullptr;
		guidanceView = nullptr;
		coRegControl = nullptr;
		coRegPlayer = nullptr;
		stbAdmin = nullptr;
		stbReviewParameters = nullptr;
		pciEpxView = nullptr;
		canvas = nullptr;
		scheduler = nullptr;
		gpu = nullptr;
	};

	PciViewContext* getPciViewContext() { return viewContex.get(); };


private:


	LicenseCollectionStub  								m_licenseCollection;
	std::unique_ptr<Scheduler>							scheduler;
	std::unique_ptr<Gpu>								gpu;
	std::unique_ptr<ExperienceIdentity::Canvas>			canvas;
	std::unique_ptr<CrmViewer>							xrayView;
	std::unique_ptr<StbBoostViewer>						stbView;
	std::unique_ptr<StbMarkersViewer>					stbAngioView;
	std::unique_ptr<StbRoiViewer>						stbRoiView;
	std::unique_ptr<StbContrastViewer>					stbContrast;
	std::unique_ptr<StbContrastFrameSelectViewer>		contrastFrameSelectViewer;
	std::unique_ptr<PciGuidanceView>					guidanceView;
	std::unique_ptr<Control>							coRegControl;
	std::unique_ptr<PciReviewPlayer<PullbackRecording>> coRegPlayer;
	std::unique_ptr<PciReviewParameters>				stbReviewParameters;
	std::unique_ptr<StbAdministration>					stbAdmin;
	std::unique_ptr<PciEpxSelectionView>				pciEpxView;
	std::unique_ptr<PciEpx>								currentEpx;
	std::unique_ptr<LicenseCollectionStub>				licenseCollection;
	std::unique_ptr<PciViewContext>						viewContex;

};

}}