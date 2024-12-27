// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "ArchivingSettings.h"
#include "Settings.h"

namespace Pci { namespace Core { 

	ArchivingSettings::ArchivingSettings(CommonPlatform::Settings& settingFile)
	{
		copyToRefEnabledMap.emplace(Feature::StentBoostLive, settingFile.get(L"CopyToReference", L"AutomaticCopySBL", false));
		copToRefViewerMap.emplace(Feature::StentBoostLive, settingFile.get(L"CopyToReference", L"ReferenceMonitorSBL", L"Ref1"));

		copyToRefEnabledMap.emplace(Feature::StentBoostPostDeploy, settingFile.get(L"CopyToReference", L"AutomaticCopySBPostDeploy", false));
		copToRefViewerMap.emplace(Feature::StentBoostPostDeploy, settingFile.get(L"CopyToReference", L"ReferenceMonitorSBPostDeploy", L"Ref1"));

		copyToRefEnabledMap.emplace(Feature::StentBoostSubtract, settingFile.get(L"CopyToReference", L"AutomaticCopySBSubtract", false));
		copToRefViewerMap.emplace(Feature::StentBoostSubtract, settingFile.get(L"CopyToReference", L"ReferenceMonitorSBSubtract", L"Ref1"));

		autoArchiveMovieMap.emplace(Feature::StentBoostLive, settingFile.get(L"General", L"AutoArchiveSblMovie", false));
		autoArchiveMovieMap.emplace(Feature::StentBoostPostDeploy, settingFile.get(L"General", L"AutoArchiveSbPostDeployMovie", false));
		autoArchiveMovieMap.emplace(Feature::StentBoostSubtract, settingFile.get(L"General", L"AutoArchiveSbSubtractMovie", false));

		autoArchiveSnapshotMap.emplace(Feature::StentBoostLive, settingFile.get(L"General", L"AutoArchiveSblSnapshot", true));
		autoArchiveSnapshotMap.emplace(Feature::StentBoostPostDeploy, settingFile.get(L"General", L"AutoArchiveSbPostDeploySnapshot", true));
		autoArchiveSnapshotMap.emplace(Feature::StentBoostSubtract, settingFile.get(L"General", L"AutoArchiveSbSubtractSnapshot", true));
	}

	ArchivingSettings::~ArchivingSettings()
	{
	}

	bool ArchivingSettings::isAutoCopyToRefEnabled(Feature feature) const
	{
		bool enabled(false);
		if (copyToRefEnabledMap.find(feature) != copyToRefEnabledMap.end())
		{
			enabled = copyToRefEnabledMap.at(feature);
		}

		return enabled;
	}

	const std::wstring ArchivingSettings::getCopyToRefViewer(Feature feature) const
	{
		std::wstring refViewer(L"");
		if (copToRefViewerMap.find(feature) != copToRefViewerMap.end())
		{
			refViewer = copToRefViewerMap.at(feature);
		}

		return refViewer;
	}

	bool ArchivingSettings::isAutoPACSArchiveMovieEnabled(Feature feature) const
	{
		bool enabled(false);
		if (autoArchiveMovieMap.find(feature) != autoArchiveMovieMap.end())
		{
			enabled = autoArchiveMovieMap.at(feature);
		}

		return enabled;
	}
	
	bool ArchivingSettings::isAutoPACSArchiveSnapshotEnabled(Feature feature) const
	{
		bool enabled(false);
		if (autoArchiveSnapshotMap.find(feature) != autoArchiveSnapshotMap.end())
		{
			enabled = autoArchiveSnapshotMap.at(feature);
		}

		return enabled;
	}

	int ArchivingSettings::getCopyToRefTargetNumber(CommonPlatform::Log& log, std::wstring& refName) const
	{
		int targetNumber(-1);

		try
		{
			size_t numberPosition = refName.find(L"Ref");

			if (numberPosition != std::string::npos)
			{
				targetNumber = std::stoi(refName.substr(numberPosition + 3));
			}
		}
		catch (std::exception& exception)
		{
			log.DeveloperError(L"Unable to determine CopyToRef target number -> %s", exception.what());
		}

		return targetNumber;
	}

}}