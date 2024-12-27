// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <winsock2.h>
#include <iphlpapi.h>
#include <iostream>
#include <stdio.h>

#include "DvlpReceiverMonitor.h"

namespace CommonPlatform {	namespace Xray {

DvlpReceiverMonitor::DvlpReceiverMonitor()
: dvlpAdapter(0)
{
}

DvlpReceiverMonitor::~DvlpReceiverMonitor()
{
}

bool DvlpReceiverMonitor::open(uint32_t adapter, uint16_t port)
{
	return open(adapter, port, 0);
}

bool DvlpReceiverMonitor::open(uint32_t adapter, uint16_t port, int bufferSize)
{
    dvlpAdapter = adapter;
    return Dvlp::Receiver::open(adapter, port, bufferSize);
}
	

bool DvlpReceiverMonitor::open(uint32_t group, uint32_t adapter, uint16_t port)
{
	return open(group, adapter, port, 0);
}

bool DvlpReceiverMonitor::open(uint32_t group, uint32_t adapter, uint16_t port, int bufferSize)
{
    dvlpAdapter = adapter;
    return Dvlp::Receiver::open(group, adapter, port, bufferSize);
}

DvlpReceiverMonitor::DvlpConnectionStatus DvlpReceiverMonitor::checkConnection()
{
    if ( !Dvlp::Receiver::isOpen() ) return DvlpConnectionStatus::NotConnectedSinceOpeningSocketFailed;
    if ( dvlpAdapter != 0 )          return getStatusOfAdapter(dvlpAdapter);
    return DvlpConnectionStatus::NotConnectedSinceNoAdapterDefined;
}

DvlpReceiverMonitor::DvlpConnectionStatus DvlpReceiverMonitor::getStatusOfAdapter(uint32_t address) const
{
    DvlpConnectionStatus connectionStatus = DvlpConnectionStatus::NotConnectedSinceNoAdapterDefined;

    ULONG size = 0;
    if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, nullptr, &size) != ERROR_BUFFER_OVERFLOW || size == 0) return connectionStatus;

    std::vector<char> buffer(size);
    if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data()), &size) == NO_ERROR)
    {
        IP_ADAPTER_ADDRESSES *adapter = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());

        while (adapter != nullptr)
        {
            IP_ADAPTER_UNICAST_ADDRESS* pUnicastAddresses = adapter->FirstUnicastAddress;

            while (pUnicastAddresses != nullptr) // for each assigned IP address
            {
                // Convert big endian (of network byte order) to little endian.
				sockaddr_in* sockedAdress = reinterpret_cast<struct sockaddr_in*>(pUnicastAddresses->Address.lpSockaddr);
                uint32_t adapterAddress = _byteswap_ulong((sockedAdress)->sin_addr.S_un.S_addr);

                if ( adapterAddress == address )
                {
					if (adapter->OperStatus == IfOperStatusUp)
					{
						connectionStatus = DvlpConnectionStatus::Connected;
					}
					else
					{
						connectionStatus = DvlpConnectionStatus::NotConnectedSinceNoCable;
					}
                }
                pUnicastAddresses = pUnicastAddresses->Next;
            }
            adapter = adapter->Next;
        }
    }
   
    return connectionStatus;
}
}}