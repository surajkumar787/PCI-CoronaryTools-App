// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StorageCommitListener.h"

#include "message.h"
#include "item.h"
#include "exception.h"
#include "DicomConstants.h"
#include "NodeSettings.h"
#include "Log.h"
#include "tlsdefines.h"

#include <mergecom.h>

static const DWORD ExitEventTimeoutInMs = 250;

namespace Pci { namespace Core { namespace PACS
{
	namespace Constants = ::CommonPlatform::Dicom::Constants;
	namespace TLS = ::CommonPlatform::Dicom::TLS;

	StorageReport ReadReport(const Merge::Message& msg, unsigned long tag)
	{
		StorageReport report;
		std::wstring recTransUid;
		msg.get(MC_ATT_TRANSACTION_UID, recTransUid);
		report.TransactionUid = recTransUid;

		std::vector<Merge::Item> items;
		msg.get(tag, items);
		for(const auto& item : items)
		{
			std::wstring recSopClass, recSopInstance;
			if (item.get(MC_ATT_REFERENCED_SOP_CLASS_UID, recSopClass) &&
				item.get(MC_ATT_REFERENCED_SOP_INSTANCE_UID, recSopInstance))
				report.Items.emplace_back(recSopClass, recSopInstance);
		}
		return report;
	}

	StorageCommitListener::StorageCommitListener(Sense::IScheduler &scheduler, const CommitNodeSettings& settings, const ConnectionSettings& connectionSettings) :
		m_log(CommonPlatform::Logger(::CommonPlatform::LoggerType::Platform)),
		m_connectionSettings(connectionSettings),
		m_exit(CreateEvent(nullptr, TRUE, FALSE, nullptr)),
		m_thread(scheduler)
	{
		m_thread.executeAsync([=]()
		{
			Merge::Service srv(Merge::sopClassUidToserviceName(Constants::SopClass::StorageCommitPush),
								Merge::SyntaxList(Merge::ArrayToVector(Constants::BasicSyntaxList)),
								// Since this is only for the report (not providing a storage commit), it is SCU and not SCP
								Merge::Service::Role::SCU);
			std::vector<Merge::Service> sl;
			sl.push_back(srv);
			Merge::Application localApp(settings.LocalAeTitle);
			Merge::Application remoteApp(settings.RemoteAeTitle);
			
			auto assoc = Merge::createAssociation(localApp, remoteApp, settings.ListeningPort, L"0.0.0.0", Merge::ProposedServiceList(sl),
												  settings.Security, settings.Certificate);
			while (WaitForSingleObject(m_exit, ExitEventTimeoutInMs) == WAIT_TIMEOUT)
			{
				AssocInfo assocInfo = {};
				// poll for incoming connections without waiting
				auto result = assoc->wait(std::chrono::seconds(0), &assocInfo);
				if (result == Merge::Association::OpenStatus::Success)
				{
					m_log.DeveloperInfo(L"StorageCommitListener: Incoming association from %S, Host: %S, %S, Implementation Class: %S, Implementation Version: %S",
										assocInfo.RemoteApplicationTitle, assocInfo.RemoteHostName, assocInfo.RemoteIPAddress,
										assocInfo.RemoteImplementationClassUID, assocInfo.RemoteImplementationVersion);

					try
					{
						for (;;)
						{
							auto msg = assoc->read(std::chrono::seconds(m_connectionSettings.TimeoutReceiveRequest));
							HandleMessage(*assoc, msg);
						}
					}
					catch (const Merge::AssociationClosedException& /*exception*/)
					{
						// remote host closed the connection, no more messages are coming
						m_log.DeveloperInfo(L"StorageCommitListener: association closed");
					}
					catch (const Merge::Exception& e)
					{
						m_log.DeveloperError(L"StorageCommitListener: Exception during read from association: %S", e.what());
						assoc->abort();
					}
				}
			}
		});

		auto status = MC_Set_Long_Config_Value(PDU_MAXIMUM_LENGTH, connectionSettings.MaxPduLength);
		if (status != MC_NORMAL_COMPLETION)
		{
			m_log.DeveloperError(L"Failed to set the Max PDU Length to %d (status: %d)", connectionSettings.MaxPduLength, status);
		}
		status = MC_Set_Int_Config_Value(ASSOC_REPLY_TIMEOUT, connectionSettings.TimeoutAssocOpen);
		if (status != MC_NORMAL_COMPLETION)
		{
			m_log.DeveloperError(L"Failed to set the association open reply timeout to %d (status: %d)", connectionSettings.TimeoutAssocOpen, status);
		}
	}

	void StorageCommitListener::HandleMessage(Merge::Association& assoc, Merge::Message& msg)
	{
		switch (msg.command())
		{
		case N_EVENT_REPORT_RQ:
			HandleEventReportMessage(assoc, msg);
			break;
		default:
			m_log.DeveloperWarning(L"StorageCommitListener: Received an unknown message command: %d", msg.command());
			break;
		}
	}

	void StorageCommitListener::HandleEventReportMessage(Merge::Association& assoc, Merge::Message& msg)
	{
		unsigned short transactionResult = 0;
		msg.get(MC_ATT_EVENT_TYPE_ID, transactionResult);
		switch (transactionResult)
		{
		case 2: // at least some failed
			{
				const auto& report = ReadReport(msg, MC_ATT_FAILED_SOP_SEQUENCE);
				if (onStoreFailure) onStoreFailure(report);
			} // intentional fallthrough
		case 1: // all succeeded
			{
				const auto& report = ReadReport(msg, MC_ATT_REFERENCED_SOP_SEQUENCE);
				if (!report.Items.empty() && onStoreSuccess) onStoreSuccess(report);
			}
			break;
		default:
			m_log.DeveloperWarning(L"StorageCommitListener: Event report with unknown transaction result %hu", transactionResult);
			break;
		}

		Merge::Message respMsg(Merge::sopClassUidToserviceName(Constants::SopClass::StorageCommitPush), N_EVENT_REPORT_RSP);
		assoc.send(respMsg, N_EVENT_SUCCESS);
		m_log.DeveloperInfo(L"StorageCommitListener: Responding to event report request");
	}

	StorageCommitListener::~StorageCommitListener()
	{
		CloseHandle(m_exit);
	}
}}}