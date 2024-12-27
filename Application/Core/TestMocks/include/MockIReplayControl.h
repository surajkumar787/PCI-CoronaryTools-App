// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gmock\gmock.h>

#include "IReplayControl.h"

class MockIReplayControl : public Pci::Core::IReplayControl
{
public:
	MockIReplayControl() = default;
	~MockIReplayControl() = default;

	MOCK_METHOD1(startXRayReplay, void(bool));
	MOCK_METHOD0(stopXRayReplay, void());
	MOCK_METHOD0(isActive, bool());
};