// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include <Log.h>
#include "CrmErrors.h"
#include <memory>

namespace Pci { namespace Core {

class CrmRoadmap;

// This class implements storage of data files for the coronary road mapping application.
// A subfolder 'Roadmaps' is created in the rootpath.
class CrmRepository
{
public:
    explicit CrmRepository(const std::wstring& rootPath);
	CrmRepository(CommonPlatform::Log &log,const std::wstring& rootPath);
    virtual ~CrmRepository(void);

    // Deletes the file that contains the administration data of the CRM application.
    static bool deletePersistentData(CommonPlatform::Log &log, std::wstring &storeRoadmapFolder);

    // Save the roadmap data to a file in the roadmap folder.
    PciSuite::CrmErrors::Error saveRoadmapToFile(const std::shared_ptr<const CrmRoadmap> &roadmap);

    // Load the roadmap data from a file in the roadmap folder.
    std::shared_ptr<const CrmRoadmap> loadRoadmapFromFile(int runIndex);

    // Deletes all the road map data files from the repository
    bool clearRoadmaps();

    // Return the path to the persistent data file.
    std::wstring getPersistentDataFilePath() const;

    // Return the path of the repository.
    std::wstring getPath() const;

	CrmRepository(const CrmRepository&) = delete;
	CrmRepository& operator=(const CrmRepository&) = delete;
private:
 	::CommonPlatform::Log&      m_log;
	std::wstring        repositoryPath;

    static std::wstring getSafeRepositoryPath       (CommonPlatform::Log &log,const std::wstring& rootPath);
    std::wstring        getRoadmapFilePath          (unsigned runIndex) const;
    static std::wstring getPersistentDataFilePath   ( const std::wstring& rootPath);
};

}}