// Copyright Koninklijke Philips N.V. 2024
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "net.h"
#include "Log.h"
#include "stlstring.h"
#include "CopyToRefClient.h"
#include <Sense.h>
#include <net.h>

namespace Pci {
	namespace PciApp {
		class NetworkIP
		{
			Sense::Timer	m_timer;

			std::string resolveIPAddress(const std::string& adapterName);
			void stopTimer();

		public:
			NetworkIP(Sense::IScheduler& scheduler);
			~NetworkIP();
			void getIPAdreess(const std::string& adapterName);

			std::function<void(const std::string&)>eventIPObtained;
		};
	}
}