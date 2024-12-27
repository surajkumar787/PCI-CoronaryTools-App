// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmRoadmapActive.h"
#include "PciViewContext.h"
#include "PciTsmPage.h"
#include "ViewStateHelper.h"
#include "Translation.h"
#include "pciGuidanceView.h"
#include "CrmRoadmapSelectViewer.h"
#include "CrmTsmPage.h"

using namespace Sense::ExperienceIdentity;
namespace Pci { namespace Core {


CrmRoadmapActive::CrmRoadmapActive(PciViewContext &viewContext) :
	CrmGuidanceBaseState(viewContext, ViewState::CrmRoadmapActive, L"CrmRoadmapActive"),
	m_roadmaps(),
	m_selectedRoadmap(-1)
{
	_guidanceModel.showProcessingPanel = false;
	_guidanceModel.showPositionPanel = false;
	_guidanceModel.showMessagePanel = false;
	_guidanceModel.showMessagePicturePanel = false;
	_guidanceModel.status = Localization::Translation::IDS_RoadmapReady;
	_guidanceModel.roadmapActive = true;
	_guidanceModel.statusIcon = nullptr;

	_guidanceModel.currentGeometry.setNotifier([&](const CommonPlatform::Xray::XrayGeometry& geo) {pciViewContext.crmRoadmapSelectViewer.setGeometry(geo); });

	tsmPage->eventButtonReleased = [this](unsigned int button)
	{
		if (button == CrmTsmPage::NavigateRightButton)
		{
			onNavigateRightDown();
		}
		if (button == CrmTsmPage::NavigateLeftButton)
		{
			onNavigateLeftDown();
		}
		if (button == CrmTsmPage::LockRoadmapButton)
		{
			onLockUnlockRoadmap();
		}
	};

	getTsmPage().navigateLeftButton->visible = true;
	getTsmPage().navigateRightButton->visible = true;
	getTsmPage().roadmapOptionsButton->visible = true;
	getTsmPage().lockRoadmapButton->visible = true;
}

void CrmRoadmapActive::onEnter()
{
	_guidanceModel.roadMapImage = pciViewContext.currentRoadmap;
	
	updateView();
}

ViewState CrmRoadmapActive::roadmapStatusChanged()
{
	if ((pciViewContext.currentEpx.getType() == PciEpx::Type::NonPci) &&
		(pciViewContext.crmAdminStatus == CrmAdministration::Status::NoRoadmap))
	{
		return ViewState::NonPciEpxState;
	}

	_guidanceModel.roadMapImage = pciViewContext.currentRoadmap;

	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
}

ViewState CrmRoadmapActive::geometryChanged()
{
	updateView();

	return CrmGuidanceBaseState::geometryChanged();
}

void CrmRoadmapActive::updateView()
{
	m_roadmaps = pciViewContext.crmAdmin.getRoadmapsForCurrentGeo();
	_guidanceModel.currentGeometry = pciViewContext.geometry;
	
	pciViewContext.crmRoadmapSelectViewer.setRoadmaps(m_roadmaps);
	pciViewContext.crmRoadmapSelectViewer.setGeometry(_guidanceModel.currentGeometry);

	auto it = std::find(m_roadmaps.begin(), m_roadmaps.end(), pciViewContext.currentRoadmap);
	if (it != m_roadmaps.end())
	{
		setSelection(static_cast<int>(std::distance(m_roadmaps.begin(), it)));
	}

	getTsmPage().lockRoadmapButton->visible = true;

	if (pciViewContext.currentRoadmap && pciViewContext.currentRoadmap->locked)
	{
		onRoadmapLocked();
	}
	else
	{
		onRoadmapUnlocked();
	}

	show(&pciViewContext.crmRoadmapSelectViewer);
}

void CrmRoadmapActive::onNavigateLeftDown()
{
	auto currentSelected = m_selectedRoadmap;
	currentSelected--;

	if (currentSelected >= 0)
	{
		setSelection(currentSelected);
	}
}

void CrmRoadmapActive::onNavigateRightDown()
{
	auto currentSelected = m_selectedRoadmap;
	currentSelected++;

	if (currentSelected < (m_roadmaps.size()))
	{
		setSelection(currentSelected);
	}
}

void CrmRoadmapActive::onLockUnlockRoadmap()
{
	if (pciViewContext.currentRoadmap->locked)
	{
		for (auto& roadmap : m_roadmaps)
		{
			pciViewContext.crmAdmin.unlockRoadmap(roadmap);
		}
		onRoadmapUnlocked();
	}
	else
	{
		pciViewContext.crmAdmin.lockRoadmap(m_roadmaps[m_selectedRoadmap]);
		onRoadmapLocked();
	}
}

void CrmRoadmapActive::onRoadmapLocked()
{
	pciViewContext.crmRoadmapSelectViewer.update();

	getTsmPage().roadmapOptionsButton->enabled = true;
	getTsmPage().roadmapOptionsButton->visible = true;
	getTsmPage().roadmapOptionsButton->selected = true;
	getTsmPage().navigateLeftButton->visible = true;
	getTsmPage().navigateLeftButton->enabled = false;
	getTsmPage().navigateRightButton->visible = true;
	getTsmPage().navigateRightButton->enabled = false;

	getTsmPage().lockRoadmapButton->centerText = Localization::Translation::IDS_RoadmapLocked;
	getTsmPage().lockRoadmapButton->selected = true;

	getTsmPage().requestUpdate();
}

void CrmRoadmapActive::onRoadmapUnlocked()
{
	pciViewContext.crmRoadmapSelectViewer.update();

	getTsmPage().roadmapOptionsButton->enabled = true;
	getTsmPage().roadmapOptionsButton->visible = true;
	getTsmPage().roadmapOptionsButton->selected = true;
	getTsmPage().navigateLeftButton->visible = true;
	getTsmPage().navigateLeftButton->enabled = m_selectedRoadmap > 0;
	getTsmPage().navigateRightButton->visible = true;
	getTsmPage().navigateRightButton->enabled = m_selectedRoadmap < (m_roadmaps.size() - 1);

	getTsmPage().lockRoadmapButton->centerText = Localization::Translation::IDS_LockRoadmap;
	getTsmPage().lockRoadmapButton->selected = false;

	getTsmPage().requestUpdate();
}

void CrmRoadmapActive::setSelection(int index)
{
	m_selectedRoadmap = index;
	pciViewContext.crmRoadmapSelectViewer.setSelection(m_selectedRoadmap);

	pciViewContext.crmAdmin.selectRoadmap(m_roadmaps[m_selectedRoadmap]);

	getTsmPage().navigateRightButton->enabled = (index < (m_roadmaps.size() - 1));
	getTsmPage().navigateLeftButton->enabled = (index > 0);
}


}}