// Copyright Koninklijke Philips N.V. 2024
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "ProductInfoUtil.h"
#include "stlstring.h"

namespace CommonPlatform {
	namespace Utilities
	{
		// According to XCN-2000927
		std::wstring getProductVersionLabel(int major, int minor, int level)
		{
			wchar_t buffer[32];
			if (level != 0)
			{
				swprintf_s(buffer, L"%d.%d (L %d)", major, minor, level);
			}
			else
			{
				swprintf_s(buffer, L"%d.%d", major, minor);
			}

			return buffer;
		}

		// According to XCN-2000927
		std::wstring getProductVersionLot(int major, int minor, int level)
		{
			wchar_t buffer[32];
			if (level != 0)
			{
				swprintf_s(buffer, L"%d.%d.%d", major, minor, level);
			}
			else
			{
				swprintf_s(buffer, L"%d.%d", major, minor);
			}

			return buffer;
		}

		bool getProductVersion(std::wstring version, int& major, int& minor, int& level)
		{
			bool validReleaseVersion = false;

			if (swscanf_s(version.c_str(), L"%d.%d.%d", &major, &minor, &level) == 3)
			{
				validReleaseVersion = true;
			}
			else
			{
				major = 0;
				minor = 0;
				level = 0;
			}
			return validReleaseVersion;
		}
	}
}
