// ***************************************************************************
// * Copyright (c) 2002-2016 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_MKXDEV_H
#define		_MKXDEV_H


#include <Platform.h>
#include <memory.h>

#include <Mkx.h>
#include <BxWire.h>
#include <Tr1.h>
#include <MkxStat.h>
#include <MkxFeatures.h>
#include <SWLib.h>
#include <IThreadPool.h>

//===================================================================================================
// CONSTANTS
//#define MKX_TIMING
#include <MkxTiming.h>

#define MKX_MARKERMODE_I MKX_CST_MARKER_FINE 
#define UC unsigned char
#define MKX_WCPLMAXNBCPLADAPTFACTOR_F                     0.5f
#define MKX_WCPLPRUNINGADAPTFACTOR_F                      0.8f
#define MKX_WIRECPLPRUNINGFACTOR_F                        0.5f
#define MKX_CST_DIST_TOLERANCE_WHEN_LOCKED               20       // set on 17-02-05 // set tolerance to 20%
#define MKX_CST_DarkPixelsPenalizeDelay                   1       //dark pixel penalizing is applied only for t >= MKX_CST_DarkPixelsPenalizeDelay
#define MKX_CST_DarkPixelsPenalizeIntegrationFactor       0.05f   //
#define MKX_CST_SWDETECTIONTIME                           4       //time at which sternal wire presence/absence is decided

#define MKX_ALLOC_INCREMENT 100

#define MKX_CST_NBPROCMAX 3


// Marker modes 
enum {
  MKX_CST_MARKER_INT = 0,           // Marker in int
  MKX_CST_MARKER_ROUGH,             // Marker in float
  MKX_CST_MARKER_FINE,              // Marker in float
};

//===================================================================================================
// MACROS
#define MKX_MIN(a,b)    (((a) <= (b)) ? (a):   (b)  )
#define MKX_MAX(a,b)    (((a) >= (b)) ? (a):   (b)  )
#define MKX_SGN(a)      (((a) >=  0 ) ? (1):   (-1) )
#define MKX_SQ(a)       ((a)*(a))
#define MKX_ABS(a)      (((a) >= 0)   ? (a): (-(a)))
#define MKX_CLIP(a,b,c) MKX_MIN(MKX_MAX((a), (b)), (c))
#define MKX_PI          (3.14159265358979f)
#define MKX_SIGN0(a)     (((a) >  0)   ? (1): ((a) < 0)   ? (-(1)): (0))
#define MKX_RND(a)       (((a) >= 0.0) ? (int)((a)+0.5) : (int)((a)-0.5))
#define MKX_EUCLI_DIST(x1,y1,x2,y2) (float)sqrt((double)(MKX_SQ((x1)-(x2)) + MKX_SQ((y1)-(y2))))
#define MKX_BIG_SHORT 128*256-1
#define MKX_N_ELEMS(a) (sizeof(a)/sizeof(a[0]))
#define MKX_DIFFANGLE(a) ( ((a)<=90) ? (a) : ((180)-(a)) )
#define MKX_PYR_DOWN(dim) (((dim) % 2) == 0 ? ((dim) >> 1) : ((dim) >> 1) + 1)
#define MKX_PYR_ODD(dim)  ((dim) % 2)

#define MKX_BORDER_FOR_GAUSS (MKX_CST_BORDER) // In MkxUsrConst.h

#define MKX_ERROR_EXIT(Func, Code, Mess)	{                                                           \
  printf("Fatal error in %s : %s \n", Func, Mess);          \
  printf("Exiting program : type something to continue\n"); \
  getchar();                                                \
  exit(Code);                                               \
}
#define MKX_FREE(x) {if (x) { free(x); x = NULL; }}



//===================================================================================================
// STRUCTURES
// Integer 2D-vector

typedef struct
{
  short X;    // X coordinate (horizontal from image left to image right hand sides)
  short Y;    // Y coordinate (vertical from image top to image bottom)
}
CMkxVs;

typedef struct
{
  int X;    // X coordinate (horizontal from image left to image right hand sides)
  int Y;    // Y coordinate (vertical from image top to image bottom)
}
CMkxVi;

typedef struct
{
  float Xf;   // X coordinate (horizontal from image left to image right hand sides)
  float Yf;   // Y coordinate (vertical from image top to image bottom)
}
CMkxVf;

// Couple coordinates
typedef struct
{
  CMkxVi V1, V2;
}
CMkxCoupleInt;

typedef struct
{
  CMkxVf V1f, V2f;
}
CMkxCoupleFloat;

typedef struct
{
  int   NbPix;         // Nb pixels
  CMkxVi V;            // Coordinates
  float Intensity;     // Intensity

}CMkxBlob;

typedef struct
{
  float Feat[MKX_CPL_N_FEATURES];            //all features for one couple
  int   Feat_available[MKX_CPL_N_FEATURES];  //availability of features for one couple

}CMkxCplFeatures;


typedef struct
{
  float  CplMeritOverall;              // overall intensity

  float  CplMeritWrtBlobs;            // couple factor of merit measured with respect to Point-enhancement 

  float  CplMeritWrtWire;             //couple factor of merit measured with respect to wire
  float  CplMeritWrtWireAvailable;    //
  float  CplMeritWrtWire_markers_correlation;   //
  float  CplMeritWrtWire_wire_correlation;      //

  float  CplMeritWrtClues;            //couple factor of merit measured with respect to clues
  int    CplMeritWrtCluesAvailable;
  float  CplMeritWrtCluesCx;
  float	 CplMeritWrtCluesCy;
  float	 CplMeritWrtCluesBreadth;
  float	 CplMeritWrtCluesAng;

  float  CplMeritWrtHistory;           //couple factor of merit measured with respect to history
  int    CplMeritWrtHistoryAvailable;
  float  CplMeritWrtHistoryCx;
  float	 CplMeritWrtHistoryCy;
  float	 CplMeritWrtHistoryBreadth;
  float	 CplMeritWrtHistoryAng;

}CMkxCplMerit;

typedef struct
{
  CMkxBlob Blob1;  // First marker in couple
  CMkxBlob Blob2;  // Second marker in couple 
  int        Rank;         // Rank
  int        MatchingRef;  // Matching marker-reference variable (1=match, 0=no-match, -2=no ref file, -1=no ref image)
  int        FirstRank;      // i.e. before maker couple wire-based rescoring
  int        Index1, Index2; //indices of the first and second blobs in the list of blobs (KBestLblBlobs)
  int        Flag;          // flag to indicate if the couple is to be taken into account

  CMkxCplMerit    CplMerit;
  CMkxCplFeatures CplFeat;   // All features of the couple

}CMkxBlobCouple;

typedef struct{
    float     a, b;                  // coordinates, long and small axes, 
    float     level_in, level_out, err;     //grey level, average least square error
    int       n;                            // total number of profiles used for computing
    float     Dir, DirConfidence;

}CMkxRefineAdvanced;

typedef struct{
  CMkxCoupleFloat CplFine;
  CMkxRefineAdvanced Advanced1;
  CMkxRefineAdvanced Advanced2;

}CMkxMarkerCoupleRefined;

typedef struct
{
  float Mean, Min, Max, Stdv, Median;
  float Aw, Bw, Meanw;
  int N;
  float Tdistal, Tproximal;

}CMkxHyStat;

typedef struct
{
  CMkxHyStat  FeatStat[MKX_CPL_N_FEATURES];
  CMkxHyStat  DevStat[MKX_CPL_N_FEATURES];
  int NbUsefulPlanes;
  float UsefulPlanesMeanTime;
  float ModulationFactor;
  float ModulationFactorBuf;

}CMkxHistoryStat;

typedef struct
{
  int   PredictVal_available;
  float PredictVal;
  float FuzzyLow;
  float FuzzyHigh;

}CMkxFeatPredict; //predictions for one feature

typedef struct
{
  CMkxFeatPredict FeatPredict[MKX_CPL_N_FEATURES];           //predictions for each feature
  int             FeatPredict_available[MKX_CPL_N_FEATURES]; //availability of prediction for each feature
  int             PredictValid;

}CMkxPredict;

typedef struct
{
  int   Time;
  int   PlaneValid;                // indicates if the structure has been filled in with MkxUpdateHistory
  int   PlaneUseful;               // indicates if the structure has been filled in with MkxUpdateHistory
  float X1, Y1;
  float X2, Y2;
  float Cx, Cy;
  float Breadth;
  float Ang;
  int   HistoryStatus;
  int   Locked;
  float a1, b1, a2, b2;			               // blobs sizes of the markers

  CMkxCplFeatures CplFeat;
  CMkxPredict     HistoryPredict;            //prediction used for the extraction (computed at t-1)
  CMkxHistoryStat HistoryStatDeep;           //stat computed at t based on all data in stack
  CMkxHistoryStat HistoryStatShallow;        //stat computed at t based on data not older than MKX_HISTORYDEPTH_I

}CMkxHistoryPlane;


typedef struct
{
  int StackSize;
  int CurrIndexInStack;
  CMkxHistoryPlane *Plane;

}CMkxHistory;

typedef struct
{
  int XMin, YMin, XMax, YMax;
  int Width, Height;

}CMkxRoiCoord;

typedef struct
{
  float XMinFac, YMinFac, XMaxFac, YMaxFac;

}CMkxRoiCoordFac;

typedef struct
{
  float X1, Y1, X2, Y2;
  float Cx;
  float Cy;
  float Breadth;
  float Ang;
}
CMkxDerivedClue;

typedef struct
{
  float CxLow;
  float CxHigh;
  float CyLow;
  float CyHigh;
  float BrLow;
  float BrHigh;
  float AnLow;
  float AnHigh;
}
CMkxHistoryFuzzyPara;

typedef struct
{
  float BlobEnhRingRadiusInPix;
  float BlobEnhRingSigInPix;
  int     BlobEnhNbDir;
  CMkxVi* BlobEnhTabRing; // coords for ring excursion
  float TrAngleHighInDeg;
  float TrAngleSkipInDeg;
  float TrBreadthHighPercent;
  float TrBreadthSkipPercent;
  float TrCentroidHighInPix;
  float TrCentroidSkipInPix;
  float HistoryCentroidXLowInPix;
  float HistoryCentroidXHighInPix;
  float HistoryCentroidYLowInPix;
  float HistoryCentroidYHighInPix;

}CMkxProcessPara;


typedef struct
{
  int               IwOri, IhOri;            // Size of the original input image (as passed to MkxStart)
  int               IwProc, IhProc;          // Size for internal calculation deduced from original image size
  int               SSFactor;                // Subsampling factor
  int               DynOri;                  // Dynamic (in bit number) of the original image (as passed to MkxStart)
  void*             WireHdl;                 // Handler for wire extraction and warping
  void*             ExtraMeasuresWireHdl;    // Handler for wire used for extra measures
  BxWireParameters  ExtraMeasuresWireParams; // Wire parameters
  float             *ImBlobMin1;             // image buffer for minimum value along time       

  float             *ImEnh1;      // Image obtained from blob-enhancement      
  short             *ImSub1;      // Possibly sub-sampled version of input image 
  CMkxVs            *ImPel1;      // In case of sub-sampling, ImPel contains the corresponding pixel coordinates in the original image
  unsigned char     *ImThr1;      // Binary image obtained after thresholding
  int               *ImLab1;      // Labelled image

  float              ImEnhMin1, ImEnhMax1;
  float              ImEnhMin2, ImEnhMax2;

  int                ExtractTime;            // Internal time of the extraction process

  int				         AdaptiveWorkNbLblBlobsKept;             //  Max number of labels to be kept (adaptive when locked)
  int				         EstimatedMaxNbPixPerBlob;    //  Estimated max number of pixels per label (calculated from radius)
  int				         AdaptiveWorkNbPixelsKept;             //  Max number of pixels to be kept (adaptive when locked)
  int                WireCplMaxNbCouple;     // Wire couple maximum number of tested couples (adaptive or equal to Val[MKX_CPLSELECTMAXNBWIRES_I].Int)
  float              WireCplPruningFactor;   // Wire couple pruning factor (adaptive or equal to Val[MKX_WIRECPLPRUNINGFACTOR_F].Float)

  int                Margin;                 // Margin to prevent border problems
  CMkxRoiCoord       MarginRoiCoord;         // The largest useable ROI 
  CMkxRoiCoord       UserRoiCoord;           // Roi (possibly clipped) defined by Roi user parameters with DEF/NODEF option
  CMkxRoiCoord       UsefulRoiCoord[3];	     // Useful area for candidates (moving, or equal to UserRoiCoord when NoAdapt)
  int                TwoRoisFlag;            // flag to indicate weither to use UsefulRoiCoord[0] or both UsefulRoiCoord[1] and [2]
  CMkxRoiCoord       WorkRoiCoord;           // Expanded UsefulRoiCoord for process (process-related margin added to it)
  CMkxRoiCoord       UserInitRoiCoord;       // Roi (possibly clipped) defined by Roi user parameters with InitRoi option
  int                NotYetLockedFlag;       // flag to indicate if it was never locked since t=0 (associated to UserInitRoiCoord)

  int                NbLblBlobs;               // actual number of labels after labelling 
  int                MaxNbCouples;           // maximum number of marker couples (computed from MaxNbBestBlobs) 
  int                NbBestLblBlobs;           // actual number of marker candidates after selection for the best ones
  CMkxBlob        *KBestLblBlobs;            // marker candidates
  int                NbRawCouples;           // actual number of marker couples candidates after rejection on distance
  CMkxBlobCouple  *RawCouples;               // marker couple candidates before tracking (coordinates in pixels in possibly subsampled image)
  int                NbTrInCouples;          // actual number of marker couples candidates at input of tracking
  int                NbTrOutCouples;         // actual number of marker couples after tracking
  CMkxBlobCouple  *TrCouples;                // marker couple candidates after tracking (coordinates in pixels in possibly subsampled image)

  CMkxCoupleFloat   PreviousCouple;         //buffer for polarize
  int                PreviousCoupleFlag;     //flag to indicate weither a PreviousCouple is available
  CMkxMarkerCoupleRefined MarkerCoupleRefined; //marker couple after refinement
  CMkxCoupleFloat    MarkerCoupleFloat;       //marker couple with possibly refined coords (in float)
  CMkxCoupleInt    MarkerCoupleInt;       //marker couple (not refined)

  CMkxClue           Clue;                   // Client's provided clues (reset after each extraction)
  CMkxDerivedClue    DerivedClue;            // Derived clues(Cx, Cy, Breadth, Ang)

  CTr1Hdl            TrackingHdl;            // Handler on current tracking process
  CTr1Input          TrIn;                   // All the input information to tracker at each iteration
  CTr1Output         TrOut;                  // Tracking results

  int                WireTime;

  CMkxRoiCoord       ClueRoi[3];             // ClueRoi[0] for a single roi ClueRoi[1], ClueRoi[2] for 2 rois (for clipping UsefulRoiCoord)
  CMkxRoiCoord       HistoryRoi;             // Roi from history (for clipping UsefulRoiCoord)

  CMkxHistory        History;                // keep all results for a period of time
  CMkxHistoryFuzzyPara HistoryFuzzyPara;     // instantaneous fuzzy para for history

  CMkxSeqPara        SeqPara;

  float              MarkersDistMm;            //mean markers distance in mm from SeqPara: MarkersDistMm = 0.5f*(SeqPara.MarkersDistMaxMm+SeqPara.MarkersDistMinMm)
  int							   CplSelectDistToler;       //markers distance in percent from SeqPara: CplSelectDistToler = MKX_RND( 100*(SeqPara.MarkersDistMaxMm-SeqPara.MarkersDistMinMm)/(SeqPara.MarkersDistMaxMm+SeqPara.MarkersDistMinMm))
  int                SubMarkersDist;           // Markers' distance in possibly sub-sampled image (ImSub):  SubMarkersDist = MarkersDistMm / SeqPara.MmPerPixel / SSFactor;
  int                AdaptiveWorkMarkersDist;         // Markers' distance (adaptive or equal to SubMarkersDist)
  float              AdaptiveDistTolerMin;            // AdaptiveDistTolerMin = MKX_MAX((AdaptiveWorkMarkersDist * (1 - CplSelectDistToler*0.01) - Offset), 0)
  float              AdaptiveDistTolerMax;            // AdaptiveDistTolerMax = AdaptiveWorkMarkersDist * (1 + CplSelectDistToler*0.01) + Offset)

  CMkxProcessPara    ProcessPara;

  CTr1AdvancedOutput Tr1AdvancedOutput;

  CTiming            Timing;
  int Status;
  int Locked;

  CSWHdl              SWHdl;               // Handler for SWLib
  int                IwProc_SW, IhProc_SW;  // Size for SWLib deduced from original image size
  int                SSFactor_SW;           // Subsampling factor for SWLib input image
  CMkxRoiCoord       UserRoiCoord_SW;       // same as UserRoiCoord but resized according to SW subsampling
  short             *ImSubSw;              // Possibly sub-sampled version of input image for SwLib (no adaptive Roi)
  CMkxVs            *ImPelSw;              // In case of sub-sampling, ImPelSw contains the corresponding pixel coordinates in the original image
  CSWRes             SWRes;
  unsigned char     *ImSwMskSub;           //mask from sternal wire removal subsampled
  unsigned char     *ImSwMskProc;           //mask from sternal wire removal rescaled according to blob enhancement process
  int                NbSwPels;             //number of Pixels in sternal wire removal mask
  CMkxVi            *SwPels;               //Pixels in sternal wire removal mask (upsampled)
  CMkxSwStatus       SwStatus;             //sternal wire removal status

  int                nbCoresMax;             //max number of cores to be used (in SWLib)
  IThreadPool       *pool;

}CMkxExtractProc;

typedef struct
{
  char               *VersionInfo;       // Library's version-related info
  char               *VersionInfoCopy;   // When returned, as a pointer, to client

  CErrHdl             ErrHdl;
  int                 ErrHdlDeleted;     // set to 0 if ErrHdl has been deleted
  CPrmHdl             PrmHdl;

  CMkxExtractProc     ExtractProc;       // Internal variables and images for the extraction process
  CPrmPara			      ActualPara;
  CPrmParaCtx         ParaContext;

}CMkx;

//===================================================================================================
// METHODS

#ifdef __cplusplus
extern "C" {
#endif

  // Mkx.c (in addition to Public prototypes of Mkx.h) */
  int MkxSetParaCtx(CPrmHdl P);
  int MkxSetVersion(CMkx *This);

  // MkxError.c (in addition to Public prototypes of Mkx.h) */
  int  MkxErrorMgInit(CMkx *This);
  int  MkxErrorIndexFromCode(CMkx *This, int ErrorCode);
  int  MkxStartErrorReportFromString(CMkx *This, char *String);
  int  MkxStartErrorReportFromLeaf(CMkx *This, char* LeafFunc, int ErrorCode, char *Reason);
  int  MkxAppendErrorReportFromString(CMkx *This, char *String);
  int  MkxAppendErrorReportFromCode(CMkx *This, int Code);

  //MkxBlobEnhance.cpp
  void   MkxBlobEnhanceRing(
    const short* Ori,
    int IwProc, int IhProc,
    float *Enh,
    float* imBlobMin,
    float * EnhMin1, float * EnhMax1, float * EnhMin2, float * EnhMax2,
    CMkxRoiCoord WorkRoi, CMkxRoiCoord UsefulRoiCoord[], int UseTwoRoisFlag/*, CMkxRoiCoord UsefulRoiCoord[1], CMkxRoiCoord UsefulRoiCoord[2]*/,
    int t,
    int NbDir,
    const CMkxVi* TabRing,
    float BFGaussSig,
    float IntensityFactor,
    int dkpxFlag,
    float dkpxFac,
    int dkpxTime
    );

  //MkxPixSelect.cpp
  int  MkxPixSelect(
    const float* enh,
    UC* thr,
    int IwProc, int IhProc,
    float ImEnhMax1, float ImEnhMax2,
    int twoRoisFlag, CMkxRoiCoord* UsefulRoiCoord,
    int AdaptiveWorkNbPixelsKept
    );

  //MkxLabelling.cpp
  void  MkxLabelling(
    const UC  *Thr,
    int *Lab,
    int *NbLblBlobs,
    int Iw, int Ih,
    CMkxRoiCoord roiCoord
    );

  //MkxBlobSelect.cpp
  void MkxBlobSelect(
    int NbLblBlobs,
    const int* Lab, const float* Enh,
    int Iw, int Ih,
    CMkxRoiCoord roiCoord,
    int *NbBestLblBlobs,
    CMkxBlob *KBestLblBlobs,
    int MaxNbBlobs
    );

  //MkxCouplesSelect.cpp
  int MkxCouplesSelect(
    int nbLabels,
    const CMkxBlob *labels,
    int *nbCouples,
    CMkxBlobCouple *couples,
    int TwoRoisFlag, CMkxRoiCoord roi1, CMkxRoiCoord roi2,
    float distMin, float distMax,
    CPrmPara *ExtractParam
    );
  void MkxBlobCoupleOrderByIntensity(CMkxBlobCouple *BestLabelCouples, int NbLabelCouples);
  void MkxCombineIntensities(int nbCouples, CMkxBlobCouple *couples, CPrmPara *extractParam);


  //MkxPredict.cpp
  int MkxCouplesSetIntensityWrtHistory(
    int nbCouples,
    CMkxBlobCouple *couples,
    CMkxHistory *history,
    float historyFactor, int historyMode
    );
  void MkxUpdateCbaFromXyOne(float x1, float y1, float x2, float y2, float* cx, float* cy, float* br, float* an);
  void MkxUpdateHistory(CMkxHistory *history, int t, CMkxExtractResults *extractResults, CMkxAdvancedResults *advancedResults, int sSFactor, CPrmPara *extractParam, CMkxProcessPara *processPara);
  void MkxResetHistory(CMkxHistory *history);
  void MkxPrintFeatPredict(CMkxPredict *p);
  void MkxPrintPredict(CMkxPredict *Predict);
  void MkxPrintHistoryStat(CMkxHistoryStat *HistoryStat);
  void MkxPrintHistory(CMkxHistory *Stack, int PrintDepth);

  void MkxFillResults(
  CMkxExtractResults *exr,
  CMkxCoupleFloat cpl,
  CMkxMarkerCoupleRefined MarkerCoupleRefined,
  int Status, int Locked,
  int twoRoisFlag, CMkxRoiCoord* roi, int SSFactor,
  int IwProc, int IhProc,
    CMkxSwStatus SwStatus, int IwProc_SW, int IhProc_SW
  );

  //MkxApplyRigidMotionMask.cpp
  int  MkxApplyRigidMotionMask(
    float *Enh, int IwEnh, int IhEnh,
    CMkxRoiCoord UsefulRoiCoord,
    unsigned char *Mask, int IwMask, int IhMask
    );

  //MkxParam.cpp
  int   MkxSetExtractProcFromOnTheFlyParam(
    CMkxProcessPara *pp, float *MarkersDistMm, int *CplSelectDistToler, int *SubMarkersDist, int *EstimatedMaxNbPixPerBlob, 
    int *WireCplMaxNbCouple, float *WireCplPruningFactor,
    CMkxRoiCoord *MarginRoiCoord, int *Margin, CMkxRoiCoord *UserInitRoiCoord, CMkxRoiCoord *UserRoiCoord,
    CTr1Hdl TrackingHdl,
    int IwProc, int IhProc,
    int SSFactor, int ExtractTime, int Locked,
    CMkxSeqPara SeqPara,
    CPrmPara *extractPara
    );
int   MkxSetExtractProcFromCriticalParam(CMkxSwStatus *SwStatus,
  int* SSFactor, int *IwProc, int *IhProc,
  int* SSFactor_SW, int *IwProc_SW, int *IhProc_SW, CMkxRoiCoord *UserRoiCoord_SW,
  int IwOri, int IhOri, CPrmPara *extractPara, CMkxSeqPara seqPara);

  void  MkxInitAdaptiveParams(int* twoRoisFlag, CMkxRoiCoord *roi,
    int* AdaptiveWorkNbLblBlobsKept, int* AdaptiveWorkNbPixelsKept, int* AdaptiveWorkMarkersDist, float* AdaptiveDistTolerMin, float* AdaptiveDistTolerMax,
    CMkxRoiCoord UserInitRoiCoord, CMkxRoiCoord UserRoiCoord, int EstimatedMaxNbPixPerBlob,
    int NotYetLockedFlag, int SubMarkersDist, int CplSelectDistToler, CPrmPara *extractPara
    );
  void PrintProcessPara(CMkxProcessPara *pp, CPrmPara *extractPara);

  void  MkxDistToler(
    float *AdaptiveDistTolerMin, float *AdaptiveDistTolerMax,
    int dist,
    int DistToler
    );
  CMkxSeqPara DefaultSeqPara(int MaxImaDim);
  int MkxFreeProcessImages(CMkxExtractProc *ExtractProc);
  int MkxAllocProcessImages(CMkxExtractProc *ExtractProc, int Iw, int Ih);
  int MkxAllocProcessImages_SW(CMkxExtractProc *ExtractProc, int Iw, int Ih);

  //MkxPolarize.cpp
void MkxPolarize(CMkxCoupleInt* polarized, CMkxCoupleInt cur, CMkxCoupleFloat prev);

  //MkxMarkerDirection.cpp
  void MkxMarkerDirection(
    const short *Ori, int IwOri, int IhOri,
    int X1, int Y1, int X2, int Y2,
    float* dir1, float* dir2, float* dir1Confidence, float* dir2Confidence,
    float BlobEnhRingRadiusInPix, int SSFactor
    );

  //MkxAdapt.cpp
  int  MkxAdaptWhenLocked(int* AdaptiveWorkMarkersDist, int* AdaptiveWorkNbPixelsKept, int* AdaptiveWorkNbLblBlobsKept, CMkxRoiCoord* roi,/* CMkxRoiCoord* roi1, CMkxRoiCoord* roi2, */float* AdaptiveDistTolerMin, float* AdaptiveDistTolerMax,
    int EstimatedMaxNbPixPerBlob, int IwProc, int IhProc, CTr1Output TrOut, int CplSelectDistToler, CPrmPara *ExtractParam);
  int MkxHistoryRoi(
    CMkxRoiCoord *historyRoi,
    const CMkxHistory *history,
    int iwProc, int ihProc,
    CPrmPara   *ExtractParam
    );

  //MkxRefining.cpp
void  MkxRefining(
  CMkxMarkerCoupleRefined *mkcplRefined,
  CMkxCoupleInt cpl,
  const short* InIma, int IwOri, int IhOri,
  float BlobEnhRingRadiusInPix, int SSFactor
  );

  //MkxWire.cpp
  int MkxGetWire(void* WireHdl, short *InIma, int IwOri, int IhOri, short *ImSub, CMkxExtractResults *exr,
    int SSFactor, int IwProc, int IhProc,
    CPrmPara *ExtractParam);

  //MkxBxMarkerCoupleWireRescore.cpp
  int MkxMarkerCoupleWireRescore(CMkxBlobCouple *RawCouples, int NbRawCouples, short *Ima, const int Iw, const int Ih,
    short *Ori, const int IwOri, const int IhOri, void *ExtraMeasuresWireHdl, int MaxNbCouples, float PruningFactor);

  // MkxTracking.cpp */
  void MkxSetTrackingInput(CTr1Input *TrIn, int NbTrInCouples, const CMkxBlobCouple*  RawCouples, CMkxClue Clue);
  void MkxGetTrackingOutput(const CTr1Output *TrOut, int* NbTrOutCouples,  CMkxBlobCouple* TrCouples, int* Status , int* Locked );

  void MkxUpdateTrackingNonCriticalParam(CTr1Hdl TrackingHdl, CMkxProcessPara *processPara, CPrmPara* ExtractParam);
  void MkxSetTrackingParam(CTr1Param* TrParam, CPrmPara *ExtractParMg, CPrmParaCtx* ParaContext, int IwProc, int IhProc, CMkxProcessPara *processPara);

  // MkxUtil.cpp 
  int  MkxClipRoi(CMkxRoiCoord *DstRoi, CMkxRoiCoord SrcRoi, CMkxRoiCoord ClippingRoi);
  void SetExtractWorkRoi(int Margin, CMkxRoiCoord UsefulRoiCoord, CMkxRoiCoord *WorkRoiCoord);
  int  MkxSetRoiCoord(CErrHdl ErrHdl, float xMinFac, float xMaxFac, float yMinFac, float yMaxFac, CMkxRoiCoordFac* RoiCoordFac);
  int MkxRoiCoord(CMkxRoiCoord *dstRoi, int xMin, int yMin, int xMax, int yMax);
  int MkxRoiCoordInit(CMkxRoiCoord *dstRoi);

  //MkxSysTime.cpp
  int  MkxSysTimeMs(char *Name, int verbose);
  int  MkxSysTimeMsAndReset(char *Name, int verbose);

  //MkxSubSampleImage.cpp
  void MkxSubSampleImage(short *Ori, int IwOri, short *Sub, CMkxVs *Pel, int IwSub, int IhSub, int SSFactor, CMkxRoiCoord roiCoord, int noSSE);
  void MkxUpSampleCoordinates(const CMkxVs *ImPel, int IwPel, int *X, int *Y);

  //MkxWire.c
  void MkxWireInit(void* WireHdl, BxWireParameters *WireParams, int DynOri, int SSFactor, CPrmPara *ExtractParam);

  //MkxStat.cpp
  //void MkxStatWeightedLinearFit(int n, float *x, float *y, float *w, float *aw, float *bw, float *meanw);

  //MkxDebug.cpp
  void SPrintCplMerit(char* str, CMkxBlobCouple cpl, int titleFlag);
  void SPrintFeatPredict(char* str, CMkxPredict historyPredict, int titleFlag);
void MkxMarkerCoupleFloat(CMkxCoupleFloat* cpl, CMkxCoupleInt mkcpl, CMkxMarkerCoupleRefined mkcplRefined, int Method);

  /***************************************************************************/
  static inline void MkxBlobInit(CMkxBlob *blob)
  {
    blob->NbPix = 0;
    blob->V.X = 0;
    blob->V.Y = 0;
    blob->Intensity = (float)0;
  }
  /***************************************************************************/
  static inline void MkxBlobCopy(const CMkxBlob *src, CMkxBlob *dst)
  {
    dst->NbPix = src->NbPix;
    dst->V.X = src->V.X;
    dst->V.Y = src->V.Y;
    dst->Intensity = src->Intensity;
  }
  /***************************************************************************/
  static inline void MkxCplMeritInit(CMkxCplMerit *cplMerit)
  {
    cplMerit->CplMeritOverall = -1.f;
    cplMerit->CplMeritWrtBlobs = -1.f;
    cplMerit->CplMeritWrtWire = -1.f;
    cplMerit->CplMeritWrtWireAvailable = 0;
    cplMerit->CplMeritWrtWire_markers_correlation = -1.f;
    cplMerit->CplMeritWrtWire_wire_correlation = -1.f;
    cplMerit->CplMeritWrtClues = -1.f;
    cplMerit->CplMeritWrtCluesAvailable = 0;
    cplMerit->CplMeritWrtCluesCx = -1.f;
    cplMerit->CplMeritWrtCluesCy = -1.f;
    cplMerit->CplMeritWrtCluesBreadth = -1.f;
    cplMerit->CplMeritWrtCluesAng = -1.f;
    cplMerit->CplMeritWrtHistory = -1.f;
    cplMerit->CplMeritWrtHistoryAvailable = 0;
    cplMerit->CplMeritWrtHistoryCx = -1.f;
    cplMerit->CplMeritWrtHistoryCy = -1.f;
    cplMerit->CplMeritWrtHistoryBreadth = -1.f;
    cplMerit->CplMeritWrtHistoryAng = -1.f;
  }
  static inline void MkxCplFeaturesInit(CMkxCplFeatures *cf)
  {
    int i;
    for (i = 0; i < MKX_CPL_N_FEATURES; i++)
    {
      cf->Feat[i] = -1;
      cf->Feat_available[i] = 0;
    }
  }
  /***************************************************************************/
  static inline void MkxBlobCoupleInit(CMkxBlobCouple *markerCouple)
  {
#if 1
    static CMkxBlobCouple emptyCouple;
    static int first = 1;
    if (first)
    {
      MkxBlobInit(&(emptyCouple.Blob1));
      MkxBlobInit(&(emptyCouple.Blob2));
      emptyCouple.Rank = 0;
      emptyCouple.MatchingRef = -2;
      MkxCplFeaturesInit(&(emptyCouple.CplFeat));
      MkxCplMeritInit(&(emptyCouple.CplMerit));
    }
    memcpy(markerCouple, &emptyCouple, sizeof(emptyCouple));
#else
    MkxBlobInit(&(markerCouple->Blob1));
    MkxBlobInit(&(markerCouple->Blob2));
    markerCouple->Rank        =  0;
    markerCouple->MatchingRef = -2;
    MkxCplFeaturesInit(&(markerCouple->CplFeat));
    MkxCplMeritInit(&(markerCouple->CplMerit));
#endif
  }
  /***************************************************************************/
  static inline void MkxBlobCoupleCopy(CMkxBlobCouple *MarkerCouple1, CMkxBlobCouple *MarkerCouple2)
  {
    MkxBlobCopy(&(MarkerCouple1->Blob1), &(MarkerCouple2->Blob1));
    MkxBlobCopy(&(MarkerCouple1->Blob2), &(MarkerCouple2->Blob2));

    MarkerCouple2->Rank = MarkerCouple1->Rank;
    MarkerCouple2->CplMerit = MarkerCouple1->CplMerit;
  }
  static inline void MkxHyStatInit(CMkxHyStat *stat)
  {
    stat->N = -1;
    stat->Max = -1;
    stat->Min = -1;
    stat->Mean = -1;
    stat->Median = -1;
    stat->Stdv = -1;
    stat->Aw = -1;
    stat->Bw = -1;
  }
  static inline void MkxHistoryStatInit(CMkxHistoryStat* hs)
  {
    int i;

    hs->NbUsefulPlanes = -1;
    hs->ModulationFactor = 0;

    for (i = 0; i < MKX_CPL_N_FEATURES; i++)
    {
      MkxHyStatInit(&hs->FeatStat[i]);
      MkxHyStatInit(&hs->DevStat[i]);
    }
  }
  static inline void MkxFeatPredictInit(CMkxFeatPredict* fp)
  {
    fp->PredictVal_available = 0;
    fp->PredictVal = -1;
    fp->FuzzyLow = -1;
    fp->FuzzyHigh = -1;
  }
  static inline void MkxPredictInit(CMkxPredict* p)
  {
    int i;

    for (i = 0; i < MKX_CPL_N_FEATURES; i++)
    {
      MkxFeatPredictInit(&p->FeatPredict[i]);
      p->FeatPredict_available[i] = 0;
      p->PredictValid = 0;
    }
  }
  static inline void MkxHistoryPlaneInit(CMkxHistoryPlane* hp)
  {
    hp->Time = -1;
    hp->PlaneValid = 0;
    hp->PlaneUseful = 0;
    hp->X1 = -1;
    hp->Y1 = -1;
    hp->X2 = -1;
    hp->Y2 = -1;
    hp->Cx = -1;
    hp->Cy = -1;
    hp->Breadth = -1;
    hp->Ang = -1;
    hp->HistoryStatus = -1;
    hp->Locked = -1;
    hp->a1 = -1;
    hp->b1 = -1;
    hp->a2 = -1;
    hp->b2 = -1;

    MkxCplFeaturesInit(&hp->CplFeat);
    MkxPredictInit(&hp->HistoryPredict);
    MkxHistoryStatInit(&hp->HistoryStatShallow);
  }

#ifdef __cplusplus
}
#endif

#endif   //_MKXDEV_H






