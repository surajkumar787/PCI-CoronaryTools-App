// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "association.h"

#include <mergecom.h>

namespace CommonPlatform { namespace Dicom
{ 
	namespace TLS
	{
		class ApplicationContext;
		enum class SecurityType;
	}

	namespace Merge
	{
		// force to __cdecl calling convention due to its usage in MergecomManaged, otherwise the compiler will turn it into __clrcall (causing build errors)
		std::shared_ptr<Merge::Association> __cdecl createAssociation(const Merge::Application& localApp, const Merge::Application& remoteApp,
		                                                              int port, const std::wstring& host, const Merge::ProposedServiceList& serviceList,
		                                                              TLS::SecurityType security, const std::wstring& certificateName);

		class SecureAssociation : public Association
		{
		public:
			SecureAssociation(const Application&         application,
							  const Application&         remoteApplication,
							  int                        port,
							  const std::wstring&        host,
							  const ProposedServiceList& proposedServiceList,
							  TLS::SecurityType          security,
							  const std::wstring&        certificateName);
			virtual ~SecureAssociation();

		protected:
			virtual MC_STATUS openAssocImpl() override;
			virtual MC_STATUS waitAssocImpl(std::chrono::seconds timeout) override;

		private:
			std::unique_ptr<TLS::ApplicationContext> m_appCtx;
		};

	}
}}