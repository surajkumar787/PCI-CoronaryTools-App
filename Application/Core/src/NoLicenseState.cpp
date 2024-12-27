// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "NoLicenseState.h"
#include "PciViewContext.h"
#include "LicensesCollection.h"
#include "Translation.h"
#include "pciGuidanceView.h"
#include "ViewType.h"

using namespace Sense::ExperienceIdentity;
namespace Pci { namespace Core 
{

NoLicenseState::NoLicenseState(PciViewContext& viewCtx) :
	UiState(viewCtx, ViewState::NoLicense, L"NoLicense", LicenseFeature::None),
	_guidanceModel()
{
	viewCtx.noLicenseView.setGuidanceModel(_guidanceModel);
}

PciXrayPositionModel& NoLicenseState::getModel()
{
	return _guidanceModel;
}

void NoLicenseState::onEnter()
{
	_guidanceModel.showMessagePanel = true;
	_guidanceModel.showMessagePicturePanel = false;
	_guidanceModel.showPositionPanel = false;
	_guidanceModel.showProcessingPanel = false;
	_guidanceModel.messageHeaderColor = pciViewContext.stbGuidance.getViewType() == ViewType::Allura ? Palette::White : Palette::Gray070;;
	_guidanceModel.messageHeaderIcon = &Icons::Information;
	_guidanceModel.statusIcon = nullptr;
	
	updateGuidanceText();

	show(&pciViewContext.noLicenseView);
}

ViewState NoLicenseState::epxChanged()
{
	updateGuidanceText();

	return UiState::epxChanged();
}

ViewState NoLicenseState::connectionStatusChanged(bool /*connected*/)
{
	return viewState;
}

void NoLicenseState::updateGuidanceText()
{
	if (!pciViewContext.licenseCollection.hasAnyProductLicense())
	{
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_NoLicense;
		_guidanceModel.guidanceText = L"";
	}
	else if (pciViewContext.currentEpx.getType() == PciEpx::Type::Roadmap)
	{
		if (pciViewContext.licenseCollection.isExpired(LicenseFeature::Roadmap))
		{
			_guidanceModel.messageHeaderText = Localization::Translation::IDS_CrmLicenseExpired;
			_guidanceModel.guidanceText = L"";
		}
		else // not expected during normal application workflow.
		{
			_guidanceModel.messageHeaderText = Localization::Translation::IDS_NoLicense;
			_guidanceModel.guidanceText = L"";
		}
	}
	else if (pciViewContext.currentEpx.getType() == PciEpx::Type::StentBoost)
	{
		if (pciViewContext.currentEpx.getSubType() == PciEpx::SubType::Live)
		{
			if (pciViewContext.licenseCollection.isExpired(LicenseFeature::StentboostLive))
			{
				_guidanceModel.messageHeaderText = Localization::Translation::IDS_StbLiveLicenseExpired;
				_guidanceModel.guidanceText = L"";
			}
			else // not expected during normal application workflow.
			{
				_guidanceModel.messageHeaderText = Localization::Translation::IDS_NoLicenseStbLive;
				_guidanceModel.guidanceText = Localization::Translation::IDS_ContactServiceProvider;
			}
		}
		else if (pciViewContext.currentEpx.getSubType() == PciEpx::SubType::PostDeployment)
		{
			if (pciViewContext.licenseCollection.isExpired(LicenseFeature::StbPostDeploy))
			{
				_guidanceModel.messageHeaderText = Localization::Translation::IDS_StbPostDeploymentLicenseExpired;
				_guidanceModel.guidanceText = L"";
			}
			else // not expected during normal application workflow.
			{
				_guidanceModel.messageHeaderText = Localization::Translation::IDS_NoLicenseStbPostDeployment;
				_guidanceModel.guidanceText = Localization::Translation::IDS_ContactServiceProvider;
			}
		}
		else if (pciViewContext.currentEpx.getSubType() == PciEpx::SubType::Subtract)
		{
			if (pciViewContext.licenseCollection.isExpired(LicenseFeature::StbSubtract))
			{
				_guidanceModel.messageHeaderText = Localization::Translation::IDS_StbSubtractLicenseExpired;
				_guidanceModel.guidanceText = L"";
			}
			else // not expected during normal application workflow.
			{
				_guidanceModel.messageHeaderText = Localization::Translation::IDS_NoLicenseStbSubtract;
				_guidanceModel.guidanceText = Localization::Translation::IDS_ContactServiceProvider;
			}
		}
	}
	else // not expected during normal application workflow.
	{
		_guidanceModel.messageHeaderText = Localization::Translation::IDS_NoLicense;
		_guidanceModel.messageText = L"";
	}
}

}}