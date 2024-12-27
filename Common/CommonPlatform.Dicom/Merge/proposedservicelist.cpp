// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "proposedservicelist.h"

#include "exception.h"

#include <mergecom.h>
#include <atomic>

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	static std::atomic<int> m_uniqueId;

	ProposedServiceList::ProposedServiceList(const std::vector<Service> & services)
		: m_name("ProposedServiceList" + std::to_string(m_uniqueId++))
		, m_services(services)
		, m_handle()
	{
		NameList<Service> serviceNames(services);
		mcCheck(MC_NewProposedServiceList(to_charp(m_name), serviceNames.getNamesPtr()), "Failed to create proposed service list");
		
		std::string name(m_name); //create local variable, member cannot be passed by value in lambda
		m_handle.reset([name]{ MC_FreeServiceList(to_charp(name)); });
	}

	ProposedServiceList::ProposedServiceList(const std::string& serviceListName) :
		m_name(serviceListName)
	{
	}

}}}