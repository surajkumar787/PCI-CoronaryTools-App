// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "NonPciEpxState.h"
#include "PciViewContext.h"
#include "LicensesCollection.h"
#include "ViewStateHelper.h"
#include "Resources.h"
#include "Translation.h"
#include "PciEpxSelectionView.h"
#include "ViewType.h"

namespace Pci { namespace Core {

NonPciEpxState::NonPciEpxState(PciViewContext &viewContext):
	UiState(viewContext, ViewState::NonPciEpxState, L"NonPciEpxState", LicenseFeature::None),
	_guidanceModel()
{
	_guidanceModel.showMessagePanel = true;
	_guidanceModel.showMessagePicturePanel = true;
	_guidanceModel.showPositionPanel = false;
	_guidanceModel.showProcessingPanel = false;
	_guidanceModel.messageHeaderColor = pciViewContext.pciEpxSelection.getViewType() == ViewType::Allura ? Sense::ExperienceIdentity::Palette::Black : Sense::ExperienceIdentity::Palette::Gray070;
	_guidanceModel.messageText = L"";
	_guidanceModel.guidanceText = L"";
	_guidanceModel.statusIcon = nullptr;
}

PciXrayPositionModel& NonPciEpxState::getModel()
{
	return _guidanceModel;
}


void NonPciEpxState::onEnter()
{

	if (pciViewContext.pciEpxSelection.getViewType() == ViewType::Allura)
	{
		_guidanceModel.messagePictureText = Localization::Translation::IDS_Acquisition;
		_guidanceModel.messagePicture = &PciSuite::Resources::PCITools_startscreen_guidance_epx;
	}
	else
	{
		_guidanceModel.messagePictureText = L"";
		_guidanceModel.messagePicture = &PciSuite::Resources::XraySettings140x140;
	}

	if (pciViewContext.licenseCollection.isActive(LicenseFeature::Roadmap) &&
		pciViewContext.licenseCollection.isAnyActive({LicenseFeature::StentboostLive ,LicenseFeature::StbPostDeploy ,LicenseFeature::StbSubtract }))
	{   
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_ActivateStbOrCrmEpx;
	}
	else if (pciViewContext.licenseCollection.isAnyActive({ LicenseFeature::StentboostLive, LicenseFeature::StbPostDeploy, LicenseFeature::StbSubtract }))
	{
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_ActivateStbEpx;
	}
	else if (pciViewContext.licenseCollection.isActive(LicenseFeature::Roadmap))
	{
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_ActivateCrmEpx;
	}
	else
	{
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_ActivateStbOrCrmEpx;
	}

	_guidanceModel.currentGeometry = pciViewContext.geometry;
	pciViewContext.pciEpxSelection.setGuidanceModel(_guidanceModel);
	show(&pciViewContext.pciEpxSelection);
}


ViewState NonPciEpxState::roadmapStatusChanged()
{
	if (pciViewContext.crmAdminStatus != CrmAdministration::Status::NoRoadmap)
	{
		return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
	}

	return viewState;
}
}}