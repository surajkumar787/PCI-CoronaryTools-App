// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_RDG_H
#define		_RDG_H

#include <RdgErrorCodes.h>


//======================================================================================
// CONSTANTS

// Rdg maximum number of scales for multi-level ridgeness
#define RDG_CST_MAX_SIGMAS          16

// Rdg functions' argument ranges
#define RDG_CST_MIN_IMAWIDTH        1
#define RDG_CST_MIN_IMAHEIGHT       1
#define RDG_CST_MIN_IMADYN          4
#define RDG_CST_MAX_IMADYN          16


// Default parameters profile types
enum {
   RDG_CST_MKX_PROFILE = 0,
   RDG_CST_TIPX_PROFILE, 
   RDG_CST_OSC_PROFILE
};

// Ridge types
enum {
   RDG_CST_RDG_DIFFAL_K1PLINE = 0,
   RDG_CST_RDG_DIFFAL_K1_LAMBDAK2_PLINE,
   RDG_CST_RDG_DIFFAL_HESSKERNEL_PLINE,
   RDG_CST_RDG_DIFFAL_ANGLE_DIFF,
   RDG_CST_RDG_DIFFAL_HESSIAN_TRACE, // Trace of Hessian, similar to Lambda1 (=L1+L2), but simpler computations
   RDG_CST_RDG_DIFFAL_FRANGI
};

// Step types (for Hessian calculation)
enum {
	RDG_CST_HALF_STEP = 0,
	RDG_CST_ONE_STEP,
};

// Threshold types
enum {
    RDG_CST_NOISE_DEP_THR = 0,
    RDG_CST_ABSOLUTE_THR,
    RDG_CST_K_MEAN_THR,        // mean Rdg value
    RDG_CST_K_PEAK_THR,        // peak Rdg value in histogram
    RDG_CST_K_CHI_PEAK_THR,    // peak in histogram of Chi variable with 2 degrees of freedom (from Rdg values)
    RDG_CST_K_CHI_BARY_THR,    // barycenter in histogram around peak of Chi variable with 2 degrees of freedom (from Rdg values)
    RDG_CST_K_CHIABS_PEAK_THR, // peak in histogram of Chi-like variable with 2 degrees of freedom (from Rdg values, sum of absolute values instead of sqrt of sum of square values of 2 variables)
    RDG_CST_K_CHIABS_BARY_THR, // barycenter in histogram around peak of Chi-like variable with 2 degrees of freedom (from Rdg values, sum of absolute values instead of sqrt of sum of square values of 2 variables)
    RDG_CST_NB_PIX_THR,
};

// Rigid motion detection results
enum {
   RDG_CST_RIGID_MOTION_FALSE = 0,
   RDG_CST_RIGID_MOTION_TRUE,
   RDG_CST_RIGID_MOTION_UNKNOWN
};



//======================================================================================
// STRUCTURES

typedef void* CRdgHdl;

typedef struct{
	
   //
   // some library options
   // used to be #define compilation flags
   //

   // Library options
   int   ExtraMeasuresFlag;                  // flag, provides extra measures in returned ridge objects
   int   SpatialParamsFor512Flag;            // flag: If true spatial params must be expressed for 512x512 pictures (once possibly sub-sampled). 
	                                          // and the library will scale them for the actual picture processing size
	                                          // If false, parameters will be directly taken into account (without scaling)
   // Speed options
   int   FastProfileFlag;                    // flag, true if some quality degradations are possible to gain some speed

   //
   // Processing Parameters for TipxLib and MkxLib
   //

   int   NormalizationOriFlag;               // flag, true if input image has to be normalized before ridgeness computation
   float NormalizationLFSigma;               // Sigma of gaussian filter for input image normalization
   int   RidgeType;                          // Ridge filter type
   float KernelSigmas[RDG_CST_MAX_SIGMAS];   // Multi-scale sigmas (Zero-terminated to indicate how many of them)
   int   EdgeFromOriFlag;                    // flag, true if the "no edge" factor has to be computed from original input picture
   float NoEdgeFactor;                       // Parameter of the "no edge" factor
   int   ThresholdType;                      // Threshold method type
   int   ThinningFlag;                       // flag, true if thinning has to be performed on ridgeness picture
   int   NoiseThreshold;                     // Parameter for noise-level thresholding
   float AbsoluteThreshold;                  // Parameter for absolute-level thresholding
   int   L1ThresholdFlag;                    // flag, true if thresholding parameter applies to L1 norm
   int   NbPixelsKept;                       // Number of pixels to keep for histogram-based thresholding
   int   NbRangePixels;                      // Range of allowable gap for connected component analysis
   int   DirLabelFlag;                       // flag, true for directional connected component analysis
   int   MaxDirDiff;                         // Range of allowable directional difference for connected component analysis
   int   DoubleBistouFlag;                   // flag, true for double bistouriquette (C) label thresholding 
   int   NbLabelsKept;                       // Maximum number of labels kept after label thresholding
   int   GlobalMotionFlag;                   // flag, true if rigid global motion has to be computed on labels
   float GlobalMotionThreshold;              // Threshold for global motion detection

   int   TipLength;                          // Length of wire-guide tip (pixels)

   // 
   // Parameters added for OscLib
   //


   float RidgeGamma;                         // Gamma exponent of the mono-scale ridgeness scaling factor
   int   BackgroundEstimationFlag;           // flag, true if background estimation will be performed
                                             // (impacts circular buffer allocation)
   int   FramesPerSecond;                    // Frames per second of input sequence, impacts background estimation stack size (which holds 1 second of input data)
   int   FreezeThresholdsAt;                 // Freeze thresholds after a number of frames
   float ThresholdPercent1;                  // Threshold percentile in hysteresis first labelling
   float WireTipThresholdPercent1;           // Threshold percentile in hysteresis first labelling (for wire guide tip, fluoro only)
   int   NbRangePixels1;                     // Number of pixels for range in hysteresis first labelling
   float MaxDirDiff1;                        // Maximum direction difference in hysteresis first labelling
   int   MinNbPelPerLab1;                    // Minimum number of pixels of kept labels after hysteresis first labelling
   int   MinNbPelPerWireTipLab1;             // Minimum number of pixels of kept labels after hysteresis first labelling (for wire guide tip, fluoro only)
   float ThresholdPercent2;                  // Threshold percentile in hysteresis second labelling
   float WireTipThresholdPercent2;           // Threshold percentile in hysteresis second labelling (for wire guide tip, fluoro only)
   int   NbRangePixels2;                     // Number of pixels for range in hysteresis second labelling
   float MaxDirDiff2;                        // Maximum direction difference in hysteresis second labelling
   int   MaxWireTipThickness;                // Maximum wire guide tip thickness

} CRdgExtractParam;                          // Extraction parameters



typedef struct
{
	int XMin, YMin, XMax, YMax;

}CRdgRoiCoord;                               // ROI parameters


// Integer 2D-vector
typedef struct
{
	int X;
	int Y;
}CRdgVi;

// Floating-point 2D-vector
typedef struct
{
	float X;
	float Y;
}CRdgVf;


// Ranking element facility
typedef struct
{
	int   Index;
	float Value;
}CRdgRankingElement;

// Ranking-list facility
typedef struct
{
	int                NbElement;
	CRdgRankingElement *Elements;
}CRdgRankingList;


// Read-only data
typedef struct
{
	int           IwProc;   // Sub-sampled image width
	int           IhProc;   // Sub-sampled image height
	int           SSFactor; // Sub-sampling factor

	short         **Sub;    // Sub-sampled original image, if SSFactor!=1 (XAny)
	CRdgVi        **Pel;    // Contains the corresponding pixel coordinates in the original image, if SSFactor!=1 (XAny)
	float         **Rwo;    // Ridge image (without No-Edge factor) (only when no-edge factor is used) (XAny)
	float         **NoE;    // No-Edge image (only when no-edge factor is used) (XAny)
	float         **Noi;    // Noise image (used in Extract only when Rdg-noise level is used) (XAny)
	float         **Gau;    // Gauss image (input of ridge detector) (XAny)
	float         **Rdg;    // Ridge image (with No-Edge factor): before background subtraction (XAny)
	float         **Dir;    // Direction image (XAny, after ridge filter)
	unsigned char **Sca;    // Scale image (Index of the best sigma at each pixel) (XAngio, XFluoro)
	float         **Bkg;    // Background image: the still stuff to remove (XAngio, XFluoro)
	float         **Frg;    // Foreground image (Rdg-Bkg) (XAngio, XFluoro)
	unsigned char **See;    // Threshold seed image (0 or 1): Hysteresis high-threshold, after small-label cleaning (XAngio, XFluoro)
	unsigned char **Thr;    // Threshold image (0 or 1): after everything (XAny)
	int           **Lab;    // Labelled image: after everything: labels in [2, NbLabel+2[ (might contain 1, not "hysterized") (XAny)
	int           **LabTm1; // Labelled image at time t-1 (XAny)
	unsigned char **Msk;    // Binary mask image (Like Thr, but only "hyterized" pixels, if hysteresis is used, else = Thr) (XAny)
	unsigned char **GMMask; // Binary image of pixels undergoing stitch/global rigid motion (XAny)

	short         **Clo;    // Closing of sub-sampled image (the wire-tip is normally removed) (XFluoro only)
	float         **TipRdg; // Wire-Tip ridge image (with No-Edge factor): Rdg on Sub - Clo (XFluoro)
	float         **TipBkg; // Wire-Tip background image (XFluoro)
	float         **TipFrg; // Wire-Tip foreground image (XFluoro)
	unsigned char **TipSee; // Wire-Tip threshold seed image (0 or 1) (XFluoro)
	unsigned char **TipThr; // Wire-Tip threshold image (0 or 1) (XFluoro)
	int           **TipLab; // Wire-Tip labelled image (XFluoro)
	unsigned char **TipMsk; // Binary mask of wire guide tip-like devices (only hysterized pixels) (XFluoro)

}CRdgAdvanced;


// Measures on object
typedef struct{

	float      Ridgeness;   // between 0 and 1 (normalized by the maximum of ridgeness on all objects)

   float      Thickness;   // Not filled in if ExtraMeasuresFlag is FALSE
	float      Sharpness;   // Not filled in if ExtraMeasuresFlag is FALSE
	float      Contrast;    // Not filled in if ExtraMeasuresFlag is FALSE

  int         RigidMotion; // indication of rigid motion, to detect stitches 
                           // WARNING: several values are possible, RDG_CST_RIGID_MOTION_TRUE, 
                           // RDG_CST_RIGID_MOTION_FALSE, RDG_CST_RIGID_MOTION_UNKNOWN

}CRdgMeasures;


// Center-line pixel
typedef struct{
	CRdgVi CenterLinePel; // Integer center-line pixel
	float  Thickness;     // Not filled in if ExtraMeasuresFlag is FALSE
	float  Sharpness;     // Not filled in if ExtraMeasuresFlag is FALSE
	float  Contrast;      // Not filled in if ExtraMeasuresFlag is FALSE
	int    ObjPelIdx;     // Corresponding index in the object pixel list
}CRdgCenterLinePel;

// Center-line pixel list
typedef struct{
	int               NbPel;           // Number of center-line pixels
	CRdgCenterLinePel *CenterLinePels; // Center-line pixels
}CRdgCenterLinePelList;


// Object pixel
typedef struct{
	CRdgVi        ObjPel;    // Integer object pixel
	float         Ori;       // Value in original image
	float         Rdg;       // Ridgeness
	float         Dir;       // Direction
	int           CenterLinePelIdx; // Corresponding index in the Center-line pixel list (-1 if not defined)
}CRdgObjPel;

// Object pixel list
typedef struct{
	int        NbPel;    // Number of object pixels
	CRdgObjPel *ObjPels; // Object pixels
	CRdgVi     MaxBox;   // Right bottom point of bounding box
	CRdgVi     MinBox;   // Left top point of bounding box
	CRdgVf     Centroid; // Centroid
}CRdgObjPelList;


// Object
typedef struct{
	CRdgObjPelList        ObjPelList;        // Object pixel list
	CRdgCenterLinePelList CenterLinePelList; // Center-line pixel list
	CRdgMeasures          Measures;          // Global measures on object
}CRdgObject;

// Object list
typedef struct{
	int        NbObject;
	CRdgObject *Objects;
}CRdgObjectList;


// Volatile extraction results (allocated and freed inside RdgLib, read-only)
typedef struct{
	CRdgObjectList  *ObjectList;      // Final object list
	CRdgAdvanced     Advanced;        // Advanced data (internal images)
}CRdgVolatile;


typedef struct{

   CRdgVolatile Volatile;                       // Volatile extraction results (read-only)

   CRdgVi       GlobalMotion;                   // Global motion of the objects
   int          StitchDetectedFlag;             // TRUE if a stitch was detected

   // OSCAR related
   int          CatheterTipFoundFlag;           // TRUE if catheter tip was found
   CRdgRoiCoord CatheterTipROI;                 // Catheter tip bounding box
   int          CatheterBodyFoundFlag;          // TRUE if catheter body was found
   float        CatheterBodyDir;                // Median direction of catheter body (in radians)
   CRdgRoiCoord CatheterBodyROI;                // Catheter body bounding box

}CRdgExtractResults;



//======================================================================================
// METHODS

#ifdef __cplusplus
extern "C" {
#endif

// Parameter passing
int RdgGetDefaultParam(CRdgExtractParam *ExtractParam, int DefaultParametersProfile);
int RdgGetParam(CRdgHdl RdgHdl, CRdgExtractParam *ExtractParam);
int RdgSetParam(CRdgHdl RdgHdl, CRdgExtractParam *ExtractParam); // After calling this function, RdgExtract() can be invoked only with time t==0


// Life-time management
int RdgCreate(CRdgHdl* PtRdgHdl, CRdgExtractParam *ExtractParam, int ImaDyn, int ImaWidth, int ImaHeight, int SSFactor);
int RdgDelete(CRdgHdl RdgHdl);
int RdgGetVersionInfo(CRdgHdl RdgHdl, char** VersionInfo);

// Processing
int RdgExtract(CRdgHdl RdgHdl, int t, short** InIma, CRdgExtractResults* ExtractResults, CRdgRoiCoord *RoiCoord);

int RdgExtractAngio(CRdgHdl RdgHdl, int t, short** InIma, CRdgExtractResults* ExtractResults, CRdgRoiCoord *RoiCoord, 
                    int CatheterBodyRemovalFlag, CRdgRoiCoord *CatheterBodyRoiCoord, float CatheterBodyDirection, float CatheterBodyAngleRange, 
                    int ExtractCatheterROIsFlag);

int RdgExtractFluoro(CRdgHdl RdgHdl, int t, short** InIma, CRdgExtractResults* ExtractResults, 
                     CRdgRoiCoord *RoiCoord, 
                     CRdgRoiCoord *CatheterTipRoiCoord,
                     int CatheterBodyRemovalFlag, CRdgRoiCoord *CatheterBodyRoiCoord, float CatheterBodyDirection, float CatheterBodyAngleRange);


#ifdef __cplusplus
};
#endif

#endif   //_RDG_H
