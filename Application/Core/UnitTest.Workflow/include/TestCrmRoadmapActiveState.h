// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "TestUiState.h"
#include "CrmRoadmapActive.h"
#include "CrmTsmPage.h"

namespace Pci { namespace Core
{

class TestCrmRoadmapActiveState : public TestUiState
{
public:
	TestCrmRoadmapActiveState() :
		TestUiState()
	{}

	void SetUp() override
	{
		state = std::unique_ptr<CrmRoadmapActive>(new CrmRoadmapActive(viewContext));
	}

	void enterState()
	{
		state->onEnter();
	}

	const CrmTsmPage& getTsmPage()
	{
		return state->getTsmPage();
	}

	const PciGuidanceModel& getModel()
	{
		return state->_guidanceModel;
	}

	void selectRoadmap(int roadmapIndex)
	{
		state->setSelection(roadmapIndex);
	}

	void lockRoadmap(std::shared_ptr<const CrmRoadmap> roadmap)
	{
		const_cast<CrmRoadmap&>(*roadmap).locked = true;
	}

	void pressLockButtonTsm()
	{
		getTsmPage().eventButtonReleased(CrmTsmPage::LockRoadmapButton);
	}

	std::vector<std::shared_ptr<const CrmRoadmap>> createRoadmaps(int numberOfRoadmaps)
	{
		std::vector<std::shared_ptr<const CrmRoadmap>> roadmaps;

		for (int i = 0; i < numberOfRoadmaps; i++)
		{
			roadmaps.emplace_back(new CrmRoadmap());
		}

		return roadmaps;
	}

	const std::wstring STATE_NAME = L"CrmRoadmapActive";

	std::unique_ptr<CrmRoadmapActive> state;
};

TEST_F(TestCrmRoadmapActiveState, StateNameIsCorrect)
{
	EXPECT_EQ(state->getStateName(), STATE_NAME);
}

TEST_F(TestCrmRoadmapActiveState, GivenASingleRoadmapAvailable_WhenEnteringTheState_ThenRoadmapActiveGuidanceIsShown)
{
	const int NR_OF_ROADMAPS(1);

	auto roadmaps = createRoadmaps(NR_OF_ROADMAPS);
	viewContext.currentRoadmap = roadmaps.front();

	EXPECT_CALL(crmAdministration, getRoadmapsForCurrentGeo()).Times(1).WillRepeatedly(testing::Return(roadmaps));

	enterState();

	EXPECT_FALSE(viewContext.crmGuidance.visible);
	EXPECT_TRUE(viewContext.crmRoadmapSelectViewer.visible);
}

TEST_F(TestCrmRoadmapActiveState, GivenMultipleRoadmapAvailable_WhenEnteringTheState_ThenRoadmapActiveGuidanceIsShown)
{
	const int NR_OF_ROADMAPS(2);

	auto roadmaps = createRoadmaps(NR_OF_ROADMAPS);
	viewContext.currentRoadmap = roadmaps.front();

	EXPECT_CALL(crmAdministration, getRoadmapsForCurrentGeo()).Times(1).WillRepeatedly(testing::Return(roadmaps));

	enterState();

	EXPECT_FALSE(viewContext.crmGuidance.visible);
	EXPECT_TRUE(viewContext.crmRoadmapSelectViewer.visible);
}

TEST_F(TestCrmRoadmapActiveState, GivenMultipleRoadmapAvailable_WhenFirstRoadmapSelected_ThenNavigateLeftTSMButtonIsDisabledAndNavigateRightButtonEnabled)
{
	const int NR_OF_ROADMAPS(2);

	auto roadmaps = createRoadmaps(NR_OF_ROADMAPS);
	viewContext.currentRoadmap = roadmaps.front();

	EXPECT_CALL(crmAdministration, getRoadmapsForCurrentGeo()).Times(1).WillRepeatedly(testing::Return(roadmaps));

	enterState();

	selectRoadmap(0);

	EXPECT_FALSE(getTsmPage().navigateLeftButton->enabled);
	EXPECT_TRUE(getTsmPage().navigateRightButton->enabled);
}

TEST_F(TestCrmRoadmapActiveState, GivenMultipleRoadmapAvailable_WhenLastRoadmapSelected_ThenNavigateRightTSMButtonIsDisabledAndNavigateLeftButtonEnabled)
{
	const int NR_OF_ROADMAPS(2);

	auto roadmaps = createRoadmaps(NR_OF_ROADMAPS);
	viewContext.currentRoadmap = *roadmaps.begin();

	EXPECT_CALL(crmAdministration, getRoadmapsForCurrentGeo()).Times(1).WillRepeatedly(testing::Return(roadmaps));

	enterState();

	selectRoadmap(NR_OF_ROADMAPS - 1);

	EXPECT_TRUE(getTsmPage().navigateLeftButton->enabled);
	EXPECT_FALSE(getTsmPage().navigateRightButton->enabled);
}

TEST_F(TestCrmRoadmapActiveState, GivenMultipleRoadmapAvailable_WhenCurrentRoadmapLocked_ThenNavigateButtonsDisabledAndLockedButtonSelected)
{
	const int NR_OF_ROADMAPS(3);

	auto roadmaps = createRoadmaps(NR_OF_ROADMAPS);
	auto currentRoadmap = roadmaps[1];
	lockRoadmap(currentRoadmap);

	viewContext.currentRoadmap = currentRoadmap;

	EXPECT_CALL(crmAdministration, getRoadmapsForCurrentGeo()).Times(1).WillRepeatedly(testing::Return(roadmaps));

	enterState();

	EXPECT_FALSE(getTsmPage().navigateLeftButton->enabled);
	EXPECT_FALSE(getTsmPage().navigateRightButton->enabled);
	EXPECT_TRUE(getTsmPage().lockRoadmapButton->selected);
	EXPECT_EQ(Localization::Translation::IDS_RoadmapLocked, getTsmPage().lockRoadmapButton->centerText);
}

TEST_F(TestCrmRoadmapActiveState, GivenMultipleLockedRoadmapAvailable_WhenUnlockingRoadmap_ThenAllCurrentRoadmapsAreUnlocked)
{
	const int NR_OF_ROADMAPS(3);

	auto roadmaps = createRoadmaps(NR_OF_ROADMAPS);
	auto& lockedRoadmap1 = roadmaps.front();
	auto& lockedRoadmap2 = roadmaps.back();
	lockRoadmap(lockedRoadmap1);
	lockRoadmap(lockedRoadmap2);

	viewContext.currentRoadmap = lockedRoadmap2;

	EXPECT_CALL(crmAdministration, getRoadmapsForCurrentGeo()).Times(1).WillRepeatedly(testing::Return(roadmaps));

	enterState();

	for (auto& roadmap : roadmaps)
	{
		EXPECT_CALL(crmAdministration, unlockRoadmap(roadmap)).Times(1);
	}

	pressLockButtonTsm();
}

TEST_F(TestCrmRoadmapActiveState, GivenLockedRoadmapAvailable_WhenUnlockingRoadmap_ThenNavigateButtonsBecomeAvailable)
{
	const int NR_OF_ROADMAPS(3);

	auto roadmaps = createRoadmaps(NR_OF_ROADMAPS);
	auto& lockedRoadmap1 = roadmaps[1];
	lockRoadmap(lockedRoadmap1);

	viewContext.currentRoadmap = lockedRoadmap1;

	EXPECT_CALL(crmAdministration, getRoadmapsForCurrentGeo()).Times(1).WillRepeatedly(testing::Return(roadmaps));

	enterState();
	pressLockButtonTsm();

	EXPECT_TRUE(getTsmPage().navigateLeftButton->enabled);
	EXPECT_TRUE(getTsmPage().navigateRightButton->enabled);
	EXPECT_FALSE(getTsmPage().lockRoadmapButton->selected);
	EXPECT_EQ(Localization::Translation::IDS_LockRoadmap, getTsmPage().lockRoadmapButton->centerText);
}

TEST_F(TestCrmRoadmapActiveState, GivenLAstLockedRoadmapAvailableAndTotalOfTwo_WhenUnlockingRoadmap_ThenNavigateButtonLeftEnabled)
{
	const int NR_OF_ROADMAPS(2);

	auto roadmaps = createRoadmaps(NR_OF_ROADMAPS);
	auto& lockedRoadmap1 = roadmaps.back();
	lockRoadmap(lockedRoadmap1);

	viewContext.currentRoadmap = lockedRoadmap1;

	EXPECT_CALL(crmAdministration, getRoadmapsForCurrentGeo()).Times(1).WillRepeatedly(testing::Return(roadmaps));

	enterState();
	pressLockButtonTsm();

	EXPECT_TRUE(getTsmPage().navigateLeftButton->enabled);
	EXPECT_FALSE(getTsmPage().navigateRightButton->enabled);
}

TEST_F(TestCrmRoadmapActiveState, GivenFirstLockedRoadmapAvailableAndTotalOfTwo_WhenUnlockingRoadmap_ThenNavigateButtonRightEnabled)
{
	const int NR_OF_ROADMAPS(2);

	auto roadmaps = createRoadmaps(NR_OF_ROADMAPS);
	auto& lockedRoadmap1 = roadmaps.front();
	lockRoadmap(lockedRoadmap1);

	viewContext.currentRoadmap = lockedRoadmap1;

	EXPECT_CALL(crmAdministration, getRoadmapsForCurrentGeo()).Times(1).WillRepeatedly(testing::Return(roadmaps));

	enterState();
	pressLockButtonTsm();

	EXPECT_FALSE(getTsmPage().navigateLeftButton->enabled);
	EXPECT_TRUE(getTsmPage().navigateRightButton->enabled);
}

}}