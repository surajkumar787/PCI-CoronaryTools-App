//Copyright (c)2012 Koninklijke Philips Electronics N.V.,
//All Rights Reserved.
//
//This source code and any compilation or derivative thereof is the
//proprietary information of Koninklijke Philips Electronics N.V.
//and is confidential in nature.
//Under no circumstances is this software to be combined with any Open Source
//Software in any way or placed under an Open Source License of any type 
//without the express written permission of Koninklijke Philips 
//Electronics N.V.

#include "CBDDev.h"

// *** Internal private parameters
typedef struct 
{ 
	int detectedCathSeq;			// Res
	CBDTrack *detectedTrackSeq;

	int nbImProcessed;
	int ShutterPos[4];
	CCBDResults * ResIms;

	CCBD *CBD;
} CCBDSeq; 