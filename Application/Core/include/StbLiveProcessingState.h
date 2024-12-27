// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "StbProcessingState.h"
#include "PciReviewPlayer.h"
#include "StbRecording.h"
#include "PciXrayModel.h"

namespace Pci { namespace Core {

class StbLiveProcessingState : public StbProcessingState
{
	friend class TestStbLiveProcessingState;
public:
	explicit StbLiveProcessingState(
		PciViewContext &viewContext, 
		IUiStateController& controller, 
		Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
		const IArchivingSettings& archivingSettings);

private:
	virtual ViewState getGuidanceState() const override;
	virtual ViewState getReviewState() const override;
	std::wstring getCopyToRefTargetName() const override;
};

}}

