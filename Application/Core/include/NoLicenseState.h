// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "uistate.h"
#include "PciGuidanceModel.h"

namespace Pci { namespace Core 
{

class NoLicenseState : public UiState
{
public:
	explicit NoLicenseState(PciViewContext& viewCtx);

	virtual void onEnter() override;
	virtual ViewState epxChanged() override;
	virtual ViewState connectionStatusChanged(bool connected) override;

private:
	virtual PciXrayPositionModel& getModel() override;
	void updateGuidanceText();


	PciGuidanceModel _guidanceModel;

};

}}