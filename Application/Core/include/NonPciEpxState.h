// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "UiState.h"
#include "PciGuidanceModel.h"

namespace Pci { namespace Core {

    class NonPciEpxState : public UiState
    {
    public:
        explicit NonPciEpxState(PciViewContext &viewContext);

        virtual void onEnter() override;
        virtual ViewState roadmapStatusChanged() override;
	protected:
		virtual PciXrayPositionModel& getModel() override;

		PciGuidanceModel _guidanceModel;
    };

}}
