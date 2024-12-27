// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

class StbPixelSizeTest : public StbTestsAllura, public ::testing::WithParamInterface<Sense::Point>
{};

TEST_P(StbPixelSizeTest, GivenExtremeMmPerPixelValues_WhenProcessing_TheAlgorithmRemainsStable)
{
	auto img = getColouredImage(Black);
	img->mmPerPixel = GetParam();

	Pci::Core::StbAlgoTransform::Transform transform;
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	auto boost = std::make_shared<Pci::Core::StbImage>();
	auto live = std::make_shared<Pci::Core::StbImage>();

	stbAlgoMarkers.process(stbParams, img, markers, false, false);
	ASSERT_TRUE(stbAlgoTransform.process(stbParams, img, markers, transform, false));
	ASSERT_FALSE(stbAlgoBoost.boost(stbParams, transform, img, boost, false));
	stbAlgoLive.process(stbParams, img, live, false);

	stbAlgoMarkers.end();
	stbAlgoTransform.end();
	stbAlgoBoost.end();
	stbAlgoLive.end();
	ASSERT_FALSE(markers->valid);
}

Sense::Point pixelSizes[] =
{
	Sense::Point(0.1),
	Sense::Point(0.5),
	Sense::Point(0.9),
	Sense::Point(1.0),
	Sense::Point(100000.0),
	Sense::Point(0.0, 0.1),
	Sense::Point(0.1, 0.0),
	Sense::Point(0.0),
	Sense::Point(-1.0),
	Sense::Point(-5.0, 1.0),
	Sense::Point(std::numeric_limits<double>::quiet_NaN()),
	Sense::Point(std::numeric_limits<double>::signaling_NaN()),
	Sense::Point(std::numeric_limits<double>::infinity()),
	Sense::Point(-std::numeric_limits<double>::infinity()),
	Sense::Point(std::numeric_limits<double>::min()),
	Sense::Point(std::numeric_limits<double>::max())
};

// non-alphanumeric (like '.') is not allowed in test names so these are hard to represent
INSTANTIATE_TEST_SUITE_P(, StbPixelSizeTest, ::testing::ValuesIn(pixelSizes));