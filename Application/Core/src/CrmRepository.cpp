// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmRepository.h"
#include "CrmAdministration.h"

#include <Shlwapi.h>

namespace Pci{namespace  Core{

static const wchar_t* RoadmapFolder         = L"Roadmaps";
static const wchar_t* PersistentFile        = L"library";
static const wchar_t* RoadmapFile           = L"roadmap_%.3d.crm.fxd";

static bool directoryExists(const std::wstring& directoryName) 
{
    DWORD ftyp = GetFileAttributes(directoryName.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES) return false;  //something is wrong with your path!
    if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;   // this is a directory!
    return false;    // this is not a directory!
}

// Public static function
bool CrmRepository::deletePersistentData(CommonPlatform::Log &log, std::wstring &storeRoadmapFolder)
{
    auto path = CrmRepository::getSafeRepositoryPath(log, storeRoadmapFolder);
    auto file = CrmRepository::getPersistentDataFilePath(path);

    if ( !DeleteFile( file.c_str() ) )
    {
        DWORD error = GetLastError();
		log.DeveloperError(L"Cannot delete persistent data file. [FileName=%s, Error=%d]", file.c_str(), error);
        if ( error != ERROR_FILE_NOT_FOUND) return false;
    }
    return true;
}


CrmRepository::CrmRepository(CommonPlatform::Log &log, const std::wstring& rootPath ):
	m_log( log),
	repositoryPath(getSafeRepositoryPath(log, rootPath))
{
    m_log.DeveloperInfo(L"Roadmap repository initialized. [RootPath=%s]", rootPath.c_str());
}


CrmRepository::CrmRepository(const std::wstring& rootPath):
	CrmRepository(CommonPlatform::Logger(CommonPlatform::LoggerType::CoronaryRoadmap),rootPath )
{

}

CrmRepository::~CrmRepository(void)
{
}

std::wstring CrmRepository::getPath() const
{
    std::wstringstream strm;
    strm << repositoryPath << L"\\";
    return strm.str();
}

PciSuite::CrmErrors::Error CrmRepository::saveRoadmapToFile(const std::shared_ptr<const CrmRoadmap> & roadmap)
{
    auto error = PciSuite::CrmErrors::Error::OK;

    auto filepath = getRoadmapFilePath(roadmap->runIndex);

    FILE *f = nullptr;
    auto fileOpenError = _wfopen_s(&f, filepath.c_str(), L"wb");
    if ( fileOpenError == 0)
    {
        if ( !roadmap->save(f) )
        {
			m_log.DeveloperError( L"Roadmap data not written to file. [RunIndex=%d, Filename=%s]", roadmap->runIndex, filepath.c_str());
            error = PciSuite::CrmErrors::Error::SaveFailure;
        }
        else
        {
			m_log.DeveloperInfo( L"Roadmap data written to file. [RunIndex=%d, Filename=%s]", roadmap->runIndex, filepath.c_str());
        }
        fflush(f);
        fclose(f);
    }
    else
    {
		m_log.DeveloperError( L"Failed to save roadmap data file. [RootPath=%s, Filename=%s, Error=%d]",
                            repositoryPath.c_str(), filepath.c_str(), fileOpenError);
        error = PciSuite::CrmErrors::Error::SaveFailure;
    }
    return error;
}

std::shared_ptr<const CrmRoadmap> CrmRepository::loadRoadmapFromFile(int runIndex)
{
    auto roadmap = std::make_shared<CrmRoadmap>();

    FILE *f = nullptr;

    auto filename = getRoadmapFilePath(runIndex);

    auto fileOpenError = _wfopen_s(&f, filename.c_str(), L"rb");
    if ( fileOpenError == 0 )
    {
        if (!roadmap->load(f)) 
        {
            roadmap = nullptr;
			m_log.DeveloperError(L"Failed to read roadmap data from file. [Filename=%s]", filename.c_str());
        }
        fclose(f);
    }
    else if ( fileOpenError == ENOENT  ) 
    {
        // There can be no file with processed roadmap data available.
        // This is expected when the roadmap generation was not successful. 
        // ENOENT is the error code for 'No such file or directory'. This is no error in this situation.
        roadmap = nullptr;
		m_log.DeveloperWarning(L"Roadmap data file does not exist. [Filename=%s]", filename.c_str());
    }
    else
    {
        roadmap = nullptr;
		m_log.DeveloperError(L"Cannot open roadmap data file. [Filename=%s, Error=%d]", filename.c_str(), fileOpenError);
    }
    return roadmap;
}

std::wstring CrmRepository::getPersistentDataFilePath() const
{
    return CrmRepository::getPersistentDataFilePath(repositoryPath);
}

std::wstring CrmRepository::getSafeRepositoryPath(CommonPlatform::Log &log, const std::wstring& path)
{
	wchar_t filepath[MAX_PATH] = {};
    if ( path.length() == 0 )
    {
        // If root path is not provided or does not exists then we use a subfolder of the path of the executable.
        GetModuleFileName   (GetModuleHandle(nullptr), filepath, MAX_PATH);
        PathRemoveFileSpec  (filepath);
    }
    else
    {
        PathAppend(filepath, path.c_str());
    }
    PathAppend(filepath, RoadmapFolder);

    if ( !directoryExists(filepath))
    {
        if ( CreateDirectory(filepath, nullptr))
		{
			log.DeveloperInfo(L"Roadmap repository folder created. [Folder=%s]", filepath);
		}
		else
		{
			log.DeveloperError(L"Cannot create directory for roadmap repository. [Folder=%s, Error=%d]", filepath, GetLastError());
        }
    }

    return filepath;   
}

// Static function
std::wstring CrmRepository::getPersistentDataFilePath(const std::wstring& rootPath) 
{
	wchar_t filepath[MAX_PATH] = {};
    PathAppend(filepath, rootPath.c_str());
    PathAppend(filepath, PersistentFile);
    return filepath;    
}

std::wstring CrmRepository::getRoadmapFilePath(unsigned runIndex) const
{
    wchar_t filename[MAX_PATH];
    swprintf_s(filename, RoadmapFile, runIndex);

	wchar_t filepath[MAX_PATH] = {};
    PathAppend(filepath, repositoryPath.c_str());
    PathAppend(filepath, filename);

    return filepath;
}

bool CrmRepository::clearRoadmaps()
{
	wchar_t filepath[MAX_PATH] = {};
    PathAppend(filepath, getPath().c_str());
    PathAppend(filepath, L"*.fxd");
    filepath[ wcslen(filepath) + 1 ] = '\0'; // add double null character for SHFileOperation

    SHFILEOPSTRUCT FileOperation;
    ZeroMemory(&FileOperation, sizeof(SHFILEOPSTRUCT));
    FileOperation.hwnd = 0;
    FileOperation.wFunc = FO_DELETE;
    FileOperation.pFrom = filepath;
    FileOperation.fFlags = FOF_NORECURSION | FOF_NO_UI | FOF_SILENT;
    int error = SHFileOperation (&FileOperation);
    if ( error != 0 )
    {
		m_log.DeveloperError( L"Failed to delete files from repository. [Path=%s, Error=%d]", filepath, error);
        return false;
    }
    return true;
}

}}