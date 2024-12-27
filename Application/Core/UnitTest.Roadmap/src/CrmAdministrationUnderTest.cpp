// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "CrmAdministrationUnderTest.h"
#include <windows.h>


namespace Pci {	namespace Test {
#undef min
#undef max

CrmAdministrationUnderTest::CrmAdministrationUnderTest( CommonPlatform::Log& log, const Pci::Core::CrmParameters &params, Pci::Core::CrmRepository &repository)
: CrmAdministration(log, params,repository)
{
}

CrmAdministrationUnderTest::~CrmAdministrationUnderTest(void)
{
}

bool CrmAdministrationUnderTest::persistentViewCreated() const
{
    return ( m_persistentView != nullptr);
}

// Invalidation principle should be the same as in CrmAdministration
void CrmAdministrationUnderTest::updatePersistent()
{
    if (m_persistentView == nullptr) return;

    // Make data invalid while we're updating....
	m_persistentView->version         = -1;

    int roadmapCount = std::min(1024, static_cast<int>(m_roadmaps.size()));
    
    wchar_t buffer[MAX_PATH];
    DWORD dword = 0;
    auto envVarLength = GetEnvironmentVariable(L"CrmAdministration_throw_exception_in_updatePersistent", buffer, dword);
	if (envVarLength > 0)
	{
		throw std::exception("Test exception in updatePersistent");
	}

	m_persistentView->runIndex		 = getPersistentRunIndex();
	m_persistentView->roadmapCount   = roadmapCount;
	m_persistentView->version        = Pci::Core::Persistent::Version;
}
}}