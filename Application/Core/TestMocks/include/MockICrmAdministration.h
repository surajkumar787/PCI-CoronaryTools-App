// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gmock\gmock.h>

#include "ICrmAdministration.h"

class MockICrmAdministration : public Pci::Core::ICrmAdministration
{
public:
	MockICrmAdministration() = default;
	~MockICrmAdministration() = default;

	MOCK_CONST_METHOD0(getRecording, std::shared_ptr<CrmRecording>());
	MOCK_CONST_METHOD0(getRoadmapsForCurrentGeo, std::vector<std::shared_ptr<const CrmRoadmap>>());

	MOCK_METHOD1(selectRoadmap, void(std::shared_ptr<const CrmRoadmap>&));
	MOCK_METHOD1(lockRoadmap, void(std::shared_ptr<const CrmRoadmap>&));
	MOCK_METHOD1(unlockRoadmap, void(std::shared_ptr<const CrmRoadmap>&));
};