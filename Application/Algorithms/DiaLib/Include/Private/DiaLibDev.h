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

// *******************************************************************
// DIALIB - Library extracting the diaphragm - VA 10/08
// *******************************************************************
// Based on Hough parabol extraction and contrast tracking

//
// Library developer-level umbrella include 
//
#ifndef		_DIALIBDEV_H
#define		_DIALIBDEV_H

#include <DiaLib.h>

#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <DiaBxGaussian.h>

//#include "vld.h"

#ifndef NDEBUG
	//#define DEBUG_INFO
	//#define DISPLAY_INT
	//
#endif

#ifdef DISPLAY_INT
	#define MX_USE_DLL_AS_STATIC 1
	#include "MxLab.h"
#endif

// *** Constants
#define EXTRA_SSFACTOR_HOUGH			2		// SS factor, specifically for Hough
#define NB_BINS_PERCENTILE				1000	// nb of bins in the gradient threshod percentile computation (for an affordable Hough computation)
#define GRAD_PERCENTILE					.85f	// Corresponding percentile
#define SIGMA_GRADIENT					12.f	// Sigma of the Gaussian used in the gradient computation
#define GRAD_BO1						2		// Margin in the gradient computation
#define GRAD_RO							2.f		// Margin in the gradient computation
#define DIA_COEFF_GX					0.3f	// Ratio of the gradient in X that is reinjected

#define NB_EXTRACTED_DIAPHRAGM_LIMIT	5		// Maximum number of extracted diaphragms (in Hough iterative extraction)
#define NB_CELLS_PER_DIM_HOUGHA			25		// Cells of the Hough accumulation matrix for the a parameter
#define NB_CELLS_PER_DIM_HOUGH			50		// Cells of the Hough accumulation matrix for the b and c parameter
#define HOUGH_A_MIN						0.f		// /Iw
#define HOUGH_A_MAX						1.f		// /Iw
#define HOUGH_B_MIN						-1.25f
#define HOUGH_B_MAX						1.25f
#define HOUGH_C_MIN						0.33f	// *Iw
#define HOUGH_C_MAX						1.25f	// *Iw
#define PERCENT_CELLS_ZEROED			.1f		// Nb of cells zeroed when a peak is extracted in the accumulation matrix (percentage of each dimension)
#define SEUILMIN_MERGEPARAB				.1f		// Percentage of the image defining the maximum distance leading to merge two parabols

#define SHIFT_CONT						40		// Delta used in the contrast computation for the final diaphragm/no diaphragm classification

#define DELTA_TRACK						40		// Distance from the parabol used in the tracking mask
#define NB_ITERATIONS_TRACK				15		// Iterations in the tracking
#define TRACK_STEPMAX					10.f	// maximum update amplitude (for the first iteration)

#define EPSILON							0.00000001f

// For classification

 

//======================================================================================
// MACROS

#define DIA_SQ(a)            ((a)*(a))
#define DIA_ABS(a)			 (((a) >= 0)   ? (a): (-(a)))
#define DIA_MIN(a,b)         (((a) <= (b)) ? (a):   (b) )
#define DIA_MAX(a,b)         (((a) >= (b)) ? (a):   (b) )
#define DIA_CLIP(a,b,c)      DIA_MIN(DIA_MAX((a), (b)), (c))
#define DIA_RND(a)           (((a) >= 0.0) ? (int)((a)+0.5) : (int)((a)-0.5))

#define DIA_ERROR_EXIT(Func, Code, Mess)	{                    \
	printf("Fatal error in %s : %s \n", Func, Mess);          \
	printf("Exiting program : type something to continue\n"); \
	getchar();                                                \
	exit(Code);                                               \
}

typedef struct
{
	int		IwSS1, IhSS1, IwSS1Alloc, IhSS1Alloc, IwOriAlloc;
	int		IwSS2, IhSS2;
	short	*ImSubSS2;
	float	*gradImSS2;
	long	*hist;
	float	*AccumulationMatrix, *AccumulationMatrix1D;
	float	*BufSS2F;
	short	*BufS1S;
	float	*cont;
}CDiaProc;

typedef struct
{
	//CErrHdl             ErrHdl;          // Handle for error management
	CDiaProc DiaProc;
}CDia;

//======================================================================================
// FUNCTIONS

// *** DiaLibHough
int DiaHoughParabolExtraction(CDiaHdl DiaHdl, float * grad, int Iw, int Ih, CDiaRoiCoord RoiCoord, float gradThres				// Gradient, size, ROI and threshold
								, int nbCellsPerDimA, int nbCellsPerDimB, int nbCellsPerDimC, float rateCellsZeroed	// Nb of cells per dimensions (for a, b and c), nb of cells that are zeroed around a peak (in percentage of cellDim)
								, int nbExtractedParamMax, float seuilMinDist, CDiaDiaphragmParabs *coeffOut);		// Max number of extracted parabols, mean distance between them two parabols (else marge), extracted parameters

int DiaFillAccumulationMatrix(CDiaHdl DiaHdl, float * Im, int Iw, int Ih, CDiaRoiCoord RoiCoord, float gradThres
								, int nbCellsPerDimA, int nbCellsPerDimB, int nbCellsPerDimC);
float DiaExtractFromAccumulationMatrix(float *AccumulationMatrix, int nbCellsPerDimA, int nbCellsPerDimB, int nbCellsPerDimC
										, float rateCellsZeroed, int Iw, int Ih, float * a, float *b, float *c);
// Idem, but only c varies
int DiaHoughParabolExtraction1D(CDiaHdl DiaHdl, float * grad, int Iw, int Ih, CDiaRoiCoord RoiCoord, float gradThres
							 				, int nbCells, float aIn, float bIn, CDiaDiaphragmParab *coeffOut);
int DiaFillAccumulationMatrix1D(CDiaHdl DiaHdl, float * Im, int Iw, int Ih, CDiaRoiCoord RoiCoord, float gradThres
											, int nbCells, float a, float b);
float DiaExtractFromAccumulationMatrix1D(float *AccumulationMatrix, int nbCells, int Iw, int Ih, float *c);


// Diaphragm tracking
int DiaParabolicContrastTracking(CDiaHdl DiaHdl, short * Im, int Iw, int Ih, CDiaRoiCoord RoiCoord	// Input image, dimension and ROI
								, int delta, float stepMaxPix, int nbIterations		// Delta to compute contrast, stepMaxPix=first update amplitude in pixels, nb gradient descent iterations
								, CDiaDiaphragmParab coeffIn, CDiaDiaphragmParab *coeffOut);

// *** DiaLibClassification.c	
int DiaClassif(CDiaHdl DiaHdl, short * ImFull, int IwFull, int SSFactor, CDiaRoiCoord RoiCoord
				, CDiaDiaphragmParabs *coeffs
				, int delta, float Rot, float Ang
				, CDiaDiaphragmParab *selectedCoeff, int *diaphragmDetected);

// *** DiaLibTracking.c
void DiaParabolicContrastTrackingStep(short * Im, short * Mask, int Iw, int Ih
									  , CDiaRoiCoord RoiCoord, int delta, float lambda
									  , CDiaDiaphragmParab coeffIn, CDiaDiaphragmParab *coeffOut);

// *** DiaUtil.c
int DiaGradientComputation(CDiaHdl DiaHdl, short * Im, float * Grad, int Iw, int Ih
							, CDiaRoiCoord RoiCoord, float sigma);	// Smoothed gradient computation
int DiaSubSampleImage(CDiaHdl DiaHdl, short *Ori, short *Sub, int IwOri, int IhOri, int IwSub, int IhSub, int SSFactor);
int DiaHistoConstruction(CDiaHdl DiaHdl, float * Im, int Iw, int Ih, CDiaRoiCoord RoiCoord, int nbBins, long * hist
					   , float *minHisto, float *maxHisto);
float DiaPercentile(int Iw, int Ih, CDiaRoiCoord RoiCoord
				 , long * hist, int nbBins, float minHisto, float maxHisto, float percentile);

// *** DiaMxDisplay.c
void MxDisPlaySetBackgroundImageUC(unsigned char* im, int Iw, int Ih, char *name);
void MxDisPlaySetBackgroundImageS(short* im, int Iw, int Ih, char *name);
void MxDisPlaySetBackgroundImageI(int* im, int Iw, int Ih, char *name);
void MxDisPlaySetBackgroundImageF(float* im, int Iw, int Ih, char *name);

#endif   //_DIALIBDEV_H