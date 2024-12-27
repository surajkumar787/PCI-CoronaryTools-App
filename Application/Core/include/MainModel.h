// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "XrayStudy.h"
#include "SystemType.h"

#include <functional>

namespace Pci { namespace Core
{
	enum class LicenseStatus;
	enum class ArchivingStatus;
	enum class CaptureArea;
	class ILicenseCollection;

	class MainModel
	{
	public:
		MainModel(const ILicenseCollection &licenseCollection, CommonPlatform::Xray::SystemType system);

		void setStudy(const CommonPlatform::Xray::XrayStudy& newStudy);
		CommonPlatform::Xray::XrayStudy getStudy() const;

		bool isCrmLicenseActive() const;
		bool isAnyStbLicenseActive() const;
		bool isCoregLicenseActive() const;
		const ILicenseCollection& getLicenseCollection()const;

		void setCoregStatusText(const std::wstring& text);
		std::wstring getCoregStatusText() const;

		void setCoregConnected(bool connected);
		bool getCoregConnected() const;

		void setTsmFunctionsAvailable(bool available);
		bool getTsmFunctionsAvailable() const;

		void setIControlConnected(bool connected);
		bool isIControlConnected() const;

		void setSnapshotStatus(ArchivingStatus status, CaptureArea area);
		void setMovieStatus(ArchivingStatus status);
		ArchivingStatus getSnapshotStatus() const;
		CaptureArea getSnapshotArea() const;
		ArchivingStatus getMovieStatus() const;

		void setSystemMenuEnabled(bool val);
		bool getSystemMenuEnabled() const;

		CommonPlatform::Xray::SystemType getSystemType() const;

		std::function<void()> eventModelChanged;

	private:
		void notify() const;

		CommonPlatform::Xray::XrayStudy study;
		const ILicenseCollection&	m_licenseCollection;


		std::wstring coregStatus;
		bool coregConnected; 
		bool iControlConnected;

		bool systemMenuEnabled;

		bool tsmFunctionsAvailable;
		
		ArchivingStatus movieStatus;
		ArchivingStatus snapshotStatus;
		CaptureArea snapshotArea;

		CommonPlatform::Xray::SystemType systemType;
	};
}}
