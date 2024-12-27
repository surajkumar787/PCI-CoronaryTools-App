// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

// prevent deprecated warning on WSAAsyncSelect
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "TcpClient.h"
#include <WinSock2.h>
#include <windows.h>
#include <Sense\System\IScheduler.h>
#include <mstcpip.h>

using namespace CommonPlatform::CoRegistration;

unsigned TcpClient::WM_SOCKET = RegisterWindowMessage(L"TcpClient");

class TcpClient::Static
{
public:
    //This static class is automatically constructed at startup and performs window class registration and winsock initialization.
    Static()
    {
        WNDCLASSEX wndClass = {sizeof(wndClass), 0, &WndProc, 0, 0, GetModuleHandle(NULL), 0, 0, 0, 0, L"TcpClient", 0};
        RegisterClassEx(&wndClass);

        WSADATA wsaData;
        WSAStartup(WINSOCK_VERSION, &wsaData);
    }

    ~Static()
    {
        UnregisterClass(L"TcpClient", GetModuleHandle(NULL));
        WSACleanup();
    }

    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        MSG message = { hWnd, msg, wParam, lParam, 0, 0 };

        if (msg == WM_CREATE) SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
        TcpClient *network = (TcpClient*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (network) network->processMessage(message);

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    static Static instance;
};

TcpClient::Static TcpClient::Static::instance;


TcpClient::TcpClient(const wchar_t *name) : 
    sock		((void*)INVALID_SOCKET),
    hwnd		(NULL),
    port		(0),
    reconnectOnFailure	(false),
    connected   (false)
{
    hwnd = CreateWindow(L"TcpClient", name, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandle(NULL), this);
}


TcpClient::~TcpClient()
{
    disconnect(); //TODO
    DestroyWindow(hwnd);
}

void TcpClient::processMessage( const MSG &msg )
{
    if (msg.message == WM_SOCKET && msg.wParam == (WPARAM)sock) switch(WSAGETSELECTEVENT(msg.lParam))
    {
    case FD_CONNECT:
        if (WSAGETSELECTERROR(msg.lParam) != 0)
        {
            if(eventConnectionChanged) eventConnectionChanged(false);
            socketReconnect();
        }
        else
        {
            if(eventConnectionChanged) eventConnectionChanged(true);
        }

        break;
    case FD_READ:
        if(eventDataAvailable)
            eventDataAvailable();
        break;
    case FD_WRITE:
        try{
            flush();
        }
        catch (SocketException &)
        {
            socketReconnect();
        }
        break;
    case FD_CLOSE:
        socketReconnect();
        break;
    }

    if (msg.message == WM_TIMER && msg.wParam == (WPARAM)this)
    {
        KillTimer(hwnd, (UINT_PTR)this);
        if (sock == (void*)INVALID_SOCKET && !host.empty() && port != 0 && reconnectOnFailure) socketOpen();
    }
}

bool TcpClient::socketOpen()
{
    socketClose();

    sock = (void*)::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == (void*)INVALID_SOCKET) return false;

    if (::WSAAsyncSelect((SOCKET)sock, hwnd, WM_SOCKET, FD_READ|FD_WRITE|FD_CLOSE|FD_CONNECT) != 0) return false;

    int flag = 1;
    ::setsockopt((SOCKET)sock, IPPROTO_TCP, TCP_NODELAY,  (char*)&flag, sizeof(flag));
    ::setsockopt((SOCKET)sock, SOL_SOCKET,  SO_KEEPALIVE, (char*)&flag, sizeof(flag));

    // set keepalive timeout to 2 seconds.
    DWORD bytes = 0;
    tcp_keepalive keepalive;
    keepalive.onoff				= 1;
    keepalive.keepalivetime		= 2000; 
    keepalive.keepaliveinterval = 2000; 

    if (::WSAIoctl((SOCKET)sock, SIO_KEEPALIVE_VALS, &keepalive, sizeof(keepalive),	NULL, 0, &bytes, NULL, NULL) != 0) return false;

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    char address[1024];
    sprintf_s(address, "%S", host.c_str());

    addr.sin_family		 = AF_INET;
	addr.sin_addr.s_addr = inet_addr(address);
    addr.sin_port		 = htons(USHORT(port));

    if (::connect((SOCKET)sock, (SOCKADDR*)&addr, sizeof(addr)) != 0) if (WSAGetLastError() !=  WSAEWOULDBLOCK) return false;


    return true;
}

void TcpClient::socketClose()
{
    KillTimer(hwnd, (UINT_PTR)this);

    sendLength	= 0;

    if (sock != (void*)INVALID_SOCKET)
    {
        closesocket((SOCKET)sock);
        sock = (void*)INVALID_SOCKET;

		if(eventConnectionChanged) eventConnectionChanged(false);
    }
}

void TcpClient::socketReconnect()
{
    socketClose();
    if (reconnectOnFailure) SetTimer(hwnd, (UINT_PTR)this, 1000, NULL);	
}

void TcpClient::send(const char *data, int length)
{
    if (sock == (void*)INVALID_SOCKET) 
        throw SocketException();

    if (sendLength + length > sendData.size())  
        sendData.resize((sendLength + length) * 2);

    if (data)
    { 
        memcpy(sendData.data() + sendLength, data, length);
        sendLength += length;			
    }

    flush();
}

void TcpClient::flush()
{
    if (sock == (void*)INVALID_SOCKET)	
        throw SocketException();

    if (sendLength <= 0) 
        return;

    // send data
    int result = ::send((SOCKET)sock, sendData.data(), sendLength, 0);
    if (result == SOCKET_ERROR)
	{
		auto err = WSAGetLastError();
		if (err == WSAENOTCONN || err == WSAECONNABORTED || err == WSAECONNRESET)
		{
			if(eventConnectionChanged) eventConnectionChanged(false);
		}
        throw SocketException(err);
	}
    //flush data
    sendLength -= result;
    if (sendLength > 0) 
        memmove(sendData.data(), sendData.data() + result, sendLength);
}

int TcpClient::receive(char* buffer, int length)
{
    auto retVal = ::recv((SOCKET) sock, buffer, length, 0);
	if (retVal == SOCKET_ERROR)
	{
		auto err = WSAGetLastError();
		if (err == WSAENOTCONN || err == WSAECONNABORTED || err == WSAECONNRESET)
		{
			if(eventConnectionChanged) eventConnectionChanged(false);
		}
	}
	return retVal;
}

bool TcpClient::connect( const std::wstring &Host, int Port )
{
    port		= Port;
    host		= Host;
    reconnectOnFailure	= true;

    return socketOpen();
}

void TcpClient::disconnect()
{
    socketClose();

    port		= 0;
    host		= L"";
    reconnectOnFailure	= false;
}

void TcpClient::reconnect()
{
    socketReconnect();
}
