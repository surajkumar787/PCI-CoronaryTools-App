// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmGuidanceBaseState.h"
#include "PciViewContext.h"
#include "CrmTsmPage.h"
#include "ViewStateHelper.h"
#include "ViewType.h"

#include <ExperienceIdentity/Resource/Palette.h>

namespace Pci { namespace Core {


CrmGuidanceBaseState::CrmGuidanceBaseState(PciViewContext &viewContext, ViewState state,const std::wstring &name) :
	UiState(viewContext, state, name, LicenseFeature::Roadmap),
	_guidanceModel()
{
	tsmPage.reset(new CrmTsmPage());
}

void CrmGuidanceBaseState::onEnter()
{
	_guidanceModel.currentGeometry = pciViewContext.geometry;
}

Sense::Color CrmGuidanceBaseState::getNormalColour(ViewType type) const
{
	return type == ViewType::Allura ? Sense::ExperienceIdentity::Palette::Black : Sense::ExperienceIdentity::Palette::Gray070;
}

Sense::Color CrmGuidanceBaseState::getInfoColour(ViewType type) const
{
	return type == ViewType::Allura ? Sense::ExperienceIdentity::Palette::White : Sense::ExperienceIdentity::Palette::Gray070;
}

PciXrayPositionModel& CrmGuidanceBaseState::getModel()
{
	return _guidanceModel;
}

ViewState CrmGuidanceBaseState::angioProcessingStarted()
{
	return ViewState::CrmProcessingAngioState;
}

ViewState CrmGuidanceBaseState::angioProcessingStopped()
{
	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
}

ViewState CrmGuidanceBaseState::overlayImageAvailable()
{
	UiState::overlayImageAvailable();
	pciViewContext.roadmapErrorState = PciSuite::CrmErrors::Error::OK;

	if (pciViewContext.crmAdminStatus == CrmAdministration::Status::Active)
	{
		return ViewState::CrmOverlayState;
	}

	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
}

ViewState CrmGuidanceBaseState::roadmapStatusChanged()
{
	if ((pciViewContext.currentEpx.getType() == PciEpx::Type::NonPci) &&
		(pciViewContext.crmAdminStatus == CrmAdministration::Status::NoRoadmap))
	{
		return ViewState::NonPciEpxState;
	}

	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
}

ViewState CrmGuidanceBaseState::geometryChanged()
{
	UiState::geometryChanged();
	pciViewContext.roadmapErrorState = PciSuite::CrmErrors::Error::OK;
	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
}

ViewState CrmGuidanceBaseState::geometryEnd()
{
	UiState::geometryEnd();
	pciViewContext.roadmapErrorState = PciSuite::CrmErrors::Error::OK;
	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
}

CrmTsmPage& CrmGuidanceBaseState::getTsmPage() const
{
	return *static_cast<CrmTsmPage*>(tsmPage.get());
}

}}