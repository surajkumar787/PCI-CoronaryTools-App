// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "UiState.h"
#include "XrayImage.h"
#include "PciGuidanceModel.h"

namespace Pci{ namespace Core
{
	class CoRegistrationGuidanceState : public UiState
	{
	public:
		explicit CoRegistrationGuidanceState(PciViewContext &viewContext);
		virtual ~CoRegistrationGuidanceState() {}

		virtual void onEnter() override;

		virtual ViewState overlayImageAvailable() override;
		virtual ViewState coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState,
			                                         const CoReg::CoRegistrationServerState& nextState) override;
		virtual ViewState angioProcessingStarted() override;
		virtual ViewState geometryChanged() override;
		virtual ViewState geometryEnd() override;
		virtual ViewState roadmapStatusChanged() override;

		CoRegistrationGuidanceState& operator=(const CoRegistrationGuidanceState&) = delete;
		CoRegistrationGuidanceState(const CoRegistrationGuidanceState&) = delete;

    protected:
		virtual PciXrayPositionModel& getModel() override;

        void enableTsmControls();
        void enableArchiveControlsOnTsm();
	private:
		inline PciTsmPage& getTsmPage() const;

		PciGuidanceModel _guidanceModel;
	};
}}

