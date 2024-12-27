// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbLiveProcessingState.h"
#include "PciViewContext.h"
#include "StbBoostViewer.h"
#include "StbMarkersViewer.h"
#include "StbTsmPage.h"
#include "FrameGrabber.h"
#include "MovieGrabber.h"
#include "PciReviewParameters.h"
#include "StbAdministration.h"
#include "UtilizationLogEvent.h"
#include "IArchivingStatus.h"
#include "ViewStateHelper.h"
#include "ViewType.h"
#include "Log.h"
#include "StbMarkers.h"
#include "StbImage.h"
#include "IArchivingSettings.h"

using namespace CommonPlatform;

namespace Pci { namespace Core{

StbLiveProcessingState::StbLiveProcessingState(
	PciViewContext &viewContext,
	IUiStateController& controller,
	Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
	const IArchivingSettings& archivingSettings)
		: StbProcessingState(viewContext, ViewState::StbLiveProcessingState, L"StbLiveProcessingState", LicenseFeature::StentboostLive, controller, copyToRefClient, archivingSettings)
{
}

ViewState StbLiveProcessingState::getGuidanceState() const
{
	return ViewState::StbLiveGuidanceState;
}

ViewState StbLiveProcessingState::getReviewState() const
{
	return ViewState::StbLiveReviewState;
}

std::wstring StbLiveProcessingState::getCopyToRefTargetName() const
{
	return m_archivingSettings.getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive);
}

}}
