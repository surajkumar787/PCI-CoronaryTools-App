// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <memory>
#include <string>

namespace CommonPlatform { namespace Dicom { namespace Merge
{

	class Library
	{
		class LibrarySingleton;
	public:
		explicit Library(const std::string& mergeIniFile);

	private:
		std::shared_ptr<LibrarySingleton> m_libSingleton;
	};

}}}