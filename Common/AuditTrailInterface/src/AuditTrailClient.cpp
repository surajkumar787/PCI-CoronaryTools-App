// Copyright Koninklijke Philips N.V. 2016
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "AuditTrailClient.h"

#include <assert.h>

#include <windows.h>
#undef GetUserName // Name clash with class method

#include "IAuditTrail.h"
#include "Log.h"

namespace AuditTrail 
{
    AuditTrailClient::AuditTrailClient(const std::wstring& userName, const std::wstring& processIdentifier):
		m_defaultUserName(userName),
        m_defaultProcessIdentifier(processIdentifier),
		m_connected(CreateEvent(NULL, TRUE, FALSE, NULL)) // default security attributes, manual-reset event, initial state is non-signaled
	{
		assert (m_connected);
		InitSscfClient();
		ConnectAsync();
    }

    AuditTrailClient::AuditTrailClient(const std::wstring& userName, const std::wstring& processIdentifier, 
		                               const NodeAuditTrailInfo& sourceNodeInfo, const NodeAuditTrailInfo& targetNodeInfo):
        m_defaultUserName(userName),
        m_defaultProcessIdentifier(processIdentifier),
        m_defaultSourceNodeInfo(new NodeAuditTrailInfo(sourceNodeInfo)),
        m_defaultTargetNodeInfo(new NodeAuditTrailInfo(targetNodeInfo)),
		m_connected(CreateEvent(NULL, TRUE, FALSE, NULL)) // default security attributes, manual-reset event, initial state is non-signaled
    {
		assert (m_connected);
		InitSscfClient();
		ConnectAsync();
	}

    AuditTrailClient::~AuditTrailClient()
    {
		Disconnect();
		DestroySscfClient();
		if (m_connected != NULL) CloseHandle(m_connected);
    }

	/** Starts connecting to the server asynchronously */
	void AuditTrailClient::ConnectAsync()
	{
		m_eventLoop->send([this]{
			assert (!m_proxy);
			m_sscfClient->connectAsync();
		});
    }

	void AuditTrailClient::Disconnect()
	{
		m_eventLoop->send([this]{
			m_proxy.reset();
			m_sscfClient->disconnect();
		});
	}

	void AuditTrailClient::InitSscfClient()
	{
		m_eventLoop.reset(new SscfTs1::EventLoop());
		m_eventLoop->start();

		m_eventLoop->send([this]{
			m_sscfClient.reset(new SscfTs1::ClientNode(*m_eventLoop, AuditTrailConstants::localHost, AuditTrailConstants::serviceResourceName));
			m_sscfEventHandler.reset(new SscfTs1::EventHandler());
			m_sscfEventHandler->subscribe(m_sscfClient->connected, [this]{onConnected();});
			m_sscfEventHandler->subscribe(m_sscfClient->disconnected, [this]{onDisconnected();});
		});
	}

	void AuditTrailClient::DestroySscfClient()
	{
		// Destroy the event handler and the SSCF client
		// This unsubscribes all events and destroys the event loop proxy,
		// so it needs to be done from the event loop thread
		m_eventLoop->send([this](){
			m_sscfEventHandler.reset();
			m_sscfClient.reset();
		});

		m_eventLoop.reset();
	}

	/** Waits until a connection has been established by the Sscf client, or until the timeout expires.
	 *  Returns true if there is an active connection before the timeout expires.
	 */
	bool AuditTrailClient::WaitForConnection(int timeoutMillis) const
	{
		return WaitForSingleObject(m_connected, timeoutMillis) == WAIT_OBJECT_0;
	}

	static std::unique_ptr<Sscf::IAuditTrail> TryCreateProxy(SscfTs1::ClientNode& clientNode)
	{
		try
		{
			assert (clientNode.providesType<Sscf::IAuditTrailProxy>());
			return std::unique_ptr<Sscf::IAuditTrailProxy>(new Sscf::IAuditTrailProxy(clientNode, AuditTrailConstants::serviceInstanceName));
		}
		catch (const SscfTs1::CommunicationException&)
		{
			auto &log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);
			log.DeveloperInfo(L"AuditTrailClient::TryCreateProxy: Caught SscfTs1::CommunicationException");

			return nullptr;
		}
	}

	void AuditTrailClient::onConnected()
    {
		assert (m_eventLoop->checkThread());

		assert (!m_proxy);
		m_proxy = TryCreateProxy(*m_sscfClient);

		// Signal WaitForConnection() if connection has been made
		if (m_proxy)
		{
			SetEvent(m_connected);
		}
    }

    void AuditTrailClient::onDisconnected()
    {
		assert (m_eventLoop->checkThread());

		m_proxy.reset();
		m_sscfClient->connectAsync();

		ResetEvent(m_connected);
	}

    const std::wstring& AuditTrailClient::GetUserName(const std::wstring& userName) const
    {
        if (userName.empty()) return m_defaultUserName;
        else return userName;
    }

	const std::wstring& AuditTrailClient::GetProcessIdentifier(const std::wstring& processIdentifier) const
	{
		if (processIdentifier.empty())
		{
			return m_defaultProcessIdentifier;
		}
		else
		{
			return processIdentifier;
		}
	}

    const Sscf::NodeAuditTrailInfo AuditTrailClient::GetSourceNodeInfo(const NodeAuditTrailInfo* sourceNodeInfo)
    {
		if (sourceNodeInfo != nullptr)
		{
			return *sourceNodeInfo;
		}

		if (m_defaultSourceNodeInfo != nullptr)
		{
			return *m_defaultSourceNodeInfo;
		}

		throw std::invalid_argument("You did not pass a default sourceNodeInfo in the constructor.");
    }

    const Sscf::NodeAuditTrailInfo AuditTrailClient::GetTargetNodeInfo(const NodeAuditTrailInfo* targetNodeInfo)
    {
		if (targetNodeInfo != nullptr)
		{
			return *targetNodeInfo;
		}

		if (m_defaultTargetNodeInfo != nullptr)
		{
			return *m_defaultTargetNodeInfo;
		}

		throw std::invalid_argument("You did not pass a default targetNodeInfo in the constructor.");
    }

	static std::vector<Sscf::InstanceAuditTrailInfo> ConvertInstancesInfoToSscf(const std::vector<InstanceAuditTrailInfo>& instancesInfo)
	{
		std::vector<Sscf::InstanceAuditTrailInfo> v;
		v.reserve(instancesInfo.size());

		std::copy(std::begin(instancesInfo), std::end(instancesInfo), std::back_inserter(v));
		return v;
	}

	template <typename ProxyFun>
	void AuditTrailClient::WithProxy(ProxyFun proxyFun)
	{
		m_eventLoop->send([this, &proxyFun]
		{
			if (!m_proxy)
			{
				auto& log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);
				log.DeveloperInfo(L"AuditTrailClient::WithProxy: client not connected");
			}
			else try
			{
				proxyFun(*m_proxy);
			}
			catch (const SscfTs1::CommunicationException&)
			{
				auto& log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);
				log.DeveloperInfo(L"AuditTrailClient::WithProxy: Caught SscfTs1::CommunicationException");

			}
		});
	}

    void AuditTrailClient::UserLogin(const std::wstring& userName)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.UserLogin(userName);
		});
    }

    void AuditTrailClient::UserLoginFailed(const std::wstring& userName)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
	        proxy.UserLoginFailed(userName);
		});
    }

    void AuditTrailClient::UserLogoff(const std::wstring& userName)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.UserLogoff(userName);
		});
    }

    void AuditTrailClient::ApplicationStarted(const std::wstring& applicationIdentifier, const std::wstring& userName)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.ApplicationStarted(applicationIdentifier, GetUserName(userName));
		});
    }

    void AuditTrailClient::ApplicationStartFailed(const std::wstring& applicationIdentifier, const std::wstring& userName)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.ApplicationStartFailed(applicationIdentifier, GetUserName(userName));
		});
    }

    void AuditTrailClient::ApplicationStopped(const std::wstring& applicationIdentifier, const std::wstring& userName)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.ApplicationStopped(applicationIdentifier, GetUserName(userName));
		});
    }

    void AuditTrailClient::ApplicationStopFailed(const std::wstring& applicationIdentifier, const std::wstring& userName)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.ApplicationStopFailed(applicationIdentifier, GetUserName(userName));
		});
    }

    void AuditTrailClient::InstancesCreated(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const std::wstring& userName, const std::wstring& processIdentifier)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.InstancesCreated(ConvertInstancesInfoToSscf(instancesInfo), GetUserName(userName), GetProcessIdentifier(processIdentifier));
		});
    }

    void AuditTrailClient::InstancesCreationFailed(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const std::wstring& userName, const std::wstring& processIdentifier)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.InstancesCreationFailed(ConvertInstancesInfoToSscf(instancesInfo), GetUserName(userName), GetProcessIdentifier(processIdentifier));
		});
    }

    void AuditTrailClient::InstancesRead(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const std::wstring& userName, const std::wstring& processIdentifier)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.InstancesRead(ConvertInstancesInfoToSscf(instancesInfo), GetUserName(userName), GetProcessIdentifier(processIdentifier));
		});
    }

    void AuditTrailClient::InstancesReadFailed(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const std::wstring& userName, const std::wstring& processIdentifier)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.InstancesReadFailed(ConvertInstancesInfoToSscf(instancesInfo), GetUserName(userName), GetProcessIdentifier(processIdentifier));
		});
    }

    void AuditTrailClient::NetworkNodeAdded(const std::wstring& nodeIdentifier)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.NetworkNodeAdded(nodeIdentifier);
		});
    }

    void AuditTrailClient::NetworkNodeRemoved(const std::wstring& nodeIdentifier)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.NetworkNodeRemoved(nodeIdentifier);
		});
    }

    void AuditTrailClient::BeginTransferring(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const NodeAuditTrailInfo* sourceNodeInfo, const NodeAuditTrailInfo* targetNodeInfo)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.BeginTransferring(ConvertInstancesInfoToSscf(instancesInfo), GetSourceNodeInfo(sourceNodeInfo), GetTargetNodeInfo(targetNodeInfo));
		});
    }

    void AuditTrailClient::BeginTransferFailed(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const NodeAuditTrailInfo* sourceNodeInfo, const NodeAuditTrailInfo* targetNodeInfo)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.BeginTransferFailed(ConvertInstancesInfoToSscf(instancesInfo), GetSourceNodeInfo(sourceNodeInfo), GetTargetNodeInfo(targetNodeInfo));
		});
    }

    void AuditTrailClient::InstancesTransferred(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const NodeAuditTrailInfo* sourceNodeInfo, const NodeAuditTrailInfo* targetNodeInfo)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.InstancesTransferred(ConvertInstancesInfoToSscf(instancesInfo), GetSourceNodeInfo(sourceNodeInfo), GetTargetNodeInfo(targetNodeInfo));
		});
    }

    void AuditTrailClient::InstancesTransferFailed(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const NodeAuditTrailInfo* sourceNodeInfo, const NodeAuditTrailInfo* targetNodeInfo)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.InstancesTransferFailed(ConvertInstancesInfoToSscf(instancesInfo), GetSourceNodeInfo(sourceNodeInfo), GetTargetNodeInfo(targetNodeInfo));
		});
    }

	static Sscf::AuditSecurityOptions ConvertSecurityOptions(AuditSecurityOptions option);

	void AuditTrailClient::SecurityAlert(AuditSecurityOptions auditSecurityOptions, const std::wstring& description,const std::wstring& participantObjectID, const std::wstring& userName, const std::wstring& processIdentifier)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.SecurityAlert(ConvertSecurityOptions(auditSecurityOptions), description, participantObjectID,
				                GetUserName(userName), GetProcessIdentifier(processIdentifier));
		});
    }

	void AuditTrailClient::SecurityFailure(AuditSecurityOptions auditSecurityOptions, const std::wstring& description, const std::wstring& userName, const std::wstring& processIdentifier)
    {
		WithProxy([&](Sscf::IAuditTrail& proxy) {
			proxy.SecurityFailure(ConvertSecurityOptions(auditSecurityOptions), description,
				                  GetUserName(userName), GetProcessIdentifier(processIdentifier));
		});
    }

	NodeAuditTrailInfo::operator Sscf::NodeAuditTrailInfo() const
	{
		Sscf::NodeAuditTrailInfo info;
		info.AETitle = AETitle;
		info.IPAddress = IPAddress;
		info.HostName = HostName;
		return info;
	}

	InstanceAuditTrailInfo::operator Sscf::InstanceAuditTrailInfo() const
	{
		Sscf::InstanceAuditTrailInfo info;
		info.PatientID = PatientID;
		info.PatientName = PatientName;
		info.StudyInstanceID = StudyInstanceID;
		info.AccessionNumber = AccessionNumber;
		info.PerformedProcedureStepID = PerformedProcedureStepID;
		info.SOPClassUID = SOPClassUID;
		return info;
	}

	Sscf::AuditSecurityOptions ConvertSecurityOptions(AuditSecurityOptions option)
	{
		switch (option)
		{
		case AuditSecurityOptions::Unknown:                         return AuditTrail::Sscf::AuditSecurityOptions::Unknown;
		case AuditSecurityOptions::NodeAuthentication:              return AuditTrail::Sscf::AuditSecurityOptions::NodeAuthentication;
		case AuditSecurityOptions::EmergencyOverrideStarted:        return AuditTrail::Sscf::AuditSecurityOptions::EmergencyOverrideStarted;
		case AuditSecurityOptions::NetworkConfiguration:            return AuditTrail::Sscf::AuditSecurityOptions::NetworkConfiguration;
		case AuditSecurityOptions::SecurityConfiguration:           return AuditTrail::Sscf::AuditSecurityOptions::SecurityConfiguration;
		case AuditSecurityOptions::HardwareConfiguration:           return AuditTrail::Sscf::AuditSecurityOptions::HardwareConfiguration;
		case AuditSecurityOptions::SoftwareConfiguration:           return AuditTrail::Sscf::AuditSecurityOptions::SoftwareConfiguration;
		case AuditSecurityOptions::UseOfRestrictedFunction:         return AuditTrail::Sscf::AuditSecurityOptions::UseOfRestrictedFunction;
		case AuditSecurityOptions::AuditRecordingStopped:           return AuditTrail::Sscf::AuditSecurityOptions::AuditRecordingStopped;
		case AuditSecurityOptions::AuditRecordingStarted:           return AuditTrail::Sscf::AuditSecurityOptions::AuditRecordingStarted;
		case AuditSecurityOptions::ObjectSecurityAttributesChanged: return AuditTrail::Sscf::AuditSecurityOptions::ObjectSecurityAttributesChanged;
		case AuditSecurityOptions::SecurityRolesChanged:            return AuditTrail::Sscf::AuditSecurityOptions::SecurityRolesChanged;
		case AuditSecurityOptions::UserSecurityAttributesChanged:   return AuditTrail::Sscf::AuditSecurityOptions::UserSecurityAttributesChanged;
		case AuditSecurityOptions::EmergencyOverrideStopped:        return AuditTrail::Sscf::AuditSecurityOptions::EmergencyOverrideStopped;
		case AuditSecurityOptions::RemoteServiceOperationStarted:   return AuditTrail::Sscf::AuditSecurityOptions::RemoteServiceOperationStarted;
		case AuditSecurityOptions::RemoteServiceOperationStopped:   return AuditTrail::Sscf::AuditSecurityOptions::RemoteServiceOperationStopped;
		case AuditSecurityOptions::LocalServiceOperationStarted:    return AuditTrail::Sscf::AuditSecurityOptions::LocalServiceOperationStarted;
		case AuditSecurityOptions::LocalServiceOperationStopped:    return AuditTrail::Sscf::AuditSecurityOptions::LocalServiceOperationStopped;
		}

		return AuditTrail::Sscf::AuditSecurityOptions::Unknown;
	}
};
