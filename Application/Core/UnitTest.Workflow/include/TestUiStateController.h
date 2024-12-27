// Copyright Koninklijke Philips N.V. 2021
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "TsmSourceStub.h"
#include "TestUiState.h"
#include "MockICopyToRefClient.h"
#include "MockIReplayControl.h"

namespace Pci { namespace Core
{

class TestUiStateController : public TestUiState
{
public:
    TestUiStateController() :
		TestUiState(),
        m_lastUsedSeriesNumber(0)
	{}

    void SetUp() override
    {
        m_uiStateController.reset(new UiStateController(viewContext, scheduler, m_tsm, m_replayControl, m_copyToRefClient, m_userPrefs));
    }

    void createValidStentBoostLiveRun(int numberOfFrames)
    {
        int runIndex = ++m_lastUsedSeriesNumber;
        for (int i = 0; i < numberOfFrames; i++)
        {
            auto result = createStbResult(runIndex, i, 720, 720, true);
            stbAdmin.setImage(result.live, result.markers, result.boost);
        }
    }

protected:
    bool hasAutoReactivateState() const
    {
        return m_uiStateController->m_autoReactivateState != nullptr;
    }

    bool isAutoReactivateStateTimerActive() const
    {
        return m_uiStateController->m_autoSwitchTimer.isActive();
    }


    TsmSourceStub m_tsm;
    testing::NiceMock<MockIReplayControl> m_replayControl;
    testing::NiceMock<MockICopyToRefClient> m_copyToRefClient;
    testing::NiceMock<MockIUserPrefs> m_userPrefs;

	std::unique_ptr<UiStateController> m_uiStateController;

private:
    int m_lastUsedSeriesNumber;
};

TEST_F(TestUiStateController, GivenAutoReactiveActive_When_StudyChanged_AutoReactivateIsCancelled)
{
    createValidStentBoostLiveRun(10);

    m_uiStateController->switchState(ViewState::StbLiveReviewState);

    m_uiStateController->overlayImageAvailable();
    m_uiStateController->lastImageHold();

    EXPECT_TRUE(isAutoReactivateStateTimerActive());
    EXPECT_TRUE(hasAutoReactivateState());

    m_uiStateController->studyChanged();

    EXPECT_FALSE(isAutoReactivateStateTimerActive());
    EXPECT_FALSE(hasAutoReactivateState());
}

TEST_F(TestUiStateController, GivenAutoReactiveActive_When_EpxChanged_AutoReactivateIsCancelled)
{
    createValidStentBoostLiveRun(10);

    m_uiStateController->switchState(ViewState::StbLiveReviewState);

    m_uiStateController->overlayImageAvailable();
    m_uiStateController->lastImageHold();

    EXPECT_TRUE(isAutoReactivateStateTimerActive());
    EXPECT_TRUE(hasAutoReactivateState());

    m_uiStateController->epxChanged();

    EXPECT_FALSE(isAutoReactivateStateTimerActive());
    EXPECT_FALSE(hasAutoReactivateState());
}
}}