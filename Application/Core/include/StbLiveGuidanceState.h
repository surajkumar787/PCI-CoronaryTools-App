// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "StbGuidanceState.h"
#include "PciGuidanceModel.h"
#include "IArchivingSettings.h"

namespace Pci { namespace Core {
    
class StbLiveGuidanceState : public StbGuidanceState
{
public:
	explicit StbLiveGuidanceState(
		PciViewContext &pciViewContext, 
		Pci::Platform::iControl::ICopyToRefClient& copyToRefClient, 
		const IArchivingSettings& archivingSettings);

	virtual void onEnter() override;

private:
	std::wstring getCopyToRefTargetName() const override;
};
}}

