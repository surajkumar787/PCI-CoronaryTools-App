// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "CrmGuidanceBaseState.h"
#include "CrmAdministration.h"


namespace Pci{ namespace Core
{
	enum class GuidanceState;
	class IUiStateController;

	class CrmErrorGuidanceState : public CrmGuidanceBaseState
	{
	public:
		explicit CrmErrorGuidanceState(PciViewContext &viewContext, IUiStateController& uiStateController);
		virtual ~CrmErrorGuidanceState() {}

		virtual void onEnter() override;
		virtual ViewState roadmapStatusChanged() override;
		virtual ViewState studyChanged() override;

    private:
		void setRoadMapState( CrmAdministration::Status crmAdminStatus);
		void setErrorText();
	};
}}