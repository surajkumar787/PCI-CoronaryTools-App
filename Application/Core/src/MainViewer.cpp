// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "MainViewer.h"
#include "ViewType.h"
#include "MainModel.h"
#include "PciBaseViewer.h"
#include "IArchivingStatus.h"
#include "LicenseStatus.h"

#include "Translation.h"
#include "Resources.h"
#include "Log.h"

#include <assert.h>
#include <locale>
#include <iterator>
#include <algorithm>

namespace Pci { namespace Core
{
	std::wstring getAlluraFormattedName(const std::wstring& Name);
	std::wstring getSmSFormattedName(const std::wstring& Name);
	std::wstring getFormattedBirthDate(const std::wstring& BirthDate);

	MainViewer::MainViewer(Sense::Control& parent, MainModel& mainModel, ViewType type) :
		Control					(parent),
		MainViewport			(*this),
		SideViewport			(*this),
		stbMarkerViewport		(SideViewport),
		crmGuidance			    (MainViewport, type),
		crmViewer			    (MainViewport, type),
		crmRoadmapSelectViewer  (MainViewport, type),
		stbGuidance			    (MainViewport, type),
		stbViewer			    (MainViewport, type),
		stbAngio			    (stbMarkerViewport, type),
		stbRoiViewer			(MainViewport, type),
		stbContrastViewer		(MainViewport,scheduler, type),
        stbContrastFrameSelectionView(MainViewport, type, false),
        stbBoostFrameSelectionView(MainViewport, type, true),
        coRegViewer			    (MainViewport, type),
		coRegControl		    (SideViewport, type),
		coRegGuidance		    (MainViewport, type),
		pciEpxView			    (MainViewport, type),
		noConnection		    (MainViewport, type),
		noLicenseView		    (MainViewport, type),
		smartSuiteAboutBox	    (*this, mainModel.getLicenseCollection()),
		problemReportDialog	    (*this),
		exportBusyDialog        (*this),
		alluraView				(*this),
		smsFlexvisionView		(*this),
		sms19InchView			(*this),
		smsFullHdView			(*this),
		currentViewType			(type),
		model				    (mainModel),
		viewers					()

	{
		smsFullHdView.tabCoronaryTools.image = PciSuite::Resources::ApplicationCoronaryTools_SmS;
		smsFlexvisionView.tabCoronaryTools.image = PciSuite::Resources::ApplicationCoronaryTools_SmS;
		sms19InchView.tabCoronaryTools.image = PciSuite::Resources::ApplicationCoronaryTools_SmS;

		smsFullHdView.tabCoronaryTools.text = Localization::Translation::IDS_CoronaryTools;
		smsFlexvisionView.tabCoronaryTools.text = Localization::Translation::IDS_CoronaryTools;
		sms19InchView.tabCoronaryTools.text = Localization::Translation::IDS_CoronaryTools;

		addAutoExclusive(alluraView.visible);
		addAutoExclusive(smsFlexvisionView.visible);
		addAutoExclusive(sms19InchView.visible);
		addAutoExclusive(smsFullHdView.visible);

		alluraView				.systemMenu.text = Localization::Translation::IDS_System;
		alluraView				.helpMenu  .text = Localization::Translation::IDS_Help;
		smsFlexvisionView		.systemMenu.text = Localization::Translation::IDS_System;
		smsFlexvisionView		.helpMenu  .text = Localization::Translation::IDS_Help;
		sms19InchView			.systemMenu.text = Localization::Translation::IDS_System;
		sms19InchView			.helpMenu  .text = Localization::Translation::IDS_Help;
		smsFullHdView			.systemMenu.text = Localization::Translation::IDS_System;
		smsFullHdView			.helpMenu  .text = Localization::Translation::IDS_Help;

		alluraView       .systemMenu.addMenuItem(Localization::Translation::IDS_Service,       [this] { if (eventClickService) eventClickService(); });
		alluraView       .systemMenu.addMenuItem(Localization::Translation::IDS_ProblemReport, [this] { if (eventClickProblemReport) eventClickProblemReport(); });
		smsFlexvisionView.systemMenu.addMenuItem(Localization::Translation::IDS_Service,       [this] { if (eventClickService) eventClickService(); });
		smsFlexvisionView.systemMenu.addMenuItem(Localization::Translation::IDS_ProblemReport, [this] { if (eventClickProblemReport) eventClickProblemReport(); });
		sms19InchView	 .systemMenu.addMenuItem(Localization::Translation::IDS_Service,       [this] { if (eventClickService) eventClickService(); });
		sms19InchView	 .systemMenu.addMenuItem(Localization::Translation::IDS_ProblemReport, [this] { if (eventClickProblemReport) eventClickProblemReport(); });
		smsFullHdView    .systemMenu.addMenuItem(Localization::Translation::IDS_Service,       [this] { if (eventClickService) eventClickService(); });
		smsFullHdView    .systemMenu.addMenuItem(Localization::Translation::IDS_ProblemReport, [this] { if (eventClickProblemReport) eventClickProblemReport(); });

		alluraView.sendToBack();
		smsFlexvisionView.sendToBack();
		sms19InchView.sendToBack();
		smsFullHdView.sendToBack();

		crmGuidance.layout			= Layout::Fill;
		crmViewer.layout			= Layout::Fill;
		crmRoadmapSelectViewer.layout = Layout::Fill;
		stbGuidance.layout			= Layout::Fill;
		stbViewer.layout			= Layout::Fill;
		stbRoiViewer.layout			= Layout::Fill;
		stbContrastViewer.layout  = Layout::Fill;
        stbContrastFrameSelectionView.layout = Layout::Fill;
        stbBoostFrameSelectionView.layout = Layout::Fill;
        coRegViewer.layout			= Layout::Fill;
		coRegGuidance.layout		= Layout::Fill;
		pciEpxView.layout			= Layout::Fill;
		noConnection.layout			= Layout::Fill;
		noLicenseView.layout		= Layout::Fill;

		stbAngio.layout				= Layout::Fill;
		coRegControl.layout			= Layout::Fill;

		exportBusyDialog.labelExport.text = Localization::Translation::IDS_ExportBusy;
		exportBusyDialog.text = Localization::Translation::IDS_ExportBusyTitle;
		exportBusyDialog.labelRemaining.text = Localization::Translation::IDS_ExportRemaining;
		exportBusyDialog.buttonAbort.text = Localization::Translation::IDS_Abort;

		setViewType(type);
	}

	MainViewer::~MainViewer()
	{
		model.eventModelChanged = nullptr;
	}

	template<typename Viewer>
	void MainViewer::setActiveViewer(Viewer& viewer)
	{
		size = viewer.size;
		MainViewport.setBounds(viewer.main.getBounds());
		SideViewport.setBounds(viewer.side.getBounds());
		stbMarkerViewport.setBounds(viewer.stbMarkerImage.getBounds());

		setAutoExclusive(viewer.visible);
	}


	template<typename Viewer>
	void MainViewer::updateHelpMenu(Viewer& viewer, MainModel& model)
	{
		viewer.helpMenu.clearMenuItems();
		if (model.isCrmLicenseActive())
		{
			viewer.helpMenu.addMenuItem(Localization::Translation::IDS_AboutDCR, [this] { if (eventClickAboutRoadmap) eventClickAboutRoadmap(); });
		}
		if (model.isAnyStbLicenseActive())
		{
			viewer.helpMenu.addMenuItem(Localization::Translation::IDS_AboutSBL, [this] { if (eventClickAboutStentBoost) eventClickAboutStentBoost(); });
		}
	}

	void MainViewer::setViewType(ViewType type)
	{
		if (type == ViewType::Allura)
		{
			setActiveViewer(alluraView);

			model.eventModelChanged = [this] { updateAllura(); };
			updateAllura();
			updateHelpMenu(alluraView, model);
		}
		else if (type == ViewType::SmartSuiteFlexVision)
		{
			setActiveViewer(smsFlexvisionView);

			model.eventModelChanged = [this] { updateSmartSuite(smsFlexvisionView, model); };
			updateSmartSuite(smsFlexvisionView, model);
			updateHelpMenu(smsFlexvisionView, model);
		}
		else if (type == ViewType::SmartSuite19Inch)
		{
			setActiveViewer(sms19InchView);

			model.eventModelChanged = [this] { updateSmartSuite(sms19InchView, model); };
			updateSmartSuite(sms19InchView, model);
			updateHelpMenu(sms19InchView, model);
		}
		else if (type == ViewType::SmartSuiteFullHD)
		{
			setActiveViewer(smsFullHdView);

			model.eventModelChanged = [this] { updateSmartSuite(smsFullHdView, model); };
			updateSmartSuite(smsFullHdView, model);
			updateHelpMenu(smsFullHdView, model);
		}
		else
		{
			assert(false);
			::CommonPlatform::Logger(::CommonPlatform::LoggerType::Platform)
				.DeveloperError(L"MainViewer::setViewType(ViewType): received an invalid ViewType value of %d", type);
			// fallback to allura view
			setActiveViewer(alluraView);
			model.eventModelChanged = [this] { updateAllura(); };
			updateAllura();
			updateHelpMenu(alluraView, model);
		}

	}

	void MainViewer::updateAllura()
	{
		alluraView.patientName.text = getAlluraFormattedName(model.getStudy().patientName);

		alluraView.labelCoregInfo.text = model.getCoregStatusText();

		if (model.isCoregLicenseActive())
		{
			alluraView.fmDisconnect.image = model.getCoregConnected() ? PciSuite::Resources::StatusPhysiologyConnected : PciSuite::Resources::StatusPhysiologyDisconnected;
			alluraView.fmDisconnect.visible = true;
		}
		else
		{
			alluraView.fmDisconnect.visible = false;
		}


		if (model.getSystemMenuEnabled())
		{
			alluraView.systemMenu.enable();
		}
		else
		{
			alluraView.systemMenu.collapse();
			alluraView.systemMenu.disable();
		}

		auto messageArea1Text = getTextMessageArea1();
		auto messageArea2Text = getTextMessageArea2();

		alluraView.labelMessageArea1.text = messageArea1Text;
		alluraView.labelMessageArea2.text = (messageArea2Text != messageArea1Text) ? messageArea2Text : L"";
	}

	template<typename Viewer>
	void MainViewer::updateSmartSuite(Viewer& viewer, MainModel& model)
	{
		PatientInfo patient;
		patient.name = getSmSFormattedName(model.getStudy().patientName);
		patient.birthdate = getFormattedBirthDate(model.getStudy().patientBirthdate);
		patient.id = model.getStudy().patientId;
		viewer.patientBar.patientData = patient;

		viewer.labelCoregInfo.text = model.getCoregStatusText();

		if (model.isCoregLicenseActive())
		{
			viewer.fmDisconnect.image = model.getCoregConnected() ? PciSuite::Resources::StatusPhysiologyConnected : PciSuite::Resources::StatusPhysiologyDisconnected;
			viewer.fmDisconnect.visible = true;
		}
		else
		{
			viewer.fmDisconnect.visible = false;
		}

		if (model.getSystemMenuEnabled())
		{
			viewer.systemMenu.enable();
		}
		else
		{
			viewer.systemMenu.collapse();
			viewer.systemMenu.disable();
		}

		if (!model.isIControlConnected())
		{
			viewer.icontrolDisconnectLabel.visible = true;
		}
		else
		{
			viewer.icontrolDisconnectLabel.visible = false;
		}

		auto messageArea1Text = getTextMessageArea1();
		auto messageArea2Text = getTextMessageArea2();

		viewer.labelMessageArea1.text = messageArea1Text;
		viewer.labelMessageArea2.text = (messageArea2Text != messageArea1Text) ? messageArea2Text : L"";
	}

	std::wstring MainViewer::getSnapshotArchivingMessage() const
	{
		const auto  snapshot = model.getSnapshotStatus();
		const auto  area = model.getSnapshotArea();

		if (area == CaptureArea::MainView)
		{
			switch (snapshot)
			{
			case ArchivingStatus::Busy:    return Localization::Translation::IDS_ArchivingSnapshot;       break;
			case ArchivingStatus::Success: return Localization::Translation::IDS_SnapshotArchived;        break;
			case ArchivingStatus::Failure: return Localization::Translation::IDS_SnapshotArchivingFailed; break;
			case ArchivingStatus::Ready:
			case ArchivingStatus::None:   break;
			}
		}
		else
		{
			switch (snapshot)
			{
			case ArchivingStatus::Busy:    return Localization::Translation::IDS_ArchivingScreenshot;       break;
			case ArchivingStatus::Success: return Localization::Translation::IDS_ScreenshotArchived;        break;
			case ArchivingStatus::Failure: return Localization::Translation::IDS_ScreenshotArchivingFailed; break;
			case ArchivingStatus::Ready:
			case ArchivingStatus::None:   break;
			}
		}

		return L"";
	}

	std::wstring MainViewer::getMovieArchivingMessage() const
	{
		const auto  movie = model.getMovieStatus();

		switch (movie)
		{
		case ArchivingStatus::Busy:    return Localization::Translation::IDS_ArchivingMovie;       break;
		case ArchivingStatus::Success: return Localization::Translation::IDS_MovieArchived;        break;
		case ArchivingStatus::Failure: return Localization::Translation::IDS_MovieArchivingFailed; break;
		case ArchivingStatus::Ready:
		case  ArchivingStatus::None:   break;
		}

		return L"";
	}

	std::wstring MainViewer::getTextMessageArea1() const
	{
		auto messageAreaText = std::wstring();
		auto snapshotArchivingMessage = getSnapshotArchivingMessage();
		
		if (!snapshotArchivingMessage.empty())
		{
			messageAreaText = snapshotArchivingMessage;
		}
		else if (model.getTsmFunctionsAvailable())
		{
			messageAreaText = Localization::Translation::IDS_UseTSMForAdditionalFunctions;
		}

		return messageAreaText;
	}

	std::wstring MainViewer::getTextMessageArea2() const
	{
		auto messageAreaText = std::wstring();
		auto movieArchivingMessage = getMovieArchivingMessage();

		if (!movieArchivingMessage.empty())
		{
			messageAreaText = movieArchivingMessage;
		}
		else if (model.getTsmFunctionsAvailable())
		{
			messageAreaText = Localization::Translation::IDS_UseTSMForAdditionalFunctions;
		}

		return messageAreaText;
	}

	// Formats the patient name according to Allura; 'lastname, firstname, middlename'
	std::wstring getAlluraFormattedName(const std::wstring& Name)
	{
		wchar_t strSrc[1024] = L"";
		wchar_t strDst[1024] = L"";
		wcscpy_s(strSrc, Name.c_str());

		int dst = 0;

		for (int src = 0; src < 1024; src++)
		{
			if (dst >= 512 - 3)			break;
			if (strSrc[src] == 0)		break;
			if (strSrc[src] == L'^')	continue;

			if (src > 0 && strSrc[src-1] == L'^')
			{
				strDst[dst+0] = L',';
				strDst[dst+1] = L' ';
				dst += 2;
			}

			strDst[dst] = strSrc[src];
			dst++;
		}

		strDst[dst] = 0;

		return strDst;
	}

	// Formats the patient name according to SmartSuite; 'LASTNAME, firstname middlename'
	std::wstring getSmSFormattedName(const std::wstring& Name)
	{
		std::wstring result;
		result.reserve(Name.size() + 10);

		auto familyNameEnd = Name.find('^', 0);
		if (familyNameEnd != std::wstring::npos)
		{
			std::transform(Name.begin(), Name.begin() + familyNameEnd, std::back_inserter(result), [](wchar_t ch) { return std::toupper(ch, std::locale("en-US")); });
		}
		else if (familyNameEnd == std::wstring::npos && Name.size() != 0)
		{
			//no '^' left but there are trailing characters
			std::transform(Name.begin(), Name.end(), std::back_inserter(result), [](wchar_t ch) {  return std::toupper(ch, std::locale("en-US")); });
		}
		else
		{
			return result;
		}

		auto givenNameEnd = Name.find('^', familyNameEnd + 1);
		if (givenNameEnd != std::wstring::npos && givenNameEnd > (familyNameEnd + 1))
		{
			if (!result.empty()) result += L", ";
			std::copy(Name.begin() + familyNameEnd + 1, Name.begin() + givenNameEnd, std::back_inserter(result));
		}
		else if (givenNameEnd == std::wstring::npos && familyNameEnd < Name.size())
		{
			// no '^' left but there are trailing characters
			if (!result.empty()) result += L", ";
			std::copy(Name.begin() + familyNameEnd + 1, Name.end(), std::back_inserter(result));
			return result;
		}

		auto middleNameEnd = Name.find('^', givenNameEnd + 1);
		if (middleNameEnd != std::wstring::npos && middleNameEnd > (givenNameEnd + 1))
		{
			if (result.size() <= familyNameEnd) result += L',';
			result += L' ';
			std::copy(Name.begin() + givenNameEnd + 1, Name.begin() + middleNameEnd, std::back_inserter(result));
		}
		else if (middleNameEnd == std::wstring::npos && givenNameEnd < Name.size())
		{
			// no '^' left but there are trailing characters
			if (result.size() <= familyNameEnd) result += L',';
			result += L' ';
			std::copy(Name.begin() + givenNameEnd + 1, Name.end(), std::back_inserter(result));
			return result;
		}

		return result;
	}


	static const int January	= 1;
	static const int February	= 2;
	static const int March		= 3;
	static const int April		= 4;
	static const int May		= 5;
	static const int June		= 6;
	static const int July		= 7;
	static const int August		= 8;
	static const int September	= 9;
	static const int October	= 10;
	static const int November	= 11;
	static const int December	= 12;


	std::wstring getShortMonth(int month)
	{
		switch (month)
		{
		case January:    return Localization::Translation::IDS_ShortMonth_January;
		case February:   return Localization::Translation::IDS_ShortMonth_February;
		case March:      return Localization::Translation::IDS_ShortMonth_March;
		case April:      return Localization::Translation::IDS_ShortMonth_April;
		case May:        return Localization::Translation::IDS_ShortMonth_May;
		case June:       return Localization::Translation::IDS_ShortMonth_June;
		case July:       return Localization::Translation::IDS_ShortMonth_July;
		case August:     return Localization::Translation::IDS_ShortMonth_August;
		case September:  return Localization::Translation::IDS_ShortMonth_September;
		case October:    return Localization::Translation::IDS_ShortMonth_October;
		case November:   return Localization::Translation::IDS_ShortMonth_November;
		case December:   return Localization::Translation::IDS_ShortMonth_December;
		default: return L"";
		}
		
	}

	std::wstring getFormattedBirthDate(const std::wstring& BirthDate)
	{
		if (BirthDate.size() == 8)
		{
			int year = _wtoi(BirthDate.substr(0, 4).c_str());
			int month = _wtoi(BirthDate.substr(4, 2).c_str());
			int day = _wtoi(BirthDate.substr(6, 2).c_str());

			if (year >= 0 && year <= 9999 && month >= January && month <= December && day >= 1 && day <= 31)
			{
				wchar_t strDst[32];
				swprintf_s(strDst, L"%02d-%s-%04d", day, getShortMonth(month).c_str(), year);
				return strDst;
			}
		}
		
		return Localization::Translation::IDS_PatientBarDobUnknown;
	}

}}