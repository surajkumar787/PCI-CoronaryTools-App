// Copyright Koninklijke Philips N.V. 2016
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace std
{
	class condition_variable;
	class mutex;
}

namespace SscfTs1
{
	class EventHandler;
	class EventLoop;
	class ClientNode;
}

namespace AuditTrail
{
	#include "AuditSecurityOptions.h"

	namespace Sscf {
		class IAuditTrail;
		struct NodeAuditTrailInfo;
		struct InstanceAuditTrailInfo;
	}

	struct NodeAuditTrailInfo
	{
		NodeAuditTrailInfo() {}

		NodeAuditTrailInfo(const std::wstring& AETitle, const std::wstring& IPAddress, const std::wstring& HostName) :
			AETitle(AETitle), IPAddress(IPAddress), HostName(HostName)
		{
		}

		operator AuditTrail::Sscf::NodeAuditTrailInfo() const;

		std::wstring AETitle;
		std::wstring IPAddress;
		std::wstring HostName;
	};

	struct InstanceAuditTrailInfo
	{
		InstanceAuditTrailInfo() {}

		InstanceAuditTrailInfo(const std::wstring& PatientID, const std::wstring& PatientName, const std::wstring& StudyInstanceID,
			                   const std::wstring& AccessionNumber, const std::wstring& PerformedProcedureStepID, const std::wstring& SOPClassUID) :
			PatientID(PatientID), PatientName(PatientName), StudyInstanceID(StudyInstanceID),
			AccessionNumber(AccessionNumber), PerformedProcedureStepID(PerformedProcedureStepID), SOPClassUID(SOPClassUID)
		{
		}

		operator AuditTrail::Sscf::InstanceAuditTrailInfo() const;

		std::wstring PatientID;
		std::wstring PatientName;
		std::wstring StudyInstanceID;
		std::wstring AccessionNumber;
		std::wstring PerformedProcedureStepID;
		std::wstring SOPClassUID;
	};


	class AuditTrailClient
    {
    public:
        /* Pass the current userName and processIdentifier, so that you don't need to specify them in some of the methods in this class. 
        It is still possible to override these default values. */
        AuditTrailClient(const std::wstring& userName, 
					     const std::wstring& processIdentifier);

        /* Pass the current userName, processIdentifier, sourceNodeInfo and targetNodeInfo, so that you don't need to specify them in some of the methods in this class. 
        It is still possible to override these default values. */
        AuditTrailClient(const std::wstring& userName, 
					     const std::wstring& processIdentifier, 
						 const NodeAuditTrailInfo& sourceNodeInfo, 
						 const NodeAuditTrailInfo& targetNodeInfo);

		~AuditTrailClient();

		bool WaitForConnection(int timeoutMillis) const;

        // API of IAuditTrail (SSCF) interface

        void UserLogin(const std::wstring& userName);
        void UserLoginFailed(const std::wstring& userName);
        void UserLogoff(const std::wstring& userName);

        /* userName is optional, when not specified, constructor value is used. */
        void ApplicationStarted(const std::wstring& applicationIdentifier, const std::wstring& userName = L"");
        /* userName is optional, when not specified, constructor value is used. */
        void ApplicationStartFailed(const std::wstring& applicationIdentifier, const std::wstring& userName = L"");
        /* userName is optional, when not specified, constructor value is used. */
        void ApplicationStopped(const std::wstring& applicationIdentifier, const std::wstring& userName = L"");
        /* userName is optional, when not specified, constructor value is used. */
        void ApplicationStopFailed(const std::wstring& applicationIdentifier, const std::wstring& userName = L"");

        /* userName and processIdentifier are optional, when not specified, constructor value is used. */
        bool PatientRead(const std::wstring& patientID, const std::wstring& patientName, const std::wstring& userName = L"", const std::wstring& processIdentifier = L"");
        /* userName and processIdentifier are optional, when not specified, constructor value is used. */
        bool PatientReadFailed(const std::wstring& patientID, const std::wstring& patientName, const std::wstring& userName = L"", const std::wstring& processIdentifier = L"");

        /* userName and processIdentifier are optional, when not specified, constructor value is used. */
        void InstancesCreated(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const std::wstring& userName = L"", const std::wstring& processIdentifier = L"");
        /* userName and processIdentifier are optional, when not specified, constructor value is used. */
        void InstancesCreationFailed(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const std::wstring& userName = L"", const std::wstring& processIdentifier = L"");
        /* userName and processIdentifier are optional, when not specified, constructor value is used. */
        void InstancesRead(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const std::wstring& userName = L"", const std::wstring& processIdentifier = L"");
        /* userName and processIdentifier are optional, when not specified, constructor value is used. */
        void InstancesReadFailed(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const std::wstring& userName = L"", const std::wstring& processIdentifier = L"");

        void NetworkNodeAdded(const std::wstring& nodeIdentifier);
        void NetworkNodeRemoved(const std::wstring& nodeIdentifier);

        void BeginTransferring(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const NodeAuditTrailInfo* sourceNodeInfo = nullptr, const NodeAuditTrailInfo* targetNodeInfo = nullptr);
        void BeginTransferFailed(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const NodeAuditTrailInfo* sourceNodeInfo = nullptr, const NodeAuditTrailInfo* targetNodeInfo = nullptr);

        void InstancesTransferred(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const NodeAuditTrailInfo* sourceNodeInfo = nullptr, const NodeAuditTrailInfo* targetNodeInfo = nullptr);
        void InstancesTransferFailed(const std::vector<InstanceAuditTrailInfo>& instancesInfo, const NodeAuditTrailInfo* sourceNodeInfo = nullptr, const NodeAuditTrailInfo* targetNodeInfo = nullptr);

        /* userName and processIdentifier are optional, when not specified, constructor value is used. */
        void SecurityAlert(AuditSecurityOptions auditSecurityOptions, const std::wstring& description,const std::wstring& participantObjectID, const std::wstring& userName= L"", const std::wstring& processIdentifier= L"");
        /* userName and processIdentifier are optional, when not specified, constructor value is used. */
		void SecurityFailure(AuditSecurityOptions auditSecurityOptions, const std::wstring& description, const std::wstring& userName = L"", const std::wstring& processIdentifier = L"");

		AuditTrailClient(const AuditTrailClient &) = delete ;
        AuditTrailClient& operator=(const AuditTrailClient &) = delete;

	private:
        void onConnected();
        void onDisconnected();

		/* Helper function to execute \p proxyFun on a connected IAuditTrail proxy.
		 * Returns true if \p proxyFun was executed; false otherwise.
		 */
		template <typename ProxyFun>
		void WithProxy(ProxyFun proxyFun);

        const std::wstring& GetUserName(const std::wstring& userName)const;
        const std::wstring& GetProcessIdentifier(const std::wstring& processIdentifier) const;

        const Sscf::NodeAuditTrailInfo GetSourceNodeInfo(const NodeAuditTrailInfo* sourceNodeInfo);
        const Sscf::NodeAuditTrailInfo GetTargetNodeInfo(const NodeAuditTrailInfo* targetNodeInfo);

		std::unique_ptr<SscfTs1::EventHandler> m_sscfEventHandler;
		std::unique_ptr<SscfTs1::EventLoop> m_eventLoop;
        std::unique_ptr<SscfTs1::ClientNode> m_sscfClient;

		std::unique_ptr<Sscf::IAuditTrail> m_proxy;

		void InitSscfClient();
		void DestroySscfClient();
		void ConnectAsync();
		void Disconnect();

		void* m_connected; // Windows Event Object

        std::wstring m_defaultUserName;
        std::wstring m_defaultProcessIdentifier;

        std::unique_ptr<NodeAuditTrailInfo> m_defaultSourceNodeInfo;
        std::unique_ptr<NodeAuditTrailInfo> m_defaultTargetNodeInfo;
    };
}
