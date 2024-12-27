// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>

namespace CommonPlatform { namespace Utilities
{

std::wstring AbsolutePathFromExeDir(const wchar_t* filePath);
std::wstring AbsolutePathFromExeDir(const std::wstring& filePath);
std::string  AbsolutePathFromExeDir(const char* filePath);
std::string  AbsolutePathFromExeDir(const std::string& filePath);

bool FileExists(const std::wstring& fullPath);
bool FileExists(const std::string& fullPath);

}}