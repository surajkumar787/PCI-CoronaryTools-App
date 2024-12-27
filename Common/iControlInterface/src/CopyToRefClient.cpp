// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CopyToRefClient.h"

#include <assert.h>
#include <windows.h>

#include "Log.h"
#include "IControl.h"

namespace Pci { namespace Platform  { namespace iControl
{
	static const std::vector<std::pair<IControl::ReferenceViewer, ReferenceViewer> > RefMonitorMappingTable
	{
		{ IControl::ReferenceViewer::Ref1, ReferenceViewer::Ref1 },
		{ IControl::ReferenceViewer::Ref2, ReferenceViewer::Ref2 },
		{ IControl::ReferenceViewer::Ref3, ReferenceViewer::Ref3 }
	};

	CopyToRefClient::CopyToRefClient() :
		m_eventLoop(new SscfTs1::EventLoop()),
		m_connected(CreateEvent(NULL, TRUE, FALSE, NULL)), // default security attributes, manual-reset event, initial state is non-signaled
		m_clientConnected(false),
		m_log(CommonPlatform::Logger(CommonPlatform::LoggerType::Platform))
	{	}

	CopyToRefClient::~CopyToRefClient()
	{
		Disconnect();
		
		// Destroy the event handler and the SSCF client
		// This unsubscribes all events and destroys the event loop proxy,
		// so it needs to be done from the event loop thread
		m_eventLoop->send([this]() {
			m_sscfEventHandler.reset();
			m_sscfClient.reset();
		});

		if (m_connected != NULL) CloseHandle(m_connected);
	}

	bool CopyToRefClient::sendSnapshot(const std::wstring& patientId, ReferenceViewer refViewer, const Snapshot& snapshot)
	{
		auto iControlSnapshot = convertToIControlSnapshot(snapshot);
		auto referenceViewer = refViewerToIControl(refViewer);
		bool result(false);

		WithProxy([&](IControl::IReviewV1& proxy) {
			result = proxy.copyToRef(patientId, referenceViewer, iControlSnapshot);
		});

		if (!result)
		{
			m_log.DeveloperError(L"CopyToRefClient::sendSnapshot: Failed to send snapshot to Ref %i", referenceViewer);
		}

		return result;
	}

	bool CopyToRefClient::sendSnapshot(const std::wstring& patientId, const std::wstring& refViewer, const Snapshot& snapshot)
	{
		auto referenceViewer = getReferenceMonitor(refViewer);
		
		return sendSnapshot(patientId, referenceViewer, snapshot);
	}

	std::vector<ReferenceViewer> CopyToRefClient::getAvailableViewers()
	{
		std::vector<IControl::ReferenceViewer> iControlRefViewers;
		std::vector<ReferenceViewer> refViewers;
		
		WithProxy([&](IControl::IReviewV1& proxy) {
			iControlRefViewers = proxy.getReferenceViewers();
		});

		for (auto refViewer : iControlRefViewers)
		{
			refViewers.emplace_back(refViewerFromIControl(refViewer));
		}

		return refViewers;
	}

    bool CopyToRefClient::isConnected() const
    {
        return m_clientConnected;
    }

    bool CopyToRefClient::isAvailable() const
    {
        return true;
    }

    void CopyToRefClient::subscribeConnectionEvents(const std::wstring& id, std::function<void(bool)> callback)
	{
		std::lock_guard<std::mutex> locked(connectionSubscribersMutex);
		connectionSubscribers[id] = callback;
	}

	void CopyToRefClient::unsubscribeConnectionEvents(const std::wstring& id)
	{
		std::lock_guard<std::mutex> locked(connectionSubscribersMutex);
		connectionSubscribers.erase(id);
	}

	void CopyToRefClient::ConnectAsync()
	{
		m_eventLoop->send([this] {
			assert(!m_proxy);
			m_log.DeveloperInfo(L"Connecting to iControl...");
			if (m_sscfClient)
			{
				m_sscfClient->connectAsync();
			}
			else
			{
				m_log.DeveloperWarning(L"SscfTs1::ClientNode is null.");
			}
		});
	}

	void CopyToRefClient::Disconnect()
	{
		m_eventLoop->send([this] {
			m_log.DeveloperInfo(L"Disconnecting from iControl...");
			m_proxy.reset();
			if (m_sscfClient)
			{
				m_sscfClient->disconnect();
			}
			else
			{
				m_log.DeveloperWarning(L"SscfTs1::ClientNode is null.");
			}
		});
	}

	void CopyToRefClient::notifySubscribers()
	{
		std::lock_guard<std::mutex> locked(connectionSubscribersMutex);
		for (auto subscriber : connectionSubscribers)
		{
			if (subscriber.second)
			{
				subscriber.second(m_clientConnected);
			}
		}
	}

	bool CopyToRefClient::WaitForConnection(int timeoutMillis) const
	{
		return WaitForSingleObject(m_connected, timeoutMillis) == WAIT_OBJECT_0;
	}

	static std::unique_ptr<IControl::IReviewV1> TryCreateProxy(SscfTs1::ClientNode& clientNode)
	{
		try
		{
			assert(clientNode.providesType<IControl::IReviewV1>());
			return std::unique_ptr<IControl::IReviewV1>(new IControl::IReviewV1Proxy(clientNode));
		}
		catch (const SscfTs1::CommunicationException& ex)
		{
			auto &log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);
			log.DeveloperError(L"CopyToRefClient::TryCreateProxy: Caught SscfTs1::CommunicationException -> %s", ex.what());

			return nullptr;
		}
	}

	void CopyToRefClient::onConnected()
	{
		assert(m_eventLoop->checkThread());

		assert(!m_proxy);

		m_proxy = TryCreateProxy(*m_sscfClient);

		if (m_proxy)
		{
			SetEvent(m_connected);
			m_clientConnected = true;
			notifySubscribers();
		}

		m_log.DeveloperInfo(L"Connected to Azurion iControl");
	}

	void CopyToRefClient::onDisconnected()
	{
		assert(m_eventLoop->checkThread());
		m_clientConnected = false;

		m_proxy.reset();
		m_sscfClient->connectAsync();
		ResetEvent(m_connected);
		notifySubscribers();

		m_log.DeveloperInfo(L"Disconnected from Azurion iControl");
	}

	template <typename ProxyFun>
	void CopyToRefClient::WithProxy(ProxyFun proxyFun)
	{
		m_eventLoop->send([this, &proxyFun]
		{
			if (!m_proxy)
			{
				m_log.DeveloperInfo(L"CopyToRefClient::WithProxy: client not connected");
			}
			else try
			{
				proxyFun(*m_proxy);
			}
			catch (const SscfTs1::CommunicationException& ex)
			{
				m_log.DeveloperError(L"CopyToRefClient::WithProxy: Caught SscfTs1::CommunicationException -> %s", ex.what());

			}
		});
	}

	const ReferenceViewer& CopyToRefClient::refViewerFromIControl(IControl::ReferenceViewer& icontrolRefViewer) const
	{
		for (auto& refViewerMapping : RefMonitorMappingTable)
		{
			if (refViewerMapping.first == icontrolRefViewer)
			{
				return refViewerMapping.second;
			}
		}

		throw std::exception("Unknown reference monitor");
	}

	const IControl::ReferenceViewer& CopyToRefClient::refViewerToIControl(ReferenceViewer& refViewer) const
	{
		for (auto& refViewerMapping : RefMonitorMappingTable)
		{
			if (refViewerMapping.second == refViewer)
			{
				return refViewerMapping.first;
			}
		}

		throw std::exception("Unknown reference monitor");
	}

	IControl::SnapShot CopyToRefClient::convertToIControlSnapshot(const Snapshot& snapshot) const
	{
		const long long WINDOWS_TIME_OFFSET(116444736000000000);

		IControl::SnapShot iControlSnapshot;
		iControlSnapshot.application = snapshot.application;
		iControlSnapshot.description = snapshot.description;
		iControlSnapshot.acquisitionTime = IControl::DateTime::fromTimeStamp(snapshot.acquisitionTime.time_since_epoch().count() + WINDOWS_TIME_OFFSET);

		std::vector<uint8_t> pixelData = snapshot.imageData;
		iControlSnapshot.image = SscfTs1::BulkData(std::move(pixelData));

		return std::move(iControlSnapshot);
	}

	void CopyToRefClient::connect(const std::string& network)
	{
		m_log.DeveloperInfo(L"CopyToRefClient::connect has been called with : %s", (str_ext::stow(network)).c_str());
		
		m_eventLoop->start();

		m_eventLoop->send([this, network] {
			m_sscfClient.reset(new SscfTs1::ClientNode(*m_eventLoop, network, IControl::IControlServerCertificate, true, IControl::IControlClientCertificate));
			m_sscfEventHandler.reset(new SscfTs1::EventHandler());
			m_sscfEventHandler->subscribe(m_sscfClient->connected, [this] {onConnected(); });
			m_sscfEventHandler->subscribe(m_sscfClient->disconnected, [this] {onDisconnected(); });
		});

		ConnectAsync();
	}

}}}