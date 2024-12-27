// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "MergeTlsAdapter.h"

#include "SChannelController.h"
#include "tlsdefines.h"
#include "applicationcontext.h"

#include "Log.h"

namespace CommonPlatform { namespace Dicom { namespace TLS
{
	static ::CommonPlatform::Log& log(::CommonPlatform::Logger(LoggerType::Platform));

	static const int TlsNegotiationTimeoutInMs = 30000;

	SS_STATUS MergeTlsAdapter::SS_Session_Start(MC_SOCKET SocketToUse, CONN_TYPE ConnectionType, void* ApplicationContext, void** /*SecurityContext*/)
	{
		log.DeveloperInfo(L"MergeTlsAdapter: Start SecureSession");
		if (ApplicationContext == nullptr) 
		{
			log.DeveloperError(L"MergeTlsAdapter: no ApplicationContext");
			return SS_ERROR;
		}

		TLS::ApplicationContext* applicationContext = reinterpret_cast<TLS::ApplicationContext*> (ApplicationContext);
		log.DeveloperInfo(L"MergeTlsAdapter: Using socket %Iu, Certificate %s, SocketType %s",
						  SocketToUse, applicationContext->certificateToUse.c_str(), (ConnectionType == ACCEPTOR_CONNECTION ? L"Server" : L"Client"));

		try
		{
			applicationContext->m_securityContext.reset(new SChannelController(
				SocketToUse,
				(ConnectionType == ACCEPTOR_CONNECTION ? SocketType::SERVER : SocketType::CLIENT),
				applicationContext->securityType,
				applicationContext->certificateToUse,
				applicationContext->personalStoreToUse,
				applicationContext->trustStoreToUse,
				applicationContext->allowSelfSignedCertificates,
				applicationContext->checkCertificateEKU,
				true,
				TlsNegotiationTimeoutInMs));
		}
		catch(const std::exception& ex)
		{
			log.DeveloperError(L"MergeTlsAdapter: SChannelController.ctor threw an exception; %S", ex.what());
			applicationContext->setErrorCode(TLS_ERR_CRED_ACQUIRE); //Only case when exception intentionally is thrown.
			return SS_ERROR;
		}

		auto& controller = applicationContext->m_securityContext;

		if(!controller->tlsNegotiate())
		{
			//AuditTrail::AuditSecurityAlert(applicationContext->DicomNodeName,applicationContext->certificateToUse,false);
			applicationContext->setErrorCode(controller->getLastError());
			log.DeveloperError(L"MergeTlsAdapter: negotiation failed (%d)", applicationContext->getErrorCode());
			return SS_ERROR;
		}
		//AuditTrail::AuditSecurityAlert(applicationContext->DicomNodeName,applicationContext->certificateToUse,true);
		log.DeveloperInfo(L"MergeTlsAdapter: Negotiation succeeded");
		return (SS_NORMAL_COMPLETION);
	}


	SS_STATUS MergeTlsAdapter::SS_Read(void*         /*SScontext*/,
									   void*         ApplicationContext,
									   char*         Buffer,
									   unsigned int  BytesToRead,
									   unsigned int* BytesRead,
									   int           Timeout)
	{
		auto applicationContext = reinterpret_cast<TLS::ApplicationContext*>(ApplicationContext);

		SS_STATUS  result;
		auto& controller = applicationContext->m_securityContext;

		// Read from the socket port
		if(!controller->tlsReceive(Buffer, BytesToRead, 0, Timeout * 1000))
		{
			int errorCode = controller->getLastError();
			applicationContext->setErrorCode(errorCode);
			switch (errorCode)
			{
			case TLS_SOCKET_TIMEOUT:
				// The connection is closed
				log.DeveloperError(L"MergeTlsAdapter: Read failed, socket timeout");
				result = SS_TIMEOUT;
				break;

			case TLS_SOCKET_ERROR:
				log.DeveloperError(L"MergeTlsAdapter: Read failed, socket error");
				result = SS_ERROR;
				break;

			case TLS_SOCKET_DISCONNECTED:
				log.DeveloperInfo(L"MergeTlsAdapter: Read failed, socket disconnect");
				result = SS_SESSION_CLOSED;
				break;

			case SEC_I_CONTEXT_EXPIRED:
				log.DeveloperInfo(L"MergeTlsAdapter: Read failed, the peer has shutdown the tls connection");
				result = SS_SESSION_CLOSED;
				break;

			case SEC_E_INCOMPLETE_MESSAGE:
				log.DeveloperError(L"MergeTlsAdapter: Read failed, receive timed out");
				result = SS_TIMEOUT;
				break;

			case SEC_E_UNSUPPORTED_FUNCTION:
				log.DeveloperError(L"MergeTlsAdapter: Read failed, socket error");
				result = SS_ERROR;
				break;

			default:
				log.DeveloperError(L"MergeTlsAdapter: Read failed, unknown error");
				result = SS_ERROR;
				break;
			}
			(*BytesRead) = 0;
			return result;
		}

		(*BytesRead) = BytesToRead;
		return SS_NORMAL_COMPLETION;
	}


	SS_STATUS MergeTlsAdapter::SS_Write(void*         /*SScontext*/,
										void*         ApplicationContext,
										char*         Buffer,
										unsigned int  BytesToWrite,
										unsigned int* BytesWritten,
										int           Timeout)
	{
		auto applicationContext = reinterpret_cast<TLS::ApplicationContext*>(ApplicationContext);
		SS_STATUS  aResult;
		auto& controller = applicationContext->m_securityContext;

		// Write to the socket port
		if(!controller->tlsSend(Buffer, BytesToWrite, 0, Timeout * 1000))
		{
			int errorCode = controller->getLastError();
			applicationContext->setErrorCode(errorCode);
			switch (errorCode)
			{
			case TLS_SOCKET_TIMEOUT:
				log.DeveloperError(L"MergeTlsAdapter: Write failed, socket timeout");
				aResult = SS_TIMEOUT;
				break;

			case TLS_SOCKET_ERROR:
				log.DeveloperError(L"MergeTlsAdapter: Write failed, socket error");
				aResult = SS_ERROR;
				break;

			default:
				log.DeveloperError(L"MergeTlsAdapter: Write failed, unknown error");
				aResult = SS_ERROR;
				break;
			}

			(*BytesWritten) = 0;
			return aResult;
		}

		(*BytesWritten) = BytesToWrite;
		return SS_NORMAL_COMPLETION;
	}


	void MergeTlsAdapter::SS_Session_Shutdown(void* /*SScontext*/, void* ApplicationContext)
	{
		log.DeveloperInfo(L"MergeTlsAdapter: Stopping Secure Session");
		auto* applicationContext = reinterpret_cast<TLS::ApplicationContext*>(ApplicationContext);

		if(applicationContext != nullptr)
		{
			auto& controller = applicationContext->m_securityContext;
			if(controller != nullptr)
			{
				log.DeveloperInfo(L"MergeTlsAdapter: Shutting down secure connection");
				controller->tlsShutdown();
				controller.reset();
			}
		}
	}

	MC_STATUS openSecureAssociation(const std::unique_ptr<ApplicationContext>& applicationContext,
									int                applicationID,
									int*               associationID,
									const std::string& remoteApplicationTitle,
									int                remoteHostPortNumber,
									const std::string& remoteHostTCPIPName,
									const std::string& serviceList)
	{
		log.DeveloperInfo(L"MergeTlsAdapter: entering openSecureAssociation");
		static SecureSocketFunctions securesocketfunctions = 
		{
			&MergeTlsAdapter::SS_Session_Start,
			&MergeTlsAdapter::SS_Read,
			&MergeTlsAdapter::SS_Write,
			&MergeTlsAdapter::SS_Session_Shutdown
		};
		// Used for Audit Trail purpose
		applicationContext->DicomNodeName = remoteApplicationTitle;
		auto status = MC_Open_Secure_Association(applicationID,
												 associationID,
												 remoteApplicationTitle.c_str(),
												 const_cast<int*>(&remoteHostPortNumber),
												 const_cast<char*>(remoteHostTCPIPName.c_str()),
												 const_cast<char*> (serviceList.c_str()),
												 &securesocketfunctions,
												 applicationContext.get());

		log.DeveloperInfo(L"MergeTlsAdapter: leaving openSecureAssociation");
		return status;
	}

	MC_STATUS processSecureAssociationRequest(const MC_SOCKET& socketToUse,
											  const std::string& ServiceList,
											  int& applicationID,
											  int& associationID,
											  const std::unique_ptr<ApplicationContext>& applicationContext)
	{
		log.DeveloperInfo(L"MergeTlsAdapter: entering processSecureAssociationRequest");
		static SecureSocketFunctions securesocketfunctions = 
		{
			&MergeTlsAdapter::SS_Session_Start,
			&MergeTlsAdapter::SS_Read,
			&MergeTlsAdapter::SS_Write,
			&MergeTlsAdapter::SS_Session_Shutdown
		};
		applicationContext->DicomNodeName = "Local Node";

		MC_STATUS status = MC_Process_Secure_Association_Request(socketToUse,
																 ServiceList.c_str(),
																 &applicationID,
																 &associationID,
																 &securesocketfunctions,
																 applicationContext.get());

		log.DeveloperInfo(L"MergeTlsAdapter: leaving processSecureAssociationRequest");
		return status;
	}

}}}