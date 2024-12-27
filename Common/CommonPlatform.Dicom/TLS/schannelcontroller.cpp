// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#ifndef SECURITY_WIN32
#define SECURITY_WIN32 //Required for SSPI.h
#endif

#ifndef CERT_CHAIN_PARA_HAS_EXTRA_FIELDS
//Must be defined before <wincrypt.h>. Used for extra fields in CERT_CHAIN_PARA structure (used in certificate verification).
#define CERT_CHAIN_PARA_HAS_EXTRA_FIELDS
#endif

#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <schannel.h>
#include <security.h>
#include <sspi.h>
#include <string>
#include <memory>
#include <sstream>

#include "SChannelController.h"

#include "include\tlsutilities.h"
#include "..\Merge\include\util.h"
#include "Log.h"

namespace CommonPlatform { namespace Dicom { namespace TLS
{
	static const unsigned int IoBufferSize = 65536;
	//TICS -CON#004	
	static const SECURITY_STATUS SecEOk						= SEC_E_OK;
	static const SECURITY_STATUS SecEIncompleteMessage		= SEC_E_INCOMPLETE_MESSAGE;
	static const SECURITY_STATUS SecESecpkgNotFound			= SEC_E_SECPKG_NOT_FOUND;
	static const SECURITY_STATUS SecIContinueNeeded			= SEC_I_CONTINUE_NEEDED;
	static const SECURITY_STATUS SecIIncompleteCredentials	= SEC_I_INCOMPLETE_CREDENTIALS;
	static const SECURITY_STATUS SecEIllegalMessage			= SEC_E_ILLEGAL_MESSAGE;
	static const SECURITY_STATUS SecICompleteNeeded			= SEC_I_COMPLETE_NEEDED;
	static const SECURITY_STATUS SecICompleteAndContinue    = SEC_I_COMPLETE_AND_CONTINUE;
	static const SECURITY_STATUS SecEInternalError			= SEC_E_INTERNAL_ERROR;
	static const SECURITY_STATUS SecEInvalidToken			= SEC_E_INVALID_TOKEN;
	static const SECURITY_STATUS SecEUnsupportedFunction	= SEC_E_UNSUPPORTED_FUNCTION;
	static const SECURITY_STATUS SecEContextExpired			= SEC_E_CONTEXT_EXPIRED;
	static const SECURITY_STATUS SecIContextExpired			= SEC_I_CONTEXT_EXPIRED;
	static const SECURITY_STATUS SecEDecryptFailure			= SEC_E_DECRYPT_FAILURE;
	static const SECURITY_STATUS SecIRenegotiate			= SEC_I_RENEGOTIATE;
	static const SECURITY_STATUS SecEMessageAltered			= SEC_E_MESSAGE_ALTERED;
	static const SECURITY_STATUS SecEBufferTooSmall			= SEC_E_BUFFER_TOO_SMALL;
	static const DWORD TrustECertSignature					= static_cast<DWORD>(TRUST_E_CERT_SIGNATURE);
	static const DWORD CertExpired							= static_cast<DWORD>(CERT_E_EXPIRED);
	static const DWORD CertEUntrustedRoot					= static_cast<DWORD>(CERT_E_UNTRUSTEDROOT);
	static const DWORD CryptENotFound						= static_cast<DWORD>(CRYPT_E_NOT_FOUND);
	static const DWORD CertFindExisting						= static_cast<DWORD>(CERT_FIND_EXISTING);
	//TICS +CON#004	
	
	SChannelController::SChannelController(const SOCKET& mergecomSocket,
										   SocketType socketType,
										   SecurityType securityType,
										   const std::wstring& certificateThumbPrint,
										   CertStoreType personalStoreType,
										   CertStoreType trustStoreType,
										   bool isSelfSignedCertificateAllowed,
										   bool isCertificateEkuToBeChecked,
										   bool isCertificateFromTrustedCaTrusted,
										   int timeoutForNegotiation) :
		allowSelfSignedCertificate(isSelfSignedCertificateAllowed),
		checkCertificateEKU(isCertificateEkuToBeChecked),
		trustCertificatesFromTrustedCa(isCertificateFromTrustedCaTrusted),
		negotiationTimeout(timeoutForNegotiation),
		m_log(::CommonPlatform::Logger(LoggerType::Platform)),
		m_errorCode(TLS_OK),
		m_socketType(socketType),
		m_socket(mergecomSocket),
		m_certificateToUse(certificateThumbPrint),
		m_personalStoreType(personalStoreType),
		m_trustStoreType(trustStoreType),
		m_myCertificateContext(loadCertificate(certificateThumbPrint, personalStoreType)),
		m_securityType(securityType),
		m_myCredentials(new Credentials(tlsCreateCredentials())),
		m_securityContext(new SecurityContext()),
		m_theSendBuffer(nullptr),
		m_theReceiveBuffer(nullptr),
		m_theOverflowBuffer(nullptr),
		m_theHeaderSize(0),
		m_theDataSize(0),
		m_theTrailerSize(0),
		m_theBufferSize(0),
		m_theUsedRecvBufferSize(0),
		m_theUsedOverflowBufferSize(0),
		m_extraDataSize(0),
		m_isNegotiated(false)
	{
	}

	SChannelController::~SChannelController()
	{
		if(m_isNegotiated)
		{
			tlsShutdown();
		}
	}

	std::unique_ptr<CredHandle> SChannelController::tlsCreateCredentials() const
	{
		static int counter = 0;
		m_log.DeveloperInfo(L"SChannelController: Creating new credentials with %d", counter);
		std::unique_ptr<CredHandle> credentials(new CredHandle);
		SecInvalidateHandle(credentials.get());

		SCHANNEL_CRED schannelCred = {};
		// Build Schannel credential structure. Currently, only the protocol to be
		// used is specified (and optionally the certificate, of course).

		//Passing in own certificate.
		schannelCred.dwVersion = SCHANNEL_CRED_VERSION;
		schannelCred.cCreds = 1;
		schannelCred.paCred = const_cast< PCCERT_CONTEXT*>(&m_myCertificateContext.handle);

		//Choosing protocols that can be used.
		if(m_socketType == SocketType::CLIENT)
		{
			schannelCred.grbitEnabledProtocols = static_cast<DWORD> (SP_PROT_SSL3_CLIENT | SP_PROT_TLS1_0_CLIENT | SP_PROT_TLS1_1_CLIENT | SP_PROT_TLS1_2_CLIENT);
		}
		else
		{
			schannelCred.grbitEnabledProtocols = static_cast<DWORD> (SP_PROT_SSL3_SERVER | SP_PROT_TLS1_0_SERVER | SP_PROT_TLS1_1_SERVER | SP_PROT_TLS1_2_SERVER);
		}

		// To force using NULL cipher suite, i.e. no encryption, only signing 
		if(m_securityType == SecurityType::AUTHENTICATION)
		{
			schannelCred.dwMinimumCipherStrength = static_cast<DWORD>(-1);   //Enables the use of null cipher suite.
			schannelCred.dwMaximumCipherStrength = static_cast<DWORD>(-1);   //Forces to use ONLY null cipher suite.
		}

		//The below flags are only for clients.
		if(m_socketType == SocketType::CLIENT)
		{
			//Prevent Schannel from attempting to automatically choose and supply a certificate chain for client authentication.
			//Functionally same as ISC_REQ_USE_SUPPLIED_CREDS
			schannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS; 
			//Prevent SChannel from automatically verifying the server certificate. 
			//MSDN says Schannel does not validate client's certificates. So no need to specify this for servers.
			//Functionally same as ISC_REQ_MANUAL_CRED_VALIDATION.
			schannelCred.dwFlags |= SCH_CRED_MANUAL_CRED_VALIDATION; 
			//Prevent Schannel from comparing the supplied target name with the subject name in the produced server certificate.
			schannelCred.dwFlags |= SCH_CRED_NO_SERVERNAME_CHECK;
		}

		// Create an SSPI credential.
		SECURITY_STATUS status = AcquireCredentialsHandle(
			nullptr,                                                                           // Name of principal
			UNISP_NAME,                                                                        // Name of package, MSDN asks to use UNISP when using SChannel 
			(m_socketType == SocketType::SERVER ? SECPKG_CRED_INBOUND : SECPKG_CRED_OUTBOUND), // Flags indicating use
			nullptr,                                                                           // Pointer to logon ID
			&schannelCred,                                                                     // Package specific data
			nullptr,                                                                           // Pointer to GetKey() func
			nullptr,                                                                           // Value to pass to GetKey()
			credentials.get(),                                                                 // (out) Cred Handle
			nullptr                                                                            // (out) Lifetime (optional)
			);

		if (status != SecEOk)
		{
			m_log.DeveloperError(L"SChannelController: Failed to acquire credentials (%s), 0x%X (GLE 0x%X)", setLastError(TLS_ERR_CRED_ACQUIRE).c_str(), status, GetLastError());
			throw std::exception("Failed to Acquire Credentials");
		}

		return std::move(credentials);
	}

	void SChannelController::CheckPeerCertificateTimeValidity()
	{
		auto res = CertVerifyTimeValidity(nullptr, m_myCertificateContext.handle->pCertInfo);
		if (res < 0)
		{
			auto certName = CertificateManager::getCertificateName(m_myCertificateContext.handle);
			SYSTEMTIME sysTime;
			FileTimeToSystemTime(&m_myCertificateContext.handle->pCertInfo->NotBefore, &sysTime);
			m_log.DeveloperWarning(L"SChannelController: Starting negotiation with the certificate \"%s\", which is not valid yet (NotBefore %04d-%02d-%02d %02d:%02d:%02d)",
				certName.c_str(), sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		}
		else if (res > 0)
		{
			auto certName = CertificateManager::getCertificateName(m_myCertificateContext.handle);
			SYSTEMTIME sysTime;
			FileTimeToSystemTime(&m_myCertificateContext.handle->pCertInfo->NotAfter, &sysTime);
			m_log.DeveloperWarning(L"SChannelController: Starting negotiation with the certificate \"%s\", which is expired (NotAfter %04d-%02d-%02d %02d:%02d:%02d)",
				certName.c_str(), sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		}
	}

	bool SChannelController::tlsNegotiate()
	{
		CheckPeerCertificateTimeValidity();

		m_isNegotiated = false;
		if (m_socketType == SocketType::SERVER)
		{
			m_isNegotiated = tlsServerNegotiate(true, true);
		} 
		else
		{
			m_isNegotiated = tlsClientNegotiate();
		}

		if (m_isNegotiated) 
		{
			traceConnectionInfo();
			return tlsVerifyCertificate();
		}
		return false;
	}

	bool SChannelController::tlsServerNegotiate(bool newContext, bool doInitialRead)
	{
		m_log.DeveloperInfo(L"SChannelController: Starting server negotiation");

		// Buffer for negotiation
		char			ioBuffer [IoBufferSize]; 

		TimeStamp       expiry;
		SECURITY_STATUS status;
		SecBufferDesc   inBuffer;
		SecBufferDesc   outBuffer;
		SecBuffer       inBuffers[2];
		SecBuffer       outBuffers[1];
		int             byteCount = 0;
		bool            doRead = doInitialRead;
		bool            initialContext = newContext;
		DWORD           sspiInFlags, sspiOutFlags;
		unsigned long   totalBytesReadIntoIoBuffer = 0;

		status = SecESecpkgNotFound;   //default error if we run out of packages

		sspiInFlags = static_cast<DWORD> (  ASC_REQ_SEQUENCE_DETECT |
											ASC_REQ_REPLAY_DETECT |
											ASC_REQ_CONFIDENTIALITY |
											ASC_REQ_EXTENDED_ERROR |
											ASC_REQ_ALLOCATE_MEMORY |
											ASC_REQ_STREAM);

		//Request for client certificate
		sspiInFlags |= ASC_REQ_MUTUAL_AUTH;


		//  Set outBuffer for InitializeSecurityContext call
		outBuffer.cBuffers = 1;
		outBuffer.pBuffers = outBuffers;
		outBuffer.ulVersion = SECBUFFER_VERSION;

		// Check to see if we've done Client Authentication with
		// this Web Server before we handshake.  If we have, we'll have a Client
		// Certificate that we can use to make this secure connection.
		status = SecIContinueNeeded;

		while (
			(status == SecIContinueNeeded) ||
			(status == SecEIncompleteMessage) ||
			(status == SecIIncompleteCredentials)
			) 
		{
			if ((0 == totalBytesReadIntoIoBuffer) || (status == SecEIncompleteMessage))
			{
				if (doRead)
				{
					byteCount = m_socket.receive(ioBuffer + totalBytesReadIntoIoBuffer, IoBufferSize, 0, negotiationTimeout);
					if (byteCount == 0)
					{
						m_log.DeveloperError(L"SChannelController: error in receive (%s)", setLastError(TLS_ERR_RECV).c_str());
						return false;
					}
					else 
					{
						totalBytesReadIntoIoBuffer += static_cast<unsigned long>(byteCount);
					}
				}
				else 
				{
					doRead = true;
				}
			}

			// inBuffers[1] is for getting extra data that SSPI/SCHANNEL doesn't
			// process on this run around the loop.
			inBuffers[0].pvBuffer = ioBuffer;
			inBuffers[0].cbBuffer = totalBytesReadIntoIoBuffer;
			inBuffers[0].BufferType = SECBUFFER_TOKEN;

			inBuffers[1].pvBuffer   = nullptr;
			inBuffers[1].cbBuffer   = 0;
			inBuffers[1].BufferType = SECBUFFER_EMPTY;

			inBuffer.cBuffers        = 2;
			inBuffer.pBuffers        = inBuffers;
			inBuffer.ulVersion       = SECBUFFER_VERSION;

			// Initialize these so if we fail, pvBuffer contains NULL,
			// so we don't try to free random garbage at the quit
			outBuffers[0].pvBuffer   = nullptr;
			outBuffers[0].BufferType = SECBUFFER_TOKEN;
			outBuffers[0].cbBuffer   = 0;

			status = AcceptSecurityContext(
				(m_myCredentials->handle),
				(initialContext ? nullptr : &m_securityContext->handle),
				&inBuffer,
				sspiInFlags,
				SECURITY_NATIVE_DREP,
				(initialContext ? &m_securityContext->handle : nullptr),
				&outBuffer,
				&sspiOutFlags,
				&expiry);

			initialContext = false;

			if (
				(status == SecEOk) ||
				(status == SecIContinueNeeded) ||
				( FAILED(status) && (0 != (sspiOutFlags & ISC_RET_EXTENDED_ERROR)) )
				)
			{
				if (outBuffers[0].cbBuffer != 0 && outBuffers[0].pvBuffer != nullptr)
				{
					// Send response to server if there is one
					if(!m_socket.send(static_cast<char*>(outBuffers[0].pvBuffer), outBuffers[0].cbBuffer, 0, negotiationTimeout))
					{
						//Send failed
						m_log.DeveloperError(L"SChannelController: error in sending (%s)", setLastError(TLS_ERR_SEND).c_str());
						return false;
					}
					FreeContextBuffer(outBuffers[0].pvBuffer);
					outBuffers[0].pvBuffer = nullptr;
				}
			}

			if (status == SecEOk)
			{
				if (inBuffers[1].BufferType == SECBUFFER_EXTRA) 
				{
					//Copy extra data for decrypting later
					m_log.DeveloperWarning(L"SChannelController: Extra data in server handshake (%u bytes)", inBuffers[1].cbBuffer);
					memmove(ioBuffer,
							ioBuffer + (totalBytesReadIntoIoBuffer - inBuffers[1].cbBuffer),
							inBuffers[1].cbBuffer);
					totalBytesReadIntoIoBuffer = inBuffers[1].cbBuffer;
				} 
				else 
				{
					totalBytesReadIntoIoBuffer = 0;
				}
				return true;
			} 
			else if (FAILED(status) && (status != SecEIncompleteMessage)) 
			{
				if(status == SecEIllegalMessage)
				{
					//Client isn't talking in the required protocol
					m_log.DeveloperError(L"SChannelController: invalid message received (%s)",
										 setLastError(TLS_ERR_NEGOTIATE_INVALID_MESSAGE).c_str());
				}
				else
				{
					// Accept Security Context Failed
					m_log.DeveloperError(L"SChannelController: no acceptable security context (%s), status: %d",
										 setLastError(TLS_ERR_NEGOTIATE_ACCEPT_SECCONTEXT).c_str(), status);
				}
				return false;
			}

			if ((status != SecEIncompleteMessage) && (status != SecIIncompleteCredentials)) 
			{
				if (inBuffers[1].BufferType == SECBUFFER_EXTRA) 
				{
					m_log.DeveloperWarning(L"SChannelController: extra data for server handshake (%u bytes)", inBuffers[1].cbBuffer);
					memmove(
						ioBuffer,
						ioBuffer + (totalBytesReadIntoIoBuffer - inBuffers[1].cbBuffer),
						inBuffers[1].cbBuffer
						);
					totalBytesReadIntoIoBuffer = inBuffers[1].cbBuffer;
				} 
				else 
				{
					// Prepare for next receive
					totalBytesReadIntoIoBuffer = 0;
				}
			}
		}

		// We should not come here
		m_log.DeveloperError(L"Error accepting security context (%s), status: %d",
							 setLastError(TLS_ERR_NEGOTIATE_INTERNAL).c_str(), status);
		return false;
	}


	bool SChannelController::tlsClientNegotiate()
	{
		m_log.DeveloperInfo(L"SChannelController: Starting client negotiation");
		SecBufferDesc   outBuffer;
		SecBuffer       outBuffers[1];
		DWORD           sspiInFlags;
		DWORD           sspiOutFlags;
		TimeStamp       tsExpiry;
		SECURITY_STATUS status;

		sspiInFlags = static_cast<DWORD> ( ISC_REQ_SEQUENCE_DETECT |
										   ISC_REQ_REPLAY_DETECT   |
										   ISC_REQ_CONFIDENTIALITY |
										   ISC_RET_EXTENDED_ERROR  |
										   ISC_REQ_ALLOCATE_MEMORY |
										   ISC_REQ_STREAM);

		//Initiate a ClientHello message and generate a token.

		//Buffer to hold token
		outBuffers[0].pvBuffer = nullptr;
		outBuffers[0].BufferType = SECBUFFER_TOKEN;
		outBuffers[0].cbBuffer = 0;

		//Set details of output buffers that you are providing
		outBuffer.cBuffers = 1;
		outBuffer.pBuffers = outBuffers;
		outBuffer.ulVersion = SECBUFFER_VERSION;

		status = InitializeSecurityContextA(
			m_myCredentials->handle,							// client cred handle
			nullptr,											// current context handle
			nullptr,											// server name, used when SChannel verifies the certificate; doesnt make sense in our context
			sspiInFlags,										// required attrs
			0,													// must be zero
			SECURITY_NATIVE_DREP,								// byte ordering
			nullptr,											// incoming token
			0,													// must be zero
			&m_securityContext->handle,							// new context handle
			&outBuffer,											// outgoing token
			&sspiOutFlags,										// resulting attrs
			&tsExpiry											// context lifetime
			);

		if ((status == SecICompleteNeeded) || (status == SecICompleteAndContinue))
		{
			// The InitializeSecurityContext call returned SecICompleteNeeded 
			// or SecICompleteAndContinue, so the client must call CompleteAuthToken.
			SECURITY_STATUS retVal = CompleteAuthToken(&m_securityContext->handle, &outBuffer);
			if(SecEOk != retVal) 
			{
				//CompleteAuthToken failed
				// Free output buffer.
				FreeContextBuffer(outBuffers[0].pvBuffer);
				outBuffers[0].pvBuffer = nullptr;
				m_log.DeveloperError(L"SChannelController: error when completing the auth token (%s) error %d",
									 setLastError(TLS_ERR_NEGOTIATE_COMPLETE_AUTH_TOKEN).c_str(), retVal);
				return false;
			}
			else
			{
				//CompleteAuthToken succeeded
				if(status == SecICompleteNeeded) 
				{	// Only CompleteAuthToken was required and it succeeded.
					// Free output buffer.
					FreeContextBuffer(outBuffers[0].pvBuffer);
					outBuffers[0].pvBuffer = nullptr;
					return true;
				}
			}
		}

		else if (status != SecIContinueNeeded) 
		{
			m_log.DeveloperError(L"SChannelController: error when negotiating the security context (%s), status %d",
								 setLastError(TLS_ERR_NEGOTIATE_INITIALIZE_SECCONTEXT).c_str(), status);
			return false;
		}

		// Send response to server if there is one.
		if (!SendBuffer(outBuffers))
		{
			return false;
		}

		return tlsClientHandshakeLoop(true);
	}


	//#TODO Rewrite completely
	bool SChannelController::tlsClientHandshakeLoop(bool fDoInitialRead)
	{
		m_log.DeveloperInfo(L"SChannelController: Entering client handshake loop");

		DWORD             res = TLS_OK;
		SecBufferDesc     inBuffer;
		SecBuffer         inBuffers[2];
		SecBufferDesc     outBuffer;
		SecBuffer         outBuffers[1];
		DWORD             sspiInFlags;
		DWORD             sspiOutFlags;
		TimeStamp         expiry;
		SECURITY_STATUS   status;
		int               byteCount;
		std::vector<char> ioBuffer(IoBufferSize);
		unsigned long     totalBytesReadIntoIoBuffer;
		bool              doRead;

		sspiInFlags = static_cast<DWORD> ( ISC_REQ_SEQUENCE_DETECT |
										   ISC_REQ_REPLAY_DETECT   |
										   ISC_REQ_CONFIDENTIALITY |
										   ISC_RET_EXTENDED_ERROR  |
										   ISC_REQ_ALLOCATE_MEMORY |
										   ISC_REQ_STREAM);

		totalBytesReadIntoIoBuffer = 0;
		doRead = fDoInitialRead;

		// Loop until the handshake is finished or an error occurs.
		status = SecIContinueNeeded;

		while (
			(status == SecIContinueNeeded) ||
			(status == SecEIncompleteMessage) ||
			(status == SecIIncompleteCredentials)
			) 
		{
			// Read data from server.
			if ((0 == totalBytesReadIntoIoBuffer) || (status == SecEIncompleteMessage)) 
			{
				if (doRead) 
				{
					byteCount = m_socket.receive(ioBuffer.data() + totalBytesReadIntoIoBuffer, IoBufferSize - totalBytesReadIntoIoBuffer, 0, negotiationTimeout);

					if (byteCount == 0) 
					{
						// Error reading data from server
						status = SecEInternalError;
						res = TLS_ERR_RECV;
						m_log.DeveloperError(L"SChannelController: error receiving data (%s)", setLastError(TLS_ERR_RECV).c_str());
						break;
					}
					totalBytesReadIntoIoBuffer += static_cast<unsigned long>(byteCount);
				} 
				else 
				{
					doRead = true;
				}
			}

			// Set up the input buffers. Buffer 0 is used to pass in data
			// received from the server. Schannel will consume some or all
			// of this. Leftover data (if any) will be placed in buffer 1 and
			// given a buffer type of SECBUFFER_EXTRA.
			inBuffers[0].pvBuffer   = &ioBuffer.at(0);
			inBuffers[0].cbBuffer   = totalBytesReadIntoIoBuffer;
			inBuffers[0].BufferType = SECBUFFER_TOKEN;

			inBuffers[1].pvBuffer   = nullptr;
			inBuffers[1].cbBuffer   = 0;
			inBuffers[1].BufferType = SECBUFFER_EMPTY;

			inBuffer.cBuffers       = 2;
			inBuffer.pBuffers       = inBuffers;
			inBuffer.ulVersion      = SECBUFFER_VERSION;

			// Set up the output buffers. These are initialized to NULL so as to
			// make it less likely we'll attempt to free random garbage later.
			outBuffers[0].pvBuffer  = nullptr;
			outBuffers[0].BufferType= SECBUFFER_TOKEN;
			outBuffers[0].cbBuffer  = 0;

			outBuffer.cBuffers      = 1;
			outBuffer.pBuffers      = outBuffers;
			outBuffer.ulVersion     = SECBUFFER_VERSION;

			status = InitializeSecurityContextA(
				m_myCredentials->handle,							// client cred handle
				&m_securityContext->handle,							// current context handle
				nullptr,											// server principal
				sspiInFlags,										// required attrs
				0,													// must be zero
				SECURITY_NATIVE_DREP,								// byte ordering
				&inBuffer,											// incoming token
				0,													// must be zero
				nullptr,											// new context handle  //VB Try setting to &hSecurityContext. MSDN ambiguously says NULL or prev context. 
				&outBuffer,											// outgoing token
				&sspiOutFlags,										// resulting attrs
				&expiry												// context lifetime
				);

			if ((status == SecICompleteNeeded) || (status == SecICompleteAndContinue))
			{
				// The InitializeSecurityContext call returned SecICompleteNeeded 
				// or SecICompleteAndContinue so the client must call CompleteAuthToken
				if(SecEOk != CompleteAuthToken (&m_securityContext->handle, &outBuffer)) 
				{
					// CompleteAuthToken failed - return an error
					res = TLS_ERR_NEGOTIATE_COMPLETE_AUTH_TOKEN;
					m_log.DeveloperError(L"SChannelController: error completing auth token (%s)", setLastError(TLS_ERR_NEGOTIATE_COMPLETE_AUTH_TOKEN).c_str());
					FreeContextBuffer(outBuffers[0].pvBuffer);
					outBuffers[0].pvBuffer = nullptr;
					return false;
				}
				else
				{
					if(status == SecICompleteNeeded) 
					{
						// Only CompleteAuthToken was required and it succeeded
						res = TLS_OK;
						// Free output buffer.
						FreeContextBuffer(outBuffers[0].pvBuffer);
						outBuffers[0].pvBuffer = nullptr;
						return true;
					}
					else {
						// CompleteAuthToken called so now the same procedure as for SecIContinueNeeded
						status = SecIContinueNeeded;
					}
				}
			}

			// If InitializeSecurityContext was successful (or if the error was
			// one of the special extended ones), send the contents of the output
			// buffer to the server.
			if (!SendBuffer(outBuffers))
			{
				return false;
			}

			// If InitializeSecurityContext returned SecEIncompleteMessage,
			// then we need to read more data from the server and try again.
			if (status == SecEIncompleteMessage) 
			{
				continue;
			}

			// If InitializeSecurityContext returned SecEOk, then the
			// handshake completed successfully.
			if (status == SecEOk) 
			{
				// If the "extra" buffer contains data, this is encrypted
				// application protocol layer stuff. It needs to be saved.
				// The application layer will later decrypt it with DecryptMessage.
				if (inBuffers[1].BufferType == SECBUFFER_EXTRA) 
				{
					//Application layer data was received in handshake, save it for decryption later when receive is called.
					m_log.DeveloperWarning(L"SChannelController: Received extra data (%u bytes)", inBuffers[1].cbBuffer);
					m_extraDataSize = inBuffers[1].cbBuffer; 
					if(!areBuffersInitialized())
					{
						if(!initializeBuffers())
						{
							return false;
						}
					}
					//#HACK to put extra data in recieveBuffer for decryption later. Could decrease decryption efficiency as recieveBuffer size will not be equal to packet size.
					memcpy((*m_theReceiveBuffer).data(),
						   ioBuffer.data() + (totalBytesReadIntoIoBuffer - inBuffers[1].cbBuffer),
						   inBuffers[1].cbBuffer);
					FreeContextBuffer(inBuffers[1].pvBuffer);
					inBuffers[1].pvBuffer = nullptr;
					inBuffers[1].cbBuffer = 0;
				}

				// Bail out to quit
				break;
			}

			// Check for fatal error, returned by InitializeSecurityContext
			if (FAILED(status)) 
			{
				if(status == SecEInvalidToken || status == SecEIllegalMessage)
				{
					//Wrongly formatted message.
					res = TLS_ERR_NEGOTIATE_INVALID_MESSAGE;
					m_log.DeveloperError(L"SChannelController: received an invalid message (%s), status %d",
										 setLastError(TLS_ERR_NEGOTIATE_INVALID_MESSAGE).c_str(), status);
				}
				else
				{
					//Unknown error code.
					res = TLS_ERR_NEGOTIATE_INITIALIZE_SECCONTEXT;
					m_log.DeveloperError(L"SChannelController: error initializing security context (%s), status %d",
										 setLastError(TLS_ERR_NEGOTIATE_INITIALIZE_SECCONTEXT).c_str(), status);
				}
				break;
			}

			// If InitializeSecurityContext returned SecIIncompleteCredentials,
			// then the server just requested client authentication.
			if (status == SecIIncompleteCredentials)
			{
				// Go around again.
				doRead = false;
				status = SecIContinueNeeded;
				continue;
			}

			// Copy any leftover data from the "extra" buffer, and go around
			// again.
			if (inBuffers[1].BufferType == SECBUFFER_EXTRA)
			{
				memmove(ioBuffer.data(),
						ioBuffer.data() + (totalBytesReadIntoIoBuffer - inBuffers[1].cbBuffer),
						inBuffers[1].cbBuffer);
				totalBytesReadIntoIoBuffer = inBuffers[1].cbBuffer;
			}
			else
			{
				totalBytesReadIntoIoBuffer = 0;
			}
		}

		// Delete the security context in the case of a fatal error.
		if (FAILED(status))
		{
			m_log.DeveloperError(L"SChannelController: fatal error %d", status);
		}

		return res == TLS_OK;
	}

	bool SChannelController::tlsShutdown()
	{
		bool isSuccess = true;
		if(m_isNegotiated)
		{
			m_log.DeveloperInfo(L"SChannelController: starting graceful shutdown");
			m_isNegotiated = false;

			DWORD           controlTokenType;
			char*           message;
			DWORD           messageSize;
			SecBufferDesc   outBuffer;
			SecBuffer       outBuffers[1];
			DWORD           sspiInFlags;
			DWORD           sspiOutFlags;
			TimeStamp       expiry;
			long           status;

			// Notify schannel that we are about to close the connection.
			controlTokenType = SCHANNEL_SHUTDOWN;

			outBuffers[0].pvBuffer   = &controlTokenType;
			outBuffers[0].BufferType = SECBUFFER_TOKEN;
			outBuffers[0].cbBuffer   = sizeof(controlTokenType);

			outBuffer.cBuffers  = 1;
			outBuffer.pBuffers  = outBuffers;
			outBuffer.ulVersion = SECBUFFER_VERSION;

			status = ApplyControlToken(&m_securityContext->handle, &outBuffer);

			if (FAILED(status)) 
			{
				m_log.DeveloperError(L"SChannelController: failed to apply control token (%s), status %u",
									 setLastError(TLS_ERR_SHUTDOWN_APPLY_CONTROL_TOKEN).c_str(), status);
				return false;
			}

			// Build an TLS close notify message.
			sspiInFlags = static_cast<DWORD> ( ASC_REQ_SEQUENCE_DETECT |
											   ASC_REQ_REPLAY_DETECT   |
											   ASC_REQ_CONFIDENTIALITY |
											   ASC_REQ_EXTENDED_ERROR  |
											   ASC_REQ_ALLOCATE_MEMORY |
											   ASC_REQ_STREAM);

			outBuffers[0].pvBuffer   = nullptr;
			outBuffers[0].BufferType = SECBUFFER_TOKEN;
			outBuffers[0].cbBuffer   = 0;

			outBuffer.cBuffers  = 1;
			outBuffer.pBuffers  = outBuffers;
			outBuffer.ulVersion = SECBUFFER_VERSION;

			status = AcceptSecurityContext(m_myCredentials->handle,
										   &m_securityContext->handle,
										   nullptr,
										   sspiInFlags,
										   SECURITY_NATIVE_DREP,
										   nullptr,
										   &outBuffer,
										   &sspiOutFlags,
										   &expiry);

			if (FAILED(status))
			{
				m_log.DeveloperError(L"SChannelController: error accepting security context during shutdown (%s), status %u",
									 setLastError(TLS_ERR_SHUTDOWN_ACCEPT_SECCONTEXT).c_str(), status);
				return false;
			}

			message = reinterpret_cast<char*>(outBuffers[0].pvBuffer);
			messageSize = outBuffers[0].cbBuffer;

			// Send the close notify message to the client.
			if ((message != nullptr) && (messageSize != 0)) 
			{
				if (!m_socket.send(message, messageSize, 0, negotiationTimeout)) 
				{
					m_log.DeveloperError(L"SChannelController: error during send (%s)", setLastError(TLS_ERR_SEND).c_str());
					return false;
				}
			}
			isSuccess = true;
		}
		else
		{
			m_log.DeveloperWarning(L"SChannelController: connection is not negotiated, no graceful shutdown");
		}

		m_securityContext.reset(); // MSDN says always release securityContext after shutdown. Explicitly deleting as a precaution
		m_myCredentials.reset();   // Clear credentials after securityContext. Explicitly deleting as a precaution.
		return isSuccess;
	}


	std::wstring SChannelController::setLastError(unsigned int errorcode) const
	{
		m_errorCode = errorcode;
		std::wstring errorMessage = L"";

		switch(m_errorCode)
		{
		case TLS_ERR_CERTIFICATE_NOSUBJECT:
			errorMessage += L"TLS_ERR_CERTIFICATE_NOSUBJECT: Supplied certificate name is empty.";
			break;
		case TLS_ERR_OWN_CERTIFICATE_NOT_FOUND:
			errorMessage += L"TLS_ERR_OWN_CERTIFICATE_NOT_FOUND: Couldn't load own certificate.";
			break;
		case TLS_ERR_CRED_ACQUIRE:
			errorMessage += L"TLS_ERR_CRED_ACQUIRE: Could not acquire credentials handle.";
			break;
		case TLS_ERR_NEGOTIATE_INVALID_MESSAGE:
			errorMessage += L"TLS_ERR_NEGOTIATE_INVALID_MESSAGE: Peer isn't using TLS or message is in wrong format.";
			break;
		case TLS_ERR_NEGOTIATE_ACCEPT_SECCONTEXT:
			errorMessage += L"TLS_ERR_NEGOTIATE_ACCEPT_SECCONTEXT: AcceptSecurityContext failed.";
			break;
		case TLS_ERR_NEGOTIATE_INTERNAL:
			errorMessage += L"TLS_ERR_RECV: Unexpected failure in negotiation.";
			break;
		case TLS_ERR_NEGOTIATE_COMPLETE_AUTH_TOKEN:
			errorMessage += L"TLS_ERR_NEGOTIATE_COMPLETE_AUTH_TOKEN.";
			break;
		case TLS_ERR_OUT_OF_MEM:
			errorMessage += L"TLS_ERR_OUT_OF_MEM.";
			break;
		case TLS_ERR_SHUTDOWN_APPLY_CONTROL_TOKEN:
			errorMessage += L"TLS_ERR_SHUTDOWN_APPLY_CONTROL_TOKEN.";
			break;
		case TLS_ERR_SHUTDOWN_ACCEPT_SECCONTEXT:
			errorMessage += L"TLS_ERR_SHUTDOWN_ACCEPT_SECCONTEXT.";
			break;
		case TLS_ERR_AUTH_PEER_CERT_NOT_FOUND:
			errorMessage += L"TLS_ERR_AUTH_PEER_CERT_NOT_FOUND: Certificate was not found.";
			break;
		case TLS_ERR_AUTH_CERT_NOT_IN_TRUST:
			errorMessage += L"TLS_ERR_AUTH_CERT_NOT_IN_TRUST: Certificate was not found in trust store.";
			break;
		case TLS_ERR_AUTH_SELFSIG_CERT_NOT_ALLOWED:
			errorMessage += L"TLS_ERR_AUTH_SELFSIG_CERT_NOT_ALLOWED: Self signed certificates aren't allowed.";
			break;
		case TLS_ERR_CIPHER_QUERY_CONTEXT:
			errorMessage += L"TLS_ERR_CIPHER_QUERY_CONTEXT: QueryContextAttrributes returned an error.";
			break;
		case TLS_ERR_CERT_EXPIRED:
			errorMessage += L"TLS_ERR_CERT_EXPIRED: Peer has produced an expired or a not-yet-valid certificate.";
			break;
		case TLS_ERR_SEND:
			errorMessage += L"TLS_ERR_SEND: Sending data on socket failed.";
			setLastError(m_socket.getLastError()); //Get and set actual error from socket.
			break;
		case TLS_ERR_RECV:
			errorMessage += L"TLS_ERR_RECV: Socket error: recv failed.";
			setLastError(m_socket.getLastError()); //Get and set actual error from socket.
			break;
		case SecEIncompleteMessage:
			errorMessage += L"SecEIncompleteMessage: Socket timed out.";
			break;
		case SecEUnsupportedFunction:
			errorMessage += L"SecEUnsupportedFunction: Socket error.";
			break;
		case SecEContextExpired: //A properly written application should not receive this error. Dont call encrptyMessage after context is dead
			errorMessage += L"SecEContextExpired: Connection closed. The application is referencing a context that has already been closed.";
			break;
		case SecIContextExpired: //Called by decryptMessage when peer has shutdown the session.
			errorMessage += L"SecIContexrExpired: Session has been shutdown by peer.";
			break; 
		case SecEDecryptFailure: //You really shouldnt get this.
			errorMessage += L"SecEDecryptFailure: Failed to decrypt the received message.";
			break;

		//Socket-level errors. Tracing for it will be done by the socket class, so return without tracing.
		case TLS_SOCKET_TIMEOUT:
		case TLS_SOCKET_ERROR:
		case TLS_SOCKET_DISCONNECTED:
			break;

		default:
			errorMessage = L"UNEXPECTED ERROR! ErrorCode: " + std::to_wstring(m_errorCode);
			break;
		}

		return errorMessage;
	}


	unsigned int SChannelController::getLastError() const
	{
		return m_errorCode;
	}

	PCCERT_CONTEXT SChannelController::loadCertificate(const std::wstring& certificateThumbPrint, const CertStoreType& storeToUse) const
	{
		if (certificateThumbPrint.empty()) 
		{
			m_log.DeveloperError(L"SChannelController: empty certificate name found (%s)", setLastError(TLS_ERR_CERTIFICATE_NOSUBJECT).c_str());
			throw std::exception("Trying to load a certificate with no name.");
		}

		CryptHashBlob hash;
		CryptHashBlobFromThumbprint(certificateThumbPrint, hash);

		// Find certificate.
		// My own certificate must be present in the Personal ("My") store.
		CertificateStore myCertStore ( static_cast<LPCSTR>(CERT_STORE_PROV_SYSTEM), static_cast<DWORD>(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING), 0, storeToUse, MyCertificatesStore.c_str());
		PCCERT_CONTEXT certificateContext = CertFindCertificateInStore(
			myCertStore.handle,
			static_cast<DWORD>( X509_ASN_ENCODING | PKCS_7_ASN_ENCODING),
			0,
			static_cast<DWORD>(CERT_FIND_SHA1_HASH),
			&hash,
			nullptr
			);

		if (certificateContext == nullptr)
		{
			m_log.DeveloperError(L"SChannelController: certificate not found (%s), 0x%X", setLastError(TLS_ERR_OWN_CERTIFICATE_NOT_FOUND).c_str(), GetLastError());
			throw std::exception("Own certificate could not be found");
		}

		return certificateContext;
	}	


	bool SChannelController::isCertificateSelfSigned(const PCCERT_CONTEXT& pCertContext) const
	{
		CERT_NAME_BLOB  issuer = pCertContext->pCertInfo->Issuer;
		CERT_NAME_BLOB  subject = pCertContext->pCertInfo->Subject;

		if (issuer.cbData != subject.cbData) 
		{
			return false;
		}
		return (0 == memcmp(issuer.pbData, subject.pbData, issuer.cbData));
	}


	//  Check whether the client certificate is correct, and thus usable.
	//  If not, error TLS_ERR_AUTHENTICATION is returned, and m_errorCode will
	//  be filled with a sub-error that better explains what is wrong.
	bool SChannelController::tlsVerifyCertificate() const
	{
		PCCERT_CONTEXT pRemoteCertContext = nullptr;
		SECURITY_STATUS res = SecEOk;

		// Read the client certificate
		res = QueryContextAttributes((&m_securityContext->handle), static_cast<DWORD>(SECPKG_ATTR_REMOTE_CERT_CONTEXT), (PVOID)&pRemoteCertContext);
		if ( SecEOk != res)
		{
			m_log.DeveloperError(L"SChannelController: no peer certificate found (%s), error %d",
								 setLastError(TLS_ERR_AUTH_PEER_CERT_NOT_FOUND).c_str(), res);
			return false;
		}
		if (pRemoteCertContext == nullptr) 
		{
			m_log.DeveloperError(L"SChannelController: received a NULL certificatecontext from the peer (%s)", setLastError(TLS_ERR_AUTH_PEER_CERT_NOT_FOUND).c_str());
			return false;
		}
		//Wrapping the cert context for RAII
		CertificateContext remoteCertContext(pRemoteCertContext);

		//Get peer's certificate name (just for logging)
		wchar_t buf[1024];
		CertNameToStr(remoteCertContext.handle->dwCertEncodingType,
					  &remoteCertContext.handle->pCertInfo->Subject,
					  CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
					  buf,
					  _countof(buf));
		m_log.DeveloperInfo(L"SChannelController: Received certificate from peer; %s", buf);

		//Check if the certificate is self-Signed
		bool isCertificateSelfSigned = this->isCertificateSelfSigned(remoteCertContext.handle);

		if (!allowSelfSignedCertificate) 
		{
			if (isCertificateSelfSigned)
			{
				m_log.DeveloperError(L"SChannelController: peer certificate is selfsigned (%s)", setLastError(TLS_ERR_AUTH_SELFSIG_CERT_NOT_ALLOWED).c_str());
				return false;
			}
		}

		//Check if the certificate is trusted
		if(trustCertificatesFromTrustedCa)
		{
			//----About the certificate validation decision----
			//If trustCertificatesFromTrustedCa is true, we build a certificate chain and check if the CA is trusted. This method has a loophole as any certificate from a trusted CA will be trusted.
			//Thus, if Verisign is a trusted CA, then ANY device which uses a Verisign certificate can communicate with our device as we don't check the name of the peer.
			//So theoretically, a hacker can plug into the network with a Verisgn certificate (given to his website www.harmlesswebsite.com) and steal our data.
			//To deal with this the other option is to install the peer's certificate in our trusted store and perform an exact match of the certificate produced by the peer.
			//Indirectly, this is like maintaining a trusted device list and we will trust only "known" devices. But it has a maintenance overhead.

			//Build certificate chain
			CERT_CHAIN_PARA chainPara = {};
			chainPara.cbSize = sizeof(CERT_CHAIN_PARA);
			chainPara.dwStrongSignFlags = CERT_CHAIN_STRONG_SIGN_DISABLE_END_CHECK_FLAG; //To disable checking minimum public key length requirements(1024 bits) in the end certificate.
			//Update: Setting dwStrongSignFlags will work only in Win8 or Win Server '12 or above(http://msdn.microsoft.com/en-us/library/windows/desktop/aa377186(v=vs.85).aspx). So I have added a workaround.


			DWORD certificateChainflags = 0;
			// When CERT_CHAIN_CACHE_END_CERT is set, the end certificate is cached, which might speed up the chain-building process. 
			//  This is useful as hospitals usually have all certificates signed by ONE private CA.
			certificateChainflags |= CERT_CHAIN_CACHE_END_CERT; 
			// Setting CERT_CHAIN_DISABLE_AUTH_ROOT_AUTO_UPDATE inhibits the auto update of third-party roots from the Windows Update Web Server.
			//  We don't want to pollute the trusted store with other certificates from Microsoft.
			certificateChainflags |= CERT_CHAIN_DISABLE_AUTH_ROOT_AUTO_UPDATE;  
			// Potential problem: Coudn't find any flag to explicitly disable CRL checking. Maybe it can only be explicity enabled.

			//----About the workaround to allow key lengths less than 512 bits----
			//After Microsoft's update KB 2661254, certificates with key length <1024 bits are not trusted and produces an error code in CertGetCertificateChain().
			//But it also impacts CertVerifyCertificateChainPolicy() and causes TrustECertSignature in PolicyStatus.dwError which is undocumented.(https://code.google.com/p/chromium/issues/detail?id=162311)
			//Since most medical systems are outdated and can use lesser key lengths, we need to ignore this error.
			//For the workaround, I refrred to http://blogs.technet.com/b/pki/archive/2012/06/12/rsa-keys-under-1024-bits-are-blocked.aspx. It says, in this case,
			//CertGetCertificateChain produces two errors
			// - CERT_TRUST_IS_NOT_SIGNATURE_VALID
			// - CERT_TRUST_HAS_WEAK_SIGNATURE
			// So we ignore the PolicyError.dwError value of TrustECertSignature when the above two errors have been already found.

			//We are following Allura's implementation which used both CertGetCertificateChain and CertVerifyCertificateChainPolicy, though both are used apparently for the same purpose.
			// It which was probably done because CertVerifyCertChainPolicy is easier to use?
			// TODO: improve this and remove redundancy.
			PCCERT_CHAIN_CONTEXT pChainContext = nullptr; //Output
			if (!CertGetCertificateChain(
				m_trustStoreType ==CertStoreType::Machine ? HCCE_LOCAL_MACHINE : HCCE_CURRENT_USER,
				remoteCertContext.handle,
				nullptr,
				nullptr, //pRemoteCertContext->hCertStore,
				&chainPara,
				certificateChainflags,
				nullptr,
				&pChainContext))
			{
				m_log.DeveloperError(L"SChannelController: failed to get the certificate chain (error %u)", GetLastError());
				return false;
			}

			CertificateChainContext chainContext(pChainContext); //Wrapping to release the chain at every exit point.
			bool isKeyLengthlessthan1024 = false;
			if(chainContext.handle->TrustStatus.dwErrorStatus != 0)
			{
				if(chainContext.handle->TrustStatus.dwErrorStatus == static_cast<DWORD>(CERT_TRUST_IS_NOT_SIGNATURE_VALID | CERT_TRUST_HAS_WEAK_SIGNATURE))
				{
					//We are here because of KB 2661254. Ignore this error
					isKeyLengthlessthan1024 = true;
					m_log.DeveloperWarning(L"SChannelController: peer certificate key length is <1024 bits");
				}
				else
				{
					m_log.DeveloperWarning(L"SChannelController: found an error in the certificate chain (error %u)",
										   chainContext.handle->TrustStatus.dwErrorStatus);
					//Intentionally allowed to continue
				}
			}
			// Validate certificate chain.
			HTTPSPolicyCallbackData  polHttps = {0};
			polHttps.cbStruct = sizeof(HTTPSPolicyCallbackData);

			if (m_socketType == SocketType::CLIENT) 
			{
				polHttps.dwAuthType = AUTHTYPE_SERVER; //The server is being authenticated.
			}
			else
			{
				polHttps.dwAuthType = AUTHTYPE_CLIENT; //The client is being authenticated
			}
			polHttps.pwszServerName = nullptr; //Not checking server name on certificate
			polHttps.fdwChecks = 0;

			CERT_CHAIN_POLICY_PARA   policyPara = {0};
			policyPara.cbSize  = sizeof(policyPara);
			policyPara.pvExtraPolicyPara = &polHttps;
			policyPara.dwFlags = CERT_CHAIN_POLICY_IGNORE_WRONG_USAGE_FLAG ; //We will check for this separately.
			policyPara.dwFlags |= CERT_CHAIN_POLICY_IGNORE_INVALID_NAME_FLAG; //We don't check server name as we don't know the expected name of the peer.
			//(unlike in the web-browser case where we know the URL that we are trying to access).
			policyPara.dwFlags |= CERT_CHAIN_POLICY_IGNORE_ALL_REV_UNKNOWN_FLAGS; //We won't have internet access in most cases. So checking CRLs is pointless.


			CERT_CHAIN_POLICY_STATUS policyStatus = {0}; //Output
			policyStatus.cbSize = sizeof(policyStatus);

			if (!CertVerifyCertificateChainPolicy(
				CERT_CHAIN_POLICY_SSL,
				chainContext.handle,
				&policyPara, //We only check for certificate trust and expiry errors.
				&policyStatus)) 
			{
				m_log.DeveloperError(L"SChannelController: failed to verify certificate chain (error %u)", GetLastError());
				// This error code may be wrong as MSDN did not say calling GetLastError on this function will return the cause of failure
				return false;
			}
			if (policyStatus.dwError != 0) 
			{
				if((policyStatus.dwError == TrustECertSignature) && isKeyLengthlessthan1024)
				{
					//Error is due to low key length, so ignore.
				}
				else if(policyStatus.dwError == CertExpired)
				{
					m_log.DeveloperError(L"SChannelController: peer certificate(chain) is expired (%s)", setLastError(TLS_ERR_CERT_EXPIRED).c_str());
					return false;
				}
				else if ( policyStatus.dwError == CertEUntrustedRoot)
				{
					m_log.DeveloperError(L"SChannelController: peer root certificate is not trusted (%s)", setLastError(TLS_ERR_AUTH_CERT_NOT_IN_TRUST).c_str());
					return false;
				}
				else
				{
					m_log.DeveloperError(L"SChannelController: error in trust policy (%s), error %u",
										 setLastError(TLS_ERR_AUTH_CERT_NOT_IN_TRUST).c_str(), policyStatus.dwError);
					return false;
				}
			}
		}
		else
		{
			//Try to find exact match of the peer certificate in our trusted store.
			//The safer option.
			if (!findCertificateInTrustStore(remoteCertContext.handle,m_trustStoreType)) 
			{
				m_log.DeveloperError(L"SChannelController: could not match the peer certificate in the local store (%s)",
									 setLastError(TLS_ERR_AUTH_CERT_NOT_IN_TRUST).c_str());
				return false;
			}

			//Check if the certificate is time valid.
			FILETIME currentTime;
			GetSystemTimeAsFileTime(&currentTime);
			if(CertVerifyTimeValidity(&currentTime, remoteCertContext.handle->pCertInfo) != 0)
			{
				m_log.DeveloperError(L"SChannelController: peer certificate is expired (%s)", setLastError(TLS_ERR_CERT_EXPIRED).c_str());
				return false;
			}
		}

		//We have decided not to check for EKU as IHE ITI TF-2a states "The Secure Node shall not require any specific certificate attribute contents, nor shall it reject
		//certificates that contain unknown attributes or other parameters.". Also EKU does not make much sense in our context and doesnt give much additional security.
		//Check if the certificate's uses are valid
		if (checkCertificateEKU) 
		{
			if (!isEKUCorrect(isCertificateSelfSigned, remoteCertContext.handle)) 
			{
				//setLastError has already been done inside the function.
				return false;
			}
		}

		return true;
	}


	//
	// Abstract  : Write data to a socket port
	// Pre       : Initialization passed, a large enough data buffer is provided
	// Post      : Returns the outcome
	// Exceptions: None
	//
	// The following Sspi errors have special meanings:
	//
	// - SEC_E_INSUFFICIENT_MEMORY:  initialization didn't pass
	// - SecEIncompleteMessage:   a write timeout occured
	// - SecEUnsupportedFunction: a socket error occured
	//
	bool SChannelController::tlsSend(const char *buffer, const unsigned int bufferSize, const int flags, const int timeout)
	{
		SECURITY_STATUS result   = SecEOk;
		unsigned long  	totalBytesSent = 0;
		unsigned long   partSize = 0;
		SecBufferDesc   aSecBufferDesc;
		SecBuffer       aSecBuffer[4];

		// Verify class initialization passed
		if (!areBuffersInitialized())
		{
			if(!initializeBuffers())
			{
				return false;
			}
		}

		// Repeat this till the whole buffer is transmitted
		while ((SecEOk == result) && (totalBytesSent < bufferSize))
		{
			// Split the whole buffer contents into parts fitting the maximum data size
			partSize = bufferSize - totalBytesSent;

			// When this remaining data is too large, use the maximum size
			if (partSize > m_theDataSize)
			{
				partSize = m_theDataSize;
			}

			// Initialize the SecBufferDesc structure
			aSecBufferDesc.ulVersion = SECBUFFER_VERSION;
			aSecBufferDesc.cBuffers  = 4;
			aSecBufferDesc.pBuffers  = &aSecBuffer[0];

			// The header is at the start of the stream buffer
			aSecBuffer[0].BufferType = SECBUFFER_STREAM_HEADER;
			aSecBuffer[0].pvBuffer   = &(*m_theSendBuffer).at(0);
			aSecBuffer[0].cbBuffer   = m_theHeaderSize;

			// The data is behind the header
			aSecBuffer[1].BufferType = SECBUFFER_DATA;
			if(m_theHeaderSize < m_theBufferSize) //Bound checking #TODO: Remove if not allowing SSL2
			{
				aSecBuffer[1].pvBuffer   = &(*m_theSendBuffer).at(m_theHeaderSize); 
				aSecBuffer[1].cbBuffer   = partSize;
			}
			else
			{
				aSecBuffer[1].pvBuffer   = nullptr;
				aSecBuffer[1].cbBuffer   = 0;
			}

			// The trailer is behind the data
			aSecBuffer[2].BufferType = SECBUFFER_STREAM_TRAILER;
			if((m_theHeaderSize + partSize) < m_theBufferSize) //Bound checking
			{
				aSecBuffer[2].pvBuffer   = &(*m_theSendBuffer).at(m_theHeaderSize + partSize);
				aSecBuffer[2].cbBuffer   = m_theTrailerSize;
			}
			else
			{
				aSecBuffer[2].pvBuffer   = nullptr;
				aSecBuffer[2].cbBuffer   = 0;
			}

			// Define the last buffer indicator
			aSecBuffer[3].BufferType = SECBUFFER_EMPTY;
			aSecBuffer[3].pvBuffer   = nullptr;
			aSecBuffer[3].cbBuffer   = 0;

			// Copy the original data into the reserved data area
			memcpy(&(*m_theSendBuffer).at(m_theHeaderSize), &buffer[totalBytesSent], partSize);

			// Fill the header, data and trailer part
			result = EncryptMessage((&m_securityContext->handle), 0, &aSecBufferDesc, 0);

			if(SecEOk != result)
			{
				// Is our authentication challenged ?
				if (SecIRenegotiate == result)
				{
					//#TODO #DECIDE what to do. Allura code said not in the scope of this class.
					m_log.DeveloperError(L"SChannelController: local authentication was challenged (SecIRenegotiate)");
					return false;
					//aResult = AnswerChallenge();
				}
				m_log.DeveloperError(L"SChannelController: Encryptmessage failed with %d", result);
				return false;
			}

			// If our authentication isn't challenged, transmit the package
			if(!m_socket.send(
				&(*m_theSendBuffer).at(0),
				aSecBuffer[0].cbBuffer + aSecBuffer[1].cbBuffer + aSecBuffer[2].cbBuffer, 
				flags,
				timeout))
			{
				m_log.DeveloperError(L"SChannelController: error during send (%s)", setLastError(TLS_ERR_SEND).c_str());
				return false;
			}

			totalBytesSent += partSize;
		}

		return true;
	}

	bool SChannelController::tlsReceive(char *buffer, unsigned int bufferSize, int flags, int timeout)
	{
		SECURITY_STATUS res = SecEOk;
		int byteCount = 0;
		unsigned long totalBytesRead = 0;

		// Verify class initialization passed
		if (!areBuffersInitialized())
		{
			if(!initializeBuffers())
			{
				return false;
			}
		}

		// Can the request be satisfied by the overflow buffer alone ?
		if (m_theUsedOverflowBufferSize >= bufferSize)
		{
			// The overflow buffer is sufficient
			// Fill the destination buffer and update the overflow buffer
			memcpy(buffer, &(*m_theOverflowBuffer).at(0), bufferSize);
			totalBytesRead += bufferSize;
			m_theUsedOverflowBufferSize -= bufferSize;

			// Move the remaining information to the beginning of the buffer
			if (m_theUsedOverflowBufferSize > 0)
			{
				memmove(&(*m_theOverflowBuffer).at(0), &(*m_theOverflowBuffer).at(0) + bufferSize, m_theUsedOverflowBufferSize);
			}
		}
		else
		{
			// The overflow buffer isn't sufficient so its whole contents can be copied
			if (m_theUsedOverflowBufferSize > 0)
			{
				memcpy(buffer, &(*m_theOverflowBuffer).at(0), m_theUsedOverflowBufferSize);
				totalBytesRead += m_theUsedOverflowBufferSize;
				m_theUsedOverflowBufferSize = 0;
			}
		}

		//
		// Second layer: the reception buffer containing the raw data retrieved from the
		//               socket port, which may contain zero or more complete packages
		//

		// Continue reading packages till the requested number of bytes is read
		while ((SecEOk == res) && (totalBytesRead < bufferSize))
		{
			// Read the next package
			res = DecryptPackage(&buffer[totalBytesRead], bufferSize - totalBytesRead, byteCount);

			// When successful, update the read counter
			if (SecEOk == res)
			{
				totalBytesRead += static_cast<unsigned long>(byteCount);
			}

			// Update the reception buffer when no more packages are found
			if (SecEIncompleteMessage == res)
			{
				byteCount = m_socket.receive(&(*m_theReceiveBuffer).at(m_theUsedRecvBufferSize), m_theBufferSize - m_theUsedRecvBufferSize, flags, timeout);
				if(byteCount<1)
				{
					if (m_socket.getLastError() == TLS_SOCKET_DISCONNECTED)
					{
						m_log.DeveloperInfo(L"SChannelController: the socket was disconnected gracefully (%s)", setLastError(TLS_SOCKET_DISCONNECTED).c_str());
					}
					else
					{
						m_log.DeveloperError(L"SChannelController: error during receive (%s)", setLastError(TLS_ERR_RECV).c_str());
					}
					return false;
				}
				res = SecEOk;
				m_theUsedRecvBufferSize += static_cast<unsigned long>(byteCount);
			}

			// Is our authentication challenged ?
			if (SecIRenegotiate == res)
			{
				//#DECIDE what to do, allura simply returned SEC_RENEGOTIATIE in answer challenge
				//aResult = AnswerChallenge();
				m_log.DeveloperError(L"SChannelController: SecIRenegotiate: Rare condition, Allura used to quit when it got this error. Following the same.");
			}
		}

		// Define the return argument and preserve the error code (if any)
		switch (res)
		{
		case SecEOk:
			return true;

		case SecIContextExpired:
			m_log.DeveloperInfo(L"SChannelController: security context expired (%s)", setLastError(SecIContextExpired).c_str());
			return false;

		case SecEUnsupportedFunction:
			// A socket error occured, so preserve the actual error cause
			m_log.DeveloperError(L"SChannelController: socket error (%s)", setLastError(res).c_str());
			return false;

		default:
			// Any other error code originated in the Sspi
			m_log.DeveloperError(L"SChannelController: unknown error (%s)", setLastError(res).c_str());
			return false;

		}
	}

	bool SChannelController::initializeBuffers()
	{
		//LoggingTracing_TraceScope(&m_trace);
		SECURITY_STATUS           result;
		SecPkgContext_StreamSizes streamSizes;

		//Reset values
		streamSizes.cbHeader         = 0;
		streamSizes.cbMaximumMessage = 0;
		streamSizes.cbTrailer        = 0;

		// Determine the size of the header, data part and trailer
		result = QueryContextAttributes((&m_securityContext->handle), SECPKG_ATTR_STREAM_SIZES,	&streamSizes);

		if(SecEOk != result)
		{
			m_log.DeveloperError(L"SChannelController: error in QueryCOntextAttributes (%s), error %d",
								 setLastError(TLS_ERR_CIPHER_QUERY_CONTEXT).c_str(), result);
			return false;  
		}

		// Reserve enough storage space for a maximum sized message.
		// Preserve the size characteristics
		m_theHeaderSize  = streamSizes.cbHeader;
		m_theDataSize    = streamSizes.cbMaximumMessage;
		m_theTrailerSize = streamSizes.cbTrailer;
		m_theBufferSize  = m_theHeaderSize + m_theDataSize + m_theTrailerSize;

		// Be sure the usage counters are also cleared
		m_theUsedRecvBufferSize = m_extraDataSize > 0? m_extraDataSize : 0; //For processing extra data
		m_extraDataSize = 0; //Resetting extra data
		m_theUsedOverflowBufferSize = 0;

		// Allocate a new buffer
		m_theSendBuffer.reset(new std::vector<char>(m_theBufferSize));  //TODO use m_sendBufferSize
		m_theReceiveBuffer.reset(new std::vector<char>(m_theBufferSize > m_theUsedRecvBufferSize? m_theBufferSize : m_theUsedRecvBufferSize)); //TODO use m_recvBufferSize
		m_theOverflowBuffer.reset(new std::vector<char>(m_theBufferSize));

		m_log.DeveloperInfo(L"SChannelController: init buffers; header %d, data %d, trailer %d",
							m_theHeaderSize, m_theDataSize, m_theTrailerSize);

		return true;
	}


	bool SChannelController::areBuffersInitialized() const
	{
		return !(m_theSendBuffer==nullptr||m_theReceiveBuffer==nullptr||m_theOverflowBuffer==nullptr);   
	}


	//  Part of authentication verification: trying to find the client certificate.
	//  The certificate must be valid and present in the Enterprise Trust store.
	//  When the certificate is not found, TLS_ERR_AUTHENTICATION is returned.
	//  Besides this, m_errorCode is set.
	bool SChannelController::findCertificateInTrustStore(const PCCERT_CONTEXT& pRemoteCertContext, const CertStoreType storeToUse) const
	{
		CertificateStore trustCertStore(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING, 0, storeToUse, TrustedCertificatesStore.c_str());
		CertificateContext  pCertContext(CertFindCertificateInStore(
			trustCertStore.handle,
			X509_ASN_ENCODING ,
			0,
			CertFindExisting,   // #DECIDE if this should be find by name or exact match
			pRemoteCertContext,
			nullptr
			));

		if (pCertContext.handle == nullptr)
		{
			return false;
		}
		return true;
	}

	//  Check the usage identifier of the client certificate. When I'm a server,
	//  my client should have enabled the client authentication purpose.
	//  If the usage is not correct, TLS_ERR_AUTHENTICATION is returned.
	//  Besides this, m_errorCode is set to TLS_ERR_AUTH_USAGE_WRONG.
	bool SChannelController::isEKUCorrect(bool selfsigned, const PCCERT_CONTEXT& pRemoteCertContext) const
	{
		bool res = false;

		DWORD size = 0;

		if (!CertGetEnhancedKeyUsage(pRemoteCertContext, 0, nullptr, &size))
		{
			m_log.DeveloperError(L"SChannelController: CertGetEnhancedKeyUsage failed (%s)", setLastError(TLS_ERR_AUTH_USAGE_WRONG).c_str());
			return false;
		}

		std::unique_ptr<CERT_ENHKEY_USAGE> pCertEnhKeyUsage(reinterpret_cast<PCERT_ENHKEY_USAGE>(new char[size])); 

		if (!CertGetEnhancedKeyUsage( pRemoteCertContext, 0, pCertEnhKeyUsage.get(), &size))
		{
			m_log.DeveloperError(L"SChannelController: CertGetEnhancedKeyUsage failed (%s)", setLastError(TLS_ERR_AUTH_USAGE_WRONG).c_str());
			return false;
		}

		//MSDN says if the cUsageIdentifier member is zero, the return from a call to GetLastError can be used to determine
		//whether the certificate is good for all uses or for none. If GetLastError returns CRYPT_E_NOT_FOUND, 
		//the certificate is good for all uses. If it returns zero, the certificate has no valid uses.
		if (0 == pCertEnhKeyUsage->cUsageIdentifier) 
		{
			// #TODO: #DECIDE of this should be allowed: Below code ignores checking usage if certificate is self-signed.
			if (selfsigned || (GetLastError() == CryptENotFound))
			{
				// If All purposes specified, then we'll end up here.
				res = true;
			} 
			else
			{   //This certificate has no valid uses.
				m_log.DeveloperError(L"SChannelController: no valid uses for the certificate (%s)", setLastError(TLS_ERR_AUTH_USAGE_WRONG).c_str());
				res = false;
			}
			return res;
		}

		LPSTR pUsageIdentifier;
		// Now we'll check the usage data of the client certificate.
		if (SocketType::SERVER == m_socketType) 
		{
			pUsageIdentifier = szOID_PKIX_KP_CLIENT_AUTH;
		} 
		else
		{
			pUsageIdentifier = szOID_PKIX_KP_SERVER_AUTH;
		}

		for (unsigned int i = 0; i < pCertEnhKeyUsage->cUsageIdentifier; i++) 
		{
			if ( 0 == strcmp(pCertEnhKeyUsage->rgpszUsageIdentifier[i], pUsageIdentifier) )
			{
				res = true;
				break;
			}
		}

		if (!res)
		{
			m_log.DeveloperError(L"SChannelController: no usage found for the certificate (%s)", setLastError(TLS_ERR_AUTH_USAGE_WRONG).c_str());
		}
		return res;
	}


	SECURITY_STATUS
		SChannelController::DecryptPackage(const LPSTR& buffer, const ULONG& bufferSize, int& usedBufferSize)
	{
		SECURITY_STATUS aResult        = SecEOk;
		ULONG           aIndex         = 0;
   
		SecBuffer       aSecBuffer[4];

		// No information is yet read
		usedBufferSize = 0;

		// Request new information when the reception buffer is empty
		if (0 == m_theUsedRecvBufferSize)
		{
			aResult = SecEIncompleteMessage;
		}

		// When the buffer isn't empty, try to extract a package from it
		if (SecEOk == aResult)
		{
			SecBufferDesc   aSecBufferDesc; 

			// Initialize the SecBufferDesc structure
			aSecBufferDesc.ulVersion = SECBUFFER_VERSION;
			aSecBufferDesc.cBuffers  = 4;
			aSecBufferDesc.pBuffers  = &aSecBuffer[0];

			// This buffer contains the received data
			aSecBuffer[0].BufferType = SECBUFFER_DATA;
			aSecBuffer[0].pvBuffer   = &(*m_theReceiveBuffer).at(0);
			aSecBuffer[0].cbBuffer   = m_theUsedRecvBufferSize;

			// Output buffer 1
			aSecBuffer[1].BufferType = SECBUFFER_EMPTY;
			aSecBuffer[1].pvBuffer   = nullptr;
			aSecBuffer[1].cbBuffer   = 0;

			// Output buffer 2
			aSecBuffer[2].BufferType = SECBUFFER_EMPTY;
			aSecBuffer[2].pvBuffer   = nullptr;
			aSecBuffer[2].cbBuffer   = 0;

			// Output buffer 3
			aSecBuffer[3].BufferType = SECBUFFER_EMPTY;
			aSecBuffer[3].pvBuffer   = nullptr;
			aSecBuffer[3].cbBuffer   = 0;

			// Interpret the data stream
			aResult = DecryptMessage((&m_securityContext->handle), &aSecBufferDesc, 0, nullptr);
		}

		// Preserve the decrypted data
		if (SecEOk == aResult)
		{
			PSecBuffer      aDecryptedData = nullptr;

			// Find the decrypted data buffer
			for (aIndex = 1; (aIndex <=3) && (nullptr == aDecryptedData); aIndex++)
			{
				if (SECBUFFER_DATA == aSecBuffer[aIndex].BufferType)
				{
					aDecryptedData = &aSecBuffer[aIndex];
				}
			}
			// Copy the decrypted information into the destination buffer and/or overflow buffer
			if (nullptr != aDecryptedData)
			{
				// Determine how many bytes can be copied to the destination buffer
				// and how many bytes remain for the overflow buffer
				if (bufferSize >= aDecryptedData->cbBuffer)
				{
					// All data fits the destination buffer
					memcpy(buffer, aDecryptedData->pvBuffer, aDecryptedData->cbBuffer);
					usedBufferSize = aDecryptedData->cbBuffer;
				}
				else
				{
					// Copy what can be copied
					memcpy(buffer, aDecryptedData->pvBuffer, bufferSize);
					usedBufferSize = bufferSize;

					// Copy the remaining bytes in the overflow buffer (when large enough)
					if (m_theBufferSize < static_cast<unsigned long>(aDecryptedData->cbBuffer - bufferSize))
					{
						aResult = SecEBufferTooSmall;
					}
					else
					{
						memcpy(&(*m_theOverflowBuffer).at(0),
							static_cast<LPSTR>(aDecryptedData->pvBuffer) + bufferSize, 
							aDecryptedData->cbBuffer - bufferSize);
						m_theUsedOverflowBufferSize = aDecryptedData->cbBuffer - bufferSize;
					}
				}
			}
		}

		// When information is extracted, update the reception buffer size
		if (SecEOk == aResult)
		{
			PSecBuffer      aRemainingData = nullptr;
			// Find the remaining bytes buffer (if any)
			for (aIndex = 1; (aIndex <=3) && (nullptr == aRemainingData); aIndex++)
			{
				if (SECBUFFER_EXTRA == aSecBuffer[aIndex].BufferType)
				{
					aRemainingData = &aSecBuffer[aIndex];
				}
			}
			// Is there already information received from the next package ?
			if (nullptr == aRemainingData)
			{
				// No, the whole reception buffer is read so flush the current contents
				m_theUsedRecvBufferSize = 0;
			}
			else
			{
				// Flush the current contents but preserve the remaining data
				m_theUsedRecvBufferSize = aRemainingData->cbBuffer;

				memmove(&(*m_theReceiveBuffer).at(0), aRemainingData->pvBuffer, aRemainingData->cbBuffer);
			}
		}

		// Handle an invalid SecEMessageAltered returned by DecryptMessage
		// According to the Platform SDK information of October 2002, this
		// status is only returned by the Microsoft Digest SSP and not by the
		// SChannel SSPs. However, while testing the application the SChannel
		// with a null cipher, the SChannel SSP was found to return the status
		// when the server intended to end the secure session
		if (SecEMessageAltered == aResult)
		{
			aResult = SecIContextExpired;
		}

		// Flush the remaining data when the context is expired
		if (SecIContextExpired == aResult)
		{
			m_theUsedRecvBufferSize = 0;
		}

		return (aResult);
	}

	void SChannelController::traceConnectionInfo()
	{
		SECURITY_STATUS Status;
		SecPkgContext_ConnectionInfo ConnectionInfo;

		Status = QueryContextAttributes((&m_securityContext->handle), SECPKG_ATTR_CONNECTION_INFO, reinterpret_cast<PVOID>(&ConnectionInfo ));
		if(Status != SecEOk)
		{
			m_log.DeveloperError(L"SChannelController: error querying connection info (status %d)", Status);
			return;
		}

		std::wstringstream strm;

		if     ((ConnectionInfo.dwProtocol & static_cast<DWORD>(SP_PROT_TLS1_0)) != 0)  strm << L"Protocol: TLS-1.0";
		else if((ConnectionInfo.dwProtocol & static_cast<DWORD>(SP_PROT_TLS1_1)) != 0)  strm << L"Protocol: TLS-1.1";
		else if((ConnectionInfo.dwProtocol & static_cast<DWORD>(SP_PROT_TLS1_2)) != 0)  strm << L"Protocol: TLS-1.2";
		else if((ConnectionInfo.dwProtocol & static_cast<DWORD>(SP_PROT_SSL3)) != 0)    strm << L"Protocol: SSL3";
		else if((ConnectionInfo.dwProtocol & static_cast<DWORD>(SP_PROT_SSL2)) != 0)    strm << L"Protocol: SSL2";
		else if((ConnectionInfo.dwProtocol & static_cast<DWORD>(SP_PROT_PCT1)) != 0)    strm << L"Protocol: PCT";
		else if((ConnectionInfo.dwProtocol & static_cast<DWORD>(SP_PROT_DTLS1_0)) != 0) strm << L"Protocol: DTLS-1.0";
		else
		{
			strm << L"Protocol: " + ConnectionInfo.dwProtocol;
		}
		strm << std::endl;

		switch(ConnectionInfo.aiCipher)
		{
		case CALG_RC4:        strm << L"Cipher: RC4";          break;
		case CALG_3DES:       strm << L"Cipher: 3DES";	       break;
		case CALG_AES_128: 	  strm << L"Cipher: AES-128";      break;
		case CALG_AES_192:	  strm << L"Cipher: AES-192";	   break;
		case CALG_AES_256 :   strm << L"Cipher: AES-256";	   break;
		case CALG_RC2: 	      strm << L"Cipher: RC2";		   break;
		case CALG_DES:        strm << L"Cipher: DES";		   break;
		case CALG_CYLINK_MEK: strm << L"Cipher: Cylink Mek";   break;
		case CALG_SKIPJACK:   strm << L"Cipher: Skipjack";	   break;
		case CALG_NO_SIGN:    strm << L"Cipher: CALG_NO_SIGN"; break;
		default:              strm << L"Cipher: " << ConnectionInfo.aiCipher; break;
		}
		strm << std::endl;

		strm << L"Cipher strength: " << ConnectionInfo.dwCipherStrength <<std::endl;

		switch(ConnectionInfo.aiHash)
		{
		case CALG_MD2      : strm << L"Hash: MD2";	     break;
		case CALG_MD4      : strm << L"Hash: MD4";	     break;
		case CALG_MD5      : strm << L"Hash: MD5";	     break;
		case CALG_MAC      : strm << L"Hash: MAC";	     break;
		case CALG_SHA1     : strm << L"Hash: SHA1";	     break;
		case CALG_SHA_256  : strm << L"Hash: SHA-256";	 break;
		case CALG_SHA_384  : strm << L"Hash: SHA-384";	 break;
		case CALG_RSA_SIGN : strm << L"Hash: RSA_SIGN";	 break;
		case CALG_DSS_SIGN : strm << L"Hash: DSS_SIGN";	 break;
		default: 			 strm << L"Hash: "<<ConnectionInfo.aiHash; break;
		}
		strm << std::endl;

		strm << L"Hash strength: " << ConnectionInfo.dwHashStrength << std::endl;

		switch(ConnectionInfo.aiExch)
		{
		case CALG_RSA_KEYX: 
		case CALG_RSA_SIGN: strm << L"Key exchange: RSA";             break;
		case CALG_KEA_KEYX: strm << L"Key exchange: KEA";             break;
		case CALG_DH_EPHEM:	strm << L"Key exchange: DH Ephemeral";    break;
		default:        	strm << L"Key exchange: "<< ConnectionInfo.aiExch; break;
		}
		strm <<std::endl;

		m_log.DeveloperInfo(L"SChannelController: connection info; %s", strm.str().c_str());
	}

	bool SChannelController::SendBuffer(SecBuffer * outBuffers)
	{
		if ((outBuffers->cbBuffer != 0) && (outBuffers->pvBuffer != nullptr))
		{
			if (!m_socket.send(reinterpret_cast<const char *>(outBuffers->pvBuffer), outBuffers->cbBuffer, 0, negotiationTimeout))
			{
				// Error sending data to server
				FreeContextBuffer(outBuffers->pvBuffer);
				m_log.DeveloperError(L"SChannelController: error when sending (%s)", setLastError(TLS_ERR_SEND).c_str());
				return false;
			}

			// Free output buffer.
			FreeContextBuffer(outBuffers->pvBuffer);
			outBuffers->pvBuffer = nullptr;
		}
		return true;
	}

}}}