// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <gmock\gmock.h>

#include "IPACSPush.h"
#include "IMessageBuilder.h"

namespace Merge = ::CommonPlatform::Dicom::Merge;

class MockIPACSPush : public Pci::Core::PACS::IPACSPush
{
public:
	MockIPACSPush() = default;
	~MockIPACSPush() = default;

	MOCK_METHOD3(sendSnapshot, void(const Pci::Core::PACS::MetaData& metaData, const Merge::Message& snapshot, std::function<void(bool success)> callback));
	MOCK_METHOD3(sendMovie, void(const Pci::Core::PACS::MetaData& metaData, const Merge::Message& movie, std::function<void(bool success)> callback));

	MOCK_CONST_METHOD0(isBusy, bool());
	MOCK_CONST_METHOD0(queueSize, int());
	MOCK_METHOD0(flush, void());

	MOCK_METHOD0(abort, void());
};