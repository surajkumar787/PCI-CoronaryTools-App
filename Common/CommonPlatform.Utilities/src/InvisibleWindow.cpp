// Copyright Koninklijke Philips N.V. 2017
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "InvisibleWindow.h"

namespace CommonPlatform { namespace Utilities
{

/**
 *  The default procedure handles a WM_DESTROY message by posting a WM_QUIT message to the message queue,
 *  which exits the message loop. Other messages are forwarded to DefWindowProc()
 */
static LRESULT CALLBACK DefaultProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

InvisibleWindow::InvisibleWindow(HINSTANCE hinstance, std::wstring className, WNDPROC proc):
	_hinstance(hinstance),
	_className(className)
{
	WNDCLASSEX wcex = {};
	wcex.cbSize        = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc   = proc ? proc : DefaultProcedure;
	wcex.hInstance     = _hinstance;
	wcex.lpszClassName = _className.c_str();
	RegisterClassEx(&wcex);

	_hwnd = CreateWindow(
		_className.c_str(),  // Name of window class
		nullptr,             // title-bar string
		WS_OVERLAPPEDWINDOW, // top-level window
		CW_USEDEFAULT,       // default horizontal position
		CW_USEDEFAULT,       // default vertical position
		CW_USEDEFAULT,       // default width
		CW_USEDEFAULT,       // default height
		nullptr,             // no owner window
		nullptr,             // use class menu
		_hinstance,          // handle to application instance
		nullptr);            // no window-creation data
}

InvisibleWindow::~InvisibleWindow()
{
	DestroyWindow(_hwnd);
	UnregisterClass(_className.c_str(), _hinstance);
}

int InvisibleWindow::processMessages() const
{
	BOOL ret;
	MSG msg;

	while((ret = GetMessage(&msg, NULL, 0, 0 )) != 0)
	{
		if (ret)
		{
			ret = TranslateMessage(&msg);
			ret &= DispatchMessage(&msg);
		}
		else
		{
			// Error processing messages
			return -1;
		}
	}
	return ret ? static_cast<int> (msg.wParam) : -1;
}

HWND InvisibleWindow::getHwnd() const
{
	return _hwnd;
}

}}
