
// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.


#include "gtest/gtest.h"
#include "MessageTester.h"

#include "MainModel.h"
#include "UserMouseKeyboardMonitor.h"
#include "LicensesCollectionStub.h"

#include <Sense.h>
#include <ExperienceIdentity.h>

#include <memory>

using namespace Sense;
using namespace Sense::ExperienceIdentity;

using namespace Pci::Core;

class TestArchivingMessages : public ::testing::Test
{
public:
	LicenseCollectionStub					  licenseCollection;
	std::unique_ptr<MainModel>                mainModel;

	std::unique_ptr<Scheduler>			      scheduler;
	std::unique_ptr<Gpu>				      gpu;
	std::unique_ptr<Canvas>                   canvas;
	std::unique_ptr<UserMouseKeyboardMonitor> user;
	std::unique_ptr<OutputScreen>             screen;

	static bool showUI, manual, review;

	void SetUp() override
	{
		Test::SetUp();

		scheduler = std::make_unique<Scheduler>();
		gpu       = std::make_unique<Gpu>(0);
		canvas    = std::make_unique<Canvas>(*gpu, *scheduler);
		user      = std::make_unique<UserMouseKeyboardMonitor>(*canvas, true);
		mainModel = std::make_unique<MainModel>(licenseCollection, CommonPlatform::Xray::SystemType::Allura);

		if (showUI)
		{
			screen = std::make_unique<OutputScreen>(*canvas, 0);
			screen->text = L"Coronary Tools";
			screen->image = Icons::HeartSegmentation;
			screen->setFullscreen(false);
			gpu->show();
		}
	}
};

bool TestArchivingMessages::showUI = false;
bool TestArchivingMessages::manual = false;
bool TestArchivingMessages::review = false;

TEST_F(TestArchivingMessages, AllMessagesFit)
{
	MessageTester::Config config { TestArchivingMessages::manual, TestArchivingMessages::review };
	MessageTester test(*scheduler, *canvas, *mainModel, config, user.get());

	auto numFailures = scheduler->run();

	ASSERT_EQ(numFailures, 0) << "Some tests failed.\n"
		                         "Please re-run the test with command line parameters 'showUI review' to inspect the failures.";
}
