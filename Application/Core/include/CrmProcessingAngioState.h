// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "UiState.h"
#include "PciGuidanceModel.h"

namespace Pci { namespace Core 
{
	class CrmProcessingAngioState : public UiState
	{
	public:
		explicit CrmProcessingAngioState(PciViewContext &viewContext);

		virtual void onEnter() override;
		virtual ViewState angioProcessingStopped() override;
		virtual ViewState roadmapStatusChanged() override;
	protected:
		virtual PciXrayPositionModel& getModel() override;

	private:
		PciGuidanceModel _guidanceModel;
	};
}}

