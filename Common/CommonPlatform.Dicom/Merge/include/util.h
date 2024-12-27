// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <mc3msg.h>

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	using std::string;
	using std::wstring;
	using std::function;

	class Service;

	//WARNING: use only for ugly c libs which don't change the content! (non const correct)
	char * to_charp(const string &);
	char * to_charp(const string &&); //No implementation! Don't create pointers to temporary values...

	string toUtf8(const wstring &text);
	wstring fromUtf8(const string &text);

	bool pathExists(const string &path);

	string serviceNameToSopClassUid(const string & serviceName);
	string sopClassUidToserviceName(const string & sopClass);

	TRANSFER_SYNTAX transferSyntaxUidToEnum(const std::string& transferSyntax);

	template <typename T>
	class NameList
	{
	public:
		inline NameList(std::vector<T> items)
		{
			for(const auto & i : items)     m_strings.push_back(i.name());
			for(const auto & s : m_strings) m_charps.push_back(to_charp(s));
			m_charps.push_back(nullptr);
		}

		inline char ** getNamesPtr() { return &m_charps[0]; }

	private:
		std::vector<string> m_strings;
		std::vector<char *>      m_charps;
	};

	class Handle
	{
	public:
		Handle();
		explicit Handle(const function<void(void)> & destructor);
		~Handle();

		bool isEmpty() const;
		void reset(const function<void(void)> & destructor);
		void reset();
		void detach();

	private:
		class Private
		{
		public:
			Private(const function<void(void)> & d) : destructor(d) {}
			~Private(){ if(destructor) destructor(); }
			void update(const function<void(void)> & d) { destructor = d; };
	
		private:
			function<void(void)> destructor;
		};
		std::shared_ptr<Private> m_handlePrivate;
	};

	void PrintService(std::wostream& strm, const Service& service);
	void PrintServices(std::wostream& strm, const std::vector<Service>& services);

	// provides a shorthand for converting a c-style array into an STL vector
	template<typename T, size_t Size>
	std::vector<T> ArrayToVector(const T (& array)[Size])
	{
		return std::vector<T>(std::begin(array), std::end(array));
	}
}}}