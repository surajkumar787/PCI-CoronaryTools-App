// Copyright Koninklijke Philips N.V. 2016
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#if defined (__cplusplus_cli)
	#if defined(INCLUDE_NATIVE)
		#pragma unmanaged
	#else
		public
	#endif
#endif

enum class AuditSecurityOptions
{
	Unknown = 0,
	NodeAuthentication,
	EmergencyOverrideStarted,
	NetworkConfiguration,
	SecurityConfiguration,
	HardwareConfiguration,
	SoftwareConfiguration,
	UseOfRestrictedFunction,
	AuditRecordingStopped,
	AuditRecordingStarted,
	ObjectSecurityAttributesChanged,
	SecurityRolesChanged,
	UserSecurityAttributesChanged,
	EmergencyOverrideStopped,
	RemoteServiceOperationStarted,
	RemoteServiceOperationStopped,
	LocalServiceOperationStarted,
	LocalServiceOperationStopped
};

#if defined(__cplusplus_cli) && defined(INCLUDE_NATIVE)
	#pragma managed
#endif
