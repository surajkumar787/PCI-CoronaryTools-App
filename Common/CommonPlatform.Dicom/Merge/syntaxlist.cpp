// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "syntaxlist.h"

#include "exception.h"

#include <mergecom.h>
#include <atomic>

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	static std::atomic<int> m_uniqueId;

	SyntaxList::SyntaxList(const std::vector<TRANSFER_SYNTAX> & syntaxes)
		: m_name    ("__AutoGeneratedSyntaxList_"+std::to_string(m_uniqueId++))
		, m_syntaxes(syntaxes)
		, m_handle  ()
	{
		m_syntaxes.push_back(static_cast<TRANSFER_SYNTAX>(0)); //Add terminator
		mcCheck(MC_NewSyntaxList(const_cast<char *>(m_name.c_str()), &m_syntaxes[0]), "Cannot create new syntax list");
		
		std::string name(m_name); //create local variable, member cannot be passed by value in lambda
		m_handle.reset([name] { MC_FreeSyntaxList(to_charp(name)); });
	}

}}}