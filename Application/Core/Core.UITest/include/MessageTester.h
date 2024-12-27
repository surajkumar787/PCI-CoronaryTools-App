
#include <memory>

namespace Sense
{
	class Scheduler;
	namespace ExperienceIdentity { class Canvas; }
};

namespace Pci { namespace Core {
	class MainModel;
}}

class UserMouseKeyboardMonitor;

struct MessageTester
{
	struct Config
	{
		bool manualStepping; // Single step through all test cases
		bool reviewFailures; // Single step through failed cases
	};

	MessageTester(Sense::Scheduler& scheduler, Sense::ExperienceIdentity::Canvas& canvas,
				  Pci::Core::MainModel& model, Config config, UserMouseKeyboardMonitor* user);
	~MessageTester();

	struct Private;
	std::unique_ptr<Private> m_p;
};
