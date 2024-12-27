// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmAlgoFluoroErrorHandlingTest.h

#pragma once
#include <gtest/gtest.h>

#include "CrmAlgoFluoro.h"
#include <CrmRepository.h>
#include "UnitTestLog.h"

//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci {	namespace Test {
using namespace Pci::Core;
using namespace PciSuite;

// This class tests the CrmRepository to handles run storage with high index numbers.
// Because the run number is used in the internal filename.
class CrmRepositoryTestRunIndex : public ::testing::TestWithParam<int>
{
public:
	CrmRepositoryTestRunIndex()
    :   m_log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		m_crmParameters       (new CrmParameters(L"crm.ini")),
        m_crmRepository       (new CrmRepository(m_log, L"")) // create a repository in the subfolder of the test executable's path.	
    {
	}

	~CrmRepositoryTestRunIndex()
	{
		m_crmRepository.reset();
		m_crmParameters.reset();
		Simplicity::system().resetHeap();
	}

    void TearDown() override
    {
        ASSERT_TRUE( m_crmRepository->clearRoadmaps());
    }

    bool persistentFileExists() const
    {
        DWORD attr = GetFileAttributes( m_crmRepository->getPersistentDataFilePath().c_str() );
        return ( INVALID_FILE_ATTRIBUTES != attr );
    }

    std::shared_ptr<const CrmRoadmap> createRoadmap(int runIndex)
    {
        auto roadmap = std::make_shared<CrmRoadmap>(runIndex,runIndex,1024,1024, 0.1, XrayGeometry(), Sense::Matrix(), 0.0);
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

    std::wstring getExpectedFileName(int runIndex) const
    {
        wchar_t formattedNumber[256];
        swprintf_s(formattedNumber, L"%.3d", runIndex);

        std::wstringstream strm;
        strm << L"roadmap_" << formattedNumber << L".crm.fxd" ;
        return strm.str();
    }

    bool fileExists( const std::wstring& filename) const
    {
        DWORD attr = GetFileAttributes( filename.c_str() );
        return ( INVALID_FILE_ATTRIBUTES != attr );
    }

	std::wstring getRepositoryPath() { return m_crmRepository->getPath(); };
	std::shared_ptr<const CrmRoadmap> loadRoadmapFromFile(int index) { return m_crmRepository->loadRoadmapFromFile(index); }
	CrmErrors::Error saveCrmRoadmapToFile(std::shared_ptr<const CrmRoadmap> roadmap) { return m_crmRepository->saveRoadmapToFile(roadmap); }

private:
	CommonPlatform::Log			  &m_log;
	std::unique_ptr<CrmParameters> m_crmParameters;
	std::unique_ptr<CrmRepository> m_crmRepository;
};

TEST_P(CrmRepositoryTestRunIndex, When_Roadmap_With_High_Index_Is_Saved_Then_It_Can_Be_Loaded)
{
    auto roadmap = createRoadmap(GetParam());

    UNITTEST_LOG( L"Test roadmap with index " << roadmap->runIndex );

    ASSERT_EQ( CrmErrors::Error::OK , saveCrmRoadmapToFile(roadmap));

    std::wstringstream strm;
    strm << getRepositoryPath() << getExpectedFileName(roadmap->runIndex);
    ASSERT_TRUE ( fileExists( strm.str() ));

    auto roadmapLoaded = loadRoadmapFromFile(roadmap->runIndex);
    ASSERT_NE (roadmapLoaded, nullptr                             );
    ASSERT_NE (roadmapLoaded->overlay, nullptr                    );
    ASSERT_GT (roadmapLoaded->data.size(), 0                      );
    ASSERT_EQ (roadmapLoaded->data.size(), roadmap->data.size()   );
    ASSERT_EQ (roadmap->runIndex, roadmapLoaded->runIndex         );
}

INSTANTIATE_TEST_SUITE_P(CrmRepositoryTestHighRunNumbers, CrmRepositoryTestRunIndex,
	::testing::Values(200, 999, 1000, 1001, 1023, 1024, 1025, 2345, 99999),
	[](const testing::TestParamInfo<int>& info) { return std::to_string(info.param); });

}}
//TICS +OLC#009  magic numbers for a test are ok.