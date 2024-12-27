// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "UserMouseKeyboardMonitor.h"

UserMouseKeyboardMonitor::UserMouseKeyboardMonitor(Sense::ICanvas &canvas, bool defaultVisibility)
:   
    UserMouseKeyboard           (canvas),
    visibilityDelay             (10), /* sec */
    mouseActivityTimer          (canvas.getScheduler(), [this] { mouseActivityTimer.stop(); onActivityTimeElapsed(); }),
    mouse                       (canvas.getMouse()), 
    defaultMouseVisibility      (defaultVisibility)
{
    if ( !defaultVisibility ) hideMouse();
}

void UserMouseKeyboardMonitor::onActivityTimeElapsed()
{
    hideMouse();
}

void UserMouseKeyboardMonitor::showMouse()
{
    mouse.enableCursor(true);
}
void UserMouseKeyboardMonitor::hideMouse()
{
    mouse.enableCursor(false);
}

void UserMouseKeyboardMonitor::virtualMouseMove (const Sense::Point & pos )
{
    UserMouseKeyboard::virtualMouseMove(pos);
    showMouse();
    if (!defaultMouseVisibility) mouseActivityTimer.start(visibilityDelay);
}

void UserMouseKeyboardMonitor::virtualKeyPress(int vkey, unsigned int scancode)
{
	UserMouseKeyboard::virtualKeyPress(vkey, scancode);
	if (onKeyPress) onKeyPress(vkey, scancode);
}
