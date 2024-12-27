// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

namespace CommonPlatform { namespace AppLauncher
{
#if defined(__cplusplus_cli)
	public
#endif
	enum class Applications
	{
		Shell,
		LoaderServer,
		PciApp,
		FieldService,
		GpduListener,
		SuSdListenerBoSS,
		SuSdListenerISuSdLab,
		CwisMouse,
		PacsConfigServiceClient,
		PacsConfigServiceServer,
		Fingerprint,
		FirstTimeConfig,
		OnScreenKeyboard,
		ProblemReport,
		ProblemReportHidden,
		DebugCmd,
		SetMonitorConfiguration,
		SetMonitorResolution,
		AuditTrailService,
		CrashRecovery,
		SystemShutdownListener,

		NetworkConnectionsPanel,
		DateTimePanel,
		NTPPanel,
		RegionalSettings,
		ServiceHubLauncher,
		ProblemReportForServiceHub
	};
}}