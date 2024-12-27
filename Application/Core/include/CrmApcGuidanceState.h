// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "CrmGuidanceBaseState.h"

namespace Pci{ namespace Core
{
	class CrmApcGuidanceState : public CrmGuidanceBaseState
	{
	public:
		explicit CrmApcGuidanceState(PciViewContext &viewContext);
		virtual ~CrmApcGuidanceState() {}

		virtual void onEnter() override;
		virtual ViewState roadmapStatusChanged() override;
		virtual ViewState geometryEnd() override;
		virtual ViewState geometryChanged() override;
		virtual ViewState apcStatusChanged() override;
	private:
		void setApcGuidance();
	};
}}
