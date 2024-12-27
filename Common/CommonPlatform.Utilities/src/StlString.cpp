// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "stlstring.h"
#include <Windows.h>

namespace str_ext
{
	std::wstring stow(const std::string& str)
	{
		int len;
		int slength = static_cast<int>(str.length()) + 1;
		len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, nullptr, 0);
		wchar_t* buf = new wchar_t[len];
		len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

	std::string wtos(const std::wstring& str)
	{
		int slength = static_cast<int>(str.length()) + 1;
		int len = WideCharToMultiByte(CP_ACP, 0, str.c_str(), slength, nullptr, 0, nullptr, nullptr);
		char* buf = new char[len];
		len = WideCharToMultiByte(CP_ACP, 0, str.c_str(), slength, buf, len, nullptr, nullptr);
		std::string r(buf);
		delete[] buf;
		return r;
	}

	std::wstring replace(std::wstring& str, const std::wstring& from, const std::wstring& to)
	{
		auto newString = str;
		size_t start_pos = str.find(from);
		if (start_pos != std::string::npos)
		{
			newString.replace(start_pos, from.length(), to);
		}

		return newString;
	}
}