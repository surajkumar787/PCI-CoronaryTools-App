// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#pragma pack(push, 16)

namespace CommonPlatform { namespace CoRegistration{

// Tags used at the Data Packet level
enum Tag
{
	StateDataTag,
	PressureAndEcgDataTag,
	FmCurrentFrameDataTag,
	FMSpotTag, 
	iFRPullbackTag,
	iFRSpotTag, // rename to iFRSpotData
	iFRSpotResultTag,
	MAPTag,

	FMCommandTag,
	TimeSyncTag,
	SysInfoTag,
	ErrorDataTag,

	ScaleOptionsTag,

	MaxTag = 150,
};

struct PacketHeader{
	Tag tag;
	int size;
};

enum State
{
	Idle = 200,
	Zero, // TODO: Review this state
	Freeze,
	Normalization,

	FfrLive,
	FfrSpotAnalyze,
	FfrSpotReview,

	IfrLive,
	IfrPullbackAnalyze,
	IfrPullbackReview,

	IfrSpotAnalyze,
	IfrSpotReview,

	FfrPullbackAnalyze,
	FfrPullbackReview,

	MaxState = 250,
};

enum StateBitmask
{
	eNormalized = 0x00000001,
	eEcgConnected = 0x00000002,
	eDemoMode     = 0x00000004,

	ePimConnected = 0x00000010,
	ePressurePlugConnected = 0x00000020,
	eWireConnected = 0x00000040,
	eWireShorted = 0x00000080, // If this bit is set, there's an issue with the wire
	eWireReady = 0x00000100, // Autozero failed, this flag will not be set
};

struct MAPdata
{
	double timestamp;
	float mapPd;
	float mapPa;
	float pdpaRatio;
};

struct StateData
{
	double timestamp;
	State state;
	int  iStateBitmask; // includes PIM Status for example, Normalized?
};

struct FMSpotData 
{
	int whichRun;
	double timestamp;
	float fFm; // in FFR/iFR => FFR or iFR
};

struct iFRSpotResultData
{
	float iFr;
};

struct iFRSpotData
{
	int whichRun;
	double iStartTime;
	double timestamp; // Referred to the timestamp of the cardiac cycle
	bool validFlag;
	double iWfp_start;
	double iWfp_end;
};

struct PressureAndEcgData
{
	double timestamp;
	float ecgVal;
	float pdVal;
	float paVal;
};

struct iFRPullbackData
{
	int whichRun;
	double iStartTime;
	double timestamp; // End of Cardiac Cycle Timestamp
	float trendValue;
	float rawValue;
	bool filtered;
	bool validFlag;
	double iWfp_start; 
	double iWfp_end;
};

struct FmCurrentFrameMarker
{
	double timestamp;
};

/* ERROR Structure and Enums start here */
enum ErrorType
{
	eNormalizationFailed = 1000,
	eFFRSpotFailed,
	eiFRSpotFailed,
	eiFR_FailedConfidenceLevel,
	eiFRPullbackFailed,
	eiFRPullbackFailedFilter,
	eiFRPullbackFailedShortRun,
	eCommandFailedWrongState,
	eMaxTimeExceeded,
	eNoMoreSpaceOnHDD,
	eHDDFull,
	eMaxRunExceeded,
	eZeroPaFailed,
	eUnknownCommand,
	eMaxError = 1100,
};

struct Error
{
	double timestamp;
	ErrorType errorType;
};


/* Commands Struct and Enum starts here*/
enum FmCommands // TO DO: add the remaining commands from the shortkeys
{
	eStartNormalize = 2000,
	eStartFFRSpot,
	eStopFFRSpot,
	eStartIfrSpot,
	eStartIfrPullback,
	eStopIfrPullback,
	eLiveMode,
	eFreeze,
	ePatientTab, //VK_F1 + VK_CONTROL
	eSettingTab, //VK_F2 + VK_CONTROL or VK_F8
	eFFRTab, //VK_F3 + VK_CONTROL
	eiFRTab, //VK_F4 + VK_CONTROL
	eNextLeftTab, //VK_F5 + VK_CONTROL
	eNextRightTab, //VK_F6 + VK_CONTROL
	eSaveFrame, //VK_F5
	eBookmark, //VK_F10

	eTimeSync,
	eSysInfo,
	eNewPatient,
	eZeroPa,
	eSysState,

	eMaxCommand = 2100,
};


struct FMCommand 
{
	FmCommands cAction;
};

struct SysInfo
{
	char swVersion[20];
	char protocolVersion[20];
	char serialNumber[20];
	bool license;
};

struct TimeSync
{
	double timestamp;
};

struct ScaleOptionsData
{
	float pressMin;
	float pressMax;
	float pullbackMin;
	float pullbackMax;
	bool showRaw;
	bool showWfp;
};

}}

#pragma pack(pop)