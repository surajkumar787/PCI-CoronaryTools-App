// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <memory>
#include <string>

#include "ICopyToRefClient.h"

namespace Pci { namespace Platform { namespace iControl {

	class NullCopyToRefClient : public ICopyToRefClient
	{
	public:
		NullCopyToRefClient() = default;
		~NullCopyToRefClient() {}

		virtual bool sendSnapshot(const std::wstring& /*patientId*/, ReferenceViewer /*refViewer*/, const Snapshot& /*snapshot*/) override { return true; };
		virtual bool sendSnapshot(const std::wstring& /*patientId*/, const std::wstring& /*refViewer*/, const Snapshot& /*snapshot*/) override { return true; };
		virtual std::vector<ReferenceViewer> getAvailableViewers() override { return std::vector<ReferenceViewer>();  };

        virtual bool isAvailable() const { return false; };
        virtual bool isConnected() const { return false; };

		virtual void subscribeConnectionEvents(const std::wstring& /*id*/, std::function<void(bool)> /*callback*/) {};
		virtual void unsubscribeConnectionEvents(const std::wstring& /*id*/) {};
		virtual void connect(const std::string& /*ip*/) {};
	};
}}}

