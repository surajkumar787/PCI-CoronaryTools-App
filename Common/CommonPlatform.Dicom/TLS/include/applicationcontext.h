// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include <memory>

namespace CommonPlatform { namespace Dicom { namespace TLS
{
	class SChannelController;
	class MergeTlsAdapter; 
	enum class CertStoreType;
	enum class SecurityType;

	/*!
	\brief  This class contains application specific data, used in MergeCom call backs.
	*/
	class ApplicationContext
	{
		friend MergeTlsAdapter; //To allow MergeTlsAdapter to access securityContext without an interface.

	public:
		ApplicationContext(SecurityType  secType,
						   const std::wstring& certificateName,
						   bool          isSelfSignedCertificateAllowed,
						   bool          isCertificateEkuToBeChecked,
						   CertStoreType personalStoreType,
						   CertStoreType trustStoreType);
		~ApplicationContext();
		unsigned int getErrorCode() const;
		void setErrorCode(unsigned int errorCode);
		

		ApplicationContext(const ApplicationContext&) = delete;
		ApplicationContext& operator= (ApplicationContext&) = delete;

	public:
		const SecurityType  securityType;
		const std::wstring  certificateToUse;
		const CertStoreType personalStoreToUse;
		const CertStoreType trustStoreToUse;
		const bool          allowSelfSignedCertificates;
		const bool          checkCertificateEKU;
		std::string         DicomNodeName;

	private:
		unsigned int						m_errorCode;
		std::unique_ptr<SChannelController> m_securityContext;
	}; 

}}}