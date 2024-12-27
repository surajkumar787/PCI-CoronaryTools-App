// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <Sense/System/UserMouseKeyboard.h>

// This class adds mouse movement detection and mouse pointer hiding functionality to the Sense::UserMouseKeyboard class.
// Note that CWIS mouse model updates are also mapped onto windows mouse events by an external CwisMouse process and therefore detected by this class.
class UserMouseKeyboardMonitor : Sense::UserMouseKeyboard
{
public:
    UserMouseKeyboardMonitor(Sense::ICanvas &canvas, bool defaultVisibility);

	std::function<void(int, unsigned int)> onKeyPress;

	UserMouseKeyboardMonitor(const UserMouseKeyboardMonitor&) = delete;
	UserMouseKeyboardMonitor& operator=(const UserMouseKeyboardMonitor&) = delete;

protected:
    virtual void virtualMouseMove	(const Sense::Point &pos) override;
	virtual void virtualKeyPress(int vkey, unsigned int scancode) override;

private:
    int visibilityDelay;
    Sense::Timer mouseActivityTimer;
    Sense::IMouse& mouse;
    bool defaultMouseVisibility;
    void onActivityTimeElapsed();
    void showMouse();
    void hideMouse();
};

