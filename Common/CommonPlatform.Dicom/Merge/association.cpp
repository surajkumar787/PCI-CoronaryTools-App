// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "association.h"

#include "message.h"
#include "exception.h"
#include "service.h"

#include <mergecom.h>
#include <sstream>
#include <map>

namespace CommonPlatform { namespace Dicom { namespace Merge
{

	Association::Association(
		const Application &         application,
		const Application &         remoteApplication,
		int                         port,
		const std::wstring &        host,
		const ProposedServiceList & proposedServiceList)
		: m_id                      (0)
		, m_application             (application)
		, m_remoteApplication       (remoteApplication)
		, m_port                    (port)
		, m_host                    (host)
		, m_proposedServiceList     (proposedServiceList)
		, m_handle                  ()
	{
	}

	Association::~Association()
	{
		try {
			close();
		} catch(...) {}
	}

	Service::Role mergeRoleToServiceRole(ROLE_TYPE role)
	{
		switch(role)
		{
			case ROLE_TYPE::SCU_ROLE    : return Service::Role::SCU;
			case ROLE_TYPE::SCP_ROLE    : return Service::Role::SCP;
			case ROLE_TYPE::BOTH_ROLES  : return Service::Role::BOTH;
		}
		throw "Invalid role";
	}

	Association::OpenStatus Association::toOpenStatus(MC_STATUS status)
	{
		switch (status)
		{
		case MC_NORMAL_COMPLETION   : return OpenStatus::Success;
		case MC_ASSOCIATION_REJECTED: return OpenStatus::Rejected;
		case MC_NEGOTIATION_ABORTED : return OpenStatus::Aborted;
		case MC_CONNECTION_FAILED   : return OpenStatus::Failed;
		case MC_TIMEOUT             : return OpenStatus::Timeout;
		default                     : return OpenStatus::Unknown;
		}
	}

	Association::OpenResult Association::open()
	{
		return open(nullptr);
	}

	Association::OpenResult Association::open(MC_Assoc_Info* assocInfo)
	{
		OpenResult result;
		result.status = OpenStatus::Unknown;
		if (m_handle.isEmpty())
		{
			auto status = openAssocImpl();
			if (status == MC_NORMAL_COMPLETION)
			{
				int id(m_id);
				m_handle.reset([id]{ int tmp(id); MC_Close_Association(&tmp); });

				if (assocInfo != nullptr)
				{
					MC_Get_Association_Info(m_id, assocInfo);
				}
				getAcceptableServices(result.services);
			}
			result.status = toOpenStatus(status);
		}
		return result;
	}

	MC_STATUS Association::openAssocImpl()
	{
		std::string serviceListName(m_proposedServiceList.name());
		std::string remoteAETitle(toUtf8(m_remoteApplication.aeTitle()));
		std::string remoteHost(toUtf8(m_host));
		return MC_Open_Association(m_application.id(), &m_id, to_charp(remoteAETitle), &m_port, to_charp(remoteHost), to_charp(serviceListName));
	}

	Association::OpenStatus Association::wait(std::chrono::seconds timeout)
	{
		return wait(timeout, nullptr);
	}

	Association::OpenStatus Association::wait(std::chrono::seconds timeout, MC_Assoc_Info* assocInfo)
	{
		if (m_handle.isEmpty())
		{
			auto status = waitAssocImpl(timeout);
			if(status == MC_NORMAL_COMPLETION)
			{
				int id(m_id);
				m_handle.reset([id]{ int tmp(id); MC_Close_Association(&tmp); });
				if (assocInfo != nullptr)
				{
					MC_Get_Association_Info(m_id, assocInfo);
				}
				// the remote host could have dropped at this point, update the status to the latest known state
				status = MC_Accept_Association(m_id);
			}
			return toOpenStatus(status);
		}
		return OpenStatus::Success;
	}

	MC_STATUS Association::waitAssocImpl(std::chrono::seconds timeout)
	{
		std::string serviceListName(m_proposedServiceList.name());
		std::string remoteHost(toUtf8(m_host));
		return MC_Wait_For_Association_On_Address(serviceListName.c_str(), static_cast<int>(timeout.count()), m_application.id(), m_port, remoteHost.c_str(), &m_id);
	}

	void Association::close()
	{
		if(!m_handle.isEmpty())
		{
			mcCheck(MC_Close_Association(&m_id), std::string("Unable to close the association"));
			m_handle.detach();
		}
	}

	void Association::abort()
	{
		if(!m_handle.isEmpty())
		{
			mcCheck(MC_Abort_Association(&m_id), std::string("Unable to abort the association"));
			m_handle.detach();
		}
	}

	void Association::send(const Message & message)
	{
		auto status = MC_Send_Request_Message(m_id, message.id());
		if (status == MC_ASSOCIATION_ABORTED) m_handle.detach();
		mcCheck(status, "Unable to send message " + std::to_string(message.id()));
	}

	void Association::send(const Message & message, unsigned short responseStatus)
	{
		auto status = MC_Send_Response_Message(m_id, responseStatus, message.id());
		if (status == MC_ASSOCIATION_ABORTED) m_handle.detach();
		mcCheck(status, "Unable to send response message " + std::to_string(message.id()));
	}

	Message Association::read(std::chrono::seconds timeout)
	{
		int           messageId;
		char*         serviceName;
		MC_COMMAND    command;
		auto status = MC_Read_Message(m_id, static_cast<int>(timeout.count()), &messageId, &serviceName, &command);
		if ((status == MC_ASSOCIATION_ABORTED) || (status == MC_ASSOCIATION_CLOSED)) m_handle.detach();
		mcCheck(status, "Error while reading from association " + std::to_string(m_id));
		return Message(messageId);
	}

	bool Association::getAcceptableServices(std::vector<Merge::Service>& services) const
	{
		ServiceInfo sInfo;
		if (MC_Get_First_Acceptable_Service(m_id, &sInfo) == MC_NORMAL_COMPLETION)
		{
			do
			{
				Merge::SyntaxList st( std::vector<TRANSFER_SYNTAX>{ 1, sInfo.SyntaxType });
				services.emplace_back(sInfo.ServiceName, st, mergeRoleToServiceRole(sInfo.RoleNegotiated));
			}
			while(MC_Get_Next_Acceptable_Service(m_id, &sInfo) == MC_NORMAL_COMPLETION);
			return true;
		}
		return false;
	}

}}}
