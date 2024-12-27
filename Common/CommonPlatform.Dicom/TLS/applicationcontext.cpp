// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "applicationcontext.h"
#include "tlsdefines.h"
#include "schannelcontroller.h"

namespace CommonPlatform { namespace Dicom { namespace TLS
{
	ApplicationContext::ApplicationContext(SecurityType        secType,
										   const std::wstring& certificateName,
										   bool                isSelfSignedCertificateAllowed,
										   bool                isCertificateEkuToBeChecked,
										   CertStoreType       personal,
										   CertStoreType       trust) :
		securityType(secType),
		certificateToUse(certificateName),
		personalStoreToUse(personal),
		trustStoreToUse (trust),
		allowSelfSignedCertificates(isSelfSignedCertificateAllowed),
		checkCertificateEKU(isCertificateEkuToBeChecked),
		DicomNodeName(),
		m_errorCode(TLS_OK),
		m_securityContext()
	{
	}

	ApplicationContext::~ApplicationContext()
	{
	}

	void ApplicationContext::setErrorCode(unsigned int errorCode)
	{
		m_errorCode = errorCode;
	}

	unsigned int ApplicationContext::getErrorCode() const
	{
		return m_errorCode;
	}


}}}