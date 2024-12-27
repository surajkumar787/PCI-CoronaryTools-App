// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_MKXTIMING_H
#define		_MKXTIMING_H


enum{	
	TIMING_Misc=0,
	TIMING_MkxSetExtractProcFromParam,
	TIMING_MkxRdg1,
	TIMING_MkxRdg2,
	TIMING_MkxRdg,
	TIMING_MkxBlobEnhance,
	TIMING_MkxApplyRigidMotionMask,
	TIMING_MkxPixSelect,
	TIMING_MkxLabelling,
	TIMING_MkxMarkersSelect, 
	TIMING_MkxCoupleSelect,
	TIMING_MkxTracking_FillResults,
	TIMING_MkxInitWire,
	TIMING_MkxGetWire,
	TIMING_MkxHistory,
	TIMING_MkxRestartTracker,
	TIMING_MkxTotal,

	TIMING_Length
};

#if MKX_TIMING
static const char *TimingNames[]={
	"Misc",
	"MkxSetExtractProcFromParam",
	"MkxRdg1",
	"MkxRdg2",
	"MkxRdg",
	"MkxBlobEnhance",
	"MkxApplyRigidMotionMask",
	"MkxPixSelect",
	"MkxLabelling",
	"MkxMarkersSelect", 
	"MkxCoupleSelect",
	"MkxTracking+FillResults",
	"MkxInitWire",
	"MkxGetWire",
	"MkxHistory",
    "MkxRestartTracker",
	"MkxTotal",
	 
	"Length"
};
#endif

typedef struct
{
	int TimeFor[64];
	float fTimeFor[64];
	int IsAvailable;

}CTiming;


#endif //_MKXTIMING_H