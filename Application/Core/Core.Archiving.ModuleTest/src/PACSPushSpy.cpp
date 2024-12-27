// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PACSPushSpy.h"
#include "MessageBuilder.h"

using namespace Pci::Core::PACS;
using namespace CommonPlatform::Dicom::Merge;

void PACSPushSpy::sendSnapshot(const MetaData& metaData, const Message&, std::function<void(bool success) > callback)
{
	sentImages.push_back(metaData);
	if (callback) callback(true);
}

void PACSPushSpy::sendMovie(const MetaData& metaData, const Message&, std::function<void(bool success) > callback)
{
	sentSequences.push_back(metaData);
	if (callback) callback(true);
}

bool PACSPushSpy::isBusy() const
{
	return false;
}

int PACSPushSpy::queueSize() const
{
	return 0;
}

void PACSPushSpy::flush()
{
}

void PACSPushSpy::abort()
{
}
