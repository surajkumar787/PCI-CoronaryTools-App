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

#include <ErrLib.h>

#include <CBDErrorCodes.h>

#pragma once

#define CBD_VERSION_MAJOR    3
#define CBD_VERSION_MINOR    0
#define CBD_VERSION_PATCH    2

// The part dedicated to the processing of the first frames (sequence) is CSBSeqLib.h

#define CBD_MAX_NB_EXTRACTED_TRACKS 100

typedef enum{
	CBD_ANGIO,
	CBD_FLUORO
} CCDBSequenceType;

/** Catheter detection parameters for initial detection */
typedef struct{

	/** Tracking parameters */
	float KernelRdg;			        /** Ridge kernel size for saliency map. Default 4 on 512x512 im. */
	float percentileL;			        /** Low percentile thresholding the ridge image. Default .1f. */
	float percentileH;			        /** High percentile thresholding the ridge image. Default .99f. */
	int nbSeedsPerSide;			        /** Number of seeds per investigated side. Default 3. */
	int trackerMaxNbTracks;		        /** Maximum track number to be kept at each iteration. Default 5. */
	float trackerSectorOut;		        /** Opposite sector left out. Default 4 pi / 3. */
	float trackerRingRadius;	        /** Also named speed. Default 10 on 512x512 im. */
	float DThetaIrregArg0;		        /** Highest DThetha producing zero-irregularity. Ddefault pi / 9. */
	float DThetaIrregArg1;		        /** Lowest DThetha producing zero-irregularity. Default pi / 2. */
	float NbStepForAngleF;		        /** Number of steps over which angle filtering is achieved. Default 5. */
	float AngleLimitForVirtual;	        /** Angle limit to introduce virtual. Default pi / 9. */
	int	  maxNbVirtual;			        /** Max nb of virtual. Default 5. */

	/** Image-based decision */
	int limitShortLong;				    /** Limit (in pixel for a 512x512 image) between short and long track. Default 200 on 512x512 ims. */
	float contrastCriteriumShort;	    /** Threshold on the mean track ridgeness to be considered a short cath body. Default .2f for angios, .15f for fluoros. */
	float homogenityCriteriumShort;	    /** Threshold on the stdDev track ridgeness to be considered a long cath body. Default .18f in angio, .1f in fluoro. */
	float contrastCriteriumLong;	    /** Threshold on the mean track ridgeness to be considered a long cath body. Default .23f for angios, .18f for fluoros. */
	float homogenityCriteriumLong;	    /** Threshold on the stdDev track ridgeness to be considered a long cath body. Default .18f in angio, .1f in fluoro. */

}CCBDParams;

/** Linetrack structure */
typedef struct{
	int nbElts;
	float * X;
	float * Y;
}CBDTrack;

/** Structure of features characterizing the track quality */
typedef struct{
	float scoreTracks,                                          /** Global score associated to track */
        orientation,                                            /** Global orientation of the track */
        length,                                                 /** Length of the track */
        contrast,                                               /** Contrast of the track. Cumulated saliency over the track. */
        homogeneity;                                            /** Homogenity indicator of the track. Standard deviation of saliency over the track.*/
}CBDIndicTrack;

/** Structure of advanced results */
typedef struct{
	int			nbTracks;                                       /** Nb of considered tracks */
	CBDTrack	*extractedTracks[CBD_MAX_NB_EXTRACTED_TRACKS];  /** Considered tracks */
	CBDIndicTrack indicTrack[CBD_MAX_NB_EXTRACTED_TRACKS];      /** Quality indicators associated to the considered tracks */
	float		*ImPotential;                                   /** Potential image on which the tracks have been extracted */
}CCBDAdvanced;

/** Structure of CBD detection result */
typedef struct{
	int detectedCath;                                           /** Detected cath binary flag */
	CBDTrack *detectedTrack;                                    /** Detected track, if any */
	CCBDAdvanced Advanced;                                      /** Advanced result structure */
}CCBDResults;

/** Library handle */
typedef struct 
{ 
	CErrHdl ErrHdl;                                             /* Error-library handle associated to library */
	void*   Pv;                                                 /* Access to private material */
} CCBDHdl; 

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

    // ****** Life management

	/** Routine creating the library handle. No allocation of internal pointers. */
	CBD_DLLEXPORT int CBDCreate(CCBDHdl* PtrCBDHdl);
    /** Routine deleting the handle */
	CBD_DLLEXPORT int CBDDelete(CCBDHdl CBDHdl);
    /** Returns the algorithm default parameters. This gives a base the client can edit from. */
	CBD_DLLEXPORT int CDBDefaultParameters(CCBDParams * Params, int Iw, CCDBSequenceType CDBSequenceType);
    /** Allocates the internal pointers, based on the maximum image size, the subsample factor, and the number of particle considered per side. */
	CBD_DLLEXPORT int CBDMemoryAllocation(CCBDHdl CBDHdl, int IwAlloc, int IhAlloc, int SSFactor, int nbSeedsPerSide);
    /** Allocates the internal pointers, based on the maximum image size, the maximum internal image size, and the number of particle considered per side. */
    CBD_DLLEXPORT int CBDMemoryAllocationImSize(CCBDHdl CBDHdl, int IwAlloc, int IhAlloc, int IwSubAlloc, int IhSubAlloc, int nbSeedsPerSide);
	/** Routine returning the version of the library. */
	CBD_DLLEXPORT int CBDGetVersionNumbers(int* Major, int* Minor, int* Patch); 
    
    /** Initial catheter body segmentation (from one image).
    The user can either call one of the CatheterBodyDetection, CatheterBodyDetectionFromPot or CatheterBodyDetectionFromPotAndParam directly, or CBDTrackerInitialization followed by CatheterTracking.*/

	//** Extracts catheter body from one orginal image. Internally computes and scales the image potential, initializes the tracker and performs the tracking and decision. Estimates the shutter position. */
	CBD_DLLEXPORT int CatheterBodyDetection(short * ImIn, int Iw, int Ih, float Rot, float Ang, int SSFactor, CCDBSequenceType CDBSequenceType, CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults);
	//** Extracts catheter body from one already computed potential image. The shutter position also is given by the client. The default algorithm parameters are used. */
	CBD_DLLEXPORT int CatheterBodyDetectionFromPot(float * ImPot, int Iw, int Ih, float Rot, float Ang, int ShutterPos[4], int angioSeqFlag, CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults);
	//** Extracts catheter body from one already computed potential image, and specified algo parameters. The shutter position also is given by the client. */
	CBD_DLLEXPORT int CatheterBodyDetectionFromPotAndParam(float * ImPot, int Iw, int Ih, float Rot, float Ang, int ShutterPos[4], CCBDParams Params, CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults);

	/** Initializes tracker. Points to the potential image, and initializes the trackers based on the shutter position.*/
	CBD_DLLEXPORT void CBDTrackerInitialization(float * ImPotentialIn, int Iw, int Ih, int ShutterPos[4], CCBDParams * Params, CCBDHdl * PtrCBDHdl);	
	//** Extracts the catheter body from the considered image, from the previously initialized line tracker. */
	CBD_DLLEXPORT int CatheterTracking(float Rot, float Ang, int Iw, int Ih, CCBDHdl * PtrCBDHdl, CCBDResults *CBDResults);

    // ****************** Tracking of the CBD, from one frame to the next
    
    /** Tracks the CBD from one frame to the next, and updatest the CBD model (radii and attenuations). */
    CBD_DLLEXPORT int CBDTrackAndComputeTubeModel(short *Im, float *ImPot, int Iw, int Ih, int ShutterPos[4], float isocenterPixSize, CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults);
    /** Returns the x position of the estimated CBD at height y.*/
    CBD_DLLEXPORT float CBDGetCBDCenterlineX(CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults, float y);
    /** Returns the estimated radius of the CBD.*/
    CBD_DLLEXPORT float CBDGetRadius(CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults);

    // ****************** Subtracts the estimated CBD model
    
    /** Subtracts the estimated CBD model. */
    CBD_DLLEXPORT int CBDSubtractCBDTube(short *Im, int Iw, int Ih, int ShutterPos[4], float isoPixSize, CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults, short *ImOut);

    // ****************** Utility functions

    /** Copies the extracted tracks. By default, the tracks are not copied to the result structure. When the handle is deleted they are erased as well. So the user needs to be able to copy them, and to delete them afterwards.*/
	CBD_DLLEXPORT int CBDCopyResults(CCBDResults *PtrCBDResIn, CCBDResults *PtrCBDResOut);
    /** Deletes the copied extracted tracks.*/
	CBD_DLLEXPORT int CBDDeleteCopiedResults(CCBDResults * PtrCBDResOut);


#ifdef __cplusplus
};
#endif