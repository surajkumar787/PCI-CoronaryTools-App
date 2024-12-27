// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

// define this definition in propertysheet.
#ifdef INCLUDE_TEST_AND_DEMO_FUNCTIONALITY

#include "StateControlUtility.h"

#include "MainControllerTestServer.h"
#include "CrmRoadmap.h"

#include <utility>
#include <Shlwapi.h>

namespace Pci { namespace Core
{
	using Sense::ExperienceIdentity::Canvas;

	UtilityCanvas::UtilityCanvas(Sense::Gpu& gpu, Sense::IScheduler& scheduler) :
		Canvas(gpu, scheduler),
		Ui(*this)
	{}

	void UtilityCanvas::onResize()
	{
		Canvas::onResize();
		Ui.setBounds(getSpace().bounds);
	}

StateControlUtility::StateControlUtility(MainControllerTestServer& pciViewer, Sense::Gpu& gpu, Sense::IScheduler& scheduler) :
	m_pciViewer(pciViewer),
	m_roadmap(std::make_shared<CrmRoadmap>()),
	m_canvas(gpu, scheduler),
	m_output(m_canvas, Sense::Rect(100, 100, 500, 900)),
	m_mouseKeyboard(m_canvas)
{
	m_stateMap.emplace(L"CRM.GUID.02: Create angiogram", GuidanceState::CreateAngio);
	m_stateMap.emplace(L"CRM.GUID.03: Processing", GuidanceState::Processing);
	m_stateMap.emplace(L"CRM.GUID.04: Roadmap Active", GuidanceState::RoadmapActive);
	m_stateMap.emplace(L"CRM.GUID.05: Roadmap available but table moved", GuidanceState::RoadmapAvailableTableMoved);

	// CrmErrors
	m_stateMap.emplace(L"CRM.GUID.09A: Roadmap creation error, No injection", GuidanceState::RoadmapCreationErrorNoInjection);
	m_stateMap.emplace(L"CRM.GUID.10A: Roadmap creation error, Injection too short", GuidanceState::RoadmapCreationErrorInjectionTooShort);
	m_stateMap.emplace(L"CRM.GUID.11A: Roadmap creation error, Too many contrast fluctuations", GuidanceState::RoadmapCreationErrorInjectionNotWellDefined);
	m_stateMap.emplace(L"CRM.GUID.12A: Roadmap creation error, Table moved", GuidanceState::RoadmapCreationErrorTableMoved);
	m_stateMap.emplace(L"CRM.GUID.13A: Roadmap creation error, C-arc moved", GuidanceState::RoadmapCreationErrorCArcMoved);
	m_stateMap.emplace(L"CRM.GUID.14A: Roadmap creation error, Angio too short", GuidanceState::RoadmapCreationErrorAngioTooShort);
	m_stateMap.emplace(L"CRM.GUID.15A: Roadmap creation error, No heart cycle", GuidanceState::RoadmapCreationErrorNoHeartCycle);

	m_stateMap.emplace(L"CRM.GUID.16: Roadmap creation error, Save Failure", GuidanceState::RoadmapCreationErrorSaveFailure);
	m_stateMap.emplace(L"CRM.GUID.17: Roadmap creation error, Unknown Failure", GuidanceState::RoadmapCreationErrorUnknownFailure);

	// CrmErrors without a previous roadmap
	m_stateMap.emplace(L"CRM.GUID.09B: Roadmap creation error, No injection (no roadmap)", GuidanceProperties(GuidanceState::RoadmapCreationErrorNoInjection, false));
	m_stateMap.emplace(L"CRM.GUID.10B: Roadmap creation error, Injection too short (no roadmap)", GuidanceProperties(GuidanceState::RoadmapCreationErrorInjectionTooShort, false));
	m_stateMap.emplace(L"CRM.GUID.11B: Roadmap creation error, Too many contrast fluctuations (no roadmap)", GuidanceProperties(GuidanceState::RoadmapCreationErrorInjectionNotWellDefined, false));
	m_stateMap.emplace(L"CRM.GUID.12B: Roadmap creation error, Table moved (no roadmap)", GuidanceProperties(GuidanceState::RoadmapCreationErrorTableMoved, false));
	m_stateMap.emplace(L"CRM.GUID.13B: Roadmap creation error, C-arc moved (no roadmap)", GuidanceProperties(GuidanceState::RoadmapCreationErrorCArcMoved, false));
	m_stateMap.emplace(L"CRM.GUID.14B: Roadmap creation error, Angio too short (no roadmap)", GuidanceProperties(GuidanceState::RoadmapCreationErrorAngioTooShort, false));
	m_stateMap.emplace(L"CRM.GUID.15B: Roadmap creation error, No heart cycle (no roadmap)", GuidanceProperties(GuidanceState::RoadmapCreationErrorNoHeartCycle, false));

	// APC states
	m_stateMap.emplace(L"CRM.GUID.APC.01: Roadmap nearby, APC not active, programming enabled", GuidanceState::RoadmapNearbyAPCNotActiveProgrammingEnabled);
	m_stateMap.emplace(L"CRM.GUID.APC.02A: Roadmap nearby, APC not active, programming disabled", GuidanceState::RoadmapNearbyAPCNotActiveProgrammingDisabled);
	m_stateMap.emplace(L"CRM.GUID.APC.02B: Roadmap nearby, APC active, programming disabled", GuidanceState::RoadmapNearbyAPCActiveProgrammingDisabled);
	m_stateMap.emplace(L"CRM.GUID.APC.03: Roadmap nearby, APC active, programming enabled", GuidanceState::RoadmapNearbyAPCActiveProgrammingEnabled);
	m_stateMap.emplace(L"CRM.GUID.APC.04: Roadmap nearby, APC active, programming enabled, Other APC program exists", GuidanceState::RoadmapNearbyAPCActiveProgrammingEnabledOtherProgramActive);
	m_stateMap.emplace(L"CRM.GUID.APC.05: Roadmap nearby, APC active, programming enabled, APC conflict", GuidanceState::RoadmapNearbyAPCActiveProgrammingEnabledConflict);

	typedef Sense::ExperienceIdentity::ListBox::Item Item;
	std::vector<Item> items;
	for(const auto& item: m_stateMap)
	{
		items.emplace_back(item.first);
	}
	std::sort(items.begin(), items.end(), [](const Item& first, const Item& second) { return first.text < second.text; });

	m_canvas.Ui.listStates.items = items;
	m_canvas.Ui.listStates.eventSelectionChanged = [this]() { setSelectedState(); };

	m_output.eventResized = [this]() { m_canvas.size = m_output.getSize(); };

	m_canvas.Ui.checkXrayConnected.eventToggled = [this]()
	{
		m_pciViewer.connectionChanged(m_canvas.Ui.checkXrayConnected.checked, CommonPlatform::Xray::IXraySourceRto::CwisStatus::Connected, CommonPlatform::Xray::IXraySourceRto::DvlpStatus::Connected);
	};

	m_canvas.Ui.checkUnreliableCoreg.eventToggled = [this]
	{
		m_pciViewer.setCoRegError(m_canvas.Ui.checkUnreliableCoreg.checked ?
			::CommonPlatform::CoRegistration::CoRegistrationError::TimestampValidationFailed : ::CommonPlatform::CoRegistration::CoRegistrationError::Unknown);
		m_canvas.Ui.checkUnreliableCoreg.enabled = false;
	};

	onGuidanceStateChanged(m_pciViewer.crmGuidance.getGuidanceState(), m_pciViewer.crmAdministration.getStatus());

	FILE* roadmapFile = nullptr;
	wchar_t file[MAX_PATH];
	GetModuleFileNameW(GetModuleHandle(nullptr), file, MAX_PATH);
	PathRemoveFileSpecW(file);
	PathAppendW(file, L"roadmap_test.crm.fxd");

	if (!_wfopen_s(&roadmapFile, file, L"rb"))
	{
		m_roadmap->load(roadmapFile);
		fclose(roadmapFile);
	}

	m_canvas.size = m_canvas.Ui.size;
	m_output.image = Sense::ExperienceIdentity::Icons::HeartSegmentation;
	m_output.text = L"State Control Utility";

	m_output.show();
}

void StateControlUtility::onGuidanceStateChanged(GuidanceState state, CrmAdministration::Status adminStatus)
{
	std::wstring stateName = L"Unknown state";
	for (const auto& kvp: m_stateMap)
	{
		if (kvp.second.State == state &&
			kvp.second.RoadmapAvailable == (adminStatus != CrmAdministration::Status::NoRoadmap))
		{
			stateName = kvp.first;
			break;
		}
	}
	m_canvas.Ui.labelCurrentState.text = stateName;
}

void StateControlUtility::setSelectedState()
{
	auto items = m_canvas.Ui.listStates.getSelectedItems();
	if (items.empty()) return;

	auto index = *items.begin();

	auto it = m_stateMap.find(m_canvas.Ui.listStates.items->at(index.item).text);
	if (it == m_stateMap.end()) return;

	auto crmAdminStatus = CrmAdministration::Status::Active;

	auto properties = it->second;
	switch (properties.State)
	{
	case GuidanceState::CreateAngio:
		break;
	case GuidanceState::Processing:
		break;
	case GuidanceState::RoadmapActive:
		m_pciViewer.crmGuidance.setRoadmap(m_roadmap);
		break;
	case GuidanceState::RoadmapAvailableTableMoved:
		{
			m_pciViewer.crmGuidance.setRoadmap(m_roadmap);
			auto geo = m_roadmap->geometry;
			geo.tableLongitudinalOffset -= 100;
			geo.tableLateralOffset += 120;
			geo.updateSpaces();
			m_pciViewer.crmGuidance.setGeometry(geo);
			crmAdminStatus = CrmAdministration::Status::MoveTable;
		}
		break;
	case GuidanceState::RoadmapCreationErrorSaveFailure:
		break;
	case GuidanceState::RoadmapCreationErrorUnknownFailure:
		break;
	case GuidanceState::RoadmapCreationErrorNoInjection:
	case GuidanceState::RoadmapCreationErrorInjectionTooShort:
	case GuidanceState::RoadmapCreationErrorInjectionNotWellDefined:
	case GuidanceState::RoadmapCreationErrorTableMoved:
	case GuidanceState::RoadmapCreationErrorCArcMoved:
	case GuidanceState::RoadmapCreationErrorAngioTooShort:
	case GuidanceState::RoadmapCreationErrorNoHeartCycle:
		if (properties.RoadmapAvailable)
		{
			m_pciViewer.crmGuidance.setRoadmap(m_roadmap);
			m_pciViewer.crmGuidance.setGeometry(m_roadmap->geometry);
		}
		else
		{
			m_pciViewer.crmGuidance.setRoadmap(nullptr);
			crmAdminStatus = CrmAdministration::Status::NoRoadmap;
		}
		break;
	case GuidanceState::RoadmapNearbyAPCNotActiveProgrammingEnabled:
	case GuidanceState::RoadmapNearbyAPCNotActiveProgrammingDisabled:
	case GuidanceState::RoadmapNearbyAPCActiveProgrammingEnabled:
	case GuidanceState::RoadmapNearbyAPCActiveProgrammingEnabledOtherProgramActive:
	case GuidanceState::RoadmapNearbyAPCActiveProgrammingEnabledConflict:
		{
			m_pciViewer.crmGuidance.setRoadmap(m_roadmap);
			auto geo = m_roadmap->geometry;
			geo.angulation += 4.0;
			geo.updateSpaces();
			m_pciViewer.crmGuidance.setGeometry(geo);
			crmAdminStatus = CrmAdministration::Status::MoveArc;
		}
		break;
	}
	m_pciViewer.crmGuidance.setGuidanceState(properties.State, crmAdminStatus);
}

}}
#endif