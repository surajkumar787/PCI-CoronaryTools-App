#include "gtest/gtest.h"
#include "Sense/System/Scheduler.h"
#include "CoRegistrationAdministration.h"
#include "CoRegistrationServerState.h"
#include "CoRegistrationControl.h"
#include <memory>

using namespace Pci::Core;
using namespace CommonPlatform::CoRegistration;

class TestPressureDisplay : public ::testing::Test
{
public:

	Scheduler scheduler;
	Gpu		  gpu;
	Canvas	  canvas; 
	Mouse	  mouse;
	Keyboard  keyboard;

	PressureDisplay display;


	TestPressureDisplay() :
		gpu(0),
		canvas(gpu, scheduler),
		mouse(canvas),
		keyboard(canvas),
		display(canvas)
	{
	}

	~TestPressureDisplay()
	{
	}

	void SetUp() override
	{
		Test::SetUp();
	}

};

TEST_F(TestPressureDisplay, TestSubsampling)
{

}