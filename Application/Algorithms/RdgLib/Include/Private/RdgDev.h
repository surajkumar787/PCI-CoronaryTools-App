// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_RDGDEV_H
#define		_RDGDEV_H

#include <Rdg.h>

#define RDG_TIMING 0 
#define MX_FLAG    0  

#if MX_FLAG
#include <MxLab.h>
#endif

//======================================================================================
// MACROS

#define RDG_DEBUG_VERBOSE   0   // 1-> set verbose to 1 in RdgImAlloc
#define RDG_PRINT_FOR_DEBUG 0   // 1-> allows various printf

#define RDG_SQ(a)            ((a)*(a))
#define RDG_SIGN(a)          (((a) >= 0)   ? (1): (-(1)))
#define RDG_ABS(a)           (((a) >= 0)   ? (a): (-(a)))
#define RDG_MIN(a,b)         (((a) <= (b)) ? (a):   (b) )
#define RDG_MAX(a,b)         (((a) >= (b)) ? (a):   (b) )
#define RDG_MIN4(a, b, c, d) RDG_MIN(RDG_MIN(a, b), RDG_MIN(c, d))
#define RDG_CLIP(a,b,c)      RDG_MIN(RDG_MAX((a), (b)), (c))
#define RDG_RND(a)           (((a) >= 0.0) ? (int)((a)+0.5) : (int)((a)-0.5))

#define RDG_DIFFANGLE(a)     ( ((a)<=90) ? (a) : ((180)-(a)) )
#define RDG_PYR_DOWN(dim)    (((dim) % 2) == 0 ? ((dim) >> 1) : ((dim) >> 1) + 1)
#define RDG_PYR_ODD(dim)     ((dim) % 2)

#define RDG_PI			        3.14159265358979323846

#define RDG_DEG2RAD(a)       ((a) * 0.0174532925)
#define RDG_RAD2DEG(a)       ((a) * 57.295779524)


#define RDG_ERROR_EXIT(Func, Code, Mess)	{                    \
     printf("Fatal error in %s : %s \n", Func, Mess);          \
     printf("Exiting program : type something to continue\n"); \
     getchar();                                                \
     exit(Code);                                               \
   }


#define UC unsigned char


//======================================================================================
// CONSTANTS

// Quality options
#define RDG_STEP_FOR_DIFFERENTIALS          RDG_CST_HALF_STEP
#define RDG_CST_GM_PAST_N_FRAMES            7   // number of past frames for global motion

// Speed options, should not change quality
#define RDG_USE_SSE_GAUSSIAN                1


//======================================================================================
// STRUCTURES

// String pixel list
typedef struct{
	int    NbPel;       // Number of string pixels
	CRdgVi *StringPels; // String pixels
}CRdgStringPelList;


typedef struct
{
    int               IwOri, IhOri;          // Size of the original input image (as passed to RdgCreate)
    int               IwProc, IhProc;        // Size of the possibly subsampled image
    int               SSFactor;              // Sub-sampling factor
    int               DynOri;                // Dynamic (in bit number) of the original image (as passed to RdgCreate)

    short             **ImSub;               // Sub-sampled original image, if SSFactor!=1 (XAny)
    CRdgVi            **ImPel;               // Contains the corresponding pixel coordinates in the original image, if SSFactor!=1 (XAny)
    float             **ImRwo;               // Ridge enhanced image (without No-Edge factor) (XAny)
    float             **ImNoE;               // No-Edge image (only when no-edge factor is used) (XAny)
    float             **ImNoi;               // Noise image (used in Extract only when Rdg-noise level is used) (XAny)
    float             **ImGau;               // Gauss image (input of ridge detector) (XAny)
    float             **ImRdg;               // Ridge image (with No-Edge factor): before background subtraction (XAny)
    float             **ImDir;               // Direction image in radian (from double-angle calculation)(XAny, after ridge filter)
    unsigned char     **ImSca;               // Scale image (Index of the best sigma at each pixel) (XAngio, XFluoro)
    float             **ImBkg;               // Background (ridgeness) image: the still stuff to remove (XAngio, XFluoro)
    float             **ImFrg;               // Foreground (ridgeness) image (Rdg-Bkg) (XAngio, XFluoro)
    unsigned char     **ImSee;               // Threshold seed image (0 or 1): Hysteresis high-threshold, after small-label cleaning (XAngio, XFluoro)
    unsigned char     **ImThr;               // Threshold image (0 or 1): after everything (XAny)
    int               **ImLab;               // Labelled image: after everything: labels in [2, NbLabel+2[ (might contain 1, not "hysterized") (XAny)
    int               **ImLabTm1;            // Labelled image at time t-1 (XAny)
    unsigned char     **ImMask;              // Binary mask image, vessels+tip (Like Thr, but only "hyterized" pixels, if hysteresis is used, else = Thr) (XAny)
    unsigned char     **ImGMMask;            // Binary image of pixels undergoing stitch/global rigid motion (XAny)
    unsigned char     ***ImPastThr;          // Array of past threshold images

    short             **ImDil;               // Dilation (Math. Morphology) of sub-sampled image (XFluoro)
    short             **ImClo;               // Closing (Math. Morphology) of sub-sampled image (the wire-tip is normally removed) (XFluoro)
    float             **ImTipRdg;            // Wire-Tip ridge image (with No-Edge factor): Rdg on Sub - Clo (XFluoro)
    float             **ImTipBkg;            // Wire-Tip background image (XFluoro)
    float             **ImTipFrg;            // Wire-Tip foreground image(XFluoro)
    unsigned char     **ImTipSee;            // Wire-Tip threshold seed image (0 or 1) (XFluoro)
    unsigned char     **ImTipThr;            // Wire-Tip threshold image (0 or 1) (XFluoro)
    int               **ImTipLab;            // Wire-Tip labelled image (XFluoro)
    unsigned char     **ImTipMask;           // Binary mask of wire guide tip-like devices (only hysterized pixels) (XFluoro)

    CRdgVi            *PastGlobalMotion;     // Array of past global motion
    int               StitchCumulativeFlag;  // Stitch detection cumulative flag
    int               ExtractTime;           // Internal time of the extraction process
    int               NewParameters;         // True if external parameters (passed by RdgSetParam()) changed
    CRdgRoiCoord      RoiCoord;              // Roi coordinates

    int               SubTipLength;          // Expected Tip Length after SSFactor

    CRdgObjectList    PreObjectList;         // Object list before tracking
    CRdgVf            *CentroidTm1;          // Object centroids at time t-1
    int               NbObjTm1;              // Number of objects at time t-1
    CRdgVi            GlobalMotion;          // Global motion of the objects

    float             TresholdTm1;           // Threshold at time t-1
    int               ThresholdWeightTm1;    // Weight (number of points) of threshold at time t-1

    float             RdgFilterNorm;         // Response of the ridge filter over an edge of depth -1 (dirac-like)

    float             ***ImCircBuffer;       // Circular buffer of images for background estimation
    float             ***ImTipCircBuffer;    // Circular buffer of images for tip background estimation
    int               CircBufferSize;        // Number of frames allocated in circular buffer size

    float             AbsHighThreshold;      // Absolute high threshold for hysteresis thresholding
    float             AbsLowThreshold;       // Absolute low  threshold for hysteresis thresholding
    float             TipAbsHighThreshold;   // Absolute high threshold for hysteresis thresholding
    float             TipAbsLowThreshold;    // Absolute low  threshold for hysteresis thresholding

    CRdgExtractParam  CachedParam;           // External parameters for the extraction process, cached from previous setting, not to be used directly, except by RdgSetParam()

    int               CatheterTipFoundFlag;  // Catheter tip  found flag
    int               CatheterTipLargestSize;// Catheter tip  largest size
    CRdgRoiCoord      CatheterTipROI;        // Catheter tip  bounding box
    int               CatheterBodyFoundFlag; // Catheter body found flag
    int               CatheterBodyLargestSize;//Catheter body largest size
    float             CatheterBodyDir;       // Median direction of catheter body
    CRdgRoiCoord      CatheterBodyROI;       // Catheter body bounding box

}CRdgExtractProc;

typedef struct
{
    char               *VersionInfo;        // String containing library's version-related info

    CRdgExtractParam    ExtractParam;       // External parameters for the extraction process, possibly adapted by RdgSetParam(), should be used internally as opposed to CachedExtractParam
    CRdgExtractProc     ExtractProc;        // Internal variables and images for the extraction process

}CRdg;



//======================================================================================
// METHODS

// RdgUtil.c
int RdgImAlloc(void **Pt1, void ***Pt2, int NbByte, int Iw, int Ih);
int RdgImFree(void *Pt1, void **Pt2);
int RdgImAlignedAlloc(void **Pt1, void ***Pt2, int NbByte, int Iw, int Ih, int Alignment);
int RdgImAlignedFree(void *Pt1, void **Pt2);
int RdgSysTimeMs(char *Name);

// RdgUtilImage.c
void RdgAddNoise(short *Ori, int Iw, int Ih, float Sig);
void RdgSubSampleImage(short **Ori, short **Sub, CRdgVi **Pel, int IwSub, int IhSub, int SSFactor);
void RdgSubtractImageAndKeepPositive(float **Dst, float **Src, int Iw, int Ih, CRdgRoiCoord *RoiCoord);

// RdgExtractLabel.c
void RdgClearObjectList(CRdgObjectList *ObjectList);
void RdgThresholdLabels(short **Ori, short **Sub, CRdgVi **Pel, int **Lab, float **Rdg, float **Dir, unsigned char **Thr,
												int IwOri, int IhOri, int IwSub, int IhSub, int SSFactor, CRdgRoiCoord *RoiCoord, CRdgObjectList *ObjectList,
												int NbLabels, int *NbPelPerLab, float *CumRdgPerLab, int NbLabKept, float RidgenessThresholdRatio,  int ExtraMeasuresFlag);

void RdgThresholdLabelsBySize(int **Lab, unsigned char **Thr, CRdgRoiCoord *RoiCoord, int NbLabels, int *NbPelPerLab, int MinNbPelPerLab, int KeepLargestLabel);

void RdgLabel(unsigned char **Thr, int **Lab, float **Rdg, float **Dir, int Iw, int Ih, 
              CRdgRoiCoord *RoiCoord, int *NbLabels, int *NbPelPerLab, float *CumRdgPerLab, 
              int NbRangePix, int DirLabelFlag, int MaxDirDiff, int TipLength);
void RdgLabelFromSeed(unsigned char **Seed, unsigned char **Thr, int **Lab, float **Rdg, float **Dir, 
                      int Iw, int Ih, CRdgRoiCoord *RoiCoord, int *NbLabels, int *NbPelPerLab, 
                      float *CumRdgPerLab, int *NbExtremitiesPerLab, const int NbRangePix, const int DirLabelFlag, 
                      const float MaxDirDiff, int TipLength);
void RdgAsymetricRidgenessLabels(int **Lab, float **Gau, float **Rdg, float **Dir, int Iw, int Ih, 
                                 float KernelSig, float RdgThresh, CRdgRoiCoord *RoiCoord, 
                                 int NbLabels, int *NbPelPerLab, float *CumRdgPerLab, int NbRangePix, 
                                 int TipLength
                                 , int ZeroOutLowRidgenessFlag
                                 , float RdgFilterNorm);
void RdgCalcGlobalMotion(int t, int **ImLab, int **ImLabTm1, float **ImRdg, int IwSub, int IhSub, 
	int GlobalMotionFlag, CRdgRoiCoord *RoiCoord, CRdgObjectList *ObjectList, 
	CRdgVf *CentroidTm1, int *NbObjTm1, float RdgThresh, int ThinningFlag, int FastProfileFlag, 
   CRdgVi *GlobalMotion);
void RdgVerifyGlobalMotion(int t, int **ImLab, int IwSub, int IhSub,  CRdgRoiCoord *RoiCoord, CRdgObjectList *ObjectList, int ThinningFlag,
													 CRdgVi *CurrentGlobalMotion, CRdgVi *PastGlobalMotion,  unsigned char ***ImPastThr, float CardiacMotionThreshold,
													 int *significant_motion_flag, int *stitch_detected_flag, unsigned char **ImGMMask);
void RdgUpSampleObjects(CRdgObjectList *ObjectList, CRdgVi **Pel, int SSFactor);
void RdgUpSampleStringPelList(CRdgStringPelList *TipPelList, CRdgVi **Pel, int SSFactor);

// RdgExtractThreshold.c
void RdgThreshold(float **Rdg, unsigned char **Thr, int Iw, int Ih, CRdgRoiCoord *RoiCoord, 
	float NoiseSig, int ThrType, int NoiThresh, float AbsThresh, int L1ThreshFlag, float L1Bg, int NbPixKept, int *NbThrPel,
	float *RdgThresh, 
   float *TresholdTm1,
   int *ThresholdWeightTm1);

// RdgExtractLabelFeatures.c
void RdgCalcCenterLinePelList(CRdgObjPelList *OPelList, CRdgCenterLinePelList *CLPelList, CRdgMeasures *Measures, 
	short **Ori, CRdgVi **Pel, float **Rdg, float **Dir, int IwOri, int IhOri, int SSFactor, int ExtraMeasuresFlag);

// RdgExtractRidgeFilter.c
void RdgEstimStat(float **Ima, int Iw, int Ih, int Off, float *pMoy, float *pSig);
void RdgEnhanceRidge(short **Ori, 
                     float **Rwo, float **NoE, 
                     float **Gau, float **Rdg, float **Dir, float **Noi, int Iw, int Ih, int DynOri,
                     CRdgRoiCoord *RoiCoord, int NormOriFlag, float LFSig, int RidgeType, 
                     float KernelSig, 
                     int EdgeFromOriFlag, float NoEdgeFac, 
                     int ThrType, 
                     int DirImgIsNeeded, 
                     float *NoiseSig, float *L1Bg);
void RdgEnhanceMultiScaleRidge(short **Ori, float **Gau, float **Rdg, float **Dir, unsigned char **Scale, int Iw, int Ih, int DynOri, CRdgRoiCoord *RoiCoord,
															 int NormOriFlag, float LFSig, int RidgeType, float *KernelSigs, int nKernelSigs, float gamma, int EdgeFromOriFlag, float NoEdgeFac);
void RdgSuppressNonMaxima(float **Rwo, float **Rdg, float **Dir, int Iw, int Ih, CRdgRoiCoord *RoiCoord);

// RdgExtractRidgeDifferential.c
void RdgRdgDifferential(short **Ori, 
                        float **Rwo, float **NoE, 
                        float **Gau, float **Rdg, float **Dir, int Iw, int Ih, int DynOri,
                        CRdgRoiCoord *RoiCoord, int RoiFlag, int NormOriFlag, float LFSig, 
                        int RidgeType, float KernelSig, 
                        int EdgeFromOriFlag, float NoEdgeFac, 
                        int DirImgIsNeeded, 
                        int ThrType, float* pSig, float *L1Bg);

// RdgExtractMorphFilter.c
void RdgMorphDilate(short ** const Ori, 
                    short **Dil, const int Iw, const int Ih, 
                    CRdgRoiCoord *RoiCoord, 
                    short ** const Se, const int Sew, const int Seh, 
                    const int Apw, const int Aph);
void RdgMorphErode (short ** const Ori, 
                    short **Dil, const int Iw, const int Ih, 
                    CRdgRoiCoord *RoiCoord, 
                    short ** const Se, const int Sew, const int Seh, 
                    const int Apw, const int Aph);
void RdgMorphDilateRectSE(short ** const Ori, 
                          short **Dil, const int Iw, const int Ih, 
                          CRdgRoiCoord *RoiCoord, 
                          const int Sew, const int Seh,
                          const int Apw, const int Aph);
void RdgMorphErodeRectSE (short ** const Ori, 
                          short **Dil, const int Iw, const int Ih, 
                          CRdgRoiCoord *RoiCoord, 
                          const int Sew, const int Seh,
                          const int Apw, const int Aph);

// RdgExtract.c
void RdgFillResults(CRdgExtractResults *ExtractResults, CRdgExtractProc *ExtractProc);

// RdgExtractBackground.c
void RdgBackgroundEstimation(int t, float **ImIn, float ***ImCircBuffer, int Iw, int Ih, int CircBufferSize, CRdgRoiCoord *RoiCoord, float **ImBkg, float **ImFrg)
;

// RdgExtractMask.c
void RdgBinaryMaskFromLabels       (unsigned char **ImMask, int **ImLab,                              int Iw, int Ih, CRdgRoiCoord *RoiCoord);
void RdgBinaryMaskFromLabelsAndMask(unsigned char **ImMask, int **ImLab, unsigned char **ImMergeMask, int Iw, int Ih, CRdgRoiCoord *RoiCoord);

// RdgExtractCatheters.c
void RdgGetCathetersBoundingBoxes(int **ImLab, int Iw, int Ih, 
                                  CRdgRoiCoord *ShuttersRoiCoord, int NbLabels, int *NbPelPerLab, 
                                  int BorderMargin,
                                  int *CatheterTipFoundFlag,  int *CatheterTipLargestSize,  CRdgRoiCoord *CatheterTipRoiCoord,
                                  int *CatheterBodyFoundFlag, int *CatheterBodyLargestSize, CRdgRoiCoord *CatheterBodyRoiCoord, float *CatheterBodyDir);
void RdgGetMedianDirection(float **ImDir, int **ImThr, int Iw, int Ih, 
                           CRdgRoiCoord *CatheterBodyRoiCoord, float *MedianDir);
void RdgRemoveCatheterBody(float **ImRdg, float **ImDir, int Iw, int Ih, CRdgRoiCoord *RoiCoord, float MedianDirection, float AngleRange);

// RdgExtractAngio.c
void RdgFillAngioResults(CRdgExtractResults *ExtractResults, CRdgExtractProc *ExtractProc);

// RdgExtractFluoro.c
void RdgFillFluoroResults(CRdgExtractResults *ExtractResults, CRdgExtractProc *ExtractProc);


#endif   //_RDGDEV_H