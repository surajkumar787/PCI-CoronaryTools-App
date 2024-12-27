// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <stdint.h>
#include <string>
#include <chrono>
#include <mergecom.h>

#include "application.h"
#include "proposedservicelist.h"

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	class Message;

	class Association
	{
	public:
		enum class OpenStatus { Unknown, Success, Rejected, Aborted, Failed, Timeout };

		struct OpenResult
		{
			OpenStatus           status;
			std::vector<Service> services;
		};

		Association(
			const Application&         application,
			const Application&         remoteApplication,
			int                        port,
			const std::wstring&        host,
			const ProposedServiceList& proposedServiceList);
		virtual ~Association();

		int id           () const { return m_id; };
		bool isConnected () const { return !m_handle.isEmpty(); }

		OpenResult open();
		OpenResult open (MC_Assoc_Info* assocInfo);
		OpenStatus wait(std::chrono::seconds timeout);
		OpenStatus wait (std::chrono::seconds timeout, MC_Assoc_Info* assocInfo);
		void close      ();
		void abort      ();
		void send       (const Message & message);
		void send       (const Message & message, unsigned short responseStatus);
		Message read    (std::chrono::seconds timeout);

		Application localApplication () const { return m_application; }
		Application remoteApplication() const { return m_remoteApplication; }
		int port                     () const { return m_port; }
		std::wstring host            () const { return m_host; };

	protected:
		virtual MC_STATUS openAssocImpl();
		virtual MC_STATUS waitAssocImpl(std::chrono::seconds timeout);

		static OpenStatus toOpenStatus(MC_STATUS status);

	private:
		bool getAcceptableServices(std::vector<Merge::Service>& services) const;

	protected:
		int                 m_id;
		Application         m_application;
		Application         m_remoteApplication;
		int                 m_port;
		std::wstring        m_host;
		ProposedServiceList m_proposedServiceList;
		Handle              m_handle;
	};

}}}