// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <memory>
#include <functional>

namespace CommonPlatform { namespace Dicom { namespace Merge { class Message; } } }

namespace Pci { namespace Core { namespace PACS
{
	namespace Merge = ::CommonPlatform::Dicom::Merge;

	struct MetaData;

	class IPACSPush
	{
	public:
		virtual ~IPACSPush() {}

		virtual void sendSnapshot(const MetaData& metaData, const Merge::Message& snapshot, std::function<void(bool success)> callback) = 0;
		virtual void sendMovie(const MetaData& metaData, const Merge::Message& movie, std::function<void(bool success)> callback) = 0;

		virtual bool isBusy() const = 0;
		virtual int queueSize() const = 0;
		virtual void flush() = 0;

		virtual void abort() = 0;
	};

}}}