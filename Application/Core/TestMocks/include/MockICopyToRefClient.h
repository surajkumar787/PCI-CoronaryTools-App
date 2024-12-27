// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gmock\gmock.h>

#include "ICopyToRefClient.h"

using namespace Pci::Platform::iControl;

class MockICopyToRefClient : public Pci::Platform::iControl::ICopyToRefClient
{
public:
	MockICopyToRefClient() = default;
	~MockICopyToRefClient() = default;

	MOCK_METHOD3(sendSnapshot, bool(const std::wstring&, ReferenceViewer, const Snapshot&));
	MOCK_METHOD3(sendSnapshot, bool(const std::wstring&, const std::wstring&, const Snapshot&));
	MOCK_METHOD0(getAvailableViewers, std::vector<ReferenceViewer>());
    MOCK_CONST_METHOD0(isAvailable, bool());
    MOCK_CONST_METHOD0(isConnected, bool());
    MOCK_METHOD2(subscribeConnectionEvents, void(const std::wstring&, std::function<void(bool)>));
	MOCK_METHOD1(unsubscribeConnectionEvents, void(const std::wstring&));
	MOCK_METHOD1(connect, void(const std::string&));
};