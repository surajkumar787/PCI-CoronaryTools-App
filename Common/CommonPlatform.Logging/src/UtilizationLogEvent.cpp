//  Copyright Koninklijke Philips N.V., 2016.
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//

#include "UtilizationLogEvent.h"
#include <stdio.h>
#include <vector>
#include <Windows.h>
#include "log.h"

using namespace CommonPlatform;

namespace CommonPlatform{ namespace UtilizationLogEvent{
	
/****************************************************************************/
// platform
/****************************************************************************/

std::wstring CreateEventName(Log &log, const int eventId)
{
	wchar_t eventName[20];
	if (swprintf_s(eventName, L"%d%06d", log.GetOriginatorId(), eventId) <= 0)
	{
		log.DeveloperError(L"could not create eventName for UtilizationLogEvent");
	}
	return std::wstring(eventName);
}

std::wstring CreateMessage(const std::wstring infoFormat, const std::wstring argument)
{
	wchar_t message[200];
	swprintf_s(message, infoFormat.c_str(), argument.c_str());
	return std::wstring(message);
}

void ConnectedHospitalNetworkToXray(Log &log, const std::wstring &networkName , const std::wstring &ipAddress )
{
	wchar_t message[1000];
	const int	eventId = 8;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"Established the hospital network connection to the X-ray system.";
	const wchar_t	*infoFormat = L"[networkName=%s;address=%s]";
	if (swprintf_s(message, infoFormat, networkName.c_str(), ipAddress.c_str()) <= 0)
	{
		log.DeveloperError(L"Unable to create Established the hospital network connection to the X-ray system message");
	}

	log.UtilizationAppEvent(eventId,eventName.c_str(),description,message);
}

void ConnectionLostToHospitalNetworkXray( Log &log)
{
	const int eventId = 9;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"Lost the hospital network connection to the X-ray system";
	const wchar_t	*message = L"";

	log.ServiceAppError(eventId, eventName.c_str(),description,message);
}

void	ConnectedRealTimeLinkToXray( Log &log , const std::wstring &networkName , const std::wstring &ipAddress  )
{
	wchar_t message[200];
	const int eventId = 10;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"Established the Realtime link connection to the X-ray system.";
	const wchar_t	*infoFormat = L"[networkName=%s;address=%s]";
	if (swprintf_s(message, infoFormat, networkName.c_str(), ipAddress.c_str()) <= 0)
	{
		log.DeveloperError(L"Unable to create Established the Realtime link connection to the X-ray system massage");
	}

	log.UtilizationAppEvent(eventId, eventName.c_str(),description,message);
}

void	ConnectionLostToRealTimeLinkXray( Log &log)
{
	const int eventId = 11;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"Lost the Realtime link connection to the X-ray system";
	const wchar_t	*infoFormat = L"";

	log.ServiceAppError(eventId, eventName.c_str(),description,infoFormat);
}

void	ConnectedToFfrIfrSystem( Log &log, const std::wstring &networkName ,const std::wstring &ipAddress, const std::wstring &interfaceVersion )
{
	wchar_t message[200];
	const int eventId = 12;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"Established network connection to the FFR/iFR system.";
	const wchar_t	*infoFormat = L"[networkName=%s;address=%s;version=%s]";

	if (swprintf_s(message, infoFormat, networkName.c_str(), ipAddress.c_str(), interfaceVersion.c_str()) <= 0)
	{
		log.DeveloperError(L"Unable to create Established network connection to the FFR/iFR system massage");
	}

	log.UtilizationAppEvent(eventId, eventName.c_str(),description,message);
}

void	ConnectionLostToFfrIfrSystem( Log &log)
{
	const int eventId = 13;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"Lost connection to the FFR/iFR system.";
	const wchar_t	*infoFormat = L"";		

	log.ServiceAppError(eventId, eventName.c_str(),description,infoFormat);
}

void	FirstTimeConfigApplied( Log &log , const std::wstring & settings )
{
	const int eventId = 14;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"Processed first time configuration file.";
	std::wstring	infoFormat(L"[%s]");	

	auto message = CreateMessage(infoFormat, settings);
	log.UtilizationUserAction(eventId, eventName.c_str(),description,message.c_str());
}

void	MissingLicense( Log &log, const std::vector<LicensesData> &licenses)
{
	std::wstring message;
	const int eventId = 15;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"Missing license on workstation.";

	for(int i = 0 ; i < licenses.size() ; i++)
	{
		i==0? message.append(L"["):message.append(L"|");
		message.append(L"license=");
		message.append(licenses[i].license);
		message.append(L", status=");
		message.append(licenses[i].status);
	}
	message.append(L"]");

	log.ServiceAppError(eventId, eventName.c_str(),description,message.c_str());
}



void	XrayLicenseMissing( Log &log, const std::wstring &license)
{
	const int eventId = 16;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"Missing license on X-ray system.";
	std::wstring	infoFormat    (L"[license=%s]"); //PkgPointingDevice|PkgTSMControl3rdParty5|APClicense

	auto message = CreateMessage(infoFormat, license );
	log.ServiceAppError(eventId, eventName.c_str(),description,message.c_str());
}


void	APCFailedToStart( Log &log)
{
	const int eventId = 18;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"Failed to start the APC function.";
	const wchar_t	*infoFormat = L"";			

	log.UtilizationUserMessage(eventId, eventName.c_str(),description,infoFormat);
}

void	NewEPXSelected( Log &log, const std::wstring &selectedEpx)
{
	const int eventId = 19;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"New EPX selected.";
	std::wstring	infoFormat    (L": [epx=%s]");//CoronaryRoadmap|StentBoostLive|Other
		
	auto message = CreateMessage(infoFormat, selectedEpx );
	log.UtilizationUserAction(eventId, eventName.c_str(),description,message.c_str());
}

void	ApplicationStateChanged( Log &log, const std::wstring &state)
{
	const int eventId = 20;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"Application state changed";
	std::wstring	infoFormat    (L"[state=%s]"); // to Coronary Tools Guidance|Dynamic Coronary Roadmap Guidance|Dynamic Coronary Roadmap Creation|Dynamic Coronary Roadmap Overlay|FFR/iFR Roadmap Guidance|FFR/iFR Roadmap Overlay|FFR/iFR Roadmap Review|Stentboost Live Guidance|StentBoost Live Active].
	
	auto message = CreateMessage(infoFormat, state );
	log.UtilizationAppEvent(eventId, eventName.c_str(),description,message.c_str());
}

void	AppUnableToProceed( Log &log, const std::wstring &details)
{
	const int eventId = 22;
	
	auto eventName = CreateEventName( log, eventId);
	const wchar_t	*description = L"Application unable to proceed.";
	std::wstring	infoFormat    (L"[details=%s]");

	auto message = CreateMessage(infoFormat, details );
	log.ServiceSwAppError(eventId, eventName.c_str(),description,message.c_str());
}

void	ArchiveSnapshotFailed( Log &log, const std::wstring &details)
{
	const int serviceErrorEventId = 29;
	const int userMessageEventId = 35;

    const wchar_t	*description = L"Failed to archive snapshot.";
    std::wstring	infoFormat    (L"[details=%s]");
	auto message = CreateMessage(infoFormat, details );

	auto eventName = CreateEventName( log, serviceErrorEventId);
	log.ServiceAppError(serviceErrorEventId, eventName.c_str(),description,message.c_str());
	
	auto userEventName = CreateEventName( log, userMessageEventId);
	log.UtilizationUserMessage(userMessageEventId, userEventName.c_str(),description,message.c_str());
}

void	ArchiveMovieFailed( Log &log, const std::wstring &details)
{
	const int serviceErrorEventId = 30;	
	const int userMessageEventId = 36;	

	
    const wchar_t	*description = L"Failed to archive movie.";
    std::wstring	infoFormat    (L"[details=%s]");
	auto message = CreateMessage(infoFormat, details );

	auto eventName = CreateEventName( log, serviceErrorEventId);		
	log.ServiceAppError(serviceErrorEventId, eventName.c_str(),description,message.c_str());
	
	auto userEventName = CreateEventName( log, userMessageEventId);		
	log.UtilizationUserMessage(userMessageEventId, userEventName.c_str(),description,message.c_str());
}


/****************************************************************************/
// Coronary roadmap
/****************************************************************************/

void	ArchiveSnapshotCrm( Log &log)
{
	const int eventId = 1;
	
	auto eventName = CreateEventName( log, eventId);
    const wchar_t	*description = L"Function [name=Archive snapshot].";
    const wchar_t	*infoFormat = L"";

	log.UtilizationUserAction(eventId, eventName.c_str(),description,infoFormat);
}

void	MovieRecordStart( Log &log)
{
	const int eventId = 3;
	
	auto eventName = CreateEventName( log, eventId);
    const wchar_t	*description = L"Function [name=Start movie record].";
    const wchar_t	*infoFormat = L"";

	log.UtilizationStartAction(eventId, eventName.c_str(),description,infoFormat);
}

void	MovieRecordEnd( Log &log)
{
	const int eventId = 4;
	
	auto eventName = CreateEventName( log, eventId);
    const wchar_t	*description = L"Function [name=End movie record].";
    const wchar_t	*infoFormat = L"";

	log.UtilizationStopAction(eventId, eventName.c_str(),description,infoFormat);
}

void	PreviousFrame( Log &log)
{
	const int eventId = 6;
	
	auto eventName = CreateEventName( log, eventId);
    const wchar_t	*description = L"Function [name=Previous frame].";
    const wchar_t	*infoFormat = L"";

	log.UtilizationUserAction(eventId, eventName.c_str(),description,infoFormat);
}

void	PlayStop( Log &log)
{
	const int eventId = 7;
	
	auto eventName = CreateEventName( log, eventId);
    const wchar_t	*description = L"Function [name=Play/Stop].";
    const wchar_t	*infoFormat = L"";

	log.UtilizationUserAction(eventId, eventName.c_str(),description,infoFormat);
}

void	NextFrame( Log &log)
{
	const int eventId = 8;
	
	auto eventName = CreateEventName( log, eventId);
    const wchar_t	*description = L"Function [name=Next frame].";
    const wchar_t	*infoFormat = L"";

	log.UtilizationUserAction(eventId, eventName.c_str(),description,infoFormat);
}

void	TimeSynchronizationFFRiFR( Log &log, const std::wstring &calculatedOffset)
{
	const int eventId = 9;
	
	auto eventName = CreateEventName( log, eventId);
    const wchar_t	*description = L"Time synchronization with FFR/iFR system established.";
    std::wstring	 infoFormat   (L"[offset=%s]");	

	auto message = CreateMessage(infoFormat, calculatedOffset);
	log.UtilizationAppEvent(eventId, eventName.c_str(),description,message.c_str());
}

void	TimeSynchronizationFFRiFR_Failed( Log &log, const std::wstring &latency)
{
	const int eventId = 10;
	
	auto eventName = CreateEventName( log, eventId);
    const wchar_t	*description = L"Failed to establish time synchronization with FFR/iFR system.";
    std::wstring	 infoFormat   (L"[latency=%s]");

	auto message = CreateMessage(infoFormat, latency );
	log.ServiceAppError(eventId, eventName.c_str(),description,message.c_str());
}

/****************************************************************************/
// StentBoost
/****************************************************************************/
void	ArchiveSnapshotSbl( Log &log)
{
	const int eventId = 1;
	
	auto eventName = CreateEventName( log, eventId);
    const wchar_t	*description = L"Function [name=Archive snapshot].";
    const wchar_t	*infoFormat = L"";

	log.UtilizationUserAction(eventId, eventName.c_str(),description,infoFormat);
}


void	ArchiveMovieSbl( Log &log)
{
	const int eventId = 3;
	
	auto eventName = CreateEventName( log, eventId);
    const wchar_t	*description = L"Function [name=Archive movie].";
    const wchar_t	*infoFormat =  L"";

	log.UtilizationUserAction(eventId, eventName.c_str(),description,infoFormat);
}
}}


