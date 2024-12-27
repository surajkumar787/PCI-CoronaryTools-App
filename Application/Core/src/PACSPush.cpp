// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PACSPush.h"

#include "syntaxlist.h"
#include "file.h"
#include "exception.h"
#include "item.h"
#include "secureassociation.h"
#include "DicomConstants.h"
#include "Dicom.h"
#include "UidUtils.h"
#include "Log.h"
#include "PciEpx.h"
#include "UtilizationLogEvent.h"
#include "IMessageBuilder.h"
#include "NodeSettings.h"
#include "tlsdefines.h"
#include "AuditTrailClient.h"

#include "stlstring.h"

#include <mergecom.h>
#include <Sense/Graphics/Image.h>
#include <assert.h>
#include <Shlwapi.h>
#include <algorithm>

using Sense::ITexture;
using namespace AuditTrail;
using namespace CommonPlatform;
using namespace CommonPlatform::Xray;

// non-const due to MergeCOM
static char ImplementationVersion[] = "PMS_CORTLS_2.1";
static char ImplementationUid[] = "1.3.46.670589." // philips root
                                  "7."             // IGT
                                  "9."             // product id
                                  "2.1";           // product version

namespace Pci { namespace Core { namespace PACS
{
	namespace Constants = ::CommonPlatform::Dicom::Constants;

	std::vector<InstanceAuditTrailInfo> ConvertStudyToAuditTrailInfo(const XrayStudy &study, const std::wstring sopClassUid)
	{
		InstanceAuditTrailInfo instanceinfo;
		instanceinfo.PatientName				= study.patientName;
		instanceinfo.PatientID					= study.patientId;
		instanceinfo.AccessionNumber			= study.accessionNumber;
		instanceinfo.PerformedProcedureStepID	= study.performedProcedureStepId;
		instanceinfo.SOPClassUID				= sopClassUid;
		instanceinfo.StudyInstanceID			= study.studyInstanceUid;

		return { instanceinfo };
	}

	DicomNode::DicomNode(const NodeSettings& settings) :
		LocalApp(settings.LocalAeTitle),
		RemoteApp(settings.RemoteAeTitle),
		RemoteHost(settings.RemoteHost),
		RemotePort(settings.RemotePort),
		Security(settings.Security),
		CertificateName(settings.Certificate)
	{
	}

	PACSPush::PACSPush(Sense::IScheduler& scheduler, const NodeSettings& storageSettings, const std::wstring &localIp, const CommitNodeSettings& commitSettings, const ConnectionSettings& connectionSettings,
					   IMessageBuilder& messageBuilder, AuditTrailClient &auditClient, bool saveDCM, const std::wstring& saveFolder) :
		log(CommonPlatform::Logger(::CommonPlatform::LoggerType::Platform)),
		messageBuilder(messageBuilder),
		auditTrailClient(auditClient),
		m_localIp( localIp),
		connectionSettings(connectionSettings),
		storageNode(storageSettings),
		commitEnabled(commitSettings.Enabled),
		commitNode(commitSettings),
		saveDCM(saveDCM),
		saveFolder(saveFolder),
		workerThread(scheduler, "PacsPush")
	{
		if (!saveFolder.empty() && saveFolder.back() != L'\\' && saveFolder.back() != L'/')
		{
			this->saveFolder.append(L"\\");
		}

		auto status = MC_Set_Long_Config_Value(PDU_MAXIMUM_LENGTH, connectionSettings.MaxPduLength);
		if (status != MC_NORMAL_COMPLETION)
		{
			log.DeveloperError(L"Failed to set the Max PDU Length to %d (status: %d)", connectionSettings.MaxPduLength, status);
		}
		status = MC_Set_Int_Config_Value(ASSOC_REPLY_TIMEOUT, connectionSettings.TimeoutAssocOpen);
		if (status != MC_NORMAL_COMPLETION)
		{
			log.DeveloperError(L"Failed to set the association open reply timeout to %d (status: %d)", connectionSettings.TimeoutAssocOpen, status);
		}

		status = MC_Set_String_Config_Value(IMPLEMENTATION_CLASS_UID, ImplementationUid);
		if (status != MC_NORMAL_COMPLETION)
		{
			log.DeveloperError(L"Failed to set the implementation class uid to %S", ImplementationUid);
		}
		status = MC_Set_String_Config_Value(IMPLEMENTATION_VERSION, ImplementationVersion);
		if (status != MC_NORMAL_COMPLETION)
		{
			log.DeveloperError(L"Failed to set the implementation version to %S", ImplementationVersion);
		}
	}

	PACSPush::~PACSPush()
	{
		workerThread.flush();
	}

	void PACSPush::sendSnapshot(const MetaData& metaData, const Merge::Message& snapshot, std::function<void(bool success)> callback)
	{
		workerThread.executeAsync([this, metaData, &snapshot, callback = std::move(callback)]
		{
			auto res = sendSCImage(metaData, snapshot);
			if (callback) callback(res);
		});
	}
	void PACSPush::sendMovie(const MetaData& metaData, const Merge::Message& movie, std::function<void(bool success)> callback)
	{
		workerThread.executeAsync([this, &metaData, &movie, callback = std::move(callback)]
		{
			auto res = sendSCMF(metaData, movie);
			if (callback) callback(res);
		});
	}

	bool PACSPush::isBusy() const
	{
		return workerThread.isBusy();
	}

	int PACSPush::queueSize() const
	{
		return workerThread.queueSize();
	}

	void PACSPush::flush()
	{
		workerThread.flush();
	}

	void PACSPush::abort()
	{
		workerThread.clear();
		try
		{
			auto assoc = currentAssociation.lock();
			if (assoc) assoc->abort();
		}
		catch (Merge::Exception&) // this is due to the association already having been aborted so we can safely ignore it
		{}
	}

	bool PACSPush::sendSCImage(const MetaData& metaData, const Merge::Message& snapshot)
	{
		std::string sopClassUid = Constants::SopClass::SecondaryCaptureImageStorage;

		auto& proposedService = messageBuilder.getSnapshotService();

		auto assoc = Merge::createAssociation(storageNode.LocalApp, storageNode.RemoteApp, storageNode.RemotePort, storageNode.RemoteHost,
		                                      Merge::ProposedServiceList({ proposedService }), storageNode.Security, storageNode.CertificateName);
		currentAssociation = assoc;

		bool success = false;
		auto result = assoc->open();
		if (HandleAssociationResult(L"Snapshot", result, proposedService, *assoc))
		{
			AuditBeginTransferring(metaData.study, str_ext::stow(sopClassUid));

			success = SendMsg(*assoc, snapshot, L"Snapshot");
			if (success)
			{
				AuditInstancesTransferred(metaData.study, str_ext::stow(sopClassUid));

				std::string sopInstance;
				success = snapshot.get(MC_ATT_SOP_INSTANCE_UID, sopInstance);
					
				if (success)
				{
					log.DeveloperInfo(L"PACS Snapshot: Snapshot has been sent with sopClass %hs and sopInstance %hs", sopClassUid.c_str(), sopInstance.c_str());
					if (commitEnabled)
					{
						sendStorageCommitRequest(sopClassUid, sopInstance);
					}
				}
				else
				{
					log.DeveloperError(L"PACS Snapshot: Request for sopInstance Failed");
				}
			}
			else
			{
				AuditInstancesTransferFailed(metaData.study, str_ext::stow(sopClassUid));
				::UtilizationLogEvent::ArchiveSnapshotFailed(log,L"Unable to transfer snapshot to PACS");
			}
		}
		else
		{
			AuditBeginTransferFailed(metaData.study, str_ext::stow(sopClassUid));
			::UtilizationLogEvent::ArchiveSnapshotFailed(log,L"PACS not reachable");
			if(storageNode.Security != TLS::SecurityType::NONE)
			{
				AuditSecurityAlert( m_localIp );
			}
		}
		SaveDcm(snapshot);
		return success;
	}

    bool PACSPush::sendSCMF(const MetaData& metaData, const Merge::Message& msg)
    {
		std::string sopClassUid = Constants::SopClass::MultiFrameSecondaryCaptureTrueColor;

        auto& proposedService = messageBuilder.getMovieService();

        auto assoc = Merge::createAssociation(storageNode.LocalApp, storageNode.RemoteApp, storageNode.RemotePort, storageNode.RemoteHost,
											  Merge::ProposedServiceList({ proposedService }), storageNode.Security, storageNode.CertificateName);
		currentAssociation = assoc;

        bool success = false;
        auto result = assoc->open();
        if (HandleAssociationResult(L"Movie", result, proposedService, *assoc))
        {
            AuditBeginTransferring(metaData.study, str_ext::stow(sopClassUid));

            success = SendMsg(*assoc, msg, L"Movie");
            if (success)
            {
                AuditInstancesTransferred(metaData.study, str_ext::stow(sopClassUid));

                std::string sopInstance;
                success = msg.get(MC_ATT_SOP_INSTANCE_UID, sopInstance);
                if (success)
                {
					log.DeveloperInfo(L"PACS Movie: Movie has been sent with sopClass %hs and sopInstance %hs", sopClassUid.c_str(), sopInstance.c_str());
					if (commitEnabled)
					{
						sendStorageCommitRequest(sopClassUid, sopInstance);
					}
                }
				else
				{
					log.DeveloperError(L"PACS Snapshot: Request for sopInstance Failed");
				}
            }
            else
            {
				::UtilizationLogEvent::ArchiveMovieFailed(log, L"Failed to sent Movie to Pacs");
                AuditInstancesTransferFailed(metaData.study, str_ext::stow(sopClassUid));
            }
        }
        else
        {
            AuditBeginTransferFailed(metaData.study, str_ext::stow(sopClassUid));
			::UtilizationLogEvent::ArchiveMovieFailed(log, L"PACS unreachable.");
            if (storageNode.Security != TLS::SecurityType::NONE)
			{
                AuditSecurityAlert(m_localIp);
			}
		}

        SaveDcm(msg);
        return success;
    }

	void PACSPush::sendStorageCommitRequest(const std::string& sopClass, const std::string& sopInstance)
	{
		Merge::Service proposedService(
			Merge::sopClassUidToserviceName(Constants::SopClass::StorageCommitPush),
			Merge::SyntaxList(Merge::ArrayToVector(Constants::BasicSyntaxList)),
			Merge::Service::Role::SCU);

		std::vector<Merge::Service> serviceList;
		serviceList.push_back(proposedService);
		auto assoc = Merge::createAssociation(commitNode.LocalApp, commitNode.RemoteApp, commitNode.RemotePort, commitNode.RemoteHost,
											  Merge::ProposedServiceList(Constants::ServiceList::StorageCommitScu), commitNode.Security, commitNode.CertificateName);
		currentAssociation = assoc;

		auto result = assoc->open();
		if (!HandleAssociationResult(L"Commit", result, proposedService, *assoc))
		{
			return;
		}

		auto msg = messageBuilder.CreateStorageCommitRequest(sopClass, sopInstance);
		if (!msg)
		{
			log.DeveloperError(L"PACS Commit: Failed creating the storage commit request message");
			return;
		}

		if (SendMsg(*assoc, *msg, L"Commit"))
		{
			std::string transUid;
			bool succes = msg->get(MC_ATT_TRANSACTION_UID, transUid);

			if (succes)
			{
				log.DeveloperInfo(L"PACS Commit: Commit Request successfully send with the transaction UID %hs for the message (%hs, %hs)",
					transUid.c_str(), sopClass.c_str(), sopInstance.c_str());
			}
			else
			{
				log.DeveloperError(L"PACS Commit: unknown the transaction UID for message (%hs, %hs)", sopClass.c_str(), sopInstance.c_str());
			}
		}
	}

	void PACSPush::SaveDcm(const Merge::Message& msg) const
	{
		if (saveDCM)
		{
			std::wstring uid;
			if (!msg.get(MC_ATT_SOP_INSTANCE_UID, uid))
			{
				log.DeveloperError(L"SaveDcm : Unknown SopInstance For file to save");
			}

			auto file = msg.toFile();
			file.write(saveFolder + uid + L".dcm");
		}
	}

	bool PACSPush::HandleAssociationResult(const wchar_t* messageType, Merge::Association::OpenResult result, const Merge::Service& service,
										   const Merge::Association& association) const
	{
		switch (result.status)
		{
		case Merge::Association::OpenStatus::Success:
			{
				std::wstringstream strm;
				strm << L"PACS " << messageType <<": opened association with the service(s):\n";
				PrintServices(strm, result.services);
				log.DeveloperInfo(strm.str().c_str());
			}
			break;
		case Merge::Association::OpenStatus::Rejected:
			{
				std::wstringstream strm;
				strm << L"PACS " << messageType << ": No service found that supports the requested service.\nRequested:\n";
				PrintService(strm, service);
				log.DeveloperError(strm.str().c_str());
			}
			break;
		case Merge::Association::OpenStatus::Aborted:
			log.DeveloperError(L"PACS %s: The remote server aborted the connection (%s, %s:%d)",
							   messageType, association.remoteApplication().aeTitle().c_str(), association.host().c_str(), association.port());
			break;
		case Merge::Association::OpenStatus::Timeout:
			log.DeveloperError(L"PACS %s: Timeout while trying to connect the remote server (%s, %s:%d)",
								messageType, association.remoteApplication().aeTitle().c_str(), association.host().c_str(), association.port());
			break;
		default:
			log.DeveloperError(L"PACS %s: Unable to connect to the remote server (%s, %s:%d), status: %d",
								messageType, association.remoteApplication().aeTitle().c_str(), association.host().c_str(), association.port(), result.status);
			break;
		}
		return result.status == Merge::Association::OpenStatus::Success;
	}

	enum class ResponseStatus
	{
		Success,
		Warning,
		Failure
	};

	ResponseStatus getResponseStatus(uint16_t responseStatus)
	{
		// All success response codes are equal to 0
		if (responseStatus == 0)
		{
			return ResponseStatus::Success;
		}
		// All warning response codes are defined to have the upper nibble set to 0xB
		else if ((responseStatus & 0xB000) == 0xB000)
		{
			return ResponseStatus::Warning;
		}

		return ResponseStatus::Failure;
	}

	bool PACSPush::SendMsg(Merge::Association& association, const Merge::Message& msg, const wchar_t* messageType) const
	{
		bool succes = true;
		try
		{
			association.send(msg);
			auto response = association.read(std::chrono::seconds(connectionSettings.TimeoutReceiveResponse));
			uint16_t status;
			response.get(MC_ATT_STATUS, status);
			auto resp = getResponseStatus(status);

			std::wstring errorComment;
			response.get(MC_ATT_ERROR_COMMENT, errorComment);
			if (!errorComment.empty())
			{
				errorComment = L" (" + errorComment + L")";
			}

			if (resp == ResponseStatus::Success)
			{
				log.DeveloperInfo(L"PACS %s: successfully send message; 0x%.4X%s", messageType, status, errorComment.c_str());
			}
			else if (resp == ResponseStatus::Warning)
			{
				
				log.DeveloperWarning(L"PACS %s: send message with a warning response; 0x%.4X%s", messageType, status, errorComment.c_str());
			}
			else
			{
				log.DeveloperError(L"PACS %s: the remote server (%s) returned a failure response: 0x%.4X%s",
					association.remoteApplication().aeTitle().c_str(), messageType, status, errorComment.c_str());
				succes = false;
			}
		}
		catch (const Merge::TimeoutException& e)
		{
			log.DeveloperError(L"PACS %s: no response received from the remote server: %S", messageType, e.what());
			succes = false;
		}
		catch (const Merge::AssociationAbortedException&)
		{
			log.DeveloperWarning(L"PACS %s: the association was aborted", messageType);
			succes = false;
		}
		catch (const Merge::Exception& e)
		{
			log.DeveloperError(L"PACS %s: failed to send the message to the remote server: %S", messageType, e.what());
			succes = false;
		}
		return succes;
	}

	void PACSPush::AuditBeginTransferring(const XrayStudy &study, const std::wstring& sopClassUid) const
	{
		auditTrailClient.BeginTransferring(ConvertStudyToAuditTrailInfo(study, sopClassUid));
	}

	void PACSPush::AuditBeginTransferFailed(const XrayStudy &study, const std::wstring& sopClassUid) const
	{
		auditTrailClient.BeginTransferFailed(ConvertStudyToAuditTrailInfo(study, sopClassUid));
	}

	void PACSPush::AuditInstancesTransferred(const XrayStudy &study, const std::wstring& sopClassUid) const
	{
		auditTrailClient.InstancesTransferred(ConvertStudyToAuditTrailInfo(study, sopClassUid));
	}

	void PACSPush::AuditInstancesTransferFailed(const XrayStudy &study, const std::wstring& sopClassUid) const
	{
		auditTrailClient.InstancesTransferFailed(ConvertStudyToAuditTrailInfo(study, sopClassUid));
	}

	void PACSPush::AuditSecurityAlert(const std::wstring& participantObjectID) const
	{
		auditTrailClient.SecurityAlert(AuditSecurityOptions::NodeAuthentication, L"Node Authentication failed" , participantObjectID, L"" , L"PciApp");
	}
}}}
