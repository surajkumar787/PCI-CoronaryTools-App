// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <gtest/gtest.h>

#include <CrmAdministration.h>
#include <CrmRepository.h>

//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci {	namespace Test {
using namespace Pci::Core;

// This class implements tests the roadmap list administration of the CrmAdministration class.
class CrmAdministrationRoadmapTest : public ::testing::Test
{
public:

	CrmAdministrationRoadmapTest()
    :   log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		crmParameters(new CrmParameters(L"crm.ini")),// make sure to use same parameters as application
		crmRepository(new CrmRepository(log, L"")),
		m_crmAdmin(new CrmAdministration(log, *crmParameters, *crmRepository)),
        lastChangedRoadmapRunNumber (-1),
        lastChangedRoadmapStatus    (CrmAdministration::Status::NoRoadmap),
        validRoadmapAvailable       (false)
    {	
		crmParameters->storeRoadmapFolder = L"";
    }

	~CrmAdministrationRoadmapTest()
	{
		// All Memory must be disposed here so the memory leak detector in the test will not report false positive results. 
		m_crmAdmin.reset();
		crmRepository.reset();
		crmParameters.reset();
		Simplicity::system().resetHeap();		
	}


    void SetUp() override
    {
        // After the CrmAdministration is constructed we cannot delete the persistent file anymore before a test.
        // As a workaround we start a new study to clear the contents of the persistent file
        clearPersistentData();

		m_crmAdmin->onRoadmapStatusChanged = [this]
        { 
            CrmAdministration::Status status = m_crmAdmin->getStatus();
            const auto &roadmap              = m_crmAdmin->getRoadmap();

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

	CommonPlatform::Xray::XrayStudy createStudy(const std::wstring& id, const std::wstring& name) const
    {
		CommonPlatform::Xray::XrayStudy study;
        study.studyId       = id;
        study.patientName   = name;
        return study;
    }

    std::shared_ptr<const CrmRoadmap> createRoadmap(int runNumber, const CommonPlatform::Xray::XrayGeometry& geometry)
    {
        auto roadmap = std::make_shared<CrmRoadmap>(runNumber, runNumber, 1024,1024, 0.1, geometry, Sense::Matrix(), 0.0);
        return roadmap;
    }

	CommonPlatform::Xray::XrayGeometry createXrayGeometry(double angulation, double rotation)
    {
		CommonPlatform::Xray::XrayGeometry geometry;
        geometry.angulation = angulation;
        geometry.rotation   = rotation;
        return geometry;
    }

    void clearPersistentData()
    {
		CommonPlatform::Xray::XrayStudy study;
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
        study.studyId = std::to_wstring(li.QuadPart);
		m_crmAdmin->setStudy(study);
		ASSERT_EQ(0, m_crmAdmin->getPersistentRunIndex());
    }

	CrmParameters& getCrmParameters() { return *crmParameters; };
	CrmRepository& getCrmRepository() { return *crmRepository; };

	CrmAdministration::Status getlastChangedRoadmapStatus() const { return lastChangedRoadmapStatus; };

	CrmAdministration::Status getCrmStatus() const { return m_crmAdmin->getStatus(); };
	int getCrmPersistentRunIndex() const{ return m_crmAdmin->getPersistentRunIndex(); };	
	int getLastChangedRoadmapRunNumber() const { return lastChangedRoadmapRunNumber; };
	int getCrmPersistentDataVersion() { return m_crmAdmin->getPersistentDataVersion(); };
	void setCrmGeometry(XrayGeometry& geomrtry) { m_crmAdmin->setGeometry(geomrtry); };
	void setCrmStudy(XrayStudy& study) { m_crmAdmin->setStudy(study); }
	void addCrmRoadmap( std::shared_ptr<const CrmRoadmap> roadmap) { m_crmAdmin->addRoadmap(roadmap); };
	void endCrmLive() { return m_crmAdmin->endLive(); };
	bool isValidRoadmapAvailable() const{ return validRoadmapAvailable; };
	bool isCrmEmpty() const{ return m_crmAdmin->isEmpty(); };
	std::shared_ptr<const CrmRoadmap> getCrmRoadmap() { return m_crmAdmin->getRoadmap(); };
	int getNumberOfRoadmapsForCurrentPosition() { return static_cast<int>(m_crmAdmin->getRoadmapsForCurrentGeo().size()); }

private:
	CommonPlatform::Log						&log;
	std::unique_ptr<CrmParameters>			crmParameters;
	std::unique_ptr<CrmRepository>			crmRepository;
	std::unique_ptr<CrmAdministration>		m_crmAdmin;

	int										lastChangedRoadmapRunNumber;
	CrmAdministration::Status				lastChangedRoadmapStatus;
	bool									validRoadmapAvailable;

}; 

TEST_F(CrmAdministrationRoadmapTest, When_Initialized_Then_No_Roadmaps_Available)
{
    ASSERT_TRUE(isCrmEmpty() );
}

TEST_F(CrmAdministrationRoadmapTest, When_Roadmap_Is_Added_For_Current_XrayGeometry_Then_Status_Becomes_Active)
{
    auto initialXrayGeometry = createXrayGeometry  (20,30); initialXrayGeometry.tableHeight = 104;
    auto roadmap             = createRoadmap       (5, initialXrayGeometry);

	setCrmGeometry(initialXrayGeometry);
	addCrmRoadmap( roadmap );

    const auto &current = getCrmRoadmap();

    // Check status of roadmap administration
    ASSERT_TRUE  ( isValidRoadmapAvailable()                                                );
    ASSERT_FALSE ( isCrmEmpty()                                                             );
    ASSERT_EQ    ( roadmap->runIndex, current->runIndex                                     );
    ASSERT_EQ    ( roadmap->mmPerPixel, current->mmPerPixel.x                               );
    ASSERT_EQ    ( roadmap->mmPerPixel, current->mmPerPixel.y                               );
    ASSERT_EQ    ( initialXrayGeometry.tableHeight, current->geometry.tableHeight           );
    ASSERT_EQ    ( 0, getCrmPersistentRunIndex()                                            );
    ASSERT_EQ    ( CrmAdministration::Status::Active, getCrmStatus()             );
}

TEST_F(CrmAdministrationRoadmapTest, When_Roadmap_Is_Added_That_Does_Not_Match_Current_XrayGeometry_Then_Status_Becomes_NoRoadmap)
{
    auto initialXrayGeometry = createXrayGeometry  (0,0);
    auto xrayGeometry        = createXrayGeometry  (20,30); xrayGeometry.tableHeight = 134;
    auto roadmap             = createRoadmap       (5, xrayGeometry);
    
    setCrmGeometry(initialXrayGeometry);
	addCrmRoadmap(roadmap);

    // Check status of roadmap administration
    ASSERT_FALSE ( isValidRoadmapAvailable()                                               );
    ASSERT_EQ    ( CrmAdministration::Status::NoRoadmap, getlastChangedRoadmapStatus()     );
    ASSERT_EQ    ( 0, getCrmPersistentRunIndex()                                           );
    ASSERT_FALSE ( isCrmEmpty()                                                            );

    setCrmGeometry(xrayGeometry);

    // Check meta data of the added roadmap
    const auto &current = getCrmRoadmap();

    ASSERT_EQ    ( roadmap->runIndex, current->runIndex                                     );
    ASSERT_EQ    ( roadmap->mmPerPixel, current->mmPerPixel.x                               );
    ASSERT_EQ    ( roadmap->mmPerPixel, current->mmPerPixel.y                               );
    ASSERT_EQ    ( xrayGeometry.tableHeight, current->geometry.tableHeight                  );
    ASSERT_EQ    ( 0, getCrmPersistentRunIndex()                                            );
    ASSERT_EQ    ( CrmAdministration::Status::Active, getCrmStatus()                        );
}

TEST_F(CrmAdministrationRoadmapTest, When_Roadmap_Is_Added_Then_Persistent_File_Is_Updated_After_Temporarely_Invalidation)
{
    auto xrayGeometry        = createXrayGeometry  (20,30); 
    auto roadmap             = createRoadmap       (5, xrayGeometry);

    int currentVersion = getCrmPersistentDataVersion();
    const int InvalidVersion = -1;

    ASSERT_NE    ( InvalidVersion, getCrmPersistentDataVersion()    );
    ASSERT_TRUE  ( isCrmEmpty()									               );

    EXPECT_NO_THROW(addCrmRoadmap(roadmap)                          );

    ASSERT_EQ    ( currentVersion, getCrmPersistentDataVersion()    );
    ASSERT_FALSE ( isCrmEmpty()                                                );
}

TEST_F(CrmAdministrationRoadmapTest, When_New_Study_Started_Then_Administration_Is_Reset)
{
    auto study1 = createStudy(L"1", L"Dummy1");
    auto study2 = createStudy(L"2", L"Dummy2");

	setCrmStudy(study1);

    // Simulate that 6 incoming runs are handled.
    const int numberOfFluoroRuns = 6;
    for ( int i=0; i<numberOfFluoroRuns ; i++ ) endCrmLive();

    auto roadmap = std::make_shared<CrmRoadmap>(1,1,1024,1024, 0.1, CommonPlatform::Xray::XrayGeometry(), Sense::Matrix(), 0.0);
	addCrmRoadmap(roadmap);

    ASSERT_FALSE ( isCrmEmpty()                               );
    ASSERT_TRUE  ( isValidRoadmapAvailable()				  );
    ASSERT_EQ    (numberOfFluoroRuns, getCrmPersistentRunIndex());

	setCrmStudy(study2);

    ASSERT_TRUE  ( isCrmEmpty()                               );
    ASSERT_FALSE ( isValidRoadmapAvailable()                  );
    ASSERT_EQ    ( 0, getCrmPersistentRunIndex()              );
}

TEST_F(CrmAdministrationRoadmapTest, When_Only_Patient_Name_Changes_Then_Administration_Is_Not_Reset)
{
    auto study1a = createStudy(L"1", L"Dummy1");
    auto study1b = createStudy(L"1", L"Dummy2");

	setCrmStudy(study1a);

    auto roadmap = std::make_shared<CrmRoadmap>(1,1,1024,1024, 0.1, CommonPlatform::Xray::XrayGeometry(), Sense::Matrix(), 0.0);

	addCrmRoadmap(roadmap);

    ASSERT_FALSE (isCrmEmpty()       );
    ASSERT_TRUE  (isValidRoadmapAvailable());

	setCrmStudy(study1b);

    ASSERT_FALSE (isCrmEmpty()       );
    ASSERT_TRUE  (isValidRoadmapAvailable());
}

TEST_F(CrmAdministrationRoadmapTest, When_Number_Of_Roadmaps_Becomes_Higher_Than_PersistencyLimit_Then_They_Are_Still_Added)
{
    // Note: MaximumNumberOfRoadmaps = 1024;
    const double TestAngle  = 50;
    const double AngleStep  = 3;

	CommonPlatform::Xray::XrayGeometry xrayGeometry;
    xrayGeometry.angulation = -TestAngle;
    xrayGeometry.rotation   = -TestAngle;
    double angleOffset = +AngleStep; // smallest angle difference between angios is 2 degrees

    for (int i=0; i<1100 ; i++)
    {
        xrayGeometry.angulation += angleOffset;
        if ( xrayGeometry.angulation > TestAngle  || xrayGeometry.angulation < -TestAngle )
        {
            angleOffset =- angleOffset;   
            xrayGeometry.rotation += AngleStep;
        }

        setCrmGeometry(xrayGeometry);

        auto roadmap = std::make_shared<CrmRoadmap>(i+1,i+1,1024,1024, 0.1, xrayGeometry, Sense::Matrix(), 0.0);
        ASSERT_NO_THROW (addCrmRoadmap(roadmap)  );
        ASSERT_FALSE    (isCrmEmpty()            );

        ASSERT_EQ       ( i+1, getLastChangedRoadmapRunNumber());
    }
}
 
TEST_F(CrmAdministrationRoadmapTest, When_Incoming_Runs_Are_Handled_Then_Runs_Are_Counted)
{
    auto roadmap = std::make_shared<CrmRoadmap>(1,1,1024,1024, 0.1, CommonPlatform::Xray::XrayGeometry(), Sense::Matrix(), 0.0);
	addCrmRoadmap(roadmap);

    ASSERT_FALSE (isCrmEmpty()       );
    ASSERT_TRUE  (isValidRoadmapAvailable());

    const int numberOfRuns = 7;
    for ( int i=0; i<numberOfRuns ; i++ ) endCrmLive();

    ASSERT_EQ    ( numberOfRuns, getCrmPersistentRunIndex() );
}

TEST_F(CrmAdministrationRoadmapTest, When_Two_Roadmaps_With_Same_SeriesNumber_Added_Then_Only_Last_One_Is_Stored)
{
	auto roadmap1 = std::make_shared<CrmRoadmap>(1, 1, 1024, 1024, 0.1, CommonPlatform::Xray::XrayGeometry(), Sense::Matrix(), 0.0);
	auto roadmap2 = std::make_shared<CrmRoadmap>(2, 1, 1024, 1024, 0.1, CommonPlatform::Xray::XrayGeometry(), Sense::Matrix(), 0.0);
	
	addCrmRoadmap(roadmap1);
	ASSERT_FALSE(isCrmEmpty());
	ASSERT_TRUE(isValidRoadmapAvailable());
	ASSERT_TRUE(getNumberOfRoadmapsForCurrentPosition() == 1);

	ASSERT_EQ(roadmap1, getCrmRoadmap());

	addCrmRoadmap(roadmap2);
	ASSERT_FALSE(isCrmEmpty());
	ASSERT_TRUE(isValidRoadmapAvailable());
	ASSERT_TRUE(getNumberOfRoadmapsForCurrentPosition() == 1);

	ASSERT_EQ(roadmap2, getCrmRoadmap());
}
}}
//TICS +OLC#009  magic numbers for a test are ok.