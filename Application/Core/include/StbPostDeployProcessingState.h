// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "StbProcessingState.h"
#include "PciReviewPlayer.h"
#include "StbRecording.h"
#include "PciXrayModel.h"


namespace Pci {	namespace Core{

	class IUiStateController;

	class StbPostDeployProcessingState : public StbProcessingState
	{
		friend class TestStbPostDeployProcessingState;

	public:
		explicit StbPostDeployProcessingState(
			PciViewContext &viewContext, 
			IUiStateController& controller, 
			Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
			const IArchivingSettings& archivingSettings);
		virtual ~StbPostDeployProcessingState();

	private:
		virtual ViewState getGuidanceState() const override;
		virtual ViewState getReviewState() const override;
		std::wstring getCopyToRefTargetName() const override;

		bool						m_switch_away;
	};

}}