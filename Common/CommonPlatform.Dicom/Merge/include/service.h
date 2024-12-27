// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "syntaxlist.h"
#include "util.h"

#include <string>

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	class Service
	{
	public:
		enum class Role {SCU, SCP, BOTH};
		Service                            (const std::string & mergeServiceName, const SyntaxList & syntaxList, Role role);
		std::string name            () const { return m_name;             }
		std::string mergeServiceName() const { return m_mergeServiceName; }
		std::string sopClassUid     () const { return m_sopClassUid;      }
		SyntaxList  syntaxList        () const { return m_syntaxList;       }

	private:
		std::string                     m_mergeServiceName;
		std::string                     m_sopClassUid     ;
		std::string                     m_name            ;
		SyntaxList                      m_syntaxList      ;
		Handle                          m_handle          ;
	};

}}}