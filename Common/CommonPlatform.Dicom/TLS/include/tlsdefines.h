// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>

namespace CommonPlatform { namespace Dicom { namespace TLS
{
	static const std::wstring MyCertificatesStore = L"My";
	static const std::wstring TrustedCertificatesStore = L"PCI Server";

	enum class SocketType
	{
		CLIENT,
		SERVER
	};

	enum class SecurityType
	{
		NONE,
		AUTHENTICATION,
		ENCRYPTION
	};

	enum class CertStoreType
	{
		User,
		Machine
	};

//TLS Error codes
#define TLS_ERR_BASE                                (0xFFFFFF00)

#define TLS_OK                                      (TLS_ERR_BASE + 0x00)

#define TLS_ERR_SEND                                (TLS_ERR_BASE + 0x03)
#define TLS_ERR_RECV                                (TLS_ERR_BASE + 0x04)

#define	TLS_ERR_NEGOTIATE_INVALID_MESSAGE			(TLS_ERR_BASE + 0x09)
#define	TLS_ERR_NEGOTIATE_INITIALIZE_SECCONTEXT		(TLS_ERR_BASE + 0x10)
#define TLS_ERR_NEGOTIATE_ACCEPT_SECCONTEXT         (TLS_ERR_BASE + 0x11)
#define TLS_ERR_NEGOTIATE_INTERNAL					(TLS_ERR_BASE + 0x12)

#define TLS_ERR_CERTIFICATE_NOSUBJECT               (TLS_ERR_BASE + 0x13)

#define TLS_ERR_NEGOTIATE_COMPLETE_AUTH_TOKEN       (TLS_ERR_BASE + 0x14)
#define TLS_ERR_CRED_ACQUIRE						(TLS_ERR_BASE + 0x15)

#define TLS_ERR_SHUTDOWN_APPLY_CONTROL_TOKEN		(TLS_ERR_BASE + 0x18)
#define	TLS_ERR_SHUTDOWN_ACCEPT_SECCONTEXT			(TLS_ERR_BASE + 0x19)


#define TLS_ERR_CIPHER_QUERY_CONTEXT                (TLS_ERR_BASE + 0x40)
#define TLS_ERR_OUT_OF_MEM                          (TLS_ERR_BASE + 0x41)

#define TLS_ERR_OWN_CERTIFICATE_NOT_FOUND           (TLS_ERR_BASE + 0x50)
#define TLS_ERR_CERT_EXPIRED                        (TLS_ERR_BASE + 0x51)
#define TLS_ERR_AUTH_PEER_CERT_NOT_FOUND            (TLS_ERR_BASE + 0x52)
#define TLS_ERR_AUTH_CERT_NOT_IN_TRUST              (TLS_ERR_BASE + 0x53)
#define TLS_ERR_AUTH_SELFSIG_CERT_NOT_ALLOWED       (TLS_ERR_BASE + 0x54)
#define TLS_ERR_AUTH_USAGE_WRONG                    (TLS_ERR_BASE + 0x55)


	//Socket errors
#define TLS_SOCKET_ERROR                            (TLS_ERR_BASE + 0x80)
#define TLS_SOCKET_TIMEOUT							(TLS_ERR_BASE + 0x81)
#define TLS_SOCKET_DISCONNECTED						(TLS_ERR_BASE + 0x82)



}}}