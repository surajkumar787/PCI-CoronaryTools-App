// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PathUtil.h"

#include <Shlwapi.h>

namespace CommonPlatform { namespace Utilities
{

std::wstring AbsolutePathFromExeDir(const wchar_t* filePath)
{
	if (PathIsRelative(filePath))
	{
		wchar_t fullPath[MAX_PATH] = {};
		GetModuleFileNameW(GetModuleHandle(NULL), fullPath, MAX_PATH);
		PathRemoveFileSpecW(fullPath);
		PathAppendW(fullPath, filePath);
		return fullPath;
	}
	return filePath;
}

std::string AbsolutePathFromExeDir(const char* filePath)
{
	if (PathIsRelativeA(filePath))
	{
		char fullPath[MAX_PATH] = {};
		GetModuleFileNameA(GetModuleHandle(NULL), fullPath, MAX_PATH);
		PathRemoveFileSpecA(fullPath);
		PathAppendA(fullPath, filePath);
		return fullPath;
	}
	return filePath;
}

std::wstring AbsolutePathFromExeDir(const std::wstring& filePath)
{
	return AbsolutePathFromExeDir(filePath.c_str());
}

std::string AbsolutePathFromExeDir(const std::string& filePath)
{
	return AbsolutePathFromExeDir(filePath.c_str());
}

bool FileExists(const std::wstring& fullPath)
{
	DWORD dwAttrib = GetFileAttributesW(fullPath.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool FileExists(const std::string& fullPath)
{
	DWORD dwAttrib = GetFileAttributesA(fullPath.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

}}