// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "IPACSPush.h"
#include "MessageBuilder.h"

#include <vector>

namespace Pci { namespace Core { namespace PACS
{
	class MessageBuilder;
}}}


class PACSPushSpy : public Pci::Core::PACS::IPACSPush
{
public:
	virtual void sendSnapshot(const Pci::Core::PACS::MetaData& metaData, const CommonPlatform::Dicom::Merge::Message& snapshot, std::function<void(bool success) > callback) override;
	virtual void sendMovie(const Pci::Core::PACS::MetaData& metaData, const CommonPlatform::Dicom::Merge::Message& movie, std::function<void(bool success) > callback) override;

	virtual bool isBusy() const override;
	virtual int queueSize() const override;
	virtual void flush() override;

	virtual void abort() override;

	std::vector<Pci::Core::PACS::MetaData>& getSentImages() { return sentImages; };
	std::vector<Pci::Core::PACS::MetaData>& getSentSequences() { return sentSequences; };


private:
	std::vector<Pci::Core::PACS::MetaData> sentImages;
	std::vector<Pci::Core::PACS::MetaData> sentSequences;
};
