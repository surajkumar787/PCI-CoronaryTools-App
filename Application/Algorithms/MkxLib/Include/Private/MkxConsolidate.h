// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


//
// Library developer-level umbrella include 
//
#ifndef _CSLDEV_H
#define _CSLDEV_H

#include <MkxDev.h>

//===================================================================================================
// CONSTANTS

#define CSL_MATCHDIST 5.f
#define CSL_CST_MIN_IMANB    1

#define CSL_CoupleConfidenceCoef       0.70f
#define CSL_SingleMarkerConfidenceCoef 0.15f
#define CSL_ConfidenceFuzzyCoefLow     0.02f
#define CSL_ConfidenceFuzzyCoefHigh    0.3f
#define CSL_nAvrForSmoothingForStableFeature 21
#define CSL_nAvrForSmoothingForMediumStableFeature 9
#define CSL_nIterForSmoothing 7

typedef enum { 
	CSL_ConfIndex_Cx = 0,
	CSL_ConfIndex_Cy,
	CSL_ConfIndex_Br,
	CSL_ConfIndex_An,
	CSL_ConfIndex_Intens1,
	CSL_ConfIndex_A1,
	CSL_ConfIndex_B1,
	CSL_ConfIndex_Dir1,
	CSL_ConfIndex_Contrast1,
	CSL_ConfIndex_Intens2,
	CSL_ConfIndex_A2,
	CSL_ConfIndex_B2,
	CSL_ConfIndex_Dir2,
	CSL_ConfIndex_Contrast2,

	CSL_ConfIndex_N_FEATURES                      // Number of features
}
CCslFeatureType;

enum
{
	MKX_NORMAL = 0
};

enum
{
	CSL_FALSE=0,
	CSL_TRUE
};


//===================================================================================================
// MACROS
//
//===================================================================================================
// STRUCTURES

typedef struct
{
	float *Intens1, *Intens2;
	float *LevelIn1, *LevelIn2;
	float *LevelOut1, *LevelOut2;
	float *Contrast1, *Contrast2;
	float *A1, *B1, *A2, *B2;
	float *Dir1, *Dir2;
	float *RefiningConf1, *RefiningConf2;
	int *Locked;
}
CslAdvanced;

typedef struct 
{
	int N;                         //number of allocated markers
	float *X1,*Y1,*X2,*Y2;         // markers coordinates
	float *Br, *An, *Cx, *Cy;      // derived coordinates
	int* CslStatus;                   // skip or not
	float *Confidence;
	BxCurve* Wire;
	int* WireStatus;
	CslAdvanced Advanced;

}Csl;

typedef struct  
{
	 float *Confidence;//confidence on each feature
	 float *Meas;     //measure of each feature
	 float *Estim;    //estimated value for each feature
}
CslFeatConfidence;

typedef struct 
{
	float *ScoreCouple; //Cx, Cy, Br, An
	float *ScoreM1;     // marker1: intensity, a, b, dir, contrast
	float *ScoreM2;
	float *ScoreGlobal; // a score computed from the 3 other 
	CslFeatConfidence *Conf; //data related to confidence on each feature (for each frame)
	float *Err1;
	float *Err2;
	int   *Used;
	int   *Locked;
	int   N;             //number of allocated markers
}
CslConfidence;

//===================================================================================================
// METHODS

#ifdef __cplusplus
extern "C" {
#endif

// MkxConsolidateFunc.cpp
void  CslXyToCbaOne(float x1, float y1, float x2, float y2, float* cx, float* cy, float* br, float* an );
void  CslXyToCba(Csl* ex);
void  CslFree(Csl* e);
void  S3dStatComput(int N, float* X, float* pt_Mean, float* pt_Min, float* pt_Max, float* pt_Stdv);
void  CslUpdateConfidence(Csl *extract, CslConfidence *mc);
void  S3dFreeMarkerConfidence(CslConfidence  *mc);
void  S3dAllocMarkerConfidence(CslConfidence  *mc);
void  CslPolarize(Csl* ex);
int   CslCreate(Csl* e, int n, CMkxExtractResults *er);
void  CslGetForImage(Csl* csl, int i, CMkxExtractResults* er);

// MkxDebugConfidence.cpp
void  CslPrintConfidence(CslConfidence *mc);
void  CslPrintConfidenceForOneFeature(CslConfidence *mc, int confIndex);
#if CSL_DEBUG_MX
void  CslMxShowConfidence(CslConfidence *mc, char* title);
#endif

//Tdi.cpp
void  S3dTdiEstim(float* ysOut, int ns, float* ts, float* ys);

#ifdef __cplusplus
}
#endif

#endif // _CSLDEV_H

