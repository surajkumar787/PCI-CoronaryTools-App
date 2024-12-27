// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "PciEpx.h"
#include "CrmAdministration.h"
#include "CoRegistrationServerState.h"
#include "CoRegistrationError.h"
#include "CrmApcController.h"
#include "SystemType.h"
#include <vector>

namespace CommonPlatform { class Thread; }

namespace Pci { namespace Core
{
	class PciGuidanceView;
	class PciXrayViewer;
	class PciEpxSelectionView;
	class ConnectingAtStartupView;
	template<typename T>
	class PciReviewPlayer;
	class PullbackRecording;
	struct PciReviewParameters;
	class StbAdministration;
	class StbBoostViewer;
	class StbMarkersViewer;
	class StbRoiViewer;
	class StbContrastViewer;
    class StbImageSelectViewer;
    class CrmRoadmapSelectViewer;

	class ILicenseCollection;
	enum class LicenseFeature;

	namespace PACS
	{
		class IPACSPush;
		class IMessageBuilder;
	}

    enum ViewXrayConnectionStatus
	{
	    NeverConnected,
        ConnectedSinceStartup,
        TimeOut,
        ClosingConnectionAtShutdown,
	};

	struct PciViewContext
	{
    public:
		PciViewContext(	PciXrayViewer &crmView, 
						StbBoostViewer &stbView, 
						StbMarkersViewer &stbAngioView,
						StbRoiViewer& stbRoiView,
						StbContrastViewer& stbContrast,
                        StbImageSelectViewer& contrastFrameSelect,
                        StbImageSelectViewer& boostFrameSelect,
                        PciXrayViewer &coRegView,
						PciGuidanceView &crmGuidanceView, 
						CrmRoadmapSelectViewer& roadmapSelectView,
						PciGuidanceView &stbGuidanceView,
						PciGuidanceView &coRegGuidanceView,
						Control &coRegControl,
						PciReviewPlayer<PullbackRecording> &coRegPlayer,
						PciReviewParameters &stbReviewParameters,
						StbAdministration &stbAdmin,
						PciEpxSelectionView& pciEpxView,
                        PciGuidanceView& noConnectionView, 
						PciGuidanceView& noLicenseView,
						const PciEpx &currentEpx,
						const ILicenseCollection& licenseCollection,
						PACS::IPACSPush* pacsPush,
						PACS::IMessageBuilder* msgBuilder,
						::CommonPlatform::Thread* threadEncoding,
						ICrmAdministration& crmAdministration,
						bool pacsSnapshots,
						bool pacsMovies,
						CommonPlatform::Xray::SystemType system);

		PciXrayViewer			&crmViewer;
		PciGuidanceView			&crmGuidance;
		CrmRoadmapSelectViewer  &crmRoadmapSelectViewer;
		StbBoostViewer			&stbViewer;
		StbMarkersViewer		&stbAngioViewer;
		StbRoiViewer			&stbRoiViewer;
		StbContrastViewer		&stbContrastViewer;
        StbImageSelectViewer    &stbContrastSelectViewer;
        StbImageSelectViewer    &stbBoostSelectViewer;
        PciGuidanceView			&stbGuidance;
        PciXrayViewer			&coRegViewer;
		PciGuidanceView			&coRegistrationGuidance;
		Control					&coRegistrationControl;

		PciReviewParameters		&stbReviewParameters;
		StbAdministration		&stbAdministration;

		PciEpxSelectionView     &pciEpxSelection;
        PciGuidanceView		    &noConnection;
		PciGuidanceView         &noLicenseView;

		PciEpx								currentEpx;
		PciSuite::CrmErrors::Error			roadmapErrorState;
		std::shared_ptr<const CrmRoadmap>	currentRoadmap;
		CrmAdministration::Status			crmAdminStatus;
		bool								crmAdminIsEmpty;
        CrmApcController::ApcModeStatus		crmApcStatus;
        bool								crmApcAutomaticActivation;

		::CommonPlatform::CoRegistration::CoRegistrationServerState coRegState;
		::CommonPlatform::CoRegistration::CoRegistrationError		coRegError;
		int									coRegRecordedRunIndex;
		bool								forceHideCoregControl;
		bool								fluoroAcquiredWhileRecording;

		PciReviewPlayer<PullbackRecording> &coRegistrationPlayer;
		const ILicenseCollection&	licenseCollection;	

		ICrmAdministration&			crmAdmin;

		PACS::IPACSPush*			pacsPush;
		PACS::IMessageBuilder*		msgBuilder;
		::CommonPlatform::Thread*	threadEncoding;
		bool						snapshotsAvailable;
		bool						moviesAvailable;

        ViewXrayConnectionStatus viewXrayConnectionStatus;

		CommonPlatform::Xray::XrayGeometry geometry;
        bool geometryIsMoving;
        bool xrayModalityConnected;

		bool exposureActive;
		std::wstring currentPatient;

		const CommonPlatform::Xray::SystemType systemType;

		PciViewContext(const PciViewContext&) = delete;
		PciViewContext& operator=(const PciViewContext&) = delete;
	};

}}