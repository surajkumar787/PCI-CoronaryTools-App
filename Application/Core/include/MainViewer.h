// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "MainAlluraView.ui.h"

#include "CrmViewer.h"
#include "PciGuidanceView.h"
#include "CrmRoadmapSelectViewer.h"
#include "StbBoostViewer.h"
#include "StbMarkersViewer.h"
#include "StbRoiViewer.h"
#include "StbContrastViewer.h"
#include "StbImageSelectViewer.h"
#include "CoRegistrationControl.h"
#include "ProblemReportDialog.h"
#include "ExportBusyDialog.ui.h"
#include "SmartSuiteAboutBox.h"
#include "PciEpxSelectionView.h"
#include "MainAlluraView.ui.h"
#include "MainSmartSuiteFlexVisionView.ui.h"
#include "MainSmartSuite19InchView.ui.h"
#include "MainSmartSuiteFullHDView.ui.h"

//TICS -INT#002 controls need to have public propertys

namespace Pci { namespace Core
{
	class MainModel;
	class PciBaseViewer;

	class MainViewer : public Sense::Control
	{
	public:
		MainViewer(Sense::Control& parent, MainModel& mainModel, ViewType type);
		~MainViewer();

		std::function<void()>		eventClickService;
		std::function<void()>		eventClickProblemReport;
		std::function<void()>		eventClickAboutRoadmap;
		std::function<void()>		eventClickAboutStentBoost;

		Sense::Control				MainViewport;
		Sense::Control				SideViewport;
		Sense::Control				stbMarkerViewport;

		PciGuidanceView				crmGuidance;
		CrmViewer					crmViewer;
		CrmRoadmapSelectViewer		crmRoadmapSelectViewer;

		PciGuidanceView				stbGuidance;
		StbBoostViewer				stbViewer;
		StbMarkersViewer			stbAngio;
		StbRoiViewer				stbRoiViewer;
		StbContrastViewer			stbContrastViewer;
        StbImageSelectViewer	    stbContrastFrameSelectionView;
        StbImageSelectViewer	    stbBoostFrameSelectionView;

		CrmViewer					coRegViewer;
		CoRegistrationControl		coRegControl;
		PciGuidanceView				coRegGuidance;

		PciEpxSelectionView			pciEpxView;
		PciGuidanceView				noConnection;
		PciGuidanceView				noLicenseView;

		SmartSuiteAboutBox			smartSuiteAboutBox;
		ProblemReportDialog			problemReportDialog;
		ExportBusyDialog			exportBusyDialog;

		MainAlluraView               alluraView;
		MainSmartSuiteFlexVisionView smsFlexvisionView;
		MainSmartSuite19InchView	 sms19InchView;
		MainSmartSuiteFullHDView     smsFullHdView;
		
		void setViewType(ViewType type);
		void updateAllura();

		template<typename Viewer>
		void updateSmartSuite(Viewer& viewer, MainModel& model);

	protected:
		template<typename Viewer>
		void setActiveViewer(Viewer& viewer);
		std::wstring getSnapshotArchivingMessage() const;
		std::wstring getMovieArchivingMessage() const;

		ViewType                     currentViewType;
		MainModel&                   model;

		std::vector<PciBaseViewer*>  viewers;

	private:
		template<typename Viewer>
		void updateHelpMenu(Viewer& viewer, MainModel& model);

		std::wstring getTextMessageArea1() const;
		std::wstring getTextMessageArea2() const;
	};

}}
//TICS +INT#002