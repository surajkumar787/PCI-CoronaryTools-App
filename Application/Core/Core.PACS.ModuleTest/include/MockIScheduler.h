// Copyright Koninklijke Philips N.V. 2021
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gmock\gmock.h>
#include <Sense/System/IScheduler.h>


namespace Pci {
    namespace Core {
        namespace PACS {

            class MockIScheduler : public Sense::IScheduler
            {
            public:
                MockIScheduler() = default;
                ~MockIScheduler() = default;

                // ITimerScheduler
                MOCK_METHOD2(startTimer, void(Sense::Timer &timer, double interval));
                MOCK_METHOD2(startTimer, void(Sense::Timer &timer, Priority priority));
                MOCK_METHOD1(stopTimer, void(Sense::Timer &timer));

                // IScheduler
                MOCK_METHOD2(addAsync, void(const Sense::IAsync &owner, const std::function<void()> &command));
                MOCK_METHOD1(delAsync, void(const Sense::IAsync &owner));

                MOCK_METHOD2(addEvent, void(HANDLE object, const std::function<void()> &command));
                MOCK_METHOD1(delEvent, void(HANDLE object));

                MOCK_CONST_METHOD0(isUserInterfaceThread, bool());
            };
        }
    }
}