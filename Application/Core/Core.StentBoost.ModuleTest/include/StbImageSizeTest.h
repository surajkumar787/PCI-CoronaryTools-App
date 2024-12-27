// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

class StbImageSizeTest : public StbTestsAllura, public ::testing::WithParamInterface<std::pair<int, int>>
{ };

TEST_P(StbImageSizeTest, GivenIrregularBlackImages_WhenProcessing_TheAlgorithmRemainsStable)
{
	auto size = GetParam();
	auto img = getColouredImage(Black, size.first, size.second);

	Pci::Core::StbAlgoTransform::Transform transform;
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	auto boost = std::make_shared<Pci::Core::StbImage>();
	auto live = std::make_shared<Pci::Core::StbImage>();

	ASSERT_TRUE(stbAlgoMarkers.process(stbParams, img, markers, false, false));
	ASSERT_TRUE(stbAlgoTransform.process(stbParams, img, markers, transform, false));
	ASSERT_FALSE(stbAlgoBoost.boost(stbParams, transform, img, boost, false));
	ASSERT_TRUE(stbAlgoLive.process(stbParams, img, live, false));

	ASSERT_TRUE(stbAlgoMarkers.end());
	ASSERT_TRUE(stbAlgoTransform.end());
	ASSERT_TRUE(stbAlgoBoost.end());
	ASSERT_TRUE(stbAlgoLive.end());
	ASSERT_FALSE(markers->valid);
}

std::pair<int, int> imageSizes[] =
{
	std::make_pair(0, 0),
	std::make_pair(63, 65),
	std::make_pair(69, 1001),
	std::make_pair(451, 64),
	std::make_pair(64, 999),
	std::make_pair(1023, 511),
	std::make_pair(1025, 1024),
	std::make_pair(2049, 2047)
};

INSTANTIATE_TEST_SUITE_P(, StbImageSizeTest, ::testing::ValuesIn(imageSizes),
	[](const ::testing::TestParamInfo<std::pair<int, int>>& info) { return std::to_string(info.param.first) + "_" + std::to_string(info.param.second); });