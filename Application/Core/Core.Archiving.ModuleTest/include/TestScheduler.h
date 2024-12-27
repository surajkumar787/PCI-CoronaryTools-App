// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#define _WINSOCKAPI_ 
#include "windows.h"
#include <Sense/System/Scheduler.h>

// This class implements a message pump that can be used to dispatch messages/events coming in on the current thread
class TestScheduler : public Sense::Scheduler
{
public:

    // Process all events until the message queue is empty
    // Other test/support will use this to process all events I.e. both Cwis and Async events.
    // The Sense processMessages is needed to process event coming from dvlp source. Because dvlp source uses addAsync()
    void processMessages()
    {
        Sense::Scheduler::processMessages();
    }

    // Process all event for a given time
    void processMessagesWithTimeOut(unsigned int timeout)
    {
        int periods =  ( static_cast<int>(timeout)/ m_retryTime);

        for (int i = 0; i < periods; i++)
        {
            Sleep(m_retryTime);
            processMessages();
        }
    }

    // Wait forever on one event
    bool waitForEvent(HANDLE event)
    {
        while(true)
        {
            if (WaitForSingleObject(event, 0) == WAIT_OBJECT_0) return true;
            Sleep(m_retryTime);
            processMessages();
		}
        return false;
    }

    // Wait for one event, that should be triggered without a certain time
    bool processMessagesAndWaitForEventWithTimeOut(HANDLE event, unsigned int timeout)
    {
        int periods = (static_cast<int>(timeout) / m_retryTime);

        for (int retry = 0; retry < periods; retry++)
        {
            if (WaitForSingleObject(event, 0) == WAIT_OBJECT_0) return true;
            Sleep(m_retryTime);
            processMessages();
		}
        return false;
    }


    // Wait for one event, that should be triggered without a certain time
    bool processMessagesAndWaitForEventsWithTimeOut(HANDLE event1, HANDLE event2, unsigned int timeout)
    {

        int periods = (static_cast<int>(timeout) / m_retryTime);

        HANDLE eventhandles[] = { event1, event2 };
        for (int retry = 0; retry < periods; retry++)
        {
            if (WaitForMultipleObjects(2, eventhandles, true, 0) == WAIT_OBJECT_0) return true;
            Sleep(m_retryTime);
            processMessages();
		}
        return false;
    }

private:
	static const int m_retryTime = 16;
};