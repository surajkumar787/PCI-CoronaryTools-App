// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#ifndef SECURITY_WIN32_SSPI
#define SECURITY_WIN32  //Required for SSPI.h
#endif

#include <Windows.h> //Required for wincrypt.h
#include <wincrypt.h>
#include <sspi.h>
#include <vector>
#include <memory>
#include <functional>

#include "tlsdefines.h"

namespace CommonPlatform { namespace Dicom { namespace TLS
{
	enum class CertStoreType;
	enum class SecurityType;

	class CertificateStore
	{
	public:
		CertificateStore(
			LPCSTR storeProvider,
			DWORD encodingType,
			HCRYPTPROV_LEGACY hCryptProv,
			CertStoreType storeType,
			const void *parameter
			);
		explicit CertificateStore(HCERTSTORE hCertStore);
		~CertificateStore();

		CertificateStore (const CertificateStore&) = delete;
		CertificateStore& operator= (CertificateStore&) =delete;

	public:
		const HCERTSTORE handle;
	};

	class SecurityContext
	{
	public:
		SecurityContext();
		~SecurityContext();

		SecurityContext (const SecurityContext&) = delete;
		SecurityContext& operator= (SecurityContext&) = delete;
	public:
		CtxtHandle handle;
	};


	class CertificateContext
	{
	public:
		explicit CertificateContext(PCCERT_CONTEXT ctxt);
		explicit CertificateContext (const CertificateContext& objToCopy);

		~CertificateContext();
		CertificateContext& operator= (const CertificateContext&);
	public:
		PCCERT_CONTEXT handle;
	};

	class CertificateChainContext
	{
	public:
		explicit CertificateChainContext(const PCCERT_CHAIN_CONTEXT&);
		~CertificateChainContext();

		CertificateChainContext (const CertificateChainContext&) = delete;
		CertificateChainContext& operator= (CertificateChainContext&) = delete;
	public:
		const PCCERT_CHAIN_CONTEXT handle;
	};

	class Credentials
	{
	public:
		explicit Credentials(std::unique_ptr<CredHandle> credHandle);
		~Credentials();

		Credentials (const Credentials&) = delete;
		Credentials& operator= (Credentials&) = delete;
	public:
		const PCredHandle handle;
	private:
		const std::unique_ptr<CredHandle> credentialsHolder;

		int id;
	};


	template<typename HandleType, typename Deleter = void(*)(HandleType)>
	class SmartHandle
	{
	public:
		SmartHandle(HandleType handle, Deleter deleter) : Handle(handle), m_deleter(deleter)
		{}

		virtual ~SmartHandle()
		{
			m_deleter(Handle);
		}


		SmartHandle(const SmartHandle&) = delete;
		SmartHandle& operator= (const SmartHandle&) = delete;
	private:
		Deleter m_deleter;

	public:
		HandleType Handle;
	};

	class CryptographicProvider : public SmartHandle<HCRYPTPROV>
	{
	public:
		CryptographicProvider(HCRYPTPROV hCryptProv) : SmartHandle(hCryptProv, [](HCRYPTPROV prov) { CryptReleaseContext(prov, 0); })
		{}
	};

	class CryptographicKey : public SmartHandle<HCRYPTKEY, decltype(&CryptDestroyKey)>
	{
	public:
		CryptographicKey(HCRYPTKEY hKey) : SmartHandle(hKey, &CryptDestroyKey)
		{}
	};

	// Note on Certificate file formats from MSDN
	//
	//1. Personal Information Exchange (PKCS #12)
	//The Personal Information Exchange format (PFX, also called PKCS #12) supports secure storage of certificates, private keys,
	//	and all certificates in a certification path.
	//The PKCS #12 format is the only file format that can be used to export a certificate and its private key.
	//
	//2. Cryptographic Message Syntax Standard (PKCS #7)
	//The PKCS #7 format supports storage of certificates and all certificates in the certification path. 
	//
	//3. DER-encoded binary X.509
	//The Distinguished Encoding Rules (DER) format supports storage of a single certificate. This format does not support storage
	//	of the private key or certification path.
	//
	//4. Base64-encoded X.509
	//The Base64 format supports storage of a single certificate. This format does not support storage of the private key or certification path.

	// For the purpose of this application, import of files in all the above formats are supported while exports are only done in the .pfx/.p12 format.    

	//This class manages import/export and backup/restore of certificates
	class CertificateManager
	{
	public:
		//Deletes a certificate from a store.
		static bool deleteCertificate(const CertStoreType storeType,
			const std::wstring& certificateSubjectName,
			const std::wstring& storeName);

		//Copies a certificate from one store to another. Replaces an existing certificate if found.
		static bool copyCertificate(const CertStoreType storeType,
									const std::wstring& certificateSubjectName, 
									const std::wstring& sourceStoreName, 
									const std::wstring& destinationStoreName);

		static PCCERT_CONTEXT makeCertificate(CertStoreType storeType, const std::wstring& subject, const std::wstring& store,
											  SYSTEMTIME* validFrom = nullptr, SYSTEMTIME* validUntil = nullptr);

		static std::wstring getCertificateName(PCCERT_CONTEXT context);
	};

	struct CryptHashBlob : public CRYPT_HASH_BLOB
	{
		CryptHashBlob()
		{
			pbData = data;
			cbData = _countof(data);
		}
	private:
		BYTE data[20];
	};

	void ThumbprintFromCryptHashBlock(const CryptHashBlob& blob, std::wstring& thumbPrint);
	void CryptHashBlobFromThumbprint(const std::wstring& thumbPrint, CryptHashBlob& blob);

}}}
