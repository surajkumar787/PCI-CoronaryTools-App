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

// V.1.0: VA 10/08. Multiframe, Hough extraction, choice of best candidate (grad, dark)
// discussion among best candidates to get best representant, detection based on grad, mean val under and 
// above, std under and above, tracking.

// V.2.0: VA 08/11. One frame, Hough extraction, classif based on grad, angulations, and parabola 
// coeffs (adaboost), tracking.
// Hough parametrization modified. Five candidates instead of three.
// False negative: 45%-->17.5%
// False positive: 22%-->14%

// Client-level umbrella include with public method prototypes

#ifndef		_DIALIB_H
#define		_DIALIB_H

// *** Includes
#include <ErrLib.h>

#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>

#include <DiaErrorCodes.h>

// ***** Constants
#define DIA_VERSION_MAJOR    3
#define DIA_VERSION_MINOR    2
#define DIA_VERSION_PATCH    0

#define NB_EXTRACTED_DIAPHRAGM_MAX 10

// ***** Structures

/** ROI structure.*/
typedef struct
{
	int XMin, YMin, XMax, YMax;

}CDiaRoiCoord;

/** Multiple candidate diaphragms structure.*/
typedef struct
{
	int nbExtractedDiaphragm;                               /** Nb of extracted diaphragms. */
	float Param[NB_EXTRACTED_DIAPHRAGM_MAX][3];             /** Parameters of the diaphragms (parabola). */
}
CDiaDiaphragmParabs;

/** Single diaphragm structure.*/
typedef struct
{
	float Param[3];                                         /** Parameters of the diaphragms (parabola). */
}
CDiaDiaphragmParab;									

/** Library handle */
typedef struct 
{ 
	CErrHdl ErrHdl;                                         /** Error-library handle associated to library */
	void*   Pv;                                             /** Access to private material */
} CDiaHdl; 

#ifdef __cplusplus
extern "C" {
#endif

	#ifdef DIA_DLLEXPORT
	#undef DIA_DLLEXPORT
	#endif // ifdef DLLEXPORT

	#ifdef DIALIB_EXPORTS
	#define DIA_DLLEXPORT __declspec(dllexport)
	#else
	#define DIA_DLLEXPORT
	#endif

    // ****** Life management

	/** Routine creating the library handle from image size and subsample factor. Internal allocations included.*/
	DIA_DLLEXPORT int DiaCreate(CDiaHdl *PtrDiaHdl, int IwOri, int IhOri, int SSFactorOri);
	/** Routine creating the library handle from image size and working image sizes. Internal allocations included.*/
    DIA_DLLEXPORT int DiaCreateGivenWorkingImSize(CDiaHdl *PtrDiaHdl, int IwOri, int IhOri, int IwProc1, int IhProc1);
    /** Routine deleting the handle */
	DIA_DLLEXPORT int DiaDelete(CDiaHdl DiaHdl);
    /** Routine returning the version number.*/
	DIA_DLLEXPORT int DiaGetVersionNumbers(int* Major, int* Minor, int* Patch); 
    
    // ****** Diaphragm extraction and tracking

    /** Diaphragm extraction (frame 0) and tracking (next frames, if diaphragm found).*/
	DIA_DLLEXPORT int DiaExtractDiaphragm(CDiaHdl DiaHdl, short *Im, int Iw, int Ih		// Input image, potentially already subsampled
										, short *ImFull					// Original image, before subsampling (needed for the final classif)
										, float Rot, float Ang			// C-arm positions
										, int SSFactor					// Extra SS factor for the Hough extraction
										, CDiaRoiCoord RoiCoord, int t	// Shutters, current time
										, CDiaDiaphragmParab coeffIn	// Input parabol coefficients (from t-1, for the tracking)
										, int *detectedDiaphragm		// Classification result (0: no, -1: unsure, 1:sure)
										, CDiaDiaphragmParab *coeffOut);// Estimated "live" parabol

    /** Routine to re-initialize the diaphragm tracker. It looks for a large translation. Typically to re-initialize diaphragm tracking between angio and fluoro.*/
	DIA_DLLEXPORT int DiaExtractDiaphragmIniUpToTR(CDiaHdl DiaHdl, short *Im, int Iw, int Ih		// Input image, potentially already subsampled
												, CDiaRoiCoord RoiCoord, int t	// Shutters, current time (Hough 1D at t=0, tracking else)
												, CDiaDiaphragmParab coeffIn	// Input parabol coefficients (from t-1, or angio at time t=0)
												, CDiaDiaphragmParab *coeffOut);// Estimated "live" parabol

	// Shutter detection
	DIA_DLLEXPORT int DiaExtractShutters(CDiaHdl DiaHdl, short* InIma, int IwOri, int IhOri, int ShutterPos[4]);
#ifdef __cplusplus
};
#endif

#endif   //_DIA_H
