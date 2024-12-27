// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <memory>
#include <string>
#include <mutex>

#include "ICopyToRefClient.h"
#include "stlstring.h"

namespace SscfTs1
{
	class EventHandler;
	class EventLoop;
	class ClientNode;
}

namespace IControl {
	class IReviewV1;
	struct SnapShot;
	enum class ReferenceViewer;
}

namespace CommonPlatform
{
	class Log;
}

namespace Pci { namespace Platform { namespace iControl {

	class CopyToRefClient : public ICopyToRefClient
	{
	public:
		CopyToRefClient();
		~CopyToRefClient();

		virtual bool sendSnapshot(const std::wstring& patientId, ReferenceViewer refViewer, const Snapshot& snapshot) override;
		virtual bool sendSnapshot(const std::wstring& patientId, const std::wstring& refViewer, const Snapshot& snapshot) override;
		virtual std::vector<ReferenceViewer> getAvailableViewers() override;

        virtual bool isAvailable() const override;
		virtual bool isConnected() const override;

		virtual void subscribeConnectionEvents(const std::wstring& id, std::function<void(bool)> callback) override;
		virtual void unsubscribeConnectionEvents(const std::wstring& id) override;

		bool WaitForConnection(int timeoutMillis) const;
		void connect(const std::string& network);

		CopyToRefClient(const CopyToRefClient &) = delete;
		CopyToRefClient& operator=(const CopyToRefClient &) = delete;

	private:
		void onConnected();
		void onDisconnected();
		
		void ConnectAsync();
		void Disconnect();

		void notifySubscribers();

		template <typename ProxyFun>
		void WithProxy(ProxyFun proxyFun);

		const ReferenceViewer& refViewerFromIControl(IControl::ReferenceViewer& iControlRefViewer) const;
		const IControl::ReferenceViewer& refViewerToIControl(ReferenceViewer& refViewer) const;

		IControl::SnapShot convertToIControlSnapshot(const Snapshot& snapshot) const;

		std::unique_ptr<SscfTs1::EventHandler> m_sscfEventHandler;
		std::unique_ptr<SscfTs1::EventLoop> m_eventLoop;
		std::unique_ptr<SscfTs1::ClientNode> m_sscfClient;

		std::unique_ptr<IControl::IReviewV1> m_proxy;

		std::mutex connectionSubscribersMutex;
		std::map<std::wstring, std::function<void(bool)>> connectionSubscribers;

		void* m_connected; // Windows Event Object
		bool m_clientConnected;

		CommonPlatform::Log& m_log;
	};
}}}