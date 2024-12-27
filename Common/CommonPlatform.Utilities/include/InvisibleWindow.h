// Copyright Koninklijke Philips N.V. 2017
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <windows.h>
#include <string>

namespace CommonPlatform { namespace Utilities
{

/**
 *  An invisible top-level window for receiving Windows messages.
 *  Applications can use this class to create an enumerable window and process Windows messages send to it.
 *
 *  For the purpose of processing messages a 'message-only' window would be enough,
 *  however such window cannot be enumerated using the ::EnumWindows() method.
 *
 *  An object of this class can be constructed with an optional user-provided windows procedure,
 *  used for processing messages sent to the window.
 *  If not provided or equal to nullptr, a default procedure will be used.
 */
class InvisibleWindow
{
public:
	/**
	 * Registers a new windows class and creates an invisible window from that class.
	 *
	 * Parameters:
	 *   hinstance: Handle to the instance of the module to be associated with the window.
	 *   classNmae: Name used for the window class and for the window title
	 *   wndProc:   Optional Windows Procedure, used for processing messages sent to the window.
	 */
	InvisibleWindow(HINSTANCE hinstance, std::wstring className, WNDPROC wndProc=nullptr);

	/**
	 * Runs a Windows message loop.
	 * Returns only after processing the WM_QUIT message or in case of an error in processing messages.
	 * Returns the wParam parameter of the WM_QUIT message, or -1 in case of an error.
	 */
	int processMessages() const;
	HWND getHwnd() const;

	/**
	 * Destroys the window and unregister the window class.
	 */
	~InvisibleWindow();

	InvisibleWindow(const InvisibleWindow&) = delete;
	InvisibleWindow& operator=(const InvisibleWindow&) = delete;

private:
	HWND _hwnd;
	HINSTANCE _hinstance;
	std::wstring _className;
};

}}
