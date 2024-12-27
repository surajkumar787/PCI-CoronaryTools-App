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

#include "CBDLib.h"

// *************************************************************
// *************************************************************
// ************ SEQUENCE-BASED PART OF THE LIBRARY *************
// *************************************************************
// *************************************************************

// The part dedicated to the processing of one single frame is CSBLib.h

// *** Catheter detection parameters for sequences
typedef struct{
	float rateImDetectionForSeriesDetection;	// Nb of independent frames where a cath body has to be detected
												// for the sequence detection to be positive (rateImDetectionForSeriesDetection * NbImMax)
												// (Default 3.2f/5.f)
	int NbImMax;								// Nb of considered first frames (default 5)
}CCBDParamsSeq;

// *** Catheter body detection results
typedef struct{
	int detectedCathSeq;				// Detection result over the sequence
	CBDTrack *detectedTrackSeq;			// Corresponding track (if any)
	int selectedIndice;					// The cath body profile has been detected over that frame
	CCBDResults CBDResultsLastIm;		// Results of the image catheter body extraction over the last processed image
}CCBDResultsSequence;

// Library handle
typedef struct 
{ 
	void*   Pv;     // Access to private material
} CCBDHdlSeq;

#ifdef __cplusplus
extern "C" {
#endif

	#ifdef CBD_DLLEXPORT
	#undef CBD_DLLEXPORT
	#endif // ifdef DLLEXPORT

	#ifdef CSBLIB_EXPORTS
	#define CBD_DLLEXPORT __declspec(dllexport)
	#else
	#define CBD_DLLEXPORT
	#endif

	// Handle management
	CBD_DLLEXPORT int CBDSeqCreate(CCBDHdlSeq * PtrCBDResSeq, CCBDParamsSeq * Params);
	CBD_DLLEXPORT int CBDSeqDelete(CCBDHdlSeq PtrCBDResSeq, CCBDParamsSeq * Params);
	CBD_DLLEXPORT int CDBDefaultParametersSeq(CCBDParamsSeq * Params);

	// Sequence processing
	CBD_DLLEXPORT int CatheterBodyDetectionSequence(short * ImIn, int t, int Iw, int Ih, float Rot, float Ang
												, int SSFactor, CCDBSequenceType CDBSequenceType
												, CCBDHdlSeq * PtrCBDHdlSeq, CCBDResultsSequence * CCBDResultsSequence);

	// Based on detection results on a series of images, decides whether a catheter body was there, 
	// and determines the best representant
	CBD_DLLEXPORT int CatheterBodyDecisionMultipleFrames(CCBDParamsSeq * Params, CCBDResults * PtrCBDDetectionsListIn, int nbIms
													, CCBDResultsSequence * PtrCBDDetectionOut);

#ifdef __cplusplus
};
#endif