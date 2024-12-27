// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <functional>
#include <string>

namespace CommonPlatform { namespace CoRegistration{


class ITcpClient
{
public:
	virtual ~ITcpClient() {}

    class SocketException : public std::exception 
    {
    public:
        SocketException() : exception("Socket exception"), Error(-1) {}
		SocketException(int errorCode) : exception("Socket exception"), Error(errorCode) {}

		int Error;
    };

    virtual bool connect(const std::wstring &host, int port) = 0;
    virtual void reconnect()  = 0;
    virtual void disconnect() = 0;

    virtual int  receive(char* buffer, int length)      = 0;
    virtual void send(const char* buffer, int length)   = 0;

    std::function<void()> eventDataAvailable;
    std::function<void(bool isConnected)> eventConnectionChanged;
};
}}