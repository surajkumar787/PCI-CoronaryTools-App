// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#ifndef SECURITY_WIN32_SSPI
#define SECURITY_WIN32  //Required for SSPI.h
#endif

#pragma warning(disable:4706)
#include <windows.h>
#include <fstream>
#include <shlwapi.h>
#include <strsafe.h>

#include "tlsutilities.h"
#include "tlsdefines.h"
#include "applicationcontext.h"
#include "Log.h"

namespace CommonPlatform { namespace Dicom { namespace TLS
{
	bool CertificateManager::deleteCertificate(const CertStoreType storeType,
											   const std::wstring& certificateSubjectName,
											   const std::wstring& storeName)
	{
		auto& log = ::CommonPlatform::Logger(LoggerType::Platform);

		if(certificateSubjectName.empty()) return true;
		CertificateStore CertStore(CertOpenStore(
			CERT_STORE_PROV_SYSTEM,
			static_cast<DWORD>(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING),
			NULL,										//Handle to the Cryptographic provider.
			(storeType == CertStoreType::Machine ? CERT_SYSTEM_STORE_LOCAL_MACHINE : CERT_SYSTEM_STORE_CURRENT_USER)
			| CERT_STORE_NO_CRYPT_RELEASE_FLAG
			| CERT_STORE_OPEN_EXISTING_FLAG,
			storeName.c_str()
			));
		if (CertStore.handle == nullptr)
		{
			log.DeveloperError(L"CertificateManager: Failed to open %s store", storeName.c_str());
			return false;
		}

		PCCERT_CONTEXT pCertContext=NULL;
		do{
			pCertContext = CertFindCertificateInStore(
				CertStore.handle,
				X509_ASN_ENCODING,
				0,
				CERT_FIND_SUBJECT_STR, //Performs substring in string match of certSubjectName. So will delete all such certs.
				certificateSubjectName.c_str(),
				NULL
				);
			if (pCertContext != nullptr)
			{
				log.DeveloperInfo(L"CertificateManager: Found requested certificate %s", certificateSubjectName.c_str());
				if (!CertDeleteCertificateFromStore(pCertContext))
				{
					log.DeveloperError(L"CertificateManager: Failed to delete the certificate %s from %s", certificateSubjectName.c_str(), storeName.c_str());
					return false;
				}
			}
		} while(pCertContext);

		return true;
	}

	//Replaces an existing certificate if found
	bool CertificateManager::copyCertificate(const CertStoreType storeType,
											 const std::wstring& certificateSubjectName,
											 const std::wstring& sourceStoreName,
											 const std::wstring& destinationStoreName)
	{
		auto& log = ::CommonPlatform::Logger(LoggerType::Platform);
		if(certificateSubjectName.empty()) return true;
		CertificateStore sourceCertStore(CertOpenStore(
			CERT_STORE_PROV_SYSTEM,
			static_cast<DWORD>(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING),
			NULL,										//Handle to the Cryptographic provider.
			(storeType == CertStoreType::Machine ? CERT_SYSTEM_STORE_LOCAL_MACHINE : CERT_SYSTEM_STORE_CURRENT_USER)
			| CERT_STORE_NO_CRYPT_RELEASE_FLAG
			| CERT_STORE_OPEN_EXISTING_FLAG,
			sourceStoreName.c_str()
			));
		if (sourceCertStore.handle ==  nullptr)
		{
			log.DeveloperError(L"CertificateManager: Failed to open %s store", sourceStoreName.c_str());
			return false;
		}

		CertificateContext sourceCertContext(CertFindCertificateInStore(
			sourceCertStore.handle,
			X509_ASN_ENCODING,
			0,
			CERT_FIND_SUBJECT_STR,
			certificateSubjectName.c_str(),
			nullptr
			));

		if (sourceCertContext.handle == nullptr)
		{
			log.DeveloperError(L"CertificateManager: Could not find the certificate %s in %s",
							   certificateSubjectName.c_str(), sourceStoreName.c_str());
			return false;
		}

		CertificateStore destCertStore(CertOpenStore(
			CERT_STORE_PROV_SYSTEM,
			static_cast<DWORD>(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING),
			NULL,										//Handle to the Cryptographic provider.
			(storeType == CertStoreType::Machine ? CERT_SYSTEM_STORE_LOCAL_MACHINE : CERT_SYSTEM_STORE_CURRENT_USER)
			| CERT_STORE_NO_CRYPT_RELEASE_FLAG
			| CERT_STORE_OPEN_EXISTING_FLAG,
			destinationStoreName.c_str()
			));
		if (destCertStore.handle == nullptr)
		{
			log.DeveloperError(L"CertificateManager: Failed to open %s store", destinationStoreName.c_str());
			return false;
		}

		bool rv = CertAddCertificateContextToStore(destCertStore.handle, sourceCertContext.handle, CERT_STORE_ADD_REPLACE_EXISTING, NULL) == 1;
		if(!rv)
		{
			log.DeveloperError(L"CertificateManager: Failed to add the certificate %s to %s", certificateSubjectName.c_str(), destinationStoreName.c_str());
		}
		return rv;
	}

	PCCERT_CONTEXT CreateCertificate(const std::wstring& name, SYSTEMTIME* start, SYSTEMTIME* end)
	{
		static uint32_t keyCounter = 0;
		auto& log = ::CommonPlatform::Logger(LoggerType::Platform);
		wchar_t KeyContainer[256] = L"PCIkey";
		auto curPos = KeyContainer + wcslen(KeyContainer);
		StringCchPrintf(curPos, _countof(KeyContainer), L"%u", keyCounter);
		curPos = KeyContainer + wcslen(KeyContainer);
		name._Copy_s(curPos, _countof(KeyContainer) - (curPos - KeyContainer), name.size());
		{
			CryptographicProvider cryptProv(0);
			CryptographicKey cryptKey(0);

			// open the existing key container
			if (!CryptAcquireContext(&cryptProv.Handle, KeyContainer, nullptr, PROV_RSA_SCHANNEL, 0))
			{
				log.DeveloperInfo(L"CreateCertificate: Opening existing key container failed with 0x%08X, creating a new one...", GetLastError());
				// otherwise create a new one
				if (!CryptAcquireContext(&cryptProv.Handle, KeyContainer, nullptr, PROV_RSA_SCHANNEL, CRYPT_NEWKEYSET))
				{
					log.DeveloperError(L"CreateCertificate: Failed to create a new key container; 0x%08X", GetLastError());
					return nullptr;
				}
			}

			if (!CryptGenKey(cryptProv.Handle, AT_KEYEXCHANGE, 0x08000000 /* 2048 bit key */, &cryptKey.Handle))
			{
				log.DeveloperError(L"CreateCertificate: Failed to generate a key pair; 0x%08X", GetLastError());
				return nullptr;
			}
		}

		std::wstring subject = L"CN=" + name;
		DWORD requiredBytes = 0;
		if (!CertStrToName(X509_ASN_ENCODING, subject.c_str(), CERT_X500_NAME_STR | CERT_NAME_STR_ENABLE_UTF8_UNICODE_FLAG, nullptr, nullptr, &requiredBytes, nullptr))
		{
			log.DeveloperError(L"CreateCertificate: Failed to get the required size of the subject blob; 0x%08X", GetLastError());
			return nullptr;
		}
		std::vector<BYTE> subjectEncoded(requiredBytes);
		if (!CertStrToName(X509_ASN_ENCODING, subject.c_str(), CERT_X500_NAME_STR | CERT_NAME_STR_ENABLE_UTF8_UNICODE_FLAG, nullptr, &subjectEncoded[0], &requiredBytes, nullptr))
		{
			log.DeveloperError(L"CreateCertificate: Failed to encode the subject; 0x%08X", GetLastError());
			return nullptr;
		}

		CERT_NAME_BLOB subjectBlob = {};
		subjectBlob.cbData = requiredBytes;
		subjectBlob.pbData = &subjectEncoded[0];

		CRYPT_KEY_PROV_INFO keyProvInfo = {};
		keyProvInfo.pwszContainerName = KeyContainer;
		keyProvInfo.dwProvType = PROV_RSA_SCHANNEL;
		keyProvInfo.dwKeySpec = AT_KEYEXCHANGE;

		CRYPT_ALGORITHM_IDENTIFIER algId = {};
		algId.pszObjId = szOID_RSA_SHA256RSA;

		PCCERT_CONTEXT cert = CertCreateSelfSignCertificate(0, &subjectBlob, 0, &keyProvInfo, &algId, start, end, nullptr);
		if (cert == nullptr)
		{
			log.DeveloperError(L"CreateCertificate: Failed to create the self signed certificate; 0x%08X", GetLastError());
			return nullptr;
		}
		if (!CertSetCertificateContextProperty(cert, CERT_KEY_PROV_INFO_PROP_ID, 0, &keyProvInfo))
		{
			log.DeveloperError(L"CreateCertificate: Failed to set the key info on the certificate; 0x%08X", GetLastError());
		}

		return cert;
	}

	bool AddCertificateToStore(CertStoreType storeType, const std::wstring& storeName, PCCERT_CONTEXT certificate)
	{
		CertificateStore store(CERT_STORE_PROV_SYSTEM, X509_ASN_ENCODING, 0, storeType, storeName.c_str());
		return CertAddCertificateContextToStore(store.handle, certificate, CERT_STORE_ADD_REPLACE_EXISTING, nullptr) == TRUE; // bool != BOOL
	}

	PCCERT_CONTEXT CertificateManager::makeCertificate(CertStoreType storeType, const std::wstring& subject, const std::wstring& store,
													   SYSTEMTIME* validFrom, SYSTEMTIME* validUntil)
	{
		auto cert = CreateCertificate(subject, validFrom, validUntil);
		if (cert != nullptr)
		{
			if (!AddCertificateToStore(storeType, store, cert))
			{
				::CommonPlatform::Logger(LoggerType::Platform).DeveloperError(L"makeCertificate: Failed to add the certificate to the store; 0x%08X", GetLastError());
			}
		}
		return cert;
	}

	CertificateStore::CertificateStore(LPCSTR storeProvider, DWORD encodingType, HCRYPTPROV_LEGACY hCryptProv, CertStoreType storeType, const void *parameter) :
		handle(CertOpenStore(storeProvider,
							 encodingType,
							 hCryptProv,
							 (storeType == CertStoreType::Machine ? CERT_SYSTEM_STORE_LOCAL_MACHINE : CERT_SYSTEM_STORE_CURRENT_USER) | CERT_STORE_MAXIMUM_ALLOWED_FLAG,
							 parameter))
	{
		if(handle == nullptr)
		{
			DWORD error = GetLastError();
			::CommonPlatform::Logger(LoggerType::Platform).DeveloperError(L"CertificateStore: Could not open the certificate Store (flags = %d) error %u",
				(DWORD)(storeType == CertStoreType::Machine? CERT_SYSTEM_STORE_LOCAL_MACHINE:CERT_SYSTEM_STORE_CURRENT_USER),
				error);
			throw std::exception("Failed to open the certificate store.");
		}
	}

	CertificateStore::CertificateStore(HCERTSTORE hCertStore)
		:handle(hCertStore)
	{
		if(handle == nullptr)
		{
			::CommonPlatform::Logger(LoggerType::Platform).DeveloperError(L"CertificateStore: HCERTSTORE is null");
			throw std::exception("Failed to open the certificate store.");
		}
	}

	CertificateStore::~CertificateStore()
	{
		CertCloseStore(handle, CERT_CLOSE_STORE_CHECK_FLAG); 
	}

	SecurityContext::SecurityContext()
	{ 
		SecInvalidateHandle(&handle);
	}

	SecurityContext::~SecurityContext()
	{
		if (SecIsValidHandle(&handle)) 
		{
			DeleteSecurityContext(&handle);
			SecInvalidateHandle(&handle);
		}
	}

	CertificateContext::CertificateContext(PCCERT_CONTEXT ctxt) :
		handle(ctxt)
	{}

	CertificateContext::CertificateContext(const CertificateContext& objToCopy) :
		handle(CertDuplicateCertificateContext(objToCopy.handle))
	{}

	CertificateContext& CertificateContext::operator= (const CertificateContext& that)
	{
		CertFreeCertificateContext(handle);
		handle = CertDuplicateCertificateContext(that.handle);
		return *this;
	}

	CertificateContext::~CertificateContext()
	{
		CertFreeCertificateContext(handle);
	}

	CertificateChainContext::CertificateChainContext(const PCCERT_CHAIN_CONTEXT& ctxt) :
		handle(ctxt)
	{}

	CertificateChainContext::~CertificateChainContext()
	{
		CertFreeCertificateChain(handle);
	}

	Credentials::Credentials(std::unique_ptr<CredHandle> credHandle) :
		handle(credHandle.get()),                //Store handle
		credentialsHolder(std::move(credHandle)) //Transfer ownership
	{   
		static int counter = 0;
		id = ++counter;
		::CommonPlatform::Logger(LoggerType::Platform).DeveloperInfo(L"Credentials: constructing credential %d", id);
	}

	Credentials::~Credentials()
	{
		::CommonPlatform::Logger(LoggerType::Platform).DeveloperInfo(L"Credentials: destroying credentials %d", id);
		FreeCredentialsHandle(handle);
	}

	std::wstring CertificateManager::getCertificateName(PCCERT_CONTEXT context)
	{
		int count = CertGetNameStringW(context, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, nullptr, nullptr, 0);
		std::unique_ptr<wchar_t[]> buffer(new wchar_t[count]);
		if (!CertGetNameStringW(context, CERT_NAME_SIMPLE_DISPLAY_TYPE, NULL, NULL, buffer.get(), count))
		{
			return L"Failed";
		}
		return buffer.get();
	}

	void ThumbprintFromCryptHashBlock(const CryptHashBlob& blob, std::wstring& thumbPrint)
	{
		thumbPrint.clear();
		auto bin2hex = [](BYTE value) -> char
		{
			return ((value < 10) ? (value + '0') : (value + ('A' - 10)));
		};

		for (size_t i = 0; i < blob.cbData; i++)
		{
			char digit;
			digit = ((blob.pbData[i] & 0xf0) >> 4);
			thumbPrint.append(1, bin2hex(digit));
			digit = blob.pbData[i] & 0x0f;
			thumbPrint.append(1, bin2hex(digit));
		}
	}

	void CryptHashBlobFromThumbprint(const std::wstring& thumbPrint, CryptHashBlob& blob)
	{
		auto hex2bin = [](wchar_t ch) -> BYTE
		{
			if (ch >= '0' && ch <= '9')
				return static_cast<BYTE>(ch - '0');
			else if (ch >= 'A' && ch <= 'F')
				return static_cast<BYTE>(ch - 'A' + 10);
			else if (ch >= 'a' && ch <= 'f')
				return static_cast<BYTE>(ch - 'a' + 10);
			else
				throw std::exception("not a hex character");
		};

		for (size_t i = 0; i < blob.cbData; i++)
		{
			BYTE value;
			value = (hex2bin(thumbPrint[i*2]) << 4);
			value |= hex2bin(thumbPrint[i*2+1]);
			blob.pbData[i] = value;
		}
	}

}}}