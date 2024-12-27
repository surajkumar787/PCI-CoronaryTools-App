// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

namespace CommonPlatform
{
	class Settings;
}

namespace Pci { namespace Core { namespace PACS 
{
	struct ConnectionSettings
	{
	public:
		explicit ConnectionSettings(::CommonPlatform::Settings& settings, const wchar_t* group = L"pacs_connect");

		int MaxPduLength;
		int TimeoutAssocOpen;
		int TimeoutReceiveRequest;
		int TimeoutReceiveResponse;
	};

}}}