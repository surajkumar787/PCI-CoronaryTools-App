// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "IArchivingSettings.h"
#include <map>

namespace CommonPlatform { class Settings; }

namespace Pci {	namespace Core {

	class ArchivingSettings : public IArchivingSettings
	{
	public:
		explicit ArchivingSettings(CommonPlatform::Settings&);
		virtual ~ArchivingSettings();

		virtual bool isAutoCopyToRefEnabled(Feature feature) const override;
		virtual const std::wstring getCopyToRefViewer(Feature feature) const override;

		virtual bool isAutoPACSArchiveMovieEnabled(Feature feature) const override;
		virtual bool isAutoPACSArchiveSnapshotEnabled(Feature feature) const override;
		virtual int getCopyToRefTargetNumber(CommonPlatform::Log& log, std::wstring& refName) const override;

	private:
		std::map<Feature, bool> copyToRefEnabledMap;
		std::map<Feature, std::wstring> copToRefViewerMap;

		std::map<Feature, bool> autoArchiveMovieMap;
		std::map<Feature, bool> autoArchiveSnapshotMap;
	};
}}