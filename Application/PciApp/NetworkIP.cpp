// Copyright Koninklijke Philips N.V. 2024
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "NetworkIP.h"

namespace Pci {
	namespace PciApp {
		const int retry_time = 2;

		NetworkIP::NetworkIP(Sense::IScheduler& scheduler) :
			m_timer(scheduler, nullptr)
		{}

		NetworkIP::~NetworkIP()
		{
			stopTimer();
		}

		void NetworkIP::getIPAdreess(const std::string& adapterName)
		{
			stopTimer();

			const auto beginTime = clock();

			m_timer.eventTick = [&, adapterName, beginTime]() {
				const auto ip = resolveIPAddress(adapterName);
				if (!ip.empty() && eventIPObtained) {
					m_timer.stop();
					auto& log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);
					log.DeveloperInfo(L"Obtained Hospital Network IP : %s, time taken for that : %f sec", (str_ext::stow(ip)).c_str(), (static_cast<float>(clock() - beginTime) / CLOCKS_PER_SEC));
					eventIPObtained(ip);
				}
			};

			m_timer.start(retry_time);
		}

		void NetworkIP::stopTimer()
		{
			if (m_timer.isActive())
			{
				m_timer.stop();
			}
		}

		std::string NetworkIP::resolveIPAddress(const std::string& adapterName)
		{
			auto& log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);
			log.DeveloperInfo(L"NetworkIP::resolveIPAddress()");
			std::string ip;
			const auto ifs = Net::Interface::listInterfaces();
			auto result = Net::IpAddress::loopback();
			bool found = false;

			for (auto& i : *ifs)
			{
				log.DeveloperInfo(L"Network Interface: %s", (str_ext::stow(i.name())).c_str());

				if (i.address().family() == Net::IpAddress::Family::IPv6) continue;

				if (adapterName == i.name())
				{
					log.DeveloperInfo(L"Hospital Network found");

					const Net::UdpSocket udpSocket(i.address());
					if (udpSocket)
					{
						log.DeveloperInfo(L"UDP socket successfully created");
						found = true;
						result = i.address();
						ip = result.toString();
						break;
					}
					log.DeveloperError(L"UDP socket creation failed");
				}
				else
				{
					log.DeveloperError(L"Hospital Network not found");
				}
			}

			if (ip.empty())
			{
				log.DeveloperError(L"Binding to Hospital Network failed");
				log.DeveloperInfo(L"Retrying...");
			}

			return ip;
		}
	}
}