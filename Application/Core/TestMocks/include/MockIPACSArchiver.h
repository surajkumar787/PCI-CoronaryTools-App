// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gmock\gmock.h>
#include "IPACSArchiver.h"


namespace Pci { namespace Core { namespace PACS
{
	class MockIPACSArchiver : public IPACSArchiver
	{
	public:
		MockIPACSArchiver() = default;
		~MockIPACSArchiver() = default;

		MOCK_METHOD3(sendSnapshot, void(std::shared_ptr<Snapshot>, int, RunType));		
		MOCK_METHOD4(sendMovie, void(std::vector<std::shared_ptr<Snapshot>>, int, int,RunType));
	};
}}}
