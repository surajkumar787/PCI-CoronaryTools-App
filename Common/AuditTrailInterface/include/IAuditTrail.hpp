// Copyright Koninklijke Philips N.V. 2016
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <sscfts1/interface.begin.h>

#ifdef SSCFTS1_BEGIN_INTERFACE

SSCFTS1_BEGIN_STRUCT(NodeAuditTrailInfo)
SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), AETitle)
SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), IPAddress)
SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), HostName)
SSCFTS1_END_STRUCT

SSCFTS1_BEGIN_STRUCT(InstanceAuditTrailInfo)
SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), PatientID)
SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), PatientName)
SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), StudyInstanceID)
SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), AccessionNumber)
SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), PerformedProcedureStepID)
SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), SOPClassUID)
SSCFTS1_END_STRUCT

SSCFTS1_BEGIN_STRUCT(ConnectionInfo)
	SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), Hostname)
	SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), Protocol)
	SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), PortNumber)
	SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), SecurityType)
	SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), CertificateName)
SSCFTS1_END_STRUCT

SSCFTS1_BEGIN_INTERFACE(IAuditTrail)
	SSCFTS1_BEGIN_METHODS(IAuditTrail)
		SSCFTS1_INTERFACE_METHOD_1(void, IAuditTrail, UserLogin, in(reftype(std::wstring) /*UserName*/))
		SSCFTS1_INTERFACE_METHOD_1(void, IAuditTrail, UserLoginFailed, in(reftype(std::wstring) /*UserName*/))
		SSCFTS1_INTERFACE_METHOD_1(void, IAuditTrail, UserLogoff, in(reftype(std::wstring) /*UserName*/))
		SSCFTS1_INTERFACE_METHOD_2(void, IAuditTrail, ApplicationStarted, in(reftype(std::wstring) /*ApplicationIdentifier*/), in(reftype(std::wstring) /*UserName*/))
		SSCFTS1_INTERFACE_METHOD_2(void, IAuditTrail, ApplicationStartFailed, in(reftype(std::wstring)/*ApplicationIdentifier*/), in(reftype(std::wstring) /*UserName*/))
		SSCFTS1_INTERFACE_METHOD_2(void, IAuditTrail, ApplicationStopped, in(reftype(std::wstring) /*ApplicationIdentifier*/), in(reftype(std::wstring) /*UserName*/))
		SSCFTS1_INTERFACE_METHOD_2(void, IAuditTrail, ApplicationStopFailed, in(reftype(std::wstring) /*ApplicationIdentifier*/), in(reftype(std::wstring) /*UserName*/))
		SSCFTS1_INTERFACE_METHOD_3(void, IAuditTrail, InstancesCreated, in(reftype(std::vector<reftype(InstanceAuditTrailInfo)>) /*InstancesInfo*/), in(reftype(std::wstring) /*UserName*/), in(reftype(std::wstring) /*ProcessIdentifier*/))
		SSCFTS1_INTERFACE_METHOD_3(void, IAuditTrail, InstancesCreationFailed, in(reftype(std::vector<reftype(InstanceAuditTrailInfo)>) /*InstancesInfo*/), in(reftype(std::wstring) /*UserName*/), in(reftype(std::wstring) /*ProcessIdentifier*/))
		SSCFTS1_INTERFACE_METHOD_3(void, IAuditTrail, InstancesRead, in(reftype(std::vector<reftype(InstanceAuditTrailInfo)>) /*InstancesInfo*/), in(reftype(std::wstring) /*UserName*/), in(reftype(std::wstring) /*ProcessIndentifier*/))
		SSCFTS1_INTERFACE_METHOD_3(void, IAuditTrail, InstancesReadFailed, in(reftype(std::vector<reftype(InstanceAuditTrailInfo)>) /*InstancesInfo*/), in(reftype(std::wstring) /*UserName*/), in(reftype(std::wstring) /*ProcessIdentifier*/))
		SSCFTS1_INTERFACE_METHOD_1(void, IAuditTrail, NetworkNodeAdded, in(reftype(std::wstring) /*NodeIdentifier*/))
		SSCFTS1_INTERFACE_METHOD_1(void, IAuditTrail, NetworkNodeRemoved, in(reftype(std::wstring) /*NodeIdentifier*/))
		SSCFTS1_INTERFACE_METHOD_3(void, IAuditTrail, BeginTransferring, in(reftype(std::vector<reftype(InstanceAuditTrailInfo)>) /*InstancesInfo*/), in(reftype(NodeAuditTrailInfo) /*SourceNodeInfo*/), in(reftype(NodeAuditTrailInfo) /*TargetNodeInfo*/))
		SSCFTS1_INTERFACE_METHOD_3(void, IAuditTrail, BeginTransferFailed, in(reftype(std::vector<reftype(InstanceAuditTrailInfo)> /*InstancesInfo*/)), in(reftype(NodeAuditTrailInfo) /*SourceNodeInfo*/), in(reftype(NodeAuditTrailInfo) /*TargetNodeInfo*/))
		SSCFTS1_INTERFACE_METHOD_3(void, IAuditTrail, InstancesTransferred, in(reftype(std::vector<reftype(InstanceAuditTrailInfo)> /*InstancesInfo*/)), in(reftype(NodeAuditTrailInfo) /*SourceNodeInfo*/), in(reftype(NodeAuditTrailInfo) /*TargetNodeInfo*/))
		SSCFTS1_INTERFACE_METHOD_3(void, IAuditTrail, InstancesTransferFailed, in(reftype(std::vector<reftype(InstanceAuditTrailInfo)> /*InstancesInfo*/)), in(reftype(NodeAuditTrailInfo) /*SourceNodeInfo*/), in(reftype(NodeAuditTrailInfo) /*TargetNodeInfo*/))
		SSCFTS1_INTERFACE_METHOD_5(void, IAuditTrail, SecurityAlert, in(AuditSecurityOptions /*AuditSecurityOptions*/), in(reftype(std::wstring) /*Description*/), in(reftype(std::wstring) /*participantObjectID*/), in(reftype(std::wstring) /*UserName*/), in(reftype(std::wstring) /*ProcessIdentifier*/))
		SSCFTS1_INTERFACE_METHOD_4(void, IAuditTrail, SecurityFailure, in(AuditSecurityOptions /*AuditSecurityOptions*/), in(reftype(std::wstring) /*Description*/), in(reftype(std::wstring) /*UserName*/), in(reftype(std::wstring) /*ProcessIdentifier*/))
	SSCFTS1_END_METHODS

	SSCFTS1_BEGIN_EVENTS(IAuditTrail)
	SSCFTS1_END_EVENTS

SSCFTS1_END_INTERFACE

SSCFTS1_BEGIN_INTERFACE(IAuditTrailConnectionTest)
	SSCFTS1_BEGIN_METHODS(IAuditTrailConnectionTest)
		SSCFTS1_INTERFACE_METHOD_1(int, IAuditTrailConnectionTest, Execute, in(reftype(ConnectionInfo) /*connectionInfo*/))
	SSCFTS1_END_METHODS

	SSCFTS1_BEGIN_EVENTS(IAuditTrailConnectionTest)
	SSCFTS1_END_EVENTS

SSCFTS1_END_INTERFACE

#endif

#include <sscfts1/interface.end.h>
