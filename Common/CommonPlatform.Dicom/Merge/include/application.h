// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include <memory>
#include <map>

#include "util.h"

namespace CommonPlatform { namespace Dicom { namespace Merge
{

	class Application
	{
	public:
		explicit Application(std::wstring aeTitle);

	public:
		int id() const;
		std::wstring aeTitle() const;

	private:
		class ApplicationSingleton
		{
			friend class Application;

			static std::shared_ptr<ApplicationSingleton> getInstance(const std::wstring & aeTitle);

		private:
			explicit ApplicationSingleton(std::wstring aeTitle);

		private:
			typedef std::map<std::wstring, std::weak_ptr<Application::ApplicationSingleton>> SingletonMap;

			std::wstring        m_aeTitle;
			int                 m_id;
			Handle              m_handle;
			static SingletonMap m_singletons;
		};
		std::shared_ptr<ApplicationSingleton> m_appSingleton;
	};

}}}