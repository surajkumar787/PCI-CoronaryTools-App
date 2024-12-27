// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "UiState.h"
#include "PciGuidanceModel.h"

namespace Pci{ namespace Core
{
	enum class ViewType;
	class CrmTsmPage;


	class CrmGuidanceBaseState : public UiState
	{
	public:
		CrmGuidanceBaseState(PciViewContext &viewContext, ViewState state,const std::wstring &name);

		virtual void onEnter() override;

		virtual ViewState angioProcessingStarted() override;
		virtual ViewState angioProcessingStopped() override;
		virtual ViewState overlayImageAvailable() override;
		virtual ViewState roadmapStatusChanged() override;
        virtual ViewState geometryChanged() override;
        virtual ViewState geometryEnd() override;

	protected:
		CrmTsmPage& getTsmPage() const;

		virtual PciXrayPositionModel& getModel() override;

		Sense::Color getNormalColour(ViewType type) const;
		Sense::Color getInfoColour(ViewType type) const;

		PciGuidanceModel _guidanceModel;
	};
}}
