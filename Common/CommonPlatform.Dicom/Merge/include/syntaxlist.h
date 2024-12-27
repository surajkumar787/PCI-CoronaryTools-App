// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "util.h"

#include <string>
#include <vector>
#include <mc3msg.h>

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	class SyntaxList
	{
	public:
		explicit SyntaxList(const std::vector<TRANSFER_SYNTAX> & syntaxes);
		inline std::string                  name    () const { return m_name;     }
		inline std::vector<TRANSFER_SYNTAX> syntaxes() const { return m_syntaxes; }

	private:
		std::string                     m_name;
		std::vector<TRANSFER_SYNTAX>    m_syntaxes;
		Handle                          m_handle;
	};

}}}