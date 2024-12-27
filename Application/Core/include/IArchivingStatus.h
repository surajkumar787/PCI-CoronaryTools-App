// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

namespace Pci { namespace Core
{
	enum class CaptureArea { MainView, MainAndSideView };
	enum class ArchivingStatus { None, Ready, Busy, Success, Failure };

	class IArchivingStatus
	{
	public:
		virtual ~IArchivingStatus() {}

		virtual void setSnapshotStatus(ArchivingStatus status, CaptureArea area) = 0;
		virtual void setMovieStatus(ArchivingStatus status) = 0;

		virtual void clearStatus() = 0;
	};

}}
