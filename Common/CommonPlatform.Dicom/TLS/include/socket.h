// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <winsock.h>

#include "TlsDefines.h" //For error codes only

namespace CommonPlatform
{
	class Log;

	namespace Dicom { namespace TLS
	{

	enum class SocketOperation
	{
		Send,
		Receive
	};

	class Socket
	{
	public:
		explicit Socket(const SOCKET& s);
		
		//CoreFunctions
		bool send(const LPCSTR& buffer, const int& bufferSize, const int& flags, const int& timeout) const;
		int receive(LPSTR buffer, const int& bufferSize, const int& flags, const int& timeout) const; //Returns number of bytes read on success, zero on failure.

		//Error handling
		int getLastError() const;

		Socket (const Socket&) = delete;
		Socket& operator= (Socket&) = delete;

	private:
		//Helper functions
		bool waitUntilReady(const int timeout, const SocketOperation operation) const;
		
		//Error handling
		void setLastError(int errorCode) const;

	private:
		const SOCKET		   m_thePlainSocket;
		mutable	int			   m_errorCode;
		::CommonPlatform::Log& m_log;
	};

}}}