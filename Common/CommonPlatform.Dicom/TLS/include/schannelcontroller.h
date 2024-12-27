// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <Winsock.h>

#include "TlsDefines.h"
#include "TlsUtilities.h"
#include "Socket.h"
#include "applicationcontext.h"

namespace CommonPlatform
{
	class Log;
	
	namespace Dicom { namespace TLS
	{
	//This class uses the SChannel API to implement TLS (negotiation, read, write and shutdown).
	//It can perform the role of either a TLS server or client based on the socket type input.
	//Exceptions thrown (only in c'tor):
	//1. Using a bad certificate (Certificate doesnt exist or Name is null).
	//2. Unable to acquire credentials from certificate (ex: private key is missing).
	class SChannelController
	{

	public:
		// C'tor, D'tor
		SChannelController::SChannelController(const SOCKET& mergecomSocket,
											   SocketType socketType,
											   SecurityType securityType,
											   const std::wstring& certificateThumbPrint,
											   CertStoreType personalStoreType,
											   CertStoreType trustStoreType,
											   bool isSelfSignedCertificateAllowed,
											   bool isCertificateEkuToBeChecked,
											   bool isCertificateFromTrustedCaTrusted,
											   int timeoutForNegotiation);
		~SChannelController();

		// Core functions
		bool tlsNegotiate();
		bool tlsShutdown();
		bool tlsSend(const char *buffer, const unsigned int bufferSize, const int flags, const int timeout);
		bool tlsReceive(char *buffer, unsigned int bufferSize, int flags, int timeout);

		// Error handling
		unsigned int getLastError() const;

		SChannelController(const SChannelController&) = delete;
		SChannelController& operator= (const SChannelController&) = delete;
	private:
		// Helper functions used for the TLS handshake
		std::unique_ptr<CredHandle> tlsCreateCredentials() const;
		bool tlsServerNegotiate(bool newContext, bool doInitialRead);
		bool tlsClientNegotiate();

		bool SendBuffer(SecBuffer * outBuffers);

		bool tlsClientHandshakeLoop(bool fDoInitialRead);
		void traceConnectionInfo();

		// Helper functions for send and receive
		inline bool areBuffersInitialized() const;
		bool initializeBuffers();
		SECURITY_STATUS SChannelController::DecryptPackage(const LPSTR& buffer, const ULONG& bufferSize, int& usedBufferSize);


		// Certificate functions
		PCCERT_CONTEXT loadCertificate(const std::wstring& certificateThumbPrint, const CertStoreType& storeToUse) const;
		bool tlsVerifyCertificate() const;
		bool findCertificateInTrustStore(const PCCERT_CONTEXT& context, const CertStoreType storeToUse) const;
		bool isEKUCorrect(bool selfsigned, const PCCERT_CONTEXT& pRemoteCertContext) const;
		bool isCertificateSelfSigned(const PCCERT_CONTEXT& pCertContext) const;

		// Error handling. Sets errorCode and returns the error message for logging.
		std::wstring setLastError(unsigned int errorcode) const;

		void CheckPeerCertificateTimeValidity();

	public:
		// Configurable members
		const bool                         allowSelfSignedCertificate;
		const bool                         checkCertificateEKU;            //EKU: Extended Key Usage
		const bool                         trustCertificatesFromTrustedCa; //CA: Certifying Authority
		const int                          negotiationTimeout;             //In milliseconds

	private:
		// Logger must be initialized first to log constructor errors
		::CommonPlatform::Log&             m_log;
		mutable unsigned int               m_errorCode;

		// The socket to operate on
		const Socket                       m_socket;
		const SocketType                   m_socketType;

		// Certificate Info
		const std::wstring                 m_certificateToUse;
		const CertStoreType                m_personalStoreType;
		const CertStoreType                m_trustStoreType;
		const CertificateContext           m_myCertificateContext;

		// Type of encryption to use
		const SecurityType                 m_securityType;

		// Security handles
		std::unique_ptr<Credentials>       m_myCredentials;   // Destruction Order is important. FreeCredentialsHandle must be called after calling DeleteSecurityContext.
															  // So always declare before securityContext.
		std::unique_ptr<SecurityContext>   m_securityContext;

		// Buffers used during read/write
		std::unique_ptr<std::vector<char>> m_theSendBuffer;
		std::unique_ptr<std::vector<char>> m_theReceiveBuffer;
		std::unique_ptr<std::vector<char>> m_theOverflowBuffer;
		//BufferInfo
		unsigned long                      m_theHeaderSize;
		unsigned long                      m_theDataSize;
		unsigned long                      m_theTrailerSize;
		unsigned long                      m_theBufferSize;

		unsigned long                      m_theUsedRecvBufferSize;
		unsigned long                      m_theUsedOverflowBufferSize;
		unsigned long                      m_extraDataSize;

		//Has the connection been negotiated?
		bool                               m_isNegotiated;

	};

}}}