// Copyright Koninklijke Philips N.V. 2016
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>

namespace Net { class IpAddress; };

namespace CommonPlatform { namespace Utilities
{
	class Network
	{
	public:
		static std::wstring GetHostName();
		static bool WaitForNetworkAdapterUpdate(int timeOut);
		static Net::IpAddress ResolveNetwork(const std::string& adapter, int timeoutInMilliseconds);
		static bool WaitForInterfaceUp(const std::string& network, int timeoutInMilliseconds);
	};
	
	
}}