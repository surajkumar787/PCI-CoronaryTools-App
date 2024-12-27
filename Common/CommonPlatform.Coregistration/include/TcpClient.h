// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "ITcpClient.h"
#include <vector>
#include <functional>

//trick to prevent including <windows.h>
#ifndef _WINDEF_
    struct tagMSG;  typedef tagMSG   MSG;
    struct HWND__;  typedef HWND__  *HWND;
#endif

namespace CommonPlatform { namespace CoRegistration{


class TcpClient : public ITcpClient
{
public:
    TcpClient(const wchar_t *name);
    ~TcpClient();

    bool connect(const std::wstring &host, int port) override;
    void reconnect()  override;
    void disconnect() override;

    int  receive(char* buffer, int length) override;
    void send   (const char* buffer, int length) override;

private:
    void processMessage(const MSG &msg);

    bool socketOpen();			//!< Begins attempting to connect to the server
    void socketClose();			//!< Disconnects from the server
    void socketReconnect();		//!< Reconnects to the server

    void receive();

    void flush();

    unsigned					sendLength;
    std::vector<char>			sendData;

    void					   *sock;
    HWND						hwnd;
    std::wstring				host;
    int							port;
    bool                        reconnectOnFailure;

    static unsigned				WM_SOCKET;

    bool connected;
    
    class Static;

};
}}
