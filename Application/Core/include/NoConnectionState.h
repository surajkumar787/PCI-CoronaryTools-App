// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "UiState.h"
#include "PciGuidanceModel.h"

namespace Pci { namespace Core {

    class NoConnectionState : public UiState
    {
    public:
        explicit NoConnectionState(PciViewContext &viewContext);

        virtual void onEnter() override;
        virtual ViewState epxChanged() override;
        virtual ViewState roadmapStatusChanged() override;
        virtual ViewState angioProcessingStarted() override;
        virtual ViewState angioProcessingStopped() override;
        virtual ViewState overlayImageAvailable() override;
        virtual ViewState lastImageHold() override;
        virtual ViewState studyChanged() override;
        virtual ViewState geometryChanged() override;
        virtual ViewState geometryEnd() override;
		virtual ViewState coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState,
			                                         const CoReg::CoRegistrationServerState& nextState) override;
        virtual ViewState connectionStatusChanged(bool connected) override;
	protected:
		virtual PciXrayPositionModel& getModel() override;

		PciGuidanceModel _guidanceModel;
    };
}}
