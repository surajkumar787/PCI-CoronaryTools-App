// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "library.h"

#include "exception.h"

#include <mergecom.h>
#include <mutex>
#include "PathUtil.h"
#include "association.h"

namespace CommonPlatform { namespace Dicom { namespace Merge
{

	class Library::LibrarySingleton
	{
	public:
		static std::shared_ptr<LibrarySingleton> instance(const std::string& mergeIniFile)
		{
			static std::weak_ptr<LibrarySingleton> s_instance;
			static std::mutex s_mutex;

			auto iniFile = CommonPlatform::Utilities::AbsolutePathFromExeDir(mergeIniFile);

			auto result(s_instance.lock());
			if(result) return result;

			std::lock_guard<std::mutex> lock(s_mutex);
			result = s_instance.lock();
			if(!result)
			{
				result.reset(new LibrarySingleton(iniFile));
				s_instance = result;
			}
			return result;
		}

		~LibrarySingleton()
		{
			MC_Enable_Unicode_Conversion(0);
			MC_Library_Release();
		}

	private:
		LibrarySingleton(const std::string& mergeIniFile)
		{
			mcCheck(MC_Set_MergeINI(to_charp(mergeIniFile)));
			mcCheck(MC_Library_Initialization(NULL, NULL, NULL), "Failed to initialize MergeCom library!");
			mcCheck(MC_Enable_Unicode_Conversion(1));
		}
	};

	Library::Library(const std::string& mergeIniFile)
		: m_libSingleton(LibrarySingleton::instance(mergeIniFile))
	{
	}

}}}