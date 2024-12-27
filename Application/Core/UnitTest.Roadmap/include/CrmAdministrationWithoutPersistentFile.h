// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <gtest/gtest.h>

#include <CrmAdministration.h>
#include <CrmRepository.h>

namespace Pci {	namespace Test {
using namespace Pci::Core;
//TICS -OLC#009  magic numbers for a test are ok.
// This class tests the roadmap list administration of the CrmAdministration in case the persistent functionality fails.
class CrmAdministrationWithoutPersistentFile : public ::testing::Test
{
public:

    CrmAdministrationWithoutPersistentFile()
		: log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		crmParameters(new CrmParameters(L"crm.ini")),// make sure to use same parameters as application
		crmRepository(new CrmRepository(log, L"")),
        lastChangedRoadmapRunNumber (-1),
        lastChangedRoadmapStatus    (CrmAdministration::Status::NoRoadmap),
        validRoadmapAvailable       (false)
    {
        crmParameters->storeRoadmapFolder = L"";
    }

	~CrmAdministrationWithoutPersistentFile()
	{
		// All Memory must be disposed here so the memory leak detector in the test will not report false positive results. 
		m_crmAdmin.reset();
		crmRepository.reset();
		crmParameters.reset();
		Simplicity::system().resetHeap();
	}

    void SetUp() override
    {
        CrmRepository::deletePersistentData(log,crmParameters->storeRoadmapFolder);    
        ASSERT_FALSE ( persistentFileExists() );

        // open to lock access to prevent creation of persistent file
        preventPersistentFile.open(crmRepository->getPersistentDataFilePath().c_str(), std::ios::binary); 

        m_crmAdmin.reset( new CrmAdministrationUnderTest(log, *crmParameters, *crmRepository));

        m_crmAdmin->onRoadmapStatusChanged = [this] 
        { 
            CrmAdministration::Status status = getCrmStatus();
            const auto &roadmap              = getCrmRoadmap();

            if ( roadmap )
            {
                lastChangedRoadmapRunNumber  = roadmap->runIndex;
                lastChangedRoadmapStatus = status; 
                validRoadmapAvailable    = true;
            }
            else
            {
                lastChangedRoadmapRunNumber  = -1;
                lastChangedRoadmapStatus = CrmAdministration::Status::NoRoadmap;
                validRoadmapAvailable    = false;
            }
        };
    }

    void TearDown() override
    {
        // close to unlock access
        preventPersistentFile.close(); 
    }

	CommonPlatform::Xray::XrayGeometry createXrayGeometry(double angulation, double rotation)
    {
		CommonPlatform::Xray::XrayGeometry geometry;
        geometry.angulation = angulation;
        geometry.rotation   = rotation;
        geometry.updateSpaces();
        return geometry;
    }

	CommonPlatform::Xray::XrayStudy createStudy(const std::wstring& id, const std::wstring& name) const
    {
		CommonPlatform::Xray::XrayStudy study;
        study.studyId       = id;
        study.patientName   = name;
        return study;
    }

    std::shared_ptr<const CrmRoadmap> createRoadmap(int runIndex)
    {
        auto roadmap = std::make_shared<CrmRoadmap>(runIndex,runIndex,1024,1024, 0.1, XrayGeometry(), Sense::Matrix(), 0.0);
        roadmap->overlay=createOverlay();
        roadmap->data.push_back('A'); 
        return roadmap;
    }

    std::shared_ptr<const CrmRoadmap> createRoadmap(int runIndex, const XrayGeometry& geometry)
    {
        auto roadmap = std::make_shared<CrmRoadmap>(runIndex, runIndex,1024,1024, 0.1, geometry, Sense::Matrix(), 0.0);
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

    bool persistentFileExists()
    {
        DWORD attr = GetFileAttributes( crmRepository->getPersistentDataFilePath().c_str() );
        return ( INVALID_FILE_ATTRIBUTES != attr );
    }

	std::shared_ptr<const CrmRoadmap> getCrmRoadmap() { return m_crmAdmin->getRoadmap(); };
	CrmAdministration::Status getCrmStatus() { return m_crmAdmin->getStatus(); };
	int getCrmPersistentRunIndex() const { return m_crmAdmin->getPersistentRunIndex(); };
	int getLastChangedRoadmapRunNumber() const { return lastChangedRoadmapRunNumber; };
	int getCrmPersistentDataVersion() { return m_crmAdmin->getPersistentDataVersion(); };
	void setCrmGeometry(XrayGeometry& geomrtry) { m_crmAdmin->setGeometry(geomrtry); };
	void setCrmStudy(XrayStudy& study) { m_crmAdmin->setStudy(study); }
	void endCrmGeometry() { m_crmAdmin->endGeometry(); };
	void addCrmRoadmap(std::shared_ptr<const CrmRoadmap> roadmap) { m_crmAdmin->addRoadmap(roadmap); };
	void endCrmLive() { return m_crmAdmin->endLive(); };
	bool isCrmEmpty() const { return m_crmAdmin->isEmpty(); };
	bool isValidRoadmapAvailable() const { return validRoadmapAvailable; };
	bool crmPersistentViewCreated() { return m_crmAdmin->persistentViewCreated(); };

private:
	CommonPlatform::Log							&log;
	std::unique_ptr<CrmParameters>				crmParameters;
	std::unique_ptr<CrmRepository>				crmRepository;
	std::unique_ptr<CrmAdministrationUnderTest> m_crmAdmin;
	int											lastChangedRoadmapRunNumber;
	CrmAdministration::Status					lastChangedRoadmapStatus;
	bool										validRoadmapAvailable;
	std::ofstream								preventPersistentFile;
};

TEST_F(CrmAdministrationWithoutPersistentFile, When_TestFixture_Started_Then_PersistentFile_Is_Not_Created)
{
    ASSERT_FALSE ( crmPersistentViewCreated() ); // pre-condition for this test fixture
}

TEST_F(CrmAdministrationWithoutPersistentFile, When_Initialized_Then_No_Roadmaps_Available)
{
    ASSERT_TRUE  ( isCrmEmpty()               );
    ASSERT_FALSE ( crmPersistentViewCreated() );
}

TEST_F(CrmAdministrationWithoutPersistentFile, When_Roadmap_Is_Added_For_Current_XrayGeometry_Then_Status_Becomes_Active)
{
    auto initialXrayGeometry = createXrayGeometry  (20,30); initialXrayGeometry.tableHeight = 104;
    auto roadmap             = createRoadmap       (5, initialXrayGeometry);

    setCrmGeometry(initialXrayGeometry);
    addCrmRoadmap( roadmap );

    ASSERT_FALSE ( crmPersistentViewCreated() );

    const auto &current = getCrmRoadmap();

    // Check status of roadmap administration
    ASSERT_TRUE  ( isValidRoadmapAvailable()                                                );
    ASSERT_FALSE ( isCrmEmpty()                                                             );
    ASSERT_EQ    ( roadmap->runIndex, current->runIndex                                     );
    ASSERT_EQ    ( roadmap->mmPerPixel, current->mmPerPixel.x                               );
    ASSERT_EQ    ( roadmap->mmPerPixel, current->mmPerPixel.y                               );
    ASSERT_EQ    ( initialXrayGeometry.tableHeight, current->geometry.tableHeight           );
    ASSERT_EQ    ( 0, getCrmPersistentRunIndex()                                            );
    ASSERT_EQ    ( CrmAdministration::Status::Active, getCrmStatus()                 );
}

TEST_F(CrmAdministrationWithoutPersistentFile, When_Roadmap_Is_Added_Then_Persistent_File_Is_Updated_After_Temporarely_Invalidation)
{
    auto roadmap             = createRoadmap       (5);

    ASSERT_FALSE ( crmPersistentViewCreated() );

    int currentVersion = getCrmPersistentDataVersion();
    const int InvalidVersion = -1;

    ASSERT_EQ    ( InvalidVersion, getCrmPersistentDataVersion()   );
    ASSERT_TRUE  ( isCrmEmpty()                                    );

    EXPECT_NO_THROW( addCrmRoadmap(roadmap));

    ASSERT_FALSE ( crmPersistentViewCreated() );

    ASSERT_EQ    ( currentVersion, getCrmPersistentDataVersion()   );
    ASSERT_FALSE ( isCrmEmpty()                                    );
}

TEST_F(CrmAdministrationWithoutPersistentFile, When_New_Study_Started_Then_Administration_Is_Reset)
{
    auto study1 = createStudy(L"1", L"Dummy1");
    auto study2 = createStudy(L"2", L"Dummy2");
	
	setCrmStudy(study1);
    ASSERT_FALSE ( crmPersistentViewCreated() );

    // Simulate that 6 incoming runs are handled.
    const int numberOfFluoroRuns = 6;
    for ( int i=0; i<numberOfFluoroRuns ; i++ ) endCrmLive();

    auto roadmap = std::make_shared<CrmRoadmap>(1,1,1024,1024, 0.1, CommonPlatform::Xray::XrayGeometry(), Sense::Matrix(), 0.0);
    addCrmRoadmap(roadmap);

    ASSERT_FALSE ( isCrmEmpty()                  );
    ASSERT_TRUE  ( isValidRoadmapAvailable()                );
    ASSERT_EQ    (numberOfFluoroRuns, getCrmPersistentRunIndex() );

	setCrmStudy(study2);

    ASSERT_TRUE  ( isCrmEmpty()                  );
    ASSERT_FALSE ( isValidRoadmapAvailable()                );
    ASSERT_EQ    ( 0, getCrmPersistentRunIndex() );
}

TEST_F(CrmAdministrationWithoutPersistentFile, When_Geometry_Moved_Towards_AngioPosition_Then_Corresponding_Roadmap_Is_Selected)
{
    auto xrayGeometry1 = createXrayGeometry(10,20);
    auto xrayGeometry2 = createXrayGeometry(-10,30);

    int runNumber1 = 1; auto roadmap1 = createRoadmap(1, xrayGeometry1);
    int runNumber2 = 2; auto roadmap2 = createRoadmap(2, xrayGeometry2);

    addCrmRoadmap(roadmap1);
    addCrmRoadmap(roadmap2);

    ASSERT_FALSE ( isCrmEmpty()       );

    setCrmGeometry(xrayGeometry1);
    ASSERT_EQ( runNumber1, getLastChangedRoadmapRunNumber());

    setCrmGeometry(xrayGeometry2);
    ASSERT_EQ( runNumber2, getLastChangedRoadmapRunNumber());

    setCrmGeometry(xrayGeometry1);
    endCrmGeometry();
    ASSERT_EQ( runNumber1, getLastChangedRoadmapRunNumber());

    setCrmGeometry(xrayGeometry2);
    endCrmGeometry();
    ASSERT_EQ( runNumber2, getLastChangedRoadmapRunNumber());
}
}}
//TICS +OLC#009  magic numbers for a test are ok.