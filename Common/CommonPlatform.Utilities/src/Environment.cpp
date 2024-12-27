// Copyright Koninklijke Philips N.V. 2016
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Environment.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Lmcons.h>

namespace CommonPlatform { namespace Environment
{
	std::wstring GetLoggedInUserName()
	{
		wchar_t username[UNLEN + 1];
		DWORD username_len = static_cast<DWORD>(UNLEN + 1);
		if (!GetUserNameW(username, &username_len))
		{
			username[0] = 0;
		}

		return username;
	}
}}