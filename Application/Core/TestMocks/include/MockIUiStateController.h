// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gmock\gmock.h>

#include "IUiStateController.h"

class MockUiStateController : public Pci::Core::IUiStateController
{
public:
	MockUiStateController() = default;
	~MockUiStateController() = default;

	MOCK_METHOD1(switchState, void(Pci::Core::ViewState));
	MOCK_METHOD1(setStbRegionOfInterest, void(Sense::Rect roi));

	MOCK_METHOD1(startReplayRun, void(bool loop));
	MOCK_METHOD0(stopReplayRun, void());
	MOCK_METHOD0(isReplayActive, bool());
};