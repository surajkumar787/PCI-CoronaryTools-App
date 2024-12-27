// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <map>
#include <functional>

namespace Pci { namespace Platform { namespace iControl 
{
	enum class ReferenceViewer
	{
		Ref1,
		Ref2,
		Ref3
	};

	struct Snapshot
	{
		std::wstring description;
		std::wstring application;
		std::chrono::system_clock::time_point acquisitionTime;
		std::vector<uint8_t> imageData;
	};

	static const std::map<std::wstring, ReferenceViewer> referenceMonitorLUT = {
		{ L"Ref1",ReferenceViewer::Ref1 },
		{ L"Ref2",ReferenceViewer::Ref2 },
		{ L"Ref3",ReferenceViewer::Ref3 }
	};

	class ICopyToRefClient
	{
	public:
		virtual ~ICopyToRefClient() {}

		virtual bool sendSnapshot(const std::wstring& patientId, ReferenceViewer refViewer, const Snapshot& snapshot) = 0;
		virtual bool sendSnapshot(const std::wstring& patientId, const std::wstring& refViewer, const Snapshot& snapshot) = 0;
		virtual std::vector<ReferenceViewer> getAvailableViewers() = 0;

        virtual bool isAvailable() const = 0;
        virtual bool isConnected() const = 0;

		virtual void subscribeConnectionEvents(const std::wstring& id, std::function<void(bool)> callback) = 0;
		virtual void unsubscribeConnectionEvents(const std::wstring& id) = 0;
		virtual void connect(const std::string& network) = 0;

		static ReferenceViewer getReferenceMonitor(const std::wstring  &referenceMonitor)
		{
			auto result = referenceMonitorLUT.find(referenceMonitor);
			ReferenceViewer refMon = ReferenceViewer::Ref1;
			if (result != referenceMonitorLUT.end())
			{
				refMon = result->second;
			}

			return refMon;
		};
	};
}}}

