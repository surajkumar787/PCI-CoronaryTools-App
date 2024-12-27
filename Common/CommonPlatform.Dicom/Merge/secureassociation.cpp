// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "secureassociation.h"

#include "applicationcontext.h"
#include "mergetlsadapter.h"
#include "tlsdefines.h"

namespace TLS = ::CommonPlatform::Dicom::TLS;

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	std::shared_ptr<Merge::Association> __cdecl createAssociation(const Merge::Application& localApp, const Merge::Application& remoteApp,
														  int port, const std::wstring& host, const Merge::ProposedServiceList& serviceList,
														  TLS::SecurityType security, const std::wstring& certificateName)
	{
		if (security == TLS::SecurityType::NONE)
		{
			return std::make_shared<Merge::Association>(localApp, remoteApp, port, host, serviceList);
		}
		return std::make_shared<Merge::SecureAssociation>(localApp, remoteApp, port, host, serviceList, security, certificateName);
	}

	SecureAssociation::SecureAssociation(const Application&         application,
										 const Application&         remoteApplication,
										 int                        port,
										 const std::wstring&        host,
										 const ProposedServiceList& proposedServiceList,
										 TLS::SecurityType          security,
										 const std::wstring&        certificateName) :
		Association(application, remoteApplication, port, host, proposedServiceList),
		m_appCtx(new TLS::ApplicationContext(security, certificateName, true, false, TLS::CertStoreType::User, TLS::CertStoreType::Machine))
	{
	}

	SecureAssociation::~SecureAssociation()
	{
		// This call is required because the MC_Close_Association sometimes triggers additional SS_Write calls
		// which requires that the ApplicationContext is still alive (which is not the case in the dtor of the base class)
		try {
			close();
		}
		catch (...) {}
	}

	MC_STATUS SecureAssociation::openAssocImpl()
	{
		std::string serviceListName(m_proposedServiceList.name());
		std::string remoteAETitle(toUtf8(m_remoteApplication.aeTitle()));
		std::string remoteHost(toUtf8(m_host));
		return TLS::openSecureAssociation(m_appCtx, m_application.id(), &m_id, remoteAETitle, m_port, remoteHost, serviceListName);
	}

	MC_STATUS SecureAssociation::waitAssocImpl(std::chrono::seconds timeout)
	{
		std::string serviceListName(m_proposedServiceList.name());

		MC_SOCKET socket;
		auto status = MC_Wait_For_Connection_On_Port(static_cast<int>(timeout.count()), m_port, &socket);
		if (status != MC_NORMAL_COMPLETION)
		{
			return status;
		}
		
		int appId = 0;
		status = TLS::processSecureAssociationRequest(socket, serviceListName, appId, m_id, m_appCtx);
		if (status != MC_NORMAL_COMPLETION)
		{
			return status;
		}
		if (appId != m_application.id())
		{
			// this is not the AE we are looking for
			MC_Reject_Association(m_id, PERMANENT_CALLING_AE_TITLE_NOT_RECOGNIZED);
			m_id = -1;
			status = MC_ASSOCIATION_REJECTED;
		}
		return status;
	}

}}}