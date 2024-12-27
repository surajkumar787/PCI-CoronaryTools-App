// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "uistate.h"
#include "PciGuidanceModel.h"
#include "IArchivingSettings.h"

namespace Pci {
    
namespace Platform {
    namespace iControl {
        class ICopyToRefClient;
    }
}
namespace Core {

class StbTsmPage;

class StbGuidanceState : public UiState
{
public:
	explicit StbGuidanceState(
		PciViewContext &viewContext, 
		ViewState state, 
		const std::wstring &name, 
		const LicenseFeature license,
		Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
		const IArchivingSettings& archivingSettings);

	virtual void onEnter() override;

	virtual ViewState overlayImageAvailable() override;
	virtual ViewState epxChanged() override;
	virtual ViewState coRegistrationStateChanged(
		const CoReg::CoRegistrationServerState& prevState,
		const CoReg::CoRegistrationServerState& nextState) override;

	virtual ViewState coRegistrationConnectionChanged(bool connected) override;
	
protected:
	virtual std::wstring getCopyToRefTargetName() const = 0;
	virtual PciXrayPositionModel& getModel() override;
	StbTsmPage&	getTsmPage() const;

	PciGuidanceModel _guidanceModel;
	const IArchivingSettings& m_archivingSettings;
	CommonPlatform::Log& m_log;
};

}}

