// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "MainModel.h"
#include "LicenseStatus.h"
#include "ILicenseCollection.h"
#include "IArchivingStatus.h"

namespace Pci { namespace Core
{
using namespace CommonPlatform::Xray;

	MainModel::MainModel(const ILicenseCollection &licenseCollection, CommonPlatform::Xray::SystemType system) :
		study(),
		m_licenseCollection(licenseCollection),
		coregStatus(),
		coregConnected(false),
		iControlConnected(false),
		systemMenuEnabled(true),
		tsmFunctionsAvailable(false),
		movieStatus(ArchivingStatus::None),
		snapshotStatus(ArchivingStatus::None),
		snapshotArea(CaptureArea::MainView),
		systemType(system)
	{
	}

	void MainModel::notify() const
	{
		if (eventModelChanged != nullptr)
		{
			eventModelChanged();
		}
	}

	void MainModel::setStudy(const XrayStudy& newStudy)
	{
		if (!study.equal(newStudy))
		{
			study = newStudy;
			notify();
		}
	}

	XrayStudy MainModel::getStudy() const
	{
		return study;
	}

	bool MainModel::isCrmLicenseActive() const
	{
		return m_licenseCollection.isActive(LicenseFeature::Roadmap);
	}


	bool MainModel::isAnyStbLicenseActive() const
	{
		return m_licenseCollection.isAnyActive({ LicenseFeature::StentboostLive,LicenseFeature::StbPostDeploy,LicenseFeature::StbSubtract });
	}


	bool MainModel::isCoregLicenseActive() const
	{
		return m_licenseCollection.isActive(LicenseFeature::Coregistration);
	}

	const ILicenseCollection& MainModel::getLicenseCollection() const
	{
		return m_licenseCollection;
	}

	void MainModel::setCoregStatusText(const std::wstring& text)
	{
		if (coregStatus != text)
		{
			coregStatus = text;
			notify();
		}
	}

	std::wstring MainModel::getCoregStatusText() const
	{
		return coregStatus;
	}

	void MainModel::setCoregConnected(bool connected)
	{
		if (coregConnected != connected)
		{
			coregConnected		= connected;
			notify();
		}
	}

	bool MainModel::getCoregConnected() const
	{
		return coregConnected;
	}

	void MainModel::setTsmFunctionsAvailable(bool available)
	{
		tsmFunctionsAvailable = available;
		notify();
	}

	bool MainModel::getTsmFunctionsAvailable() const
	{
		return tsmFunctionsAvailable;
	}

	void MainModel::setIControlConnected(bool connected)
	{
		iControlConnected = connected;
		notify();
	}

	bool MainModel::isIControlConnected() const
	{
		return iControlConnected;
	}

	void MainModel::setSnapshotStatus(ArchivingStatus status, CaptureArea area)
	{
		if ((snapshotStatus != status) || (snapshotArea != area))
		{
			snapshotStatus = status;
			snapshotArea = area;
			notify();
		}
	}

	void MainModel::setMovieStatus(ArchivingStatus status)
	{
		if (movieStatus != status)
		{
			movieStatus = status;
			notify();
		}
	}

	ArchivingStatus MainModel::getSnapshotStatus() const
	{
		return snapshotStatus;
	}

	CaptureArea MainModel::getSnapshotArea() const
	{
		return snapshotArea;
	}

	ArchivingStatus MainModel::getMovieStatus() const
	{
		return movieStatus;
	}

	bool MainModel::getSystemMenuEnabled() const
	{
		return systemMenuEnabled;
	}

	void MainModel::setSystemMenuEnabled(bool val)
	{
		systemMenuEnabled = val;
	}

	CommonPlatform::Xray::SystemType MainModel::getSystemType() const
	{
		return systemType;
	}

}}