// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Settings.h"
#include <windows.h>
#include <sstream>
#include <locale>

#include "Log.h"
#include "PathUtil.h"

namespace CommonPlatform{

static ::CommonPlatform::Log& log(::CommonPlatform::Logger(LoggerType::Platform));

Settings::Settings(const std::wstring &filename)
{
	wcscpy_s(m_fileName, Utilities::AbsolutePathFromExeDir(filename).c_str());
}

std::wstring Settings::get(const wchar_t *group, const wchar_t *name, const wchar_t *def) const
{
	wchar_t result[512];
	int len = GetPrivateProfileString(group, name, def, result, 512, m_fileName);
	return len == 0 ? std::wstring() : result;
}

int Settings::get(const wchar_t *group, const wchar_t *name, int def) const
{
	wchar_t result[512];
	int len = GetPrivateProfileString(group, name, L"", result, 512, m_fileName);
	if (len == 0 || (result[0] == 0))
	{
		return def;
	}
	return _wtoi(result);
}

bool Settings::get(const wchar_t *group, const wchar_t *name, bool def) const
{
	wchar_t result[512];
	int len = GetPrivateProfileString(group, name, L"", result, 512, m_fileName);
	if (len == 0 || (result[0] == 0))
	{
		return def;
	}
	return result[0] == L't' || result[0] == L'T' || result[0] == L'1';
}

double Settings::get(const wchar_t *group, const wchar_t *name, double def) const
{
	wchar_t result[512];
	int len = GetPrivateProfileString(group, name, L"", result, 512, m_fileName);
	if (len == 0 || (result[0] == 0))
	{
		return def;
	}

	double r = def;
	std::wistringstream str(result);
	str.imbue(std::locale("C"));
	str >> r;

	return r;
}

float Settings::get(const wchar_t *group, const wchar_t *name, float def) const
{
	return static_cast<float>(get(group,name,static_cast<double>(def)));
}

void Settings::set(const wchar_t *group, const wchar_t *name, const wchar_t *value)
{
	if (!WritePrivateProfileString(group, name, value, m_fileName))
	{
		log.DeveloperError(L"Unable to write %s into config file", name);
	}
}

void Settings::set(const wchar_t *group, const wchar_t *name, const std::wstring &value)
{
	if (!WritePrivateProfileString(group, name, value.c_str(), m_fileName))
	{
		log.DeveloperError(L"Unable to write %s into config file",name);
    }
}

void Settings::set(const wchar_t *group, const wchar_t *name, int value)
{
	wchar_t result[512];
	_itow_s(value, result, 10);
	if (!WritePrivateProfileString(group, name, result, m_fileName))
	{
		log.DeveloperError(L"Unable to write %s into config file", name);
	}
}

void Settings::set(const wchar_t *group, const wchar_t *name, bool value)
{
	if (!WritePrivateProfileString(group, name, value ? L"true" : L"false", m_fileName))
	{
		log.DeveloperError(L"Unable to write %s into config file", name);
	}
}

void Settings::set(const wchar_t *group, const wchar_t *name, double value)
{
	std::wostringstream result;
	result.imbue(std::locale("C"));
	result << value;

	if (!WritePrivateProfileString(group, name, result.str().c_str(), m_fileName))
	{
		log.DeveloperError(L"Unable to write %s into config file", name);
	}
}

void Settings::set(const wchar_t *group, const wchar_t *name, float value)
{
	set(group, name, static_cast<double>(value));
}

const wchar_t *Settings::filename()
{
	return m_fileName;
}

std::vector<std::wstring> CommonPlatform::Settings::getGroups() const
{
	std::vector<std::wstring> res;
	wchar_t buffer[4096] = {};

	if (GetPrivateProfileSectionNames(buffer, _countof(buffer), m_fileName))
	{
		wchar_t* cur = buffer;
		while (*cur)
		{
			res.emplace_back(cur);
			cur += wcslen(cur) + 1;
		}
	}
	return res;
}

std::vector<std::wstring> CommonPlatform::Settings::getGroupKeys(const std::wstring& group) const
{
	std::vector<std::wstring> res;
	wchar_t buffer[4096] = {};

	if (GetPrivateProfileString(group.c_str(), nullptr, L"", buffer, _countof(buffer), m_fileName))
	{
		wchar_t* cur = buffer;
		while (*cur)
		{
			res.emplace_back(cur);
			cur += wcslen(cur) + 1;
		}
	}
	return res;
}
}