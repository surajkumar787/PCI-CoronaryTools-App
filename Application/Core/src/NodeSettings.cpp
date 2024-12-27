// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "NodeSettings.h"
#include "Log.h"
#include "stlstring.h"
#include "util.h"
#include "exception.h"
#include "tlsdefines.h"

#include <mc3msg.h>

namespace Pci { namespace Core { namespace PACS 
{
	namespace Merge = ::CommonPlatform::Dicom::Merge;
	namespace TLS = ::CommonPlatform::Dicom::TLS;

	static const std::wstring DefaultLocalAe(L"PCIe");
	static const std::wstring DefaultRemoteAe( L"PacsSrv");
	static const std::wstring DefaultRemoteHost( L"127.0.0.1");
	static const int DefaultRemotePort = 250;
	static const std::wstring DefaultSecurity( L"None");
	static const std::wstring DefaultCertificate( L"");

	static const std::wstring DefaultSnapshotTs( L"1.2.840.10008.1.2.4.70");
	static const TRANSFER_SYNTAX DefaultParsedSnapshotTs = IMPLICIT_LITTLE_ENDIAN;
	static const std::wstring DefaultMovieTs( L"1.2.840.10008.1.2.4.50");
	static const TRANSFER_SYNTAX DefaultParsedMovieTs = JPEG_LOSSLESS_HIER_14;
	static const bool DefaultSupportSnapshots = true;
	static const bool DefaultSupportMovies = true;

	static const bool DefaultCommitEnabled = true;
	static const int DefaultLocalPort = 241;

	NodeSettings::NodeSettings(CommonPlatform::Settings& settings, const wchar_t* group) :
		LocalAeTitle(settings.get(group, L"localAE", DefaultLocalAe.c_str())),
		RemoteAeTitle(settings.get(group, L"remoteAE", DefaultRemoteAe.c_str())),
		RemoteHost(settings.get(group, L"remoteHost", DefaultRemoteHost.c_str())),
		RemotePort(settings.get(group, L"remotePort", DefaultRemotePort)),
		Certificate(settings.get(group, L"certificate", DefaultCertificate.c_str()))
	{
		std::wstring strSec = settings.get(group, L"security", DefaultSecurity.c_str());
		if (strSec == L"Authentication")  Security = TLS::SecurityType::AUTHENTICATION;
		else if (strSec == L"Encryption") Security = TLS::SecurityType::ENCRYPTION;
		else if (strSec == L"None")       Security = TLS::SecurityType::NONE;
		else
		{
			Security = TLS::SecurityType::NONE;
			::CommonPlatform::Logger(::CommonPlatform::LoggerType::Platform).DeveloperWarning(L"NodeSettings: found an unknown value for securitytype; %s", strSec.c_str());
		}

		if (Security != TLS::SecurityType::NONE && Certificate.empty())
		{
			Security = TLS::SecurityType::NONE;
			::CommonPlatform::Logger(::CommonPlatform::LoggerType::Platform).DeveloperWarning(L"NodeSettings: found an empty certificate specifier with security enabled. Security disabled");
		}
	}

	StorageNodeSettings::StorageNodeSettings(CommonPlatform::Settings& settings, const wchar_t* group) :
		NodeSettings(settings, group),
		Enabled(settings.get(group, L"enabled", true)),
		SupportSnapshots(settings.get(group, L"supportSnapshots", DefaultSupportSnapshots) && Enabled),
		SupportMovies(settings.get(group, L"supportMovies", DefaultSupportMovies) && Enabled),
		SnapshotTransferSyntax(DefaultParsedSnapshotTs),
		MovieTransferSyntax(DefaultParsedMovieTs)
	{
		auto& log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);

		auto strTs = settings.get(group, L"snapshotTransferSyntax", DefaultSnapshotTs.c_str());
		// empty strings parse as "EXPLICIT_BIG_ENDIAN" in MC_Get_Enum_From_Transfer_Syntax, using that will fail at runtime since mergecom does not support it
		if (strTs.empty() || (MC_Get_Enum_From_Transfer_Syntax(str_ext::wtos(strTs).c_str(), &SnapshotTransferSyntax) != MC_NORMAL_COMPLETION))
		{
			SnapshotTransferSyntax = DefaultParsedSnapshotTs;
			SupportSnapshots = false;
			log.DeveloperWarning(L"Invalid snapshot transfer syntax found (%s), disabling snapshots", strTs.c_str());
		}

		strTs = settings.get(group, L"movieTransferSyntax", DefaultMovieTs.c_str());
		// empty strings parse as "EXPLICIT_BIG_ENDIAN" in MC_Get_Enum_From_Transfer_Syntax, using that will fail at runtime since mergecom does not support it
		if (strTs.empty() || (MC_Get_Enum_From_Transfer_Syntax(str_ext::wtos(strTs).c_str(), &MovieTransferSyntax) != MC_NORMAL_COMPLETION))
		{
			MovieTransferSyntax = DefaultParsedMovieTs;
			SupportMovies = false;
			log.DeveloperWarning(L"Invalid movie transfer syntax found (%s), disabling movies", strTs.c_str());
		}

		log.DeveloperInfo(L"Loaded PACS Storage Node settings.\n"
						  L"Enabled = %d\n"
						  L"LocalAe = %s\n"
						  L"RemoteAe = %s\n"
						  L"RemoteHost = %s\n"
						  L"RemotePort = %d\n"
						  L"Snapshots = %d\n"
						  L"Movies = %d\n"
						  L"Snapshot TS = %d\n"
						  L"Movie TS = %d\n"
						  L"Security = %d\n"
						  L"Certificate = %s",
						  Enabled, LocalAeTitle.c_str(), RemoteAeTitle.c_str(), RemoteHost.c_str(), RemotePort,
						  SupportSnapshots, SupportMovies, SnapshotTransferSyntax, MovieTransferSyntax,
						  Security, Certificate.c_str());
	}

	CommitNodeSettings::CommitNodeSettings(CommonPlatform::Settings& settings, const wchar_t* group) :
		NodeSettings(settings, group),
		Enabled(settings.get(group, L"enabled", DefaultCommitEnabled)),
		ListeningPort(settings.get(group, L"listeningPort", DefaultLocalPort))
	{
		auto& log = CommonPlatform::Logger(::CommonPlatform::LoggerType::Platform);

		log.DeveloperInfo(L"Loaded PACS Commit Node settings.\n"
						  L"Enabled = %d\n"
						  L"LocalAe = %s\n"
						  L"ListeningPort = %d\n"
						  L"RemoteAe = %s\n"
						  L"RemoteHost = %s\n"
						  L"RemotePort = %d\n"
						  L"Security = %d\n"
						  L"Certificate = %s",
						  Enabled, LocalAeTitle.c_str(), ListeningPort, RemoteAeTitle.c_str(), RemoteHost.c_str(), RemotePort,
						  Security, Certificate.c_str());
	}

}}}