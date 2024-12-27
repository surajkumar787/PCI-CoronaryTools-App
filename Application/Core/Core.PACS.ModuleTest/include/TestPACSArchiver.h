// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <gtest/gtest.h>
#include <Sense.h>

#include "PACSArchiver.h"
#include "MockIPACSPush.h"
#include "MockIMessageBuilder.h"
#include "Thread.h"
#include "MockIScheduler.h"

using namespace Pci::Core::PACS;

class TestPACSArchiver : public ::testing::Test
{
public:

	TestPACSArchiver() : 
		scheduler(),
		mockPacsPush(),
		thread(scheduler, "WorkerThread"),
		mockMsgBuilder()
	{
	}

    void SetUp() override
	{
        EXPECT_CALL(scheduler, delAsync(::testing::Ref(thread))).Times(1);
        ON_CALL(scheduler, addAsync(testing::_, testing::_)).WillByDefault(testing::WithArg<1>(testing::Invoke([](std::function<void()> func) {func(); })));
	}

	std::shared_ptr<Snapshot> createSnapshot(int width, int height)
	{
		const uint32_t BLUE_COLOR(0x002DFF);
		std::vector<uint32_t> imageData(width * height, BLUE_COLOR);

		return PACS::Snapshot::fromRawData(width, height, PACS::Format::Rgba32, reinterpret_cast<char*>(imageData.data()), imageData.size() * sizeof(imageData.front()));
	}

	MetaData createMetaData()
	{
		CommonPlatform::Xray::XrayStudy study;
		CommonPlatform::Xray::XrayGeometry geometry;

		return MetaData(study, geometry, 0, 0);
	}

	~TestPACSArchiver()
	{
	}

protected:
	MockIScheduler scheduler;
	testing::StrictMock<MockIPACSPush> mockPacsPush;
	::CommonPlatform::Thread thread;
	testing::StrictMock<MockIMessageBuilder> mockMsgBuilder;
};

TEST_F(TestPACSArchiver, WhenSendSnapshotCalledWithNullptrImage_ThenSnapshotShouldNotBeArchived)
{
	const std::shared_ptr<Snapshot> SNAPSHOT(nullptr);
	const int SERIES_NUMBER(100);

	PACSArchiver archiver(scheduler, mockPacsPush, mockMsgBuilder, thread);
	EXPECT_CALL(mockPacsPush, sendSnapshot(::testing::_, ::testing::_, ::testing::_)).Times(0);

	archiver.sendSnapshot(SNAPSHOT, SERIES_NUMBER, RunType::UNKNOWN);
	thread.flush();
}

TEST_F(TestPACSArchiver, WhenSendSnapshotCalledWithEmptySnapshot_ThenSnapshotShouldNotBeArchived)
{
	const std::shared_ptr<Snapshot> SNAPSHOT(createSnapshot(0,0));
	const int SERIES_NUMBER(100);

	PACSArchiver archiver(scheduler, mockPacsPush, mockMsgBuilder, thread);
	EXPECT_CALL(mockPacsPush, sendSnapshot(::testing::_, ::testing::_, ::testing::_)).Times(0);

	archiver.sendSnapshot(SNAPSHOT, SERIES_NUMBER, RunType::UNKNOWN);
	thread.flush();
}

TEST_F(TestPACSArchiver, WhenSendSnapshotCalledWithValidSnapshot_ThenSnapshotShouldBeArchived)
{
	const std::shared_ptr<Snapshot> SNAPSHOT(createSnapshot(100, 100));
	const int SERIES_NUMBER(100);
	const MetaData META_DATA(createMetaData());   

	auto msg = std::make_shared<CommonPlatform::Dicom::Merge::Message>("", C_STORE_RQ);

	PACSArchiver archiver(scheduler, mockPacsPush, mockMsgBuilder, thread);

	testing::InSequence sequence;
	EXPECT_CALL(mockMsgBuilder, setSeriesNumber(SERIES_NUMBER)).Times(1);  
	EXPECT_CALL(mockMsgBuilder, getMetaData()).Times(1).WillOnce(testing::Return(META_DATA));
	EXPECT_CALL(mockMsgBuilder, CreateSnapshotMessage(testing::_, *SNAPSHOT, RunType::UNKNOWN)).Times(1).WillOnce(testing::Return(msg));
	EXPECT_CALL(mockPacsPush, sendSnapshot(::testing::_, ::testing::_, ::testing::_)).Times(1);

	archiver.sendSnapshot(SNAPSHOT, SERIES_NUMBER, RunType::UNKNOWN);
	thread.flush();
}

TEST_F(TestPACSArchiver, WhenSendMovieCalledAndStartMovieReturnNullptr_ThenOnSentFalseIsCalled)
{
    std::vector<std::shared_ptr<Snapshot>> movieImages;

    for (int i = 0; i < 10; i++)
    {
        movieImages.push_back(createSnapshot(100, 100));
    }

    const int SERIES_NUMBER(100);
    const MetaData META_DATA(createMetaData());
  
    auto msg = std::make_shared<CommonPlatform::Dicom::Merge::Message>("", C_STORE_RQ);

    PACSArchiver archiver(scheduler, mockPacsPush, mockMsgBuilder, thread);

    const int SUCCESS = 0;
    const int FAILURE = 1;
    auto result = -1;
    auto grabTypeResult = GrabType::SingleFrame;
    archiver.onSent = [&](GrabType grabType, bool finishedSuccessfully)
    {
        result = finishedSuccessfully ? SUCCESS : FAILURE;
        grabTypeResult = grabType;
    };

    testing::InSequence sequence;
    EXPECT_CALL(mockMsgBuilder, setSeriesNumber(SERIES_NUMBER)).Times(1);  
   EXPECT_CALL(mockMsgBuilder, getMetaData()).Times(1).WillOnce(testing::Return(META_DATA));
    EXPECT_CALL(mockMsgBuilder, StartMovie(testing::_, 5, 100, 100,RunType::UNKNOWN)).Times(1).WillOnce(testing::Return(nullptr));

    EXPECT_CALL(scheduler, addAsync(testing::Ref(archiver), testing::_)).Times(1);

    archiver.sendMovie(movieImages, 5, SERIES_NUMBER, RunType::UNKNOWN);
    thread.flush();

    EXPECT_EQ(FAILURE, result);
    EXPECT_EQ(GrabType::MultiFrame, grabTypeResult);
}

TEST_F(TestPACSArchiver, WhenSendMovieCalledWithValudFrames_ThenMovieIsSent)
{
    const int FRAME_WIDTH(100);
    const int FRAME_HEIGHT(100);

    std::vector<std::shared_ptr<Snapshot>> movieImages;

    for (int i = 0; i < 10; i++)
    {
        movieImages.push_back(createSnapshot(FRAME_WIDTH, FRAME_HEIGHT));
    }

    const int SERIES_NUMBER(100);   
    const MetaData META_DATA(createMetaData());
    const int FRAMES_PER_SECOND(5);
   
    auto msg = std::make_unique<CommonPlatform::Dicom::Merge::Message>("", C_STORE_RQ);
    auto imageSequence = std::make_shared<ImageSequence>(META_DATA, std::move(msg), FRAMES_PER_SECOND, FRAME_WIDTH, FRAME_HEIGHT);

    PACSArchiver archiver(scheduler, mockPacsPush, mockMsgBuilder, thread);

    const int SUCCESS = 0;
    const int FAILURE = 1;
    auto result = -1;
    auto grabTypeResult = GrabType::SingleFrame;
    archiver.onSent = [&](GrabType grabType, bool finishedSuccessfully)
    {
        result = finishedSuccessfully ? SUCCESS : FAILURE;
        grabTypeResult = grabType;
    };

    testing::InSequence sequence;
    EXPECT_CALL(mockMsgBuilder, setSeriesNumber(SERIES_NUMBER)).Times(1); 
    EXPECT_CALL(mockMsgBuilder, getMetaData()).Times(1).WillOnce(testing::Return(META_DATA));
    EXPECT_CALL(mockMsgBuilder, StartMovie(testing::_, 5, 100, 100, RunType::UNKNOWN)).Times(1).WillOnce(testing::Return(imageSequence));
    for (auto& frame : movieImages)
    {
        EXPECT_CALL(mockMsgBuilder, AddFrame(testing::Ref(*imageSequence), testing::Ref(*frame))).Times(1).WillOnce(testing::Return(true));
    }

    EXPECT_CALL(mockMsgBuilder, EndMovie(testing::Ref(*imageSequence))).Times(1).WillOnce(testing::Return(true));
    EXPECT_CALL(mockPacsPush, sendMovie(::testing::_, ::testing::_, ::testing::_)).Times(1).WillOnce(testing::WithArg<2>(testing::Invoke([](std::function<void(bool success)> func) {func(true); })));

    EXPECT_CALL(scheduler, addAsync(testing::Ref(archiver), testing::_)).Times(1);
 
    archiver.sendMovie(movieImages, 5, SERIES_NUMBER, RunType::UNKNOWN);
    thread.flush();

    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(GrabType::MultiFrame, grabTypeResult);
}