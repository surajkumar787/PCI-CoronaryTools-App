// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include <Log.h>

namespace CommonPlatform
{
	class Log;
}

namespace Pci {	namespace Core {

	class IArchivingSettings
	{
	public:
		enum class Feature
		{
			StentBoostLive,
			StentBoostPostDeploy,
			StentBoostSubtract
		};

		virtual ~IArchivingSettings() = default;

		virtual bool isAutoCopyToRefEnabled(Feature feature) const = 0;
		virtual const std::wstring getCopyToRefViewer(Feature feature) const = 0;

		virtual bool isAutoPACSArchiveMovieEnabled(Feature featuer) const = 0;
		virtual bool isAutoPACSArchiveSnapshotEnabled(Feature featuer) const = 0;

		virtual int getCopyToRefTargetNumber(CommonPlatform::Log& log, std::wstring& refName) const = 0;
	};
}}