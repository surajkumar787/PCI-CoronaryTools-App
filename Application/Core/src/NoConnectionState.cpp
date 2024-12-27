// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "NoConnectionState.h"
#include "PciViewContext.h"
#include "Translation.h"
#include "Resources.h"
#include "pciGuidanceView.h"
#include "ViewType.h"

using namespace Pci::Core;
using namespace Sense::ExperienceIdentity;

NoConnectionState::NoConnectionState(PciViewContext &viewContext):
	UiState(viewContext, ViewState::NoConnection, L"NoConnectionState", LicenseFeature::None),
	_guidanceModel()
{
	Sense::Color normal = pciViewContext.stbGuidance.getViewType() == ViewType::Allura ? Palette::Black : Palette::Gray070;
	Sense::Color info = pciViewContext.stbGuidance.getViewType() == ViewType::Allura ? Palette::White : Palette::Gray070;

	_guidanceModel.showMessagePanel = true;   
	_guidanceModel.showMessagePicturePanel = false;
	_guidanceModel.showPositionPanel = false;
	_guidanceModel.showProcessingPanel = false;
		_guidanceModel.messageHeaderColor = info;
		_guidanceModel.messageColor = normal;
	_guidanceModel.guidanceText = L"";
	_guidanceModel.statusIcon = nullptr;
}

PciXrayPositionModel& NoConnectionState::getModel()
{
	return _guidanceModel;
}

void NoConnectionState::onEnter()
{
    connectionStatusChanged(pciViewContext.xrayModalityConnected);
	pciViewContext.noConnection.setGuidanceModel(_guidanceModel);
    show(&pciViewContext.noConnection);
}

ViewState NoConnectionState::connectionStatusChanged(bool connected)
{
    if ( !connected )
    {
		
		if ( pciViewContext.viewXrayConnectionStatus == NeverConnected )
        {
				_guidanceModel.messageHeaderText  = Localization::Translation::IDS_ConnectionEstablishing;
				_guidanceModel.messageText = L"";
				_guidanceModel.messageHeaderIcon = &Icons::Information;
        }
        else
        {
				_guidanceModel.messageHeaderText = Localization::Translation::IDS_ConnectionFailure;
				_guidanceModel.messageText = Localization::Translation::IDS_RestartOrContactServiceProvider;
				_guidanceModel.messageHeaderIcon = &Icons::GeneralError;
        }

        return viewState;
    }
    else
    {
        // Use base class implementation, because override of this class is designed to ignore incoming EPX changes.
        return UiState::epxChanged();
    }
}

ViewState NoConnectionState::epxChanged()
{
    return viewState;
}

ViewState NoConnectionState::roadmapStatusChanged()
{
    return viewState;
}

ViewState NoConnectionState::angioProcessingStarted()
{
    return viewState;
}

ViewState NoConnectionState::angioProcessingStopped()
{
    return viewState;
}

ViewState NoConnectionState::overlayImageAvailable()
{
    return viewState;
}

ViewState NoConnectionState::lastImageHold()
{
    return viewState;
}

ViewState NoConnectionState::studyChanged()
{
	UiState::studyChanged();
    return viewState;
}

ViewState NoConnectionState::geometryChanged()
{
    return viewState;
}

ViewState NoConnectionState::geometryEnd()
{
    return viewState;
}

ViewState NoConnectionState::coRegistrationStateChanged(const CoReg::CoRegistrationServerState& /* prevState */,
														const CoReg::CoRegistrationServerState& /*nextState*/)
{
	return viewState;
}
