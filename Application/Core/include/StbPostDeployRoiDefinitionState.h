// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "StbRoiDefinitionState.h"

namespace Pci {	namespace Core {

	class IUiStateController;

	class StbPostDeployRoiDefinitionState : public StbRoiDefinitionState
	{
	public:
		StbPostDeployRoiDefinitionState(
			PciViewContext &viewContext, 
			IUiStateController& controller, 
			Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
			const IArchivingSettings& archivingSettings);

		virtual ~StbPostDeployRoiDefinitionState();

	private:
		std::wstring getCopyToRefTargetName() const override;
	};
}}



