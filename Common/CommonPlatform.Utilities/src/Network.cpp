// Copyright Koninklijke Philips N.V. 2016
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Network.h"

#include <winsock2.h>
#include <windows.h>
#include <Iphlpapi.h>
#include "stlstring.h"

#include <net/interface.h>

namespace CommonPlatform { namespace Utilities
{
	std::wstring Network::GetHostName()
	{
		char hostname[255];

		gethostname(hostname, sizeof(hostname));

		return str_ext::stow(hostname);
	}

	bool Network::WaitForNetworkAdapterUpdate(const int timeOut)
	{
		OVERLAPPED overlap;
		DWORD ret;

		HANDLE hand = NULL;
		overlap.hEvent = WSACreateEvent();

		ret = NotifyAddrChange(&hand, &overlap);

		bool result(false);
		if (ret == NO_ERROR || WSAGetLastError() == WSA_IO_PENDING)
		{
			if (WaitForSingleObject(overlap.hEvent, timeOut) == WAIT_OBJECT_0)
			{
				result = true;
			}
		}

		WSACloseEvent(overlap.hEvent);

		return result;
	}

	Net::IpAddress Network::ResolveNetwork(const std::string& adapter, int timeoutinMilliseconds)
	{
		do
		{
			const auto interfaces = Net::Interface::listInterfaces(adapter);
			for (const auto& ip : *interfaces)
			{
				if (ip.address().family() != Net::IpAddress::Family::IPv4) continue;

				if (!ip.address().isLinkLocal())
				{
					return ip.address();
				}
			}

			if (Net::IpAddress(adapter).isLoopback()) return Net::IpAddress::loopback();

		} while (WaitForNetworkAdapterUpdate(timeoutinMilliseconds));

		return Net::IpAddress::any();
	}

	bool Network::WaitForInterfaceUp(const std::string& network, int timeoutInMilliseconds)
	{
		do
		{
			auto foundInterface = Net::Interface::findInterface(network);
			if (foundInterface.type() == Net::Interface::Type::Ethernet)
			{
				return true;
			}

		} while (WaitForNetworkAdapterUpdate(timeoutInMilliseconds));

		return false;
	}
}}
