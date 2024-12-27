// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "util.h"

#include "exception.h"
#include "service.h"

#include <mergecom.h>
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <shlwapi.h>

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	char * to_charp(const string & s)
	{
		return const_cast<char *>(s.c_str());
	}

	bool pathExists(const string &path)
	{
		return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
	}

	string serviceNameToSopClassUid(const string & serviceName)
	{
		char buffer[255] = {};
		mcCheck(MC_Get_UID_From_MergeCOM_Service(serviceName.c_str(), buffer, sizeof(buffer)));
		return buffer;
	}

	string sopClassUidToserviceName(const string & sopClass)
	{
		char buffer[255] = {};
		mcCheck(MC_Get_MergeCOM_Service(sopClass.c_str(), buffer, sizeof(buffer)));
		return buffer;
	}

	TRANSFER_SYNTAX transferSyntaxUidToEnum(const std::string& transferSyntax)
	{
		TRANSFER_SYNTAX ts = INVALID_TRANSFER_SYNTAX;
		mcCheck(MC_Get_Enum_From_Transfer_Syntax(transferSyntax.c_str(), &ts));
		return ts;
	}


	string toUtf8(const wstring &text)
	{
		if (!text.empty())
		{
			std::vector<char> out;
			out.resize(text.size() * 4);

			const int size = ::WideCharToMultiByte(
				CP_UTF8, 0,
				text.c_str(), static_cast<int>(text.size()),
				&(out[0]), static_cast<int>(out.size()),
				NULL, NULL);

			return string(out.begin(), out.begin() + size);
		}

		return string();
	}

	wstring fromUtf8(const string &text)
	{
		if (!text.empty())
		{
			std::vector<wchar_t> out;
			out.resize(text.size());

			const int size = ::MultiByteToWideChar(
				CP_UTF8, 0,
				text.c_str(), static_cast<int>(text.size()),
				&(out[0]), static_cast<int>(out.size()));

			return wstring(out.begin(), out.begin() + size);
		}

		return wstring();
	}

	Handle::Handle()
		: m_handlePrivate()
	{
	}

	Handle::Handle(const function<void(void)> & destructor)
		: m_handlePrivate(std::make_shared<Private>(destructor))
	{
	}

	Handle::~Handle(){}

	void Handle::reset(const function<void(void)> & destructor)
	{
		m_handlePrivate = std::make_shared<Private>(destructor);
	}

	void Handle::detach()
	{
		m_handlePrivate->update(nullptr);
		m_handlePrivate.reset();
	}

	bool Handle::isEmpty() const
	{
		return !m_handlePrivate;
	}

	void PrintService(std::wostream& strm, const Service& service)
	{
		strm << L"- " << fromUtf8(sopClassUidToserviceName(service.sopClassUid()));
		strm << L" (";
		const auto& syntaxList = service.syntaxList().syntaxes();
		strm << syntaxList[0];
		const size_t upperLimit = syntaxList.size() - 1;
		for (size_t i = 1; i < upperLimit; i++)
		{
			strm << L"," << syntaxList[i];
		}
		strm << L")" << std::endl;
	}

	void PrintServices(std::wostream& strm, const std::vector<Service>& services)
	{
		for(const auto& s : services)
		{
			PrintService(strm, s);
		}
	}

}}}