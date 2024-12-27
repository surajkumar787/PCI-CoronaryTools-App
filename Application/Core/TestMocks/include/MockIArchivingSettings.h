// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gmock\gmock.h>
#include "IArchivingSettings.h"

class MockIArchivingSettings : public Pci::Core::IArchivingSettings
{
public:
	MockIArchivingSettings() = default;
	~MockIArchivingSettings() = default;

	MOCK_CONST_METHOD1(isAutoCopyToRefEnabled, bool(Feature));
	MOCK_CONST_METHOD1(getCopyToRefViewer, const std::wstring(Feature));

	MOCK_CONST_METHOD1(isAutoPACSArchiveMovieEnabled, bool(Feature));
	MOCK_CONST_METHOD1(isAutoPACSArchiveSnapshotEnabled, bool(Feature));	

	MOCK_CONST_METHOD2(getCopyToRefTargetNumber, int(CommonPlatform::Log&, std::wstring&));
};