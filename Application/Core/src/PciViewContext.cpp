// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PciViewContext.h"
#include "CrmAdministration.h"
#include "ILicenseCollection.h"
#include "pciGuidanceView.h"
#include "PciEpxSelectionView.h"

namespace Pci { namespace Core{

PciViewContext::PciViewContext(PciXrayViewer &crmView, 
							   StbBoostViewer &stbView, 
							   StbMarkersViewer &stbAngioView,
							   StbRoiViewer& stbRoiView,
							   StbContrastViewer& stbContrastViewer,
                               StbImageSelectViewer& contrastFrameSelect,
                               StbImageSelectViewer& boostFrameSelect,
                               PciXrayViewer &coRegView,
							   PciGuidanceView &crmGuidanceView,
							   CrmRoadmapSelectViewer& roadmapSelectView,
                               PciGuidanceView &stbGuidanceView,
							   PciGuidanceView &coRegGuidanceView,
							   Control &coRegControl,
							   PciReviewPlayer<PullbackRecording> &coRegPlayer,
							   PciReviewParameters &stbReviewParams,
							   StbAdministration &stbAdmin,
							   PciEpxSelectionView& pciEpxView,
							   PciGuidanceView& noConnectionView,
							   PciGuidanceView& noLicView,
							   const PciEpx &pciEpx,
							   const ILicenseCollection& licenses,
							   PACS::IPACSPush* pacs,
							   PACS::IMessageBuilder* msgBuild,
							   ::CommonPlatform::Thread* threadEnc,
							   ICrmAdministration& crmAdministration,
							   bool pacsSnapshots,
							   bool pacsMovies,
							   CommonPlatform::Xray::SystemType system) :
	crmViewer(crmView),
	crmGuidance(crmGuidanceView),
	crmRoadmapSelectViewer(roadmapSelectView),
	stbViewer(stbView),
	stbAngioViewer(stbAngioView),
	stbRoiViewer(stbRoiView),
	stbContrastViewer(stbContrastViewer),
    stbContrastSelectViewer(contrastFrameSelect),
    stbBoostSelectViewer(boostFrameSelect),
    stbGuidance(stbGuidanceView),
	coRegViewer(coRegView),
	coRegistrationGuidance(coRegGuidanceView),
	coRegistrationControl(coRegControl),
	stbReviewParameters(stbReviewParams),
	stbAdministration(stbAdmin),
	pciEpxSelection(pciEpxView),
	noConnection(noConnectionView),
	noLicenseView(noLicView),
    currentEpx(pciEpx),
	roadmapErrorState(PciSuite::CrmErrors::Error::OK),
	crmAdminStatus(CrmAdministration::Status::NoRoadmap),
	crmAdminIsEmpty(true),
	crmApcStatus(CrmApcController::ApcModeStatus::AutomaticActivation),
	crmApcAutomaticActivation(false),
	coRegState(),
	coRegError(::CommonPlatform::CoRegistration::CoRegistrationError::OK),
	coRegRecordedRunIndex(0),
	forceHideCoregControl(true),
	fluoroAcquiredWhileRecording(false),
	coRegistrationPlayer(coRegPlayer),
	licenseCollection(licenses),
	crmAdmin(crmAdministration),
	pacsPush(pacs),
	msgBuilder(msgBuild),
	threadEncoding(threadEnc),
	snapshotsAvailable(pacsSnapshots),
	moviesAvailable(pacsMovies),
    viewXrayConnectionStatus(NeverConnected),
	geometry(),
    geometryIsMoving(false),
    xrayModalityConnected(false),
	exposureActive(false),
	currentPatient(L""),
	systemType(system)
{
}
}}