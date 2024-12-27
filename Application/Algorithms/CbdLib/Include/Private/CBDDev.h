//Copyright (c)2014 Koninklijke Philips Electronics N.V.,
//All Rights Reserved.
//
//This source code and any compilation or derivative thereof is the
//proprietary information of Koninklijke Philips Electronics N.V.
//and is confidential in nature.
//Under no circumstances is this software to be combined with any Open Source
//Software in any way or placed under an Open Source License of any type 
//without the express written permission of Koninklijke Philips 
//Electronics N.V.

#include <stdio.h> 
#include <stdlib.h> 
//#include <iostream> 

#pragma once

#define VIRTUAL_VINCENT
#define FILTER_ANGLE_VINCENT
//#define VA_DEBUG
//#define MEDIAN_DEFINED


//#include "vld.h"

#include "LtTracker.h"
#include "LtTreeNodeAttribs.h"
#include "LtTreeTrackAttribs.h"
#include "LtStopTracksOnBorders.h"

#include "LtComputeFeatureLocalSmoothness.h"
#include "LtComputeFeatureFilteredGlobalSmoothness.h"

#include "LtExtendTrackOnRingFromFeatureIm.h"
#include "LtExtendTrackAddVirtual.h"

#include "LtSamplerRingLinear.h"
#include "LtPruningTracksScore.h"

#include "LtComputeFeatureLinInterScalarIm.h"
#include "LtComputeFeatureLength.h"

#include "CBDBxGaussian.h"
#include "CBDSeqLib.h"

//#include "ipp.h"
#ifndef _PI
	#define _PI 3,14159265
#endif

#define CBD_SHUTTERS_SAFETY_MARGIN 4
#define CBD_SHUTTERS_NON_EXTREMA_SCALE 10
#define CBD_SHUTTERS_EDGENESS_SCALE 4
#define CBD_SHUTTER_NON_SHUTTER_MIN_LEVEL_RATIO 1.3f
#define CBD_PICTURE_BORDER_NORMALIZED_VARIANCE .001f
#define CBD_REFERENCE_SCALE_IW 1024
#define CBD_SHUTTER_SYMMETRICITYLIMIT 40

#define CBD_SIGMASMOOTH 1.f

#define CBD_ORIENTATION_LIMIT                               0.67f

#define CBD_DEGREEFITTINGCONIQUE                             4
#define CBD_TUBE_RMIN                                        1.f
#define CBD_TUBE_RMAX                                        3.f
#define CBD_TUBE_MIN_TUBE_THICKNESS                          0.1f
#define CBD_TUBE_MAX_TUBE_THICKNESS                          0.5f
#define CBD_TUBE_PRECISION                                   0.05f
#define CBD_TUBE_MAXDELTAADJUSTMENT_INI                      0.5f
#define CBD_TUBE_MAXDELTAADJUSTMENT                          1.f
#define CBD_TUBE_MAXDPTX_UPDATE2                             6.f
#define CBD_TUBE_MAXDPTX_UPDATE3                             6.f
#define CBD_TUBE_DEGREE_PROFILEBACKGROUND                    2
#define CBD_TUBE_NBROBUSTITERATIONS_PROFILEBACKGROUND        10
#define CBD_TRACK_NBSAMPLESADAPTATION                         30
#define CBD_TRACK_SIZEPROFILE_ADAPTATION                      50
#define CBD_TRACK_ADAPTATION_RATIO_MAXPROF                    0.5f
#define CBD_TRACK_ADAPTATION_NBROBUSTITERATIONS               10
#define CBD_TRACK_MIN_DIST_FOR_CENTERLINE_WEIGHTS_PIX         1//.f
#define CBD_DIST_NBSAMPLESDISTANCE                            30 // per dim
#define CBD_TUBE_MAXNBOFCONTRASTS                             4
#define CBD_DISTANCELIMIT_TOACCEPTCBD                         0.5f // mm
#define CBD_TUBE_MAXNBSAMPLES_TUBEPARAMSCOMPUTATION           100
#define CBD_MIN_RELATIVE_LENGTH                             0.5f //relative to Y ROI


#define CBD_SQ(a)            ((a)*(a))
#define CBD_ABS(a)			 (((a) >= 0)   ? (a): (-(a)))
#define CBD_ABS_F(a)		 (float)fabs(a)
#define CBD_MIN(a,b)         (((a) <= (b)) ? (a):   (b) )
#define CBD_MAX(a,b)         (((a) >= (b)) ? (a):   (b) )
#define CBD_CLIP(a,b,c)      CBD_MIN(CBD_MAX((a), (b)), (c))
#define CBD_RND(a)           (((a) >= 0.0) ? (int)((a)+0.5) : (int)((a)-0.5))

//#define DISPLAY_DETAILS_DISTANCE_CBDS

#ifdef DISPLAY_DETAILS_DISTANCE_CBDS
#include "Mxlab.h"
#endif


#ifdef DLLEXPORT
#undef DLLEXPORT
#endif // ifdef DLLEXPORT

#define DLLEXPORT __declspec(dllexport)



// *** Definitions
typedef Lt::LtTreeTrackAttribsBase<
		Lt::LtTreeTrackAttribsLocalSmoothness<
		Lt::LtTreeTrackAttribsFilteredGlobalSmoothness<
		Lt::LtTreeTrackAttribsEmpty>>> TrackBaseType;

struct LtTreeTrack : public TrackBaseType{

		LtTreeTrack& operator=(const LtTreeTrack& o){
			TrackBaseType::operator=(o);
			return *this;
		}

		void Reset(){
			TrackBaseType::Reset();
		};

		template<typename TNode>
		void CopyInfoToEndPoint(TNode* endpoint){
		}

		void ComputeScore(){
			m_Score  = m_FeatSum/m_Length - m_IrregLocalScale - 10 * m_IrregFilteredLargeScale;
		}
	};


struct LtTreeNode : public Lt::LtTreeNodeAttribsBase, Lt::LtTreeNodeAttribsGlobalFilteredSmoothness{
};

//Type of the tree that will hold the tracks
typedef Lt::LtTree<LtTreeTrack,LtTreeNode> TreeType;



// *** Internal private parameters
typedef struct 
{ 
	CErrHdl             ErrHdl;          // Handle for error management
	char               *VersionInfo;     // Library's version-related info
	char               *VersionInfoCopy; // Copy of VersionInfo returned, as a pointer, to client

	float				*ImPotential;	// Input data
	int					Iw, Ih;
	int					ShutterPos[4];

	short				*ImSub;
	float				*SaliencyMap, *Rdg;
	int					SSFactor, IwSub, IhSub, IwAlloc, IhAlloc, IwSubAlloc, IhSubAlloc;

	float				*Vect;
	int					*selectedIndices, *seedsX, *seedsY;
	Lt::LtTrack			**bestTrack;
	float				*scoreTracks, *orientation, *length, *contrast, *homogeneity;
	int					nbSeedsPerSide;

	Lt::LtTracker<Lt::LtTree<LtTreeTrack,LtTreeNode>> * lt;		// Line tracker
	Lt::LtStopTracksOnBorders<Lt::LtTree<LtTreeTrack,LtTreeNode>>* ltstopcriterionTop;		// Stop condition - top border
	Lt::LtStopTracksOnBorders<Lt::LtTree<LtTreeTrack,LtTreeNode>>* ltstopcriterionBottom;	// Stop condition - bottom border
	Lt::LtStopTracksOnBorders<Lt::LtTree<LtTreeTrack,LtTreeNode>>* ltstopcriterionLeft;		// Stop condition - left border
	Lt::LtStopTracksOnBorders<Lt::LtTree<LtTreeTrack,LtTreeNode>>* ltstopcriterionRight;	// Stop condition - right border

	CCBDParams Params;	// Parameters

	int		nbExtractedTracks;
	CBDTrack * extractedTracks[CBD_MAX_NB_EXTRACTED_TRACKS];	// Extracted tracks
	CBDTrack * detectedTrack;	// Detected track

    // For tracking and tube model subtraciton
    float           CoeffConiqueCTCBD[CBD_DEGREEFITTINGCONIQUE+1];
    float           CoeffConiqueCTCBDPrev[CBD_DEGREEFITTINGCONIQUE+1];
    float           CBDYNormFittingConique;
    float           CBD_radiusInside, CBD_radiusOutside
                    , CBD_attenuationInside[CBD_TUBE_MAXNBOFCONTRASTS], CBD_attenuationTube[CBD_TUBE_MAXNBOFCONTRASTS];
    int             CBD_contrastCPY[CBD_TUBE_MAXNBOFCONTRASTS];
    int             CBD_nbEstimatedContrasts;
    float           CBDDistWithPrevious;

} CCBD; 

// CBDLib.cpp
int CBDSetVersion(CCBD *This);
int CBDGetVersionInfo(CCBDHdl CBDHdl, char** VersionInfo);
void CBDUpdateStopCriteria(int ShutterPos[4], CCBDParams * Params, CCBDHdl * PtrCBDHdl);

// CBDSeqLib.cpp
int CBDCopyParams(CCBDParams ParamsIn, CCBDParams *ParamsOut);

// CBDExtractShutters.cpp
int CBDExtractShutters(CCBD * This, short* InIma, int IwOri, int IhOri, int ShutterPos[4]);

// CathBDtRidgeComputation.cpp
void CathBDtRdgCalcGaussHessian2(float *Gau, int i, int j, int Iw, int Ih
							 , float *Lxx, float *Lyy, float *Lxy);
void CathBDtRdgCalcGaussHessEigenValues(float *Gau, int i, int j, int Iw, int Ih, 
								 float *Lxx, float *Lyy, float *Lxy, float *L1, float *L2);
int CathBDtRdgMonoscaleComputation(short *Ori, int Iw, int Ih, int ShutterPos[4]
									, float KernelSig, float *Rdg, CCBD * This);
int CathBDtRdgEnhanceMultiScaleRidge(short *Ori, float *Rdg, int Iw, int Ih, int ShutterPos[4]
								, float *KernelSigs, int nKernelSigs, float gamma, CCBD * This);
float CathBDtPercentileComputation(float *Rdg, int Iw, int Ih, int ShutterPos[4], int NbPixKept);
int CathBDtImAdjust(CCBD * This, float * Rdg, int Iw, int Ih, int ShutterPos[4], float percentileL
					, float percentileH, float * RdgRescaled);

// CBDTrack.cpp
int CBDAdaptCBDPosToNewFrame(CCBD * This, float *ImPot, int Iw, int Ih, int ShutterPos[4], float isoPixSize
                            , float *coeffsPolynomPrev, int degreePolynom, float yNorm, float *coeffsPolynomOut);
void CBDFilteredImProfileY(float *ImPot, int Iw, int Ih, float y, float xMin, float xMax, int nbElts
                            , float *profOut, float *corrX, float *profBuf);
int CBDFitPolynom(CCBD * This, float *XCPIn, float *YCPIn, int nbCPIn, float *coeffsPolynom, int degreePolynom, float *yNorm);
int CBDFitPolynomWithWeights(CCBD * This, float *XCPIn, float *YCPIn, float *w, int nbCPIn, float *coeffsPolynom, int degreePolynom, float yNorm);
int CBDUpdateRobustWeights(CCBD * This, float *XCPIn, float *YCPIn, float *w, int nbCPIn, float *coeffsPolynom, int degreePolynom, float yNorm);

// CBDTube.cpp
int CBDEstimateCBDParametersfromProfile(CCBD * This, short *Im, int Iw, int Ih, int ShutterPos[4], float isoPixSize
                                    , float *coeffsPolynom, int degreePolynom, float yNorm
                                    , float RMin, float RMax, float minThicknessTube, float maxThicknessTube
                                    , float precisionMM, float deltaCentMax
                                    , float yMin, float yMax, int nbSamples
                                    , float *R1Out, float *R2Out, float *alpha1, float *alpha2, int *yCP, int *nbEstimatedContrasts);
int CBDComputeProfile(CCBD * This, short *Im, int Iw, int Ih, int ShutterPos[4], float isoPixSize
                        , float *coeffsPolynom, int degreePolynom, float yNorm, float precisionMM
                        , float yMinCT, float yStep, int firstSample, int lastSample
                        , float centerAnalysis, int nbBinsProfile, bool *noElts
                        , float *profile, int *nbEltProfile, float *profBkg);
void CBDEstimateBestContrasts(float *profile, int *nbEltsProf, float *profBkg, float R1, float R2, float centerAnalysis, float deltaCent, float precisionMM
                           , float *A, float *B, float *contIn, float *contTube);
float CBDComputeProfileFittingError(float *profile, int *nbEltProfile, float *profBkg, float R1, float R2, float centerAnalysis
                                 , float deltaCent, float precisionMM, float contIn, float contTube);
int CBDEstimationBackgroundProfile(CCBD * This, float * prof, int *nbEltsProf, int nbElts, float *Bkg);

// CBDSubtract.cpp
int CBD_ValidateCBD(float *coeffPolynom, int degreePolynome, float yCBDNorm, int ShutPos[4]);
float CBD_DistanceBetweenCenterlines(float *coeffsPolynomPrev, float *coeffsPolynom, int degreePolynom, float yNorm
                                        , int ShutterPos[4], float isoPixSize, float maxDeltaX, float maxDeltaY);

// CathBDtSubfunctions.cpp
void LocalMaxima(float * VectIn, int sizeVect, int nbSeeds, int * indicesSeeds);
float median(float * stack1, float * stack2, int x0, int pos, int xEnd);
void ComputeTrackIndicators(Lt::LtTrack * Track, float trackerSpeed, float NbStepForAngleF
							, float DThetaIrregArg0, float DThetaIrregArg1, float * Saliency
							, int Iw, int Ih, float *orientation, float *contrast, float *homogeneity);
void CBDSubSampleImage(short *Ori, short *Sub, int IwOri, int IhOri, int IwSub, int IhSub, int SSFactor);
int CBDSysTimeMs(char *Name);
int CBDGaussNewton(float *A, float *B, int size);
int CBD_Combination(int j, int i);
float CBDComputeFromPolynom(float yIn, float *coeffsPolynom, int degreePolynom, float yNorm);
float CBDInterpBilinear(short *Im, int Iw, int Ih, float x, float y);
float CBDInterpBilinearF(float *Im, int Iw, int Ih, float x, float y);
float CBDDerivativeTukey(float r, float C);
int CBDMedian(CCBD * This, float * stack1, float * stack2, int x0, int pos, int xEnd, float *medOut);
