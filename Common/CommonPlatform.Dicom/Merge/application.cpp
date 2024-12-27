// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "application.h"

#include "exception.h"

#include <mergecom.h>
#include <mutex>

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	static std::mutex appMutex;

	std::shared_ptr<Application::ApplicationSingleton> Application::ApplicationSingleton::getInstance(const std::wstring & aeTitle)
	{
		std::lock_guard<std::mutex> lock(appMutex);
		auto result(m_singletons[aeTitle].lock());
		if(!result)
		{
			result.reset(new ApplicationSingleton(aeTitle));
			m_singletons[aeTitle] = result;
		}
		return result;
	}

	Application::ApplicationSingleton::ApplicationSingleton(std::wstring aeTitle)
		: m_aeTitle(aeTitle)
		, m_id(-1)
		, m_handle()
	{
		mcCheck(MC_Register_Application(&m_id, toUtf8(m_aeTitle).c_str()), std::string("MC_Register_Application failed for ") + toUtf8(m_aeTitle));
		int id(m_id);
		m_handle.reset([id]{ int tmp(id); MC_Release_Application(&tmp); });
	}

	Application::ApplicationSingleton::SingletonMap Application::ApplicationSingleton::m_singletons;

	Application::Application(std::wstring aeTitle)
		: m_appSingleton(ApplicationSingleton::getInstance(aeTitle))
	{
	}

	int Application::id() const
	{
		return m_appSingleton->m_id;
	};

	std::wstring Application::aeTitle() const
	{
		return m_appSingleton->m_aeTitle;
	};

}}}