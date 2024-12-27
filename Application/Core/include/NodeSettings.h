// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "Settings.h"

#include <mc3msg.h>

namespace CommonPlatform { namespace Dicom { namespace TLS
{
	enum class SecurityType;
}}}

namespace Pci { namespace Core { namespace PACS 
{
	struct NodeSettings
	{
		explicit NodeSettings(::CommonPlatform::Settings& settings, const wchar_t* group);

		std::wstring LocalAeTitle;
		std::wstring RemoteAeTitle;
		std::wstring RemoteHost;
		int RemotePort;
		::CommonPlatform::Dicom::TLS::SecurityType Security;
		std::wstring Certificate;
	};

	struct StorageNodeSettings : public NodeSettings
	{
		explicit StorageNodeSettings(::CommonPlatform::Settings& settings, const wchar_t* group = L"pacs_storage");

		bool Enabled;
		bool SupportSnapshots;
		bool SupportMovies;
		TRANSFER_SYNTAX SnapshotTransferSyntax;
		TRANSFER_SYNTAX MovieTransferSyntax;
	};

	struct CommitNodeSettings : public NodeSettings
	{
		explicit CommitNodeSettings(::CommonPlatform::Settings& settings, const wchar_t* group = L"pacs_commit");

		bool Enabled;
		int ListeningPort;
	};

}}}