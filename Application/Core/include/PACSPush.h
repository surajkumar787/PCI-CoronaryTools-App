// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once


#include "IPACSPush.h"

#include "Thread.h"
#include "ConnectionSettings.h"
#include "application.h"
#include "association.h"

#include <stdint.h>

namespace CommonPlatform
{
	class Log;
	namespace Dicom { namespace TLS
	{
		enum class SecurityType;
	}}
	namespace Xray
	{
		struct XrayGeometry;
		struct XrayStudy;
	}
}

namespace AuditTrail { class AuditTrailClient; }

namespace Pci { namespace Core { namespace PACS
{
	namespace Merge = ::CommonPlatform::Dicom::Merge;
	namespace TLS = ::CommonPlatform::Dicom::TLS;
	namespace Dicom = ::CommonPlatform::Dicom;

	class IMessageBuilder;
	struct NodeSettings;
	struct CommitNodeSettings;

	struct DicomNode
	{
		explicit DicomNode(const NodeSettings& settings);

		Merge::Application LocalApp;
		Merge::Application RemoteApp;
		std::wstring RemoteHost;
		int RemotePort;
		TLS::SecurityType Security;
		std::wstring CertificateName;
	};

	class PACSPush : public IPACSPush
	{
	public:
		PACSPush(Sense::IScheduler& scheduler, const NodeSettings& storageSettings, const std::wstring &localIp,  const CommitNodeSettings& commitSettings, const ConnectionSettings& connectionSettings,
				 IMessageBuilder& msgBuilder, AuditTrail::AuditTrailClient &auditTrailclient, bool saveDCM = false, const std::wstring& saveFolder = L"");

		PACSPush(const PACSPush&) = delete;
		PACSPush& operator= (const PACSPush&) = delete;

		virtual ~PACSPush();

		virtual void sendSnapshot(const MetaData& metaData, const Merge::Message& snapshot, std::function<void(bool success)> callback) override;
		virtual void sendMovie(const MetaData& metaData, const Merge::Message& movie, std::function<void(bool success)> callback) override;

		virtual bool isBusy() const override;
		virtual int queueSize() const override;
		virtual void flush() override;

		virtual void abort() override;

	private:
		void sendStorageCommitRequest(const std::string& sopClass, const std::string& sopInstance);
		bool sendSCMF(const MetaData& metaData, const Merge::Message& message);
		bool sendSCImage(const MetaData& metaData, const Merge::Message& snapshot);

		bool HandleAssociationResult(const wchar_t* messageType, Merge::Association::OpenResult result, const Merge::Service& service,
									 const Merge::Association& association) const;
		

		bool SendMsg(Merge::Association& association, const Merge::Message& msg, const wchar_t* messageType) const;
		void SaveDcm(const Merge::Message& msg) const;

		void AuditBeginTransferring(const CommonPlatform::Xray::XrayStudy &study, const std::wstring& sopClassUid) const;
		void AuditBeginTransferFailed(const CommonPlatform::Xray::XrayStudy &study, const std::wstring& sopClassUid) const;
		void AuditInstancesTransferred(const CommonPlatform::Xray::XrayStudy &study, const std::wstring& sopClassUid) const;
		void AuditInstancesTransferFailed(const CommonPlatform::Xray::XrayStudy &study, const std::wstring& sopClassUid) const;
		void AuditSecurityAlert(const std::wstring& participantObjectID) const;

		::CommonPlatform::Log& log;
		IMessageBuilder& messageBuilder;
		AuditTrail::AuditTrailClient& auditTrailClient;

		std::wstring m_localIp;
		ConnectionSettings connectionSettings;

		DicomNode storageNode;

		bool commitEnabled;
		DicomNode commitNode;

		bool saveDCM;
		std::wstring saveFolder;

		CommonPlatform::Thread workerThread;
		std::weak_ptr<Merge::Association> currentAssociation;
	};

}}}
