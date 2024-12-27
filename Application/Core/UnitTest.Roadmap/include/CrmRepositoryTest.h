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
#include <CrmAdministration.h>
#include <CrmRepository.h>

#include <Shlwapi.h>
#include "UnitTestLog.h"



//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci {	namespace Test {
using namespace Pci::Core;
using namespace PciSuite;

class CrmRepositoryTest : public ::testing::Test
{
public:
	CrmRepositoryTest()
    :   m_log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		m_crmParameters       (new CrmParameters(L"crm.ini")),
        m_crmRepository       (new CrmRepository(m_log, L"")), // create a repository in the subfolder of the test executable's path.	
        m_runNumberCount      (0)
    {
	}

	~CrmRepositoryTest()
	{
		m_crmRepository.reset();
		m_crmParameters.reset();
		Simplicity::system().resetHeap();
	}

    void SetUp() override
    {
        deleteFilesFromFolder( m_crmRepository->getPath(), L"*.*" );
    }

    void TearDown() override
    {
        deleteFilesFromFolder( m_crmRepository->getPath(), L"*.*" );
    }

    bool deleteFilesFromFolder(const std::wstring& folder, const std::wstring& fileFiler)
	{
        if ( directoryExists(folder))
        {
            wchar_t filepath[MAX_PATH];
            PathAppend(filepath, folder.c_str());
            PathAppend(filepath, fileFiler.c_str());
            filepath[ wcslen(filepath) + 1 ] = '\0';

            SHFILEOPSTRUCT FileOperation;
            ZeroMemory(&FileOperation, sizeof(SHFILEOPSTRUCT));
            FileOperation.hwnd = 0;
            FileOperation.wFunc = FO_DELETE;
            FileOperation.pFrom = filepath;
            FileOperation.fFlags = FOF_NORECURSION | FOF_NO_UI | FOF_SILENT;
            int error = SHFileOperation (&FileOperation);
            if ( error != 0 )
            {
                UNITTEST_LOG( L"Failed to delete files from path " << filepath << L"[Error=" << error << "]" );
                return false;
            }
        }
        return true;
	}

    bool directoryExists(const std::wstring& directoryName) 
    {
        DWORD ftyp = GetFileAttributes(directoryName.c_str());
        if (ftyp == INVALID_FILE_ATTRIBUTES) return false;  //something is wrong with your path!
        if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;   // this is a directory!
        return false;    // this is not a directory!
    }

    bool persistentFileExists()
    {
        DWORD attr = GetFileAttributes( m_crmRepository->getPersistentDataFilePath().c_str() );
        return ( INVALID_FILE_ATTRIBUTES != attr );
    }

    std::shared_ptr<const CrmRoadmap> createRoadmap(int runIndex = -1)
    {
        if ( runIndex == -1 ) runIndex = ++m_runNumberCount;
        auto roadmap = std::make_shared<CrmRoadmap>(runIndex, runIndex,1024,1024, 0.1, XrayGeometry(), Sense::Matrix(), 0.0);
        roadmap->overlay=createOverlay();
        roadmap->data.push_back('A'); 
        return roadmap;
    }

    std::shared_ptr<const CrmRoadmap> createRoadmap(bool addOverlay, bool addData)
    {
        auto roadmap = std::make_shared<CrmRoadmap>(++m_runNumberCount, m_runNumberCount,1024,1024, 0.1, XrayGeometry(), Sense::Matrix(), 0.0);
        if (addOverlay)  roadmap->overlay=createOverlay();
        if (addData)     roadmap->data.push_back('A');;
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

    bool fileExists( const std::wstring& filename)
    {
        DWORD attr = GetFileAttributes( filename.c_str() );
        return ( INVALID_FILE_ATTRIBUTES != attr );
    }

	std::wstring getRepositoryPath() { return m_crmRepository->getPath(); };
	std::shared_ptr<const CrmRoadmap> loadRoadmapFromFile(int index) { return m_crmRepository->loadRoadmapFromFile(index); }
	CrmErrors::Error saveCrmRoadmapToFile(std::shared_ptr<const CrmRoadmap> roadmap) { return m_crmRepository->saveRoadmapToFile(roadmap); }
	bool clearRoadmaps() { return m_crmRepository->clearRoadmaps(); };

	CrmParameters& getCrmParameters() { return *m_crmParameters; };
	CrmRepository& getCrmRepository() { return *m_crmRepository; };

protected:
	CommonPlatform::Log&		m_log;

private:
	std::unique_ptr<CrmParameters> m_crmParameters;
	std::unique_ptr<CrmRepository> m_crmRepository;
	int                            m_runNumberCount;
};

TEST_F(CrmRepositoryTest, When_Repository_Is_Initialized_Then_Path_Is_Correct)
{
    CrmRepository testRepository(m_log, L"C:\\TEMP\\");

    ASSERT_EQ( L"C:\\TEMP\\Roadmaps\\", testRepository.getPath() );
    ASSERT_EQ( L"C:\\TEMP\\Roadmaps\\library", testRepository.getPersistentDataFilePath() );
}

TEST_F(CrmRepositoryTest, When_RepositoryInstance_Is_Destructed_Then_Files_Still_Exist)
{
    std::wstringstream strm;

    {
        CrmRepository testRepository( m_log, L"C:\\TEMP\\");

        auto roadmap = createRoadmap();
        ASSERT_EQ( CrmErrors::Error::OK , testRepository.saveRoadmapToFile(roadmap));

        strm << testRepository.getPath() << getExpectedFileName(roadmap->runIndex);
        ASSERT_TRUE ( fileExists( strm.str() ));

    } // Destructor is called

    ASSERT_TRUE ( fileExists( strm.str() ));
}

TEST_F(CrmRepositoryTest, When_PersistentFile_Is_Deleted_Then_PersistentFile_Does_Not_Exist)
{
    {
        CrmAdministration crmAdmin(m_log, getCrmParameters(), getCrmRepository());
        ASSERT_TRUE(  persistentFileExists() );
    }

    // After destructor is called
    ASSERT_TRUE(  persistentFileExists() );

    // Delete files from folder specified in crm.ini configuration file.
    CrmRepository::deletePersistentData(m_log, getCrmParameters().storeRoadmapFolder);

    ASSERT_FALSE( persistentFileExists() );
}

TEST_F(CrmRepositoryTest, When_RootPath_Is_Empty_Then_Repository_Is_Created_In_Subfolder_Of_Executable_Path)
{
    CrmRepository testRepository(m_log, L"");

    auto roadmap = createRoadmap();
    ASSERT_EQ( CrmErrors::Error::OK , testRepository.saveRoadmapToFile(roadmap));

    std::wstringstream strm;
    strm << testRepository.getPath() << getExpectedFileName(roadmap->runIndex);
    ASSERT_TRUE ( fileExists( strm.str() ));
}

TEST_F(CrmRepositoryTest, When_RootPath_Is_Given_In_Existing_Folder_Then_Repository_Is_Created_In_Correct_Folder)
{
    CrmRepository testRepository(m_log, L"C:\\TEMP\\");

    auto roadmap = createRoadmap();
    ASSERT_EQ( CrmErrors::Error::OK , testRepository.saveRoadmapToFile(roadmap));

    std::wstringstream strm;
    strm << testRepository.getPath() << getExpectedFileName(roadmap->runIndex);
    ASSERT_TRUE ( fileExists( strm.str() ));
}

TEST_F(CrmRepositoryTest, When_Roadmap_Without_Overlay_Is_Saved_Then_It_Cannot_Be_Loaded)
{
    auto roadmap = createRoadmap(false,true);

    ASSERT_EQ( CrmErrors::Error::SaveFailure , saveCrmRoadmapToFile(roadmap));
    
    auto roadmapLoaded = loadRoadmapFromFile(roadmap->runIndex);
    ASSERT_FALSE(roadmapLoaded);
}

TEST_F(CrmRepositoryTest, When_Roadmap_Without_Data_Is_Saved_Then_It_Cannot_Be_Loaded)
{
    auto roadmap = createRoadmap(true, false);

    ASSERT_EQ( CrmErrors::Error::SaveFailure , saveCrmRoadmapToFile(roadmap));

    auto roadmapLoaded = loadRoadmapFromFile(roadmap->runIndex);
    ASSERT_FALSE(roadmapLoaded);
}

TEST_F(CrmRepositoryTest, When_Roadmap_Is_Saved_Then_It_Can_Be_Loaded)
{
    auto roadmap = createRoadmap();

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

TEST_F(CrmRepositoryTest, When_Roadmap_Does_Not_Exists_Then_It_Cannot_Be_Loaded)
{
    ASSERT_FALSE( loadRoadmapFromFile(888) );
    ASSERT_FALSE( loadRoadmapFromFile( -2) );
}

TEST_F(CrmRepositoryTest, When_Roadmaps_Are_Cleared_Then_All_Files_Are_Deleted)
{
    auto roadmap1 = createRoadmap();
    auto roadmap2 = createRoadmap();

    ASSERT_EQ( CrmErrors::Error::OK , saveCrmRoadmapToFile(roadmap1));
    ASSERT_EQ( CrmErrors::Error::OK , saveCrmRoadmapToFile(roadmap2));

    std::wstringstream strm1; strm1 << getRepositoryPath() << getExpectedFileName(roadmap1->runIndex);
    std::wstringstream strm2; strm2 << getRepositoryPath() << getExpectedFileName(roadmap2->runIndex);

    ASSERT_TRUE ( fileExists( strm1.str() ));
    ASSERT_TRUE ( fileExists( strm2.str() ));

    ASSERT_TRUE( clearRoadmaps());

    ASSERT_FALSE ( fileExists( strm1.str() ));
    ASSERT_FALSE ( fileExists( strm2.str() ));

    ASSERT_FALSE ( loadRoadmapFromFile(roadmap1->runIndex));
    ASSERT_FALSE ( loadRoadmapFromFile(roadmap2->runIndex));
}

TEST_F(CrmRepositoryTest, When_Roadmap_Cannot_Be_Saved_Then_It_Cannot_Be_Loaded)
{
    auto roadmap = createRoadmap();

    std::wstringstream filename;
    filename << getRepositoryPath() << getExpectedFileName(roadmap->runIndex);
    ASSERT_FALSE ( fileExists( filename.str() ));

    std::ofstream file;
    file.open(filename.str(), std::ios::binary); // open to lock access

    ASSERT_EQ( CrmErrors::Error::SaveFailure , saveCrmRoadmapToFile(roadmap));
    auto roadmapLoaded = loadRoadmapFromFile(roadmap->runIndex);
    ASSERT_FALSE (roadmapLoaded                                      );

    file.close(); // close to unlock access

    // Try saving again after lock has been lifted. File used to lock can be overwritten.d
    ASSERT_EQ( CrmErrors::Error::OK , saveCrmRoadmapToFile(roadmap));

    roadmapLoaded = loadRoadmapFromFile(roadmap->runIndex);
    ASSERT_NE (roadmapLoaded, nullptr                             );
    ASSERT_NE (roadmapLoaded->overlay, nullptr                    );
    ASSERT_GT (roadmapLoaded->data.size(), 0                      );
    ASSERT_EQ (roadmapLoaded->data.size(), roadmap->data.size()   );
    ASSERT_EQ (roadmap->runIndex, roadmapLoaded->runIndex         );
}
}}
//TICS -OLC#009  magic numbers for a test are ok.