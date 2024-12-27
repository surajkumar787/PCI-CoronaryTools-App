// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmAlgoFluoroErrorHandlingTest.h

#pragma once
#include <gtest/gtest.h>
#include <algorithm>

#include "CrmAlgoFluoro.h"
#include <CrmRepository.h>
#include "UnitTestLog.h"


//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci {	namespace Test {
using namespace Pci::Core;
using namespace PciSuite;

// This class tests the CrmRepository to storage multiple roadmaps. Different sized roadmap sets will be used.
class CrmRepositoryTestMultipleRoadmaps : public ::testing::TestWithParam<int>
{
public:
	CrmRepositoryTestMultipleRoadmaps()
		: m_log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		m_crmParameters       (new CrmParameters(L"crm.ini")),
        m_crmRepository       (new CrmRepository(m_log, L"")) // create a repository in the subfolder of the test executable's path.	
    {
	}

	~CrmRepositoryTestMultipleRoadmaps()
	{
		m_crmRepository.reset();
		m_crmParameters.reset();
		Simplicity::system().resetHeap();
	}

    void TearDown() override
    {
        ASSERT_TRUE(m_crmRepository->clearRoadmaps());
		Simplicity::system().resetHeap();
    }

    std::shared_ptr<const CrmRoadmap> createRoadmap(int runIndex)
    {
        auto roadmap = std::make_shared<CrmRoadmap>(runIndex, runIndex,1024,1024, 0.1, XrayGeometry(), Sense::Matrix(), 0.0);
        roadmap->overlay=createOverlay();
        roadmap->data.push_back('A'); 
        return roadmap;
    }

    std::shared_ptr<const CrmOverlay> createOverlay()
    {
        auto overlay = std::make_shared<CrmOverlay>();
        overlay->resize(256,256);
        return overlay;
    }

	std::shared_ptr<const CrmRoadmap> loadRoadmapFromFile(int index) { return m_crmRepository->loadRoadmapFromFile(index); }
	CrmErrors::Error saveCrmRoadmapToFile(std::shared_ptr<const CrmRoadmap> roadmap) { return m_crmRepository->saveRoadmapToFile(roadmap); }

private:
	CommonPlatform::Log			  &m_log;
	std::unique_ptr<CrmParameters> m_crmParameters;
	std::unique_ptr<CrmRepository> m_crmRepository;
};

TEST_P(CrmRepositoryTestMultipleRoadmaps, When_Multiple_Roadmap_Are_Saved_Then_They_Can_Be_Loaded)
{
    const int numberOfRoadmapsToTest = GetParam();
    std::vector<int> indices;

    for ( int i=0; i<numberOfRoadmapsToTest ; i++)
    {
        auto roadmap = createRoadmap(i);
        ASSERT_EQ( CrmErrors::Error::OK , saveCrmRoadmapToFile(roadmap));
        indices.push_back(i);
    }

    // Load runs in random order
    std::random_shuffle(indices.begin(), indices.end());

    for(auto run:indices)
    {
        UNITTEST_LOG( L"Loading roadmap " << run );

        auto roadmapLoaded = loadRoadmapFromFile(run);
        ASSERT_NE (roadmapLoaded, nullptr                             );
        ASSERT_NE (roadmapLoaded->overlay, nullptr                    );
        ASSERT_GT (roadmapLoaded->data.size(), 0                      );
        ASSERT_EQ (run, roadmapLoaded->runIndex                       );
    }
}

INSTANTIATE_TEST_SUITE_P(CrmRepositoryTestManyRuns, CrmRepositoryTestMultipleRoadmaps, ::testing::Values(10,200,1025),
	[](const testing::TestParamInfo<int>& info) { return std::to_string(info.param); });

}}

//TICS +OLC#009  magic numbers for a test are ok.