// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <gtest/gtest.h>

#include <CrmAdministration.h>
#include <CrmRepository.h>
#include "CrmAdministrationUnderTest.h"

//TICS -OLC#009  magic numbers for a test are oke.
namespace Pci {	namespace Test {
using namespace Pci::Core;

// This class implements tests the persistent file behavior of the CrmAdministration class.
class CrmAdministrationPersistentFileTest : public ::testing::Test
{
public:

	static const int            InvalidVersion  = -1;
    static const int            ExpectedVersion = 3;

    CrmAdministrationPersistentFileTest():
		m_log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		crmParameters(new CrmParameters(L"crm.ini")),
		crmRepository(new CrmRepository(m_log, L""))
    {
    }

	~CrmAdministrationPersistentFileTest()
	{
		// All Memory must be disposed here so the memory leak detector in the test will not report false positive results. 
		crmRepository->clearRoadmaps();
		crmRepository.reset();
		crmParameters.reset();
		Simplicity::system().resetHeap();
	}

    void SetUp() override
    {
        SetEnvironmentVariable(L"CrmAdministrationPersistentFileTest_throw_exception_in_updatePersistent", nullptr );

        deletePersistentFile();
        if ( persistentFileExists() ) std::cout << "Start condition for test has failed." << std::endl;
        ASSERT_FALSE( persistentFileExists() );
    }

    bool persistentFileExists() const
    {
        DWORD attr = GetFileAttributes( crmRepository->getPersistentDataFilePath().c_str() );
        return ( INVALID_FILE_ATTRIBUTES != attr );
    }

    void deletePersistentFile() const
    {
        DeleteFile( crmRepository->getPersistentDataFilePath().c_str() );    
    }

    void createCorruptPersistentFile()
    {
        std::ofstream file;
        file.open(crmRepository->getPersistentDataFilePath().c_str(), std::ios::binary);
        file << 909 << 2 << 3 << 4 << 5;
        file.close();
    }

	CommonPlatform::Xray::XrayStudy createStudy(const std::wstring& id, const std::wstring& name) const
    {
		CommonPlatform::Xray::XrayStudy study;
        study.studyId       = id;
        study.patientName   = name;
        return study;
    }

    std::shared_ptr<const CrmRoadmap> createRoadmap(int runNumber)
    {
        auto roadmap = std::make_shared<CrmRoadmap>(runNumber, runNumber, 1024,1024, 0.1, CommonPlatform::Xray::XrayGeometry(), Sense::Matrix(), 0.0);
        return roadmap;
    }

	CrmParameters& getCrmParameters() { return *crmParameters; };
	CrmRepository& getCrmRepository() { return *crmRepository; };

protected:
	CommonPlatform::Log						&m_log;

private:
	std::unique_ptr<CrmParameters>			crmParameters;
	std::unique_ptr<CrmRepository>			crmRepository;
};

// It must be possible to delete this file to run these tests
TEST_F(CrmAdministrationPersistentFileTest, When_PersistentFile_Is_Deleted_Then_File_Is_Removed)
{
    ASSERT_TRUE  ( CrmRepository::deletePersistentData(m_log, getCrmParameters().storeRoadmapFolder) );
    ASSERT_FALSE ( persistentFileExists() );
}

TEST_F(CrmAdministrationPersistentFileTest, When_Administration_Constructed_Then_PersistentFile_Is_Created)
{
    CrmAdministration crmAdmin(m_log, getCrmParameters(),getCrmRepository());
    ASSERT_TRUE ( persistentFileExists() );
}

TEST_F(CrmAdministrationPersistentFileTest, When_Administration_Constructed_Then_PersistentFile_Is_Created_With_CorrectVersion)
{
    CrmAdministration crmAdmin(m_log, getCrmParameters(),getCrmRepository());
    ASSERT_TRUE ( persistentFileExists()                                            );
    ASSERT_EQ   ( ExpectedVersion, crmAdmin.getPersistentDataVersion() );
}

TEST_F(CrmAdministrationPersistentFileTest, When_PersistentFile_Cannot_Be_Created_Then_There_Is_No_Persistency)
{
    {
        std::ofstream file;
        file.open(getCrmRepository().getPersistentDataFilePath().c_str(), std::ios::binary); // open to lock access

        CrmAdministration crmAdmin(m_log, getCrmParameters(),getCrmRepository());

        file.close(); // close to unlock access

        ASSERT_EQ   ( InvalidVersion, crmAdmin.getPersistentDataVersion());
        ASSERT_TRUE ( crmAdmin.isEmpty() );

        crmAdmin.addRoadmap( createRoadmap(1) );

        ASSERT_FALSE ( crmAdmin.isEmpty() );
    }

    // A valid but empty persistent file will be created
    CrmAdministration crmAdmin2(m_log, getCrmParameters(),getCrmRepository());
    ASSERT_NE   ( InvalidVersion, crmAdmin2.getPersistentDataVersion());
    ASSERT_TRUE ( crmAdmin2.isEmpty() );
}

TEST_F(CrmAdministrationPersistentFileTest, When_Administration_Destructed_Then_PersistentFile_Still_Exists)
{
    {
        CrmAdministration crmAdmin(m_log, getCrmParameters(),getCrmRepository());
        ASSERT_TRUE(  persistentFileExists() );
    }
    
    // After destructor is called
    ASSERT_TRUE(  persistentFileExists() );
}

TEST_F(CrmAdministrationPersistentFileTest, When_Administration_Restarted_Then_PersistentFile_Must_Be_Reloaded)
{
    const int numberOfRuns = 3;
    auto study = createStudy(L"123",L"Dummy123");

    // Scope for first instance
	{
		CrmAdministration crmAdmin(m_log, getCrmParameters(), getCrmRepository());
		ASSERT_TRUE(persistentFileExists());
		ASSERT_TRUE(crmAdmin.isEmpty());

		crmAdmin.setStudy(study);
		crmAdmin.addRoadmap(createRoadmap(3));
		ASSERT_FALSE(crmAdmin.isEmpty());

		// Simulate that 3 incoming runs have been handled.
		for (int i = 0; i < numberOfRuns; i++)
		{
			crmAdmin.endLive();
		}
	}

    // After destructor is called
    ASSERT_TRUE  ( persistentFileExists() );

    // Scope for second instance
    {
        // Restart, i.e. construct a new instance.
        CrmAdministration crmAdmin2(m_log, getCrmParameters(),getCrmRepository());
        ASSERT_TRUE  ( persistentFileExists() );

        // For this unit test we do not actually create valid roadmap data, so the roadmap list remains empty.
        // But the study ID must be read from the persistent data.
        ASSERT_TRUE  ( crmAdmin2.isEmpty()                              );
        ASSERT_EQ    ( study.studyId, crmAdmin2.getStudyID()            );
        ASSERT_EQ    ( numberOfRuns, crmAdmin2.getPersistentRunIndex()  );
    }
}

TEST_F(CrmAdministrationPersistentFileTest, When_PersistentFile_Is_Corrupt_Then_PersistentFile_Must_Not_Be_Loaded)
{
    createCorruptPersistentFile();
    ASSERT_TRUE  ( persistentFileExists() );

    CrmAdministration crmAdmin(m_log, getCrmParameters(),getCrmRepository());
    ASSERT_TRUE     ( crmAdmin.isEmpty()                                                  );
    ASSERT_EQ       ( L"", crmAdmin.getStudyID()                                          );
}

TEST_F(CrmAdministrationPersistentFileTest, When_Too_Many_Roadmaps_Have_Been_Added_Then_PersistentFile_Can_Still_Be_Be_Loaded)
{
    auto study = createStudy(L"456",L"Dummy456");

    // Note: MaximumNumberOfRoadmaps = 1024;
    {
        CrmAdministration crmAdmin(m_log, getCrmParameters(),getCrmRepository());

		CommonPlatform::Xray::XrayGeometry xrayGeometry;
        xrayGeometry.angulation = -50.0;
        xrayGeometry.rotation   = -50.0;
        double angleStep = +3.0; // smallest angle difference between angio's is 2 degrees

        crmAdmin.setStudy(study);

        for (int i=0; i<1100 ; i++)
        {
            xrayGeometry.angulation += angleStep;
            if (( xrayGeometry.angulation > 50.0 ) || ( xrayGeometry.angulation < -50.0 ))
            {
                angleStep =- angleStep;   
                xrayGeometry.rotation += 3;
            }
            crmAdmin.setGeometry(xrayGeometry);

            std::shared_ptr<CrmRoadmap> roadmap = std::make_shared<CrmRoadmap>(i+1, i+1, 1024,1024, 0.1, xrayGeometry, Sense::Matrix(), 0.0);
            ASSERT_NO_THROW ( crmAdmin.addRoadmap(roadmap)  );
            ASSERT_FALSE    ( crmAdmin.isEmpty()            );
        }
    }

    CrmAdministration crmAdmin2(m_log, getCrmParameters(),getCrmRepository());
    ASSERT_TRUE  ( persistentFileExists() );

    // For this unit test we do not actually create valid roadmap data, so the roadmap list remains empty.
    // But the study ID must be read from the persistent data.
    ASSERT_TRUE  ( crmAdmin2.isEmpty());
    ASSERT_EQ    ( study.studyId, crmAdmin2.getStudyID());
}

TEST_F(CrmAdministrationPersistentFileTest, When_Update_Fails_Then_Persisent_Data_Must_Be_Invalidated)
{
    auto study = createStudy(L"456",L"Dummy456");

    {
        CrmAdministrationUnderTest crmAdmin(m_log, getCrmParameters(),getCrmRepository());
        ASSERT_TRUE     ( crmAdmin.persistentViewCreated()      );

        crmAdmin.setStudy(study);
        EXPECT_NO_THROW( crmAdmin.addRoadmap( createRoadmap(1) ));

        ASSERT_FALSE    ( crmAdmin.isEmpty()                    );
        ASSERT_EQ       ( study.studyId, crmAdmin.getStudyID()  );

        SetEnvironmentVariable(L"CrmAdministration_throw_exception_in_updatePersistent", L"Hallo");

        EXPECT_ANY_THROW( crmAdmin.addRoadmap( createRoadmap(2) ));

        // Data can still be valid
        ASSERT_FALSE    ( crmAdmin.isEmpty()                    );
        ASSERT_EQ       ( study.studyId, crmAdmin.getStudyID()  );

        
        // Persistent data must be valid
        ASSERT_EQ   ( InvalidVersion, crmAdmin.getPersistentDataVersion() );

    } // Now we will close the process and destruct the CrmAdministration

    SetEnvironmentVariable(L"CrmAdministration_throw_exception_in_updatePersistent", nullptr );

    CrmAdministration crmAdmin2(m_log, getCrmParameters(),getCrmRepository());

    // Expect an valid but empty administration
    ASSERT_TRUE ( crmAdmin2.isEmpty()                                                );
    ASSERT_EQ   ( L"", crmAdmin2.getStudyID()                                        );
    ASSERT_EQ   ( ExpectedVersion, crmAdmin2.getPersistentDataVersion() );
}
}}
//TICS +OLC#009  magic numbers for a test are oke.