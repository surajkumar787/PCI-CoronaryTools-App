// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>

namespace str_ext
{
	std::wstring stow(const std::string& str);

	std::string wtos(const std::wstring& str);

	std::wstring replace(std::wstring& str, const std::wstring& from, const std::wstring& to);
}
