// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "StbProcessingState.h"
#include "PciReviewPlayer.h"
#include "StbRecording.h"
#include "PciXrayModel.h"
#include "PciGuidanceModel.h"


namespace Pci { namespace Core{

class IUiStateController;

class StbContrastProcessingState : public StbProcessingState
{
	friend class TestStbContrastProcessingState;

public:
	explicit StbContrastProcessingState(
		PciViewContext &viewContext, 
		IUiStateController& controller, 
		Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
		const IArchivingSettings& archivingSettings);
	virtual ~StbContrastProcessingState();

	virtual void onEnter() override;
	virtual void onExit() override;

private:
	void timeUpdate();

	virtual PciXrayPositionModel& getModel() override;

	virtual ViewState getGuidanceState() const override;
	virtual ViewState getReviewState() const override;
	std::wstring getCopyToRefTargetName() const override;

	Sense::Timer boostPhaseTimer;	
};

}}