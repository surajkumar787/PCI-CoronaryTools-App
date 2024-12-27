// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmAdministration.h"
#include "CrmRepository.h"
#include "CrmRecording.h"
#include "Log.h"

#include <Shlwapi.h>
#include <algorithm>
#include <assert.h>

#undef min
#undef max

namespace Pci {namespace Core {

using namespace CommonPlatform::Xray;


static const int      InvalidVersion = -1;
static const int      UninitializedVersion = 0;

static const int      MaxFramesRecording = 300;



CrmAdministration::CrmAdministration(const CrmParameters &params, CrmRepository &repository)
	: CrmAdministration( CommonPlatform::Logger( CommonPlatform::LoggerType::CoronaryRoadmap),	params, repository)
	{}


CrmAdministration::CrmAdministration(CommonPlatform::Log& log, const CrmParameters &params, CrmRepository &repository):
	m_persistentView(nullptr),
	m_persistentMapping(nullptr),
	m_log(log),
	m_repository(repository),
	m_geometryMoving(false),
	m_status(Status::NoRoadmap),
	m_currentRoadmap(nullptr),
	m_params(params),
	m_persistentRunIndex(0),
	m_automatic3dApcActivation(true),
	m_recording(new CrmRecording(MaxFramesRecording))
{
	createAndLoadPersistent();
	update();

	m_log.DeveloperInfo(L"CrmAdministration initialized [RunIndex=%d, RoadmapCount=%llu, Automatic3dApcActivation=%s]",
		m_persistentRunIndex,
		m_roadmaps.size(),
		m_automatic3dApcActivation ? L"True" : L"False");
}

CrmAdministration::~CrmAdministration()
{
	UnmapViewOfFile(m_persistentView);
	CloseHandle(m_persistentMapping);
}

void CrmAdministration::createAndLoadPersistent()
{
	assert(!m_persistentView);

	auto file = m_repository.getPersistentDataFilePath();

	HANDLE persistentFile = CreateFile(file.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (persistentFile == INVALID_HANDLE_VALUE)
	{
		m_log.DeveloperError(L"Failed to create file for persistent data. [File=%s, Error=%u]", file.c_str(), GetLastError());
	}
	else
	{
		m_persistentMapping = CreateFileMapping(static_cast<HANDLE>(persistentFile), nullptr, PAGE_READWRITE, 0, sizeof(Persistent), nullptr);

		CloseHandle(persistentFile); // After CreateFileMapping, we do not need this file handle anymore.

		if (m_persistentMapping == nullptr)
		{
			m_log.DeveloperError(L"Failed to create file mapping for persistent data. [File=%s, Error=%u]", file.c_str(), GetLastError());
		}
		else
		{
			m_persistentView = static_cast<Persistent*>(MapViewOfFile(static_cast<HANDLE>(m_persistentMapping), FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Persistent)));
		
			if (m_persistentView == nullptr)
			{
				m_log.DeveloperError(L"Failed to map view of persistent data file into memory. [File=%s, Error=%u]", file.c_str(), GetLastError());
			}
		}
	}

	if (m_persistentView != nullptr)
	{
		if (!loadPersistent())
		{
			m_log.DeveloperInfo(L"Failed to load persistent data. Persistent data will be re-initialized.");
		}
		updatePersistent(); // For version number and roadmap count, and to re-initialize when load failed.
	}
}

bool CrmAdministration::loadPersistent()
{
	if (m_persistentView == nullptr) return false;

	if (m_persistentView->version != Persistent::Version)
	{
		if (m_persistentView->version == UninitializedVersion)
		{
			// No error (due to initial installation), do not load data.
			m_log.DeveloperInfo(L"Persistent data file is not yet initialized.");
		}
		else
		{
			// Unexpected version due to failed update, do not load data.
			m_log.DeveloperError(L"Persistent data file was not correctly updated. Version mismatch. [Version=%d, ExpectedVersion=%d]",
				m_persistentView->version, Persistent::Version);
		}
		return false;
	}

	bool loadOk = true;

	if (m_persistentView->runIndex >= 0 &&
		m_persistentView->runIndex < MaxNumberOfRoadmaps  &&
		m_persistentView->roadmapCount >= 0 &&
		m_persistentView->roadmapCount <= MaxNumberOfRoadmaps)
	{
		// Make sure the string is always null-terminated...
		m_persistentView->studyID[_countof(m_persistentView->studyID) - 1] = 0;

		m_studyID = m_persistentView->studyID;
		m_persistentRunIndex = m_persistentView->runIndex;
		m_automatic3dApcActivation = m_persistentView->automatic3dApcActivation;

		for (int i = 0; i < m_persistentView->roadmapCount; i++)
		{
			if (m_persistentView->roadmaps[i] >= 0 &&
				m_persistentView->roadmaps[i] < MaxNumberOfRoadmaps)
			{
				auto roadmap = m_repository.loadRoadmapFromFile(m_persistentView->roadmaps[i]);

				if (roadmap)
				{
					m_roadmaps.push_back(roadmap);
				}
				else
				{
					loadOk = false;
					m_log.DeveloperError(L"Failed to load roadmap data. [RunIndex=%d]", m_persistentView->roadmaps[i]);
				}
			}
		}
	}
	else
	{
		loadOk = false;
		m_log.DeveloperError(L"Run or roadmap index out of range. [RunIndex=%d, RoadmapCount=%d, MaxNumberOfRoadmaps=%d]",
			m_persistentView->runIndex, m_persistentView->roadmapCount, MaxNumberOfRoadmaps);
	}

	return loadOk;
}

void CrmAdministration::updatePersistent()
{
	if (m_persistentView == nullptr) return;

	// Make data invalid while we're updating....
	m_persistentView->version = InvalidVersion;

	int roadmapCount = std::min(MaxNumberOfRoadmaps, static_cast<int>(m_roadmaps.size()));

	wcscpy_s(m_persistentView->studyID, m_studyID.c_str());
	for (int i = 0; i < roadmapCount; i++)
	{
		m_persistentView->roadmaps[i] = m_roadmaps[i]->runIndex;
	}

	m_persistentView->runIndex = m_persistentRunIndex;
	m_persistentView->roadmapCount = roadmapCount;
	m_persistentView->version = Persistent::Version;
	m_persistentView->automatic3dApcActivation = m_automatic3dApcActivation;

	m_log.DeveloperInfo(L"Persistent file updated. [Version=%d, RunIndex=%d, RoadmapCount=%d, Automatic3dApcActivation=%s]",
		m_persistentView->version,
		m_persistentView->runIndex,
		m_persistentView->roadmapCount,
		m_persistentView->automatic3dApcActivation ? L"True" : L"False");
}

int CrmAdministration::getPersistentRunIndex() const
{
	return m_persistentRunIndex;
}

int CrmAdministration::getPersistentDataVersion() const
{
	if (m_persistentView == nullptr) return InvalidVersion;
	return m_persistentView->version;
}

std::wstring CrmAdministration::getStudyID() const
{
	return m_studyID;
}

void CrmAdministration::endLive()
{
	m_log.DeveloperInfo(L"CrmAdministration::endLive is entered.");
	m_persistentRunIndex++;
	updatePersistent();
}

void CrmAdministration::setStudy(const XrayStudy &study)
{
	if (study.studyId != m_studyID)
	{
		m_studyID = study.studyId;
		m_log.DeveloperInfo(L"CrmAdministration: New Study ID.");

		m_roadmaps.clear();
		m_repository.clearRoadmaps();

		m_currentRoadmap = nullptr;
		m_persistentRunIndex = 0;

		updatePersistent();
		update();
	}
}

void CrmAdministration::addRoadmap(const std::shared_ptr<const CrmRoadmap> &newRoadmap)
{
	if (newRoadmap == nullptr) return;

	m_log.DeveloperInfo(L"CrmAdministration: Add roadmap data of run. [RunIndex=%d]", newRoadmap->runIndex);

	// Remove already existing roadmaps that have the same id
	auto it = std::remove_if(m_roadmaps.begin(), m_roadmaps.end(), [&newRoadmap, this](const std::shared_ptr<const CrmRoadmap> &roadmap)
	{
		return (roadmap->runIndex == newRoadmap->runIndex) || (roadmap->seriesNumber == newRoadmap->seriesNumber);
	});
	m_roadmaps.erase(it, m_roadmaps.end());

	auto roadmapsCurrentGeo = getRoadmapsForCurrentGeo();
	for (auto& roadmap : roadmapsCurrentGeo)
	{
		const_cast<CrmRoadmap&>(*roadmap).selected = false;
	}

	m_roadmaps.push_back(newRoadmap);

	m_currentRoadmap = nullptr; // Invalidate current selected roadmap

	updatePersistent();
	update();
}

const std::shared_ptr<const CrmRoadmap> &CrmAdministration::getRoadmap() const
{
	return m_currentRoadmap;
}

CrmAdministration::Status CrmAdministration::getStatus() const
{
	return this->m_status;
}

bool CrmAdministration::isEmpty() const
{
	return m_roadmaps.empty();
}

void CrmAdministration::setGeometry(const XrayGeometry &geo)
{
	m_geometryPosition = geo;
	m_geometryMoving = true;
	update();
}

void CrmAdministration::endGeometry()
{
	m_geometryMoving = false;
	update();
}

bool CrmAdministration::update()
{
	std::shared_ptr<const CrmRoadmap> targetRoadmap(nullptr);

	auto targetRoadmaps = findMatchingRoadmaps(m_geometryPosition);

	Status targetStatus = Status::NoRoadmap;

	if (targetRoadmaps.empty())
	{
		targetRoadmap = findBestMatchingRoadmap(m_geometryPosition);
	}
	else
	{
		targetRoadmap = targetRoadmaps.back();
		for (auto& roadmap : targetRoadmaps)
		{
			if (roadmap->locked)
			{
				if (!targetRoadmap->locked || roadmap == m_currentRoadmap)
				{
					targetRoadmap = roadmap;
				}
			}
			else if (roadmap->selected && !targetRoadmap->locked)
			{
				if (!targetRoadmap->selected || roadmap == m_currentRoadmap)
				{
					targetRoadmap = roadmap;
				}
			}
			else if (!targetRoadmap->locked && !targetRoadmap->selected && roadmap == m_currentRoadmap)
			{
				targetRoadmap = m_currentRoadmap;
			}
		}
	}

	if (!targetRoadmap)
	{
		targetStatus = Status::NoRoadmap;
	}
	else
	{
		if (targetRoadmap->getAngleDistance(m_geometryPosition) >= static_cast<double>( m_params.angleThresholdActive))
		{
			targetStatus = Status::MoveArc;
		}
		else
		{
			if (targetRoadmap->getPanningDistance(m_geometryPosition) >= static_cast<double>(m_params.panningThresholdActive))
			{
				targetStatus = Status::MoveTable;
			}
			else
			{
				targetStatus = Status::Active;
			}
		}
	}

	bool result = (m_currentRoadmap != targetRoadmap || m_status != targetStatus);

	m_currentRoadmap = targetRoadmap;
	m_status = targetStatus;

	if (result && onRoadmapStatusChanged) onRoadmapStatusChanged();

	return result;
}

std::shared_ptr<const CrmRoadmap> CrmAdministration::findBestMatchingRoadmap(const XrayGeometry &geometry) const
{
	std::shared_ptr<const CrmRoadmap> targetRoadmap = m_currentRoadmap;

	if ((!targetRoadmap) ||
	    (targetRoadmap->getAngleDistance(geometry) > static_cast<double>(m_params.angleThresholdRelease)) ||
		(targetRoadmap->getPanningDistance(geometry) > static_cast<double>(m_params.panningThresholdRelease)))
	{
		// Current run too far away; outside the release boundaries, find closest run...
		targetRoadmap = nullptr;

		double angleDistanceBestRoadmapToCurrentGeometry = 0.0;

		// Find a roadmap that fulfills the angle and table conditions and is closest to the current geometry angles.
		for (const auto &roadmap : m_roadmaps)
		{
			double angleDistance = roadmap->getAngleDistance(geometry);
			double tableDistance = roadmap->getPanningDistance(geometry);

			if ((!targetRoadmap || (!targetRoadmap->selected && angleDistance <= angleDistanceBestRoadmapToCurrentGeometry)) &&
				(angleDistance <= static_cast<double>(m_params.angleThresholdAvailable) )      &&
				(tableDistance <= static_cast<double>(m_params.panningThresholdAvailable) ))
			{
				targetRoadmap = roadmap;
				angleDistanceBestRoadmapToCurrentGeometry = angleDistance;
			}
		}
	}

	return targetRoadmap;
}

std::vector<std::shared_ptr<const CrmRoadmap>> CrmAdministration::findMatchingRoadmaps(const XrayGeometry &geometry) const
{
	std::vector<std::shared_ptr<const CrmRoadmap>> matchingRoadmaps;

	// Find a roadmap that fulfills the angle and table conditions and is closest to the current geometry angles.
	for (const auto &roadmap : m_roadmaps)
	{
		double angleDistance = roadmap->getAngleDistance(geometry);
		double tableDistance = roadmap->getPanningDistance(geometry);

		if ((angleDistance < static_cast<double>(m_params.angleThresholdActive)) &&
			(tableDistance < static_cast<double>(m_params.panningThresholdActive)))
		{
			matchingRoadmaps.push_back(roadmap);
		}
	}

	return matchingRoadmaps;
}


PciSuite::CrmErrors::Error CrmAdministration::testRoadmap(const std::shared_ptr<const CrmRoadmap> &roadmap)
{
	if (!roadmap) return PciSuite::CrmErrors::Error::NoInjection;

	//find out if we're gonna overwrite a roadmap...
	bool overwrite = false;
	for (auto i = m_roadmaps.begin(); i != m_roadmaps.end(); i++)
	{
		if ((*i)->getAngleDistance(roadmap->geometry) < static_cast<double>(m_params.angleMinimumDistance))
		{
			overwrite = true;
		}
	}

	m_log.DeveloperInfo(L"CrmAdministration: Test roadmap quality. [Quality=%.2f, ExistingRoadmapOnPosition=%s, Threshold=%.2f]",
		roadmap->quality, (overwrite ? L"Yes" : L"No"), m_params.thresholdAcceptOverwrite);

	//test the quality of the roadmap; if there's no roadmap yet we accept a lower quality, because there's nothing to lose...
	if ((overwrite && roadmap->quality < m_params.thresholdAcceptOverwrite) ||
		(!overwrite && roadmap->quality < m_params.thresholdAcceptNew))
	{
		if (roadmap->warning == PciSuite::CrmErrors::Error::OK)
		{
			return PciSuite::CrmErrors::Error::NoInjection;
		}
		else
		{
			return roadmap->warning;
		}
	}


	return PciSuite::CrmErrors::Error::OK;
}

void CrmAdministration::setPersistent3dApcAutomaticActivation(bool enabled)
{
	m_automatic3dApcActivation = enabled;
	updatePersistent();
}

bool CrmAdministration::getPersistent3dApcAutomaticActivation() const
{
	return m_automatic3dApcActivation;
}

void CrmAdministration::setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image, const std::shared_ptr<const CrmOverlay> &roadmap)
{
	if ((image != nullptr) && (m_recording->getRunIndex() != image->runIndex))
	{
		m_recording.reset(new CrmRecording(MaxFramesRecording));
		m_recording->setRunIndex(image->runIndex);
	}

	m_recording->addImage(image, roadmap);
}

std::shared_ptr<CrmRecording> CrmAdministration::getRecording() const
{
	return m_recording;
}

std::vector<std::shared_ptr<const CrmRoadmap>> CrmAdministration::getRoadmapsForCurrentGeo() const
{
	return findMatchingRoadmaps(m_geometryPosition);
}

void CrmAdministration::selectRoadmap(std::shared_ptr<const CrmRoadmap>& newSelectedRoadmap)
{
	if (m_currentRoadmap != newSelectedRoadmap)
	{
		if (m_currentRoadmap)
		{
			const_cast<CrmRoadmap&>(*m_currentRoadmap).selected = false;
			m_repository.saveRoadmapToFile(m_currentRoadmap);
		}

		if (newSelectedRoadmap)
		{
			const_cast<CrmRoadmap&>(*newSelectedRoadmap).selected = true;
			m_repository.saveRoadmapToFile(newSelectedRoadmap);
		}

		updatePersistent();
		update();
	}
}

void CrmAdministration::lockRoadmap(std::shared_ptr<const CrmRoadmap>& roadmap)
{
	const_cast<CrmRoadmap&>(*roadmap).locked = true;

	m_repository.saveRoadmapToFile(roadmap);

	updatePersistent();
	update();
}

void CrmAdministration::unlockRoadmap(std::shared_ptr<const CrmRoadmap>& roadmap)
{
	const_cast<CrmRoadmap&>(*roadmap).locked = false;

	m_repository.saveRoadmapToFile(roadmap);

	updatePersistent();
	update();
}


}}