// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.


#include "WindowKeyHandler.h"
#include "windef.h"


namespace Pci {	namespace PciApp {

static const DWORD KeyDown = 0x8000;
HHOOK keyEventCallBackHook = NULL; // hook handle

WindowKeyHandler::WindowKeyHandler(bool disableAltTab) 
{
	if(disableAltTab)
	{
		registerKeyEventHook();
	}
}


WindowKeyHandler::~WindowKeyHandler()
{
	if( keyEventCallBackHook != nullptr)
	{
		UnhookWindowsHookEx(keyEventCallBackHook);
	}
}

LRESULT CALLBACK CallBackHook(int hookCode, WPARAM wParm, LPARAM lParm)
{
	KBDLLHOOKSTRUCT *keyBoardData = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParm);

	if( hookCode == HC_ACTION  )
	{
		if( keyBoardData->vkCode == VK_ESCAPE )
		{
			short key = GetAsyncKeyState(VK_SHIFT);

			if((key & KeyDown) != 0)
			{
				return 1;
			}
			key = GetAsyncKeyState(VK_CONTROL);
			if((key & KeyDown) != 0)
			{
				return 1;
			}

		}
		else if (keyBoardData->vkCode == VK_TAB && keyBoardData->flags & LLKHF_ALTDOWN)
		{
			return 1;
		}
	}
	return CallNextHookEx(keyEventCallBackHook, hookCode, wParm, lParm);
}


void WindowKeyHandler::registerKeyEventHook()
{
	keyEventCallBackHook = SetWindowsHookEx(((int)WH_KEYBOARD_LL),CallBackHook, NULL,0);
}


}}