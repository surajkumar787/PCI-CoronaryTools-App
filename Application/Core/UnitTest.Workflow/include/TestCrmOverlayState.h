// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "TestUiState.h"
#include "CrmOverlayState.h"
#include "CrmTsmPage.h"
#include "MockIUiStateController.h"
#include "MockICopyToRefClient.h"
#include "MockIArchivingSettings.h"
#include "MockIPACSArchiver.h"
#include "PACSArchiver.h"

namespace Pci { namespace Core
{

class TestCrmOverlayState : public TestUiState
{
public:
	TestCrmOverlayState() :
		TestUiState(),
		uiStateController()
	{}

	void SetUp() override
	{
		state = std::unique_ptr<CrmOverlayState>(new CrmOverlayState(viewContext, uiStateController));
		state->setArchivingStatusSink(this);
		state->initPACSArchiving(std::unique_ptr<PACS::IPACSArchiver>(new PACS::MockIPACSArchiver()));
	}

	void enterState()
	{
		state->onEnter();
	}

	void archiveMovie()
	{
		state->getTsmPage().onButtonRelease(state->getTsmPage().MovieButton);
	}

	void gotoRoadmapOptions()
	{
		state->getTsmPage().onButtonRelease(state->getTsmPage().RoadmapSelectionButton);
	}

	void failMovieArchive()
	{
		state->onMovieGrabberFinished(false, nullptr);
	}

	void lastImageHold()
	{
		state->lastImageHold();
	}

	PACS::MockIPACSArchiver& getMockArchiver() const
	{
		return *static_cast<PACS::MockIPACSArchiver*>(state->m_pacsArchiver.get());
	}

	const CrmTsmPage& getTsmPage()
	{
		return state->getTsmPage();
	}

	const std::wstring STATE_NAME = L"CrmOverlayState";

	MockUiStateController uiStateController;
	std::unique_ptr<CrmOverlayState> state;
};

TEST_F(TestCrmOverlayState, StateNameIsCorrect)
{
	EXPECT_EQ(state->getStateName(), STATE_NAME);
}

TEST_F(TestCrmOverlayState, GivenArchiveMovieTriggerd_WhenArchivingSuccessful_ThenMovieButtonIsDisabled)
{
	const int MAX_FRAMES_IN_RECORDING(10);
	auto recording = std::make_shared<CrmRecording>(MAX_FRAMES_IN_RECORDING);

	testing::InSequence sequence;
	EXPECT_CALL(crmAdministration, getRecording()).Times(1).WillOnce(testing::Return(recording));

	enterState();
	lastImageHold();

	EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);

	archiveMovie();

	EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);
}

TEST_F(TestCrmOverlayState, GivenArchiveMovieTriggerd_WhenArchivingFailed_ThenMovieButtonIsEnabledAgain)
{
	const int MAX_FRAMES_IN_RECORDING(10);
	auto recording = std::make_shared<CrmRecording>(MAX_FRAMES_IN_RECORDING);

	testing::InSequence sequence;
	EXPECT_CALL(crmAdministration, getRecording()).Times(1).WillOnce(testing::Return(recording));

	enterState();
	lastImageHold();

	EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);

	archiveMovie();

	EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);

	failMovieArchive();

	EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);
}

TEST_F(TestCrmOverlayState, GivenLastImageHold_WhenMovieNotAvailable_ThenMovieButtonIsDisabled)
{
	viewContext.moviesAvailable = false;

	enterState();
	lastImageHold();

	EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);
}

TEST_F(TestCrmOverlayState, GivenLastImageHold_WhenOneRoadmapAvailable_ThenRoadmapOptionsIsEnabled)
{
    auto roadmap = std::make_shared<const CrmRoadmap>();
    std::vector<std::shared_ptr<const CrmRoadmap>> roadmaps;
    roadmaps.push_back(roadmap);
    
    EXPECT_CALL(crmAdministration, getRoadmapsForCurrentGeo).Times(1).WillOnce(testing::Return(roadmaps));

    enterState();
    lastImageHold();

    EXPECT_TRUE(getTsmPage().roadmapOptionsButton->enabled);
}

TEST_F(TestCrmOverlayState, GivenLastImageHold_WhenTwoRoadmapsAvailable_ThenRoadmapOptionsIsEnabled)
{
    auto roadmap1 = std::make_shared<const CrmRoadmap>();
    auto roadmap2 = std::make_shared<const CrmRoadmap>();
    std::vector<std::shared_ptr<const CrmRoadmap>> roadmaps;
    roadmaps.push_back(roadmap1);
    roadmaps.push_back(roadmap2);

    EXPECT_CALL(crmAdministration, getRoadmapsForCurrentGeo).Times(1).WillOnce(testing::Return(roadmaps));

    enterState();
    lastImageHold();

    EXPECT_TRUE(getTsmPage().roadmapOptionsButton->enabled);
}

TEST_F(TestCrmOverlayState, GivenLastImageHold_WhenRoadmapOptionsTSMButtonClicked_ThenApplicationsSwitchesToRoadmapActiveState)
{
	enterState();
	lastImageHold();

	EXPECT_CALL(uiStateController, switchState(ViewState::CrmRoadmapActive)).Times(1);

	gotoRoadmapOptions();
}

}}