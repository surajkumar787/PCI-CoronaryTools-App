// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "MessageBuilderTests.h"

namespace Constants = ::CommonPlatform::Dicom::Constants;

static Merge::Service DummyService()
{
	Merge::SyntaxList syntaxList({ IMPLICIT_LITTLE_ENDIAN });
	return Merge::Service(Merge::sopClassUidToserviceName(Constants::SopClass::SecondaryCaptureImageStorage), syntaxList, Merge::Service::Role::BOTH);
}

class SnapshotSyntaxTests : public MessageBuilderTest,
							public ::testing::WithParamInterface<TRANSFER_SYNTAX>
{};

TEST_P(SnapshotSyntaxTests, CheckValidTransferSyntax)
{
	const int imgSize = 123;

	std::unique_ptr<CommonPlatform::Dicom::CharsetManager> charsetManager(new CommonPlatform::Dicom::CharsetManager());
	std::vector<std::string> characterSets = { CommonPlatform::Dicom::Constants::ISO_IR_100 };
	charsetManager->setPossibleCharacterset(characterSets);

	Merge::SyntaxList syntaxList({ GetParam() });
	Merge::Service snapshots(Merge::sopClassUidToserviceName(Constants::SopClass::SecondaryCaptureImageStorage), syntaxList, Merge::Service::Role::SCU);
	PACS::MessageBuilder msgBuilder(snapshots, DummyService(), nullptr, std::move(charsetManager));

	auto msg = msgBuilder.CreateSnapshotMessage(GenerateMetaData(), *GenerateImage(imgSize, imgSize),RunType::UNKNOWN);

	EXPECT_TRUE(msg);
	ValidateMessage(*msg);
}

class MovieSyntaxTests : public MessageBuilderTest,
						 public ::testing::WithParamInterface<TRANSFER_SYNTAX>
{};

TEST_P(MovieSyntaxTests, CheckValidTransferSyntax)
{
	const int imgSize = 79;
	const int frameRate = 17;
	const int frames = 3;
	
	std::unique_ptr<CommonPlatform::Dicom::CharsetManager> charsetManager(new CommonPlatform::Dicom::CharsetManager());
	std::vector<std::string> characterSets = { CommonPlatform::Dicom::Constants::ISO_IR_100 };
	charsetManager->setPossibleCharacterset(characterSets);

	Merge::SyntaxList syntaxList({ GetParam() });
	Merge::Service movies(Merge::sopClassUidToserviceName(Constants::SopClass::MultiFrameSecondaryCaptureTrueColor), syntaxList, Merge::Service::Role::SCU);
	PACS::MessageBuilder msgBuilder(DummyService(), movies, nullptr, std::move(charsetManager));

	auto snapshot = GenerateImage(imgSize, imgSize);
	auto sequence = msgBuilder.StartMovie(GenerateMetaData(), frameRate, imgSize, imgSize,RunType::UNKNOWN);

	EXPECT_TRUE(sequence);

	for (int i = 0; i < frames; i++)
	{
		EXPECT_TRUE(msgBuilder.AddFrame(*sequence, *snapshot));
	}

	EXPECT_TRUE(msgBuilder.EndMovie(*sequence));
	ValidateMessage(*sequence->m_message);
}

INSTANTIATE_TEST_SUITE_P(Snapshots, SnapshotSyntaxTests, ::testing::ValuesIn(Constants::SnapshotSyntaxList));
INSTANTIATE_TEST_SUITE_P(Movies, MovieSyntaxTests, ::testing::ValuesIn(Constants::MovieSyntaxList));
