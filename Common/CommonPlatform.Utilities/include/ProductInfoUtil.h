#pragma once
// Copyright Koninklijke Philips N.V. 2024
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <string>

namespace CommonPlatform {
	namespace Utilities
	{
		std::wstring getProductVersionLabel(int major, int minor, int level);
		std::wstring getProductVersionLot(int major, int minor, int level);
		bool getProductVersion(std::wstring version, int& major, int& minor, int& level);		
	}
}

