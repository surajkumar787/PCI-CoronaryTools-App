// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "message.h"

#include "exception.h"
#include "service.h"
#include "exception.h"
#include "file.h"

#include <mc3media.h>

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	Message::Message(const Service& service, MC_COMMAND command)
	{
		std::string serviceName(service.mergeServiceName());
		mcCheck(MC_Open_Message(&m_id, to_charp(serviceName), command),
				std::string("Error creating message with service name: ") + serviceName);

		if (!service.syntaxList().syntaxes().empty())
		{
			setTransferSyntax(service.syntaxList().syntaxes().front());
		}

		addAttribute(MC_ATT_SOP_CLASS_UID);
		set(MC_ATT_SOP_CLASS_UID, service.sopClassUid());

		int id(m_id); //create local variable, member cannot be passed by value in lambda
		m_handle.reset([id]{int tmp(id); MC_Free_Message(&tmp);});
	}

	Message::Message(const std::string& serviceName, MC_COMMAND command)
	{
		mcCheck(MC_Open_Message(&m_id, to_charp(serviceName), command),
				std::string("Error creating message with service name: ") + serviceName);

		int id(m_id); //create local variable, member cannot be passed by value in lambda
		m_handle.reset([id]{int tmp(id); MC_Free_Message(&tmp);});
	}

	Message::Message(int id)
	{
		m_id = id;
		m_handle.reset([id]{int tmp(id); MC_Free_Message(&tmp);});
	}

	OutputFile Message::toFile() const
	{
		int dupId = 0;
		mcCheck(MC_Duplicate_Message(m_id, &dupId, getTransferSyntax(), nullptr, nullptr));
		Message dupMsg(dupId);
		mcCheck(MC_Message_To_File(dupMsg.id(), ""), "Unable to convert " + std::to_string(dupMsg.id()) + " to file.");
		OutputFile f(dupMsg.id(), getTransferSyntax());
		dupMsg.m_handle.detach();
		return f;
	}

	std::string Message::service() const
	{
		MC_COMMAND command;
		char * serviceName;
		MC_Get_Message_Service(m_id, &serviceName, &command);
		return serviceName;
	}

	int Message::command() const
	{
		MC_COMMAND command;
		char * serviceName;
		MC_Get_Message_Service(m_id, &serviceName, &command);
		return command;
	}

}}}