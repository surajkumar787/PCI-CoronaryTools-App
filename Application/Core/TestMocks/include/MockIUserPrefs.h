// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gmock\gmock.h>

#include "IUserPrefs.h"
#include "IArchivingSettings.h"
#include "MockIArchivingSettings.h"

class MockIUserPrefs : public Pci::Core::IUserPrefs
{
public:
	MockIUserPrefs() = default;
	~MockIUserPrefs() = default;

	const MockIArchivingSettings& getArchivingSettings() const {return mockArchiveSettings;};
	MOCK_CONST_METHOD0(getDefaultShowInvertedStent, bool());

	MOCK_CONST_METHOD0(getApcAutoActivation, bool());

	::testing::NiceMock<MockIArchivingSettings> mockArchiveSettings;
};