// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Socket.h"

#include "Log.h"


namespace CommonPlatform { namespace Dicom { namespace TLS
{
	static const int millisecondsInASecond = 1000;
	static const int MicroSecondsInMilliSecond = 1000;

	Socket::Socket(const SOCKET& s) :
		m_thePlainSocket(s),
		m_errorCode(0),
		m_log(::CommonPlatform::Logger(LoggerType::Platform))
	{
	}

	//Transmits bufferSize amount of data. Returns false if it couldn't
	bool Socket::send(const LPCSTR& buffer, const int& bufferSize, const int& flags, const int& timeout) const
	{
		int totalBytesSent = 0;
		int bytesSent      = 0;

		// Transmit the whole buffer contents
		bool continueSending = false;
		do
		{
			if(!waitUntilReady(timeout, SocketOperation::Send))
			{
				//Wait failed
				return false;
			}
			// Try to transmit the remaining buffer contents
			bytesSent = ::send(m_thePlainSocket, &buffer[totalBytesSent], bufferSize - totalBytesSent, flags);

			// Determine the outcome of the send attempt
			if (bytesSent == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				m_log.DeveloperError(L"TLS::Socket: send error %d", error);
				setLastError(TLS_SOCKET_ERROR);
				return false;
			}

			// Keep track of the total number of bytes transmitted.
			totalBytesSent += bytesSent;
			continueSending = totalBytesSent < bufferSize;

		} while (continueSending);

		return true;
	}

	int Socket::receive(LPSTR buffer, const int& bufferSize, const int& flags, const int& timeout) const
	{
		int bytesRead = 0;
		int totalBytesRead = 0;
		// Wait till data is available for reading
		if(!waitUntilReady(timeout, SocketOperation::Receive))
		{
			//Error in wating
			return 0;
		}

		bytesRead = ::recv(m_thePlainSocket, &buffer[totalBytesRead], bufferSize - totalBytesRead, flags);

		// Determine the outcome of the read operation
		switch (bytesRead)
		{
		case SOCKET_ERROR:
			{
				// Socket operation failed
				int error = WSAGetLastError();
				m_log.DeveloperError(L"TLS::Socket: receive error %d", error);
				setLastError(TLS_SOCKET_ERROR);
			}
			return 0;

		case 0:
			// Socket is disconnected gracefully
			setLastError(TLS_SOCKET_DISCONNECTED);
			return 0;

		default:
			// Success
			return bytesRead;
		}
	}

	//
	// Abstract  : Wait till the socket is ready for transmission of data
	// Pre       : The socket is connected
	// Post      : Returns true or false to indicate success or failure
	// Exceptions: None
	//
	bool Socket::waitUntilReady(const int timeout, const SocketOperation operation) const
	{
		int             outCome = 0;
		fd_set          socketList;

		// Initialize the socket list
		socketList.fd_count    = 1;
		socketList.fd_array[0] = m_thePlainSocket;

		// Is a timeout value specified ?
		if (timeout < 0)
		{
			if (operation == SocketOperation::Send)
			{
				//Write 
				outCome = ::select(0, nullptr, &socketList, nullptr, nullptr);
			}
			else
			{
				//Read
				outCome = ::select(0, &socketList, nullptr, nullptr, nullptr);
			}
		}
		else
			// When no timeout is specified, wait forever till the socket is ready
		{
			// Set the timeval structure
			timeval         time;
			time.tv_sec = static_cast<long>(timeout / millisecondsInASecond);
			time.tv_usec = static_cast<long>(timeout % millisecondsInASecond)*MicroSecondsInMilliSecond;

			// Wait for the specified amount of time
			if (operation == SocketOperation::Send)
			{
				//Write 
				outCome = ::select(0, nullptr, &socketList, nullptr, &time);
			}
			else
			{
				//Read
				outCome = ::select(0, &socketList, nullptr, nullptr, &time);
			}
		}

		// Determine the outcome of the select statement
		switch (outCome)
		{
		case SOCKET_ERROR:
			// An error occurred on the controlled socket
			setLastError(TLS_SOCKET_ERROR);
			return false;

		case 0:
			// A timeout occurred
			m_log.DeveloperError(L"TLS::Socket: timeout waiting for %s operation", operation == SocketOperation::Receive ? L"read.": L"write.");
			setLastError(TLS_SOCKET_TIMEOUT);
			return false;

		default:
			// The operation succeeded
			return true;
		}

	}


	int Socket::getLastError() const
	{
		return m_errorCode;
	}

	void Socket::setLastError(int errorCode) const
	{
		m_errorCode = errorCode;

		switch(m_errorCode)
		{
		case TLS_SOCKET_ERROR:
			m_log.DeveloperError(L"TLS::Socket: socket error");
			break;

		case TLS_SOCKET_TIMEOUT:
			m_log.DeveloperError(L"TLS::Socket: socket timeout");
			break;

		case TLS_SOCKET_DISCONNECTED:
			m_log.DeveloperError(L"TLS::Socket: socket disconnected");
			break;

		default:
			m_log.DeveloperError(L"TLS::Socket: unknown error (%d)", errorCode);
			break;
		}
	}
}}}