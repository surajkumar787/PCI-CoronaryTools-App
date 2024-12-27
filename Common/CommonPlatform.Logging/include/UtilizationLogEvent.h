// Copyright Koninklijke Philips N.V., 2016.
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//

#pragma once

#include "Log.h"
#include <vector>

namespace  CommonPlatform{ namespace UtilizationLogEvent{

	struct LicensesData
	{
		std::wstring license;
		std::wstring status;
	};

	// platform loggings
	void	ConnectedHospitalNetworkToXray(::CommonPlatform::Log &log, const std::wstring &networkName , const std::wstring &ipAddress );
	void	ConnectionLostToHospitalNetworkXray( ::CommonPlatform::Log &log);
	void	ConnectedRealTimeLinkToXray( ::CommonPlatform::Log &log , const std::wstring &networkName , const std::wstring &ipAddress  );
	void	ConnectionLostToRealTimeLinkXray( ::CommonPlatform::Log &log);
	void	ConnectedToFfrIfrSystem( ::CommonPlatform::Log &log, const std::wstring &networkName ,const std::wstring &ipAddress, const std::wstring &interfaceVersion );
	void	ConnectionLostToFfrIfrSystem( ::CommonPlatform::Log &log);
	void	FirstTimeConfigApplied( ::CommonPlatform::Log &log , const std::wstring &anny );
	void	MissingLicense( ::CommonPlatform::Log &log, const std::vector<LicensesData> &licenses);
	void	XrayLicenseMissing( ::CommonPlatform::Log &log, const std::wstring &license);
	void	APCFailedToStart( ::CommonPlatform::Log &log);
	void	NewEPXSelected( ::CommonPlatform::Log &log, const std::wstring &selectedEpx);
	void	ApplicationStateChanged( ::CommonPlatform::Log &log, const std::wstring &state);
	void	AppUnableToProceed( ::CommonPlatform::Log &log, const std::wstring &details);
	void	ArchiveSnapshotFailed( ::CommonPlatform::Log &log, const std::wstring &details);
	void	ArchiveMovieFailed( ::CommonPlatform::Log &log, const std::wstring &details);
	// crm loggings

	void	ArchiveSnapshotCrm( ::CommonPlatform::Log &log);
	void	MovieRecordStart( ::CommonPlatform::Log &log);
	void	MovieRecordEnd( ::CommonPlatform::Log &log);
	void	PreviousFrame( ::CommonPlatform::Log &log);
	void	PlayStop( ::CommonPlatform::Log &log);
	void	NextFrame( ::CommonPlatform::Log &log);
	void	TimeSynchronizationFFRiFR( ::CommonPlatform::Log &log, const std::wstring &calculatedOffset);
	void	TimeSynchronizationFFRiFR_Failed( ::CommonPlatform::Log &log, const std::wstring &latency);
	
	// stantBoost
	void	ArchiveSnapshotSbl( ::CommonPlatform::Log &log);
	void	ArchiveMovieSbl( ::CommonPlatform::Log &log);
}}

