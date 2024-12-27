// ***************************************************************************
// * Copyright (c) 2002-2016 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************

#ifndef		_MKX_H
#define		_MKX_H

#include <ErrLib.h>
#include <PrmLib.h>
#include <BxCurve.h>
#include <MkxParaCodes.h>
#include <MkxErrCodes.h>
#include <IThreadPool.h>

//===================================================================================================
// VERSION NUMBERS
#define MKX_VERSION_MAJOR    8
#define MKX_VERSION_MINOR    7
#define MKX_VERSION_PATCH    0

//===================================================================================================
// CONSTANTS

#define MKX_CST_BORDER       4

//default sequence dependent para settings
#define MKX_CST_MmPerPixel_Def(MaxImaDim) (0.11f*(1024.f/(float)MaxImaDim))
#define MKX_CST_Fps_Def 15
#define MKX_CST_MarkersDist_Def 30
#define MKX_CST_MarkersDistMin_Def 5
#define MKX_CST_MarkersDistMax_Def 45
#define MKX_CST_DefRoiXMin_Def 0
#define MKX_CST_DefRoiXMax_Def 1
#define MKX_CST_DefRoiYMin_Def 0
#define MKX_CST_DefRoiYMax_Def 1
#define MKX_CST_InitRoiXMin_Def 0
#define MKX_CST_InitRoiXMax_Def 1
#define MKX_CST_InitRoiYMin_Def 0
#define MKX_CST_InitRoiYMax_Def 1


// Mkx functions' argument ranges
#define	MKX_CST_MIN_IMAWIDTH  64
#define	MKX_CST_MIN_IMAHEIGHT 64
#define	MKX_CST_MIN_IMADYN    4
#define	MKX_CST_MAX_IMADYN    15
#define	MKX_CST_MIN_FPS       7
#define	MKX_CST_MAX_FPS       30

// Number of characters for parameter AKA (Also.Known.As) and TEXT (info)
#define MKX_CST_PARA_AKA_LENGTH 64
#define MKX_CST_PARA_TXT_LENGTH 256

//===================================================================================================
// TYPEDEF

// Interpolation methods (used for "manual" warping)
typedef enum {
  MKX_CST_NEAREST = 0,
  MKX_CST_BILINEAR
} CMkxInterMethod;

// Client-provided clue's nature values
typedef enum {
  MKX_CST_CLUE_NOTAVAILABLE = 0,   // No clue provided by the client application
  MKX_CST_CLUE_DEBUG,              // Clues provided for debug purposes only
  MKX_CST_CLUE_STRONG,             // Strong clues -> change some parameters and processing
} CMkxClueNature;

// Track status result returned to the client
typedef enum {
  MKX_CST_EXTRACT_RES_OK = 0,  // Best track Ok
  MKX_CST_EXTRACT_RES_SKIP     // Best track's head is virtual (no matching alarm)
} CMkxExtractionStatus;

// Track Lock result returned to the client
typedef enum {
  MKX_CST_EXTRACT_RES_UNLOCKED = 0,// Best track unlocked
  MKX_CST_EXTRACT_RES_LOCKED       // Best track locked
} CMkxLock;

// Sternal Wire Detection result returned to the client
typedef enum {
  MKX_CST_EXTRACT_RES_STERNALWIRE_NOTAVAILABLE = 0,// no sternal wire removal process
  MKX_CST_EXTRACT_RES_STERNALWIRE_INIT_PHASE,      // sternal wire initialisation phase
  MKX_CST_EXTRACT_RES_STERNALWIRE_NOT_DETECTED,    // no sternal wire detection
  MKX_CST_EXTRACT_RES_STERNALWIRE_DETECTED         // sternal wires were detected
} CMkxSwStatus;

// Wire status: availability returned to the client
typedef enum {
  MKX_CST_WIRE_NOT_AVAILABLE = 0,		// No wire-info in extract results
  MKX_CST_WIRE_AVAILABLE,			      // Wire-info present in extract results
} CMkxWireStatus;

// Parameter nature (Used to define in parameter Context if a param can be changed after Init/ReInit (t=0))
typedef enum  {
  MKX_CST_PARA_NORMAL = 0,   // Can be changed at any time
  MKX_CST_PARA_CRITICAL      // Can be changed only before t=0
} CMkxParamNature;

// Roi mode (See InitRoiFlag parameter)
enum {
  MKX_CST_EXTRACT_ROI_NODEF = 0, // No Default ROI (full image) 
  MKX_CST_EXTRACT_ROI_DEF        // Default ROI
};

// Adaptive mode (see adatation parameters such as AdaptiveRoiFlag, AdaptiveNbLabelKeptFlag, AdaptiveMarkerDistFlag, AdaptiveWireCplSelectFlag)
enum {
  MKX_CST_NOADAPT = 0,          // Not adaptive
  MKX_CST_ADAPT,                // adaptive
  MKX_CST_ADAPT_2ROIS           // adaptive with 2 Rois
};

// Subsampling modes (see BlobEnhSubSamplingMode parameter)
enum {
  MKX_CST_SUBSAMPLING_AUTO = 0,     // subsampling automatic
  MKX_CST_SUBSAMPLING_NONE,         // subsampling 1
  MKX_CST_SUBSAMPLING_2,            // subsampling 2 
  MKX_CST_SUBSAMPLING_4             // subsampling 4
};

// SkipFromConfidence modes (see SkipFromConfidenceMode parameter)
typedef enum {
  MKX_CST_SKIPFROMCONFIDENCE_NOSKIPPING = 0,          // no skipping done by consolidate
  MKX_CST_SKIPFROMCONFIDENCE_NBBESTBASEDSKIPPING,      // keep nb images with best confidence 
  MKX_CST_SKIPFROMCONFIDENCE_PERCENTBESTBASEDSKIPPING,     // keep % images with best confidence 
  MKX_CST_SKIPFROMCONFIDENCE_THRESHOLDBASEDSKIPPING,  // skipping images with confidence below threshold
} CMkxSkipFromConfidenceMode;

// History modes (see HistoryFlag parameter)
enum {
  MKX_CST_HISTORY_NOTUSED = 0,         // not used
  MKX_CST_HISTORY_UPDATEONLY,          // update
  MKX_CST_HISTORY_APPLYONLY,           // apply but no update 
  MKX_CST_HISTORY_APPLYANDUPDATE       // apply and update 
};

// polarity
enum{
  MKX_CST_DIRECT = 0,
  MKX_CST_REVERSE
};

//===================================================================================================
// MACROS

// Macro returning the necesseary margin for marker extraction 
// (The extraction ROI should not occupy this margin)
#define MKX_GET_X_MARGIN(Radius)    ((Radius)+MKX_CST_BORDER+1)

//===================================================================================================
// STRUCTURES

typedef struct
{
  CErrHdl ErrHdl;   // Access to instance of ErrLib used to manage lib error
  CPrmHdl PrmHdl;   // Parameter-library handle associated to library
  void* Pv;         // Access to private material
}
CMkxHdl;


// Client's provided clue
typedef struct
{
  float   X1f, Y1f;               // First marker's coordinates provided by the client
  float   X2f, Y2f;               // Second marker's coordinates provided by the client
  CMkxClueNature  Nature;       // Nature of the clue (no clue, strong clue, for debug...)
}
CMkxClue;

typedef struct
{
  float MmPerPixel;                                                      // Number of mm per pixel in the patient observed on the detector
  float Fps;                                                             // Get the Frame-Per-Second value (used for max inter-frame displacement estimation)
  float MarkersDistMinMm, MarkersDistMaxMm;                              // Get Min/Max inter-marker distance in mm
  float DefRoiFacXMin, DefRoiFacXMax, DefRoiFacYMin, DefRoiFacYMax;      // Normalized Default ROI coord ([0,1])
  float InitRoiFacXMin, InitRoiFacXMax, InitRoiFacYMin, InitRoiFacYMax;  // Normalized Init ROI coord ([0,1])
}
CMkxSeqPara;

typedef struct
{
  CMkxSkipFromConfidenceMode Mode; // Mode for skipping less confident images (0: no skipping, 1: keep nb best images, 2: keep % best images, 3: w.r.t threshold)
  int NbBest;                      // Nb best images associated to SkipFromConfidenceMode = 1 (default=16)
  float PercentBest;               // % best images associated to SkipFromConfidenceMode = 2 (default=50)
  float Threshold;                 // Threshold associated to SkipFromConfidenceMode = 3      (0 to 1 default=0)
  int MinNbImages;                 // Min number of images to be kept when skipping by SkipFromConfidence (is applied for PercentBest and Threshold modes) (default=10)
}
CMkxSkipFromConfidencePara;

// Advanced extraction results: details on markers and temporal-tracker
typedef struct{
  float     Dir1, Dir2;                                  // Marker 1 and 2 directions (in degrees)
  float     Dir1Confidence, Dir2Confidence;              // Marker 1 and 2 confidence on directions (0 is the best)
  float     Length1, Thickness1, Length2, Thickness2;    // Marker 1 and 2 length and thickness
  float     Intens1, Intens2;                            // Marker 1 and 2 blobiness sum 
  float     LevelIn1, LevelIn2;                          // Average grey level inside markers
  float     LevelOut1, LevelOut2;                        // Average grey level close outside markers
  float     RefiningConf1, RefiningConf2;                // Confidence index on refinement process for each marker
  float     Radius1, Radius2;                            // Estimation of marker radii
  float     Cx, Cy, Br, An;                              // Marker couple (centroid, breadth, angle)
}
CMkxAdvancedResults;


// Extraction results
typedef struct{
  float                 X1Refined, Y1Refined, X2Refined, Y2Refined;  // Marker 1 and 2 coordinates (final measures -possibly refined)
  CMkxExtractionStatus  Status;                                      // Extraction status (skip / normal) (final status, possibly after MkxGetSkipFromConfidence)
  CMkxLock              Locked;                                      // Locked flag (Unlocked at init (several frames) or when lost of confience (larger ROI used)
  int                   RoiXMin, RoiYMin, RoiXMax, RoiYMax;          // Roi computed from extraction results 
  int                   RoiXMin1, RoiYMin1, RoiXMax1, RoiYMax1;      // Roi computed from extraction results 
  int                   RoiXMin2, RoiYMin2, RoiXMax2, RoiYMax2;      // Roi computed from extraction results 
  int                   TwoRoisFlag;                                 // if 1 uses RoiXMin1 and 2 etc.  else uses RoiXMin etc.
  BxCurve               Wire;                                        // Wire extracted from image and markers
  CMkxWireStatus        WireStatus;                                  // wire available or not 
  float                 Confidence;                                  // Confidence level in [0,1] (1 = highest confidence): filled by MkxConsolidate
  CMkxAdvancedResults   Advanced;                                    // Advanced results on markers and temporal tracking
  int                   IwProc, IhProc;                              // Image size used for processing (after potential subsampling) 
  int                   IwProc_SW, IhProc_SW;                        // Image size used for sternal wires processing (after potential subsampling) 
  CMkxSwStatus          SwStatus;                                    // Sternal wire detection status

  void*                 _Volatile;                                   // Used for debug/analysis/timing purposes

}CMkxExtractResults;




//===================================================================================================
// METHODS

#ifdef __cplusplus
extern "C" {
#endif

  // REMARKS about memory allocation
  // - For MkxGetVersionInfo, a pointer to a char* must be passed WITHOUT allocation
  //   Do NOT attempt to free the returned pointer (allocation & free done by library)
  // - The input images are allocated by caller (library's client)
  // - All the fields in the result-structures are OUTPUT values only (not to be modified)

  // Class-level functions (no handle required)
  //////////////////// MkxGetVersionNumbers ///////////////////////////////////////////////////////////////
  void MkxGetVersionNumbers
    (
    int* Major,
    int* Minor,
    int* Patch
    );
  /*
  Description:
  Returns the Major, Minor and Patch numbers of the current library's version.

  Arguments:
  Major           : pointer on the Major version number
  Minor           : pointer on the Minor version number
  Patch           : pointer on the Patch version number

  Return value :
  void
  */

  // Life-time management
  //////////////////// MkxCreate ///////////////////////////////////////////////////////////////
  int MkxCreate
    (
    CMkxHdl*      PtMkxHdl,
    int           ImaDyn,
    int           ImaWidth,
    int           ImaHeight,
    IThreadPool*  Pool
    );
  /*
  Description:
  Creates an instance of CMkx and returns a handler on that instance. A CMkx instance gives access to the Mkx process. This process is defined in relation with an image format (image's size and dynamics). It also returns a handler for the error management (performed by the library ErrLib) and a handler for the parameters management (performed by the library PrmLib)

  Arguments:
  PtMkxHdl  : a pointer on the returned CMkx handler, the 2 fields PtMkxHdl-> ErrHdl and PtMkxHdl-> PrmHdl give access to error and parameter management
  ImaDyn    : image dynamics in bits. In [MKX_CST_MIN_IMADYN, MKX_CST_MAX_IMADYN]
  ImaWidth  : image width in pixels (? MKX_CST_MIN_IMAWIDTH )
  ImaHeight : image height in lines (? MKX_CST_MIN_IMAHEIGHT )
  Pool        : a pointer on a IThreadPool instance.

  Return value:
  0                       : no error
  MKX_ERR_IMADYN_RANGE    : ImaDyn argument out of range
  MKX_ERR_IMAWIDTH_RANGE  : ImaWidth argument out of range
  MKX_ERR_IMAHEIGHT_RANGE : ImaHeight argument out of range
  MKX_ERR_MEM_ALLOC       : memory allocation problem (not enough memory?)

  Remark:
  Even when MkxCreate returns an error, it is possible to analyse this error by the MkxLib error-related functions applied to the returned handler. The other MkxLib kinds of functions should not be applied on a handler returned by a failing MkxCreate call.
  */

  //////////////////// MkxDelete ///////////////////////////////////////////////////////////////
  void MkxDelete
    (
    CMkxHdl MkxHdl
    );
  /*
  Description:
  De-allocates the memory space devoted to a given CMkx instance. This releases all the content the object would bear. The handler on which MkxDelete was invoked should not be used anymore.

  Arguments:
  MkxHdl : handler of the targeted CMkx instance

  Return value:
  0 : no error
  */

  //////////////////// MkxGetVersionInfo ///////////////////////////////////////////////////////////////
  void MkxGetVersionInfo
    (
    CMkxHdl MkxHdl,
    char**  VersionInfo
    );
  /*
  Description:
  Allocates a character array and returns information pertaining to the current library's version into that array.

  Arguments:
  MkxHdl         : handler of the targeted CMkx instance
  VersionInfo    : pointer on the character array allocated by the function and containing the library version related information

  Return value :
  0            : no error
  */


  // Set SeqParams
  //////////////////// MkxSetSeqPara ///////////////////////////////////////////////////////////////
  int MkxSetSeqPara
    (
    CMkxHdl     hdl,
    CMkxSeqPara seqPara
    );                                            // Set the seq params
  /*
  Description:
  Sets the Sequence parameters of the current CMkx object with the content of structures allocated and filled by the client.

  Arguments:
  MkxHdl         : handler of the targeted CMkx instance

  SeqPara: pointer on a client's allocated and filled structure from which the current Sequence parameters will be set.

  Return value:
  0                         : no error
  */

  //////////////////// MkxGetDefaultSeqPara ///////////////////////////////////////////////////////////////
  int MkxGetDefaultSeqPara
    (
    CMkxHdl      hdl,
    CMkxSeqPara *defSeqPara
    );                                 // Get the default seq params
  /*
  Description:
  Returns a copy of the default Sequence parameters into client's allocated structure.

  Arguments:
  MkxHdl       : handler of the targeted CMkx instance

  SeqPara      : pointer on a client's allocated structure receiving the default Sequence parameters.

  Return value :
  0            : no error
  */

  //////////////////// MkxGetCurrentSeqPara ///////////////////////////////////////////////////////////////
  int MkxGetCurrentSeqPara
    (
    CMkxHdl       hdl,
    CMkxSeqPara*  curSeqPara
    );                                 // Get the current seq params
  /*
  Description:
  Returns a copy of the current Sequence parameters into client's allocated structures.

  Arguments:
  MkxHdl         : handler of the targeted CMkx instance

  SeqPara        : pointer on a client's allocated structure receiving the current Sequence parameters.

  Return value :
  0            : no error
  */

  // Processing for a single frame
  //////////////////// MkxExtract ///////////////////////////////////////////////////////////////
  int  MkxExtract
    (
    CMkxHdl                 MkxHdl,
    int                     t,
    short*                  InIma,
    CMkxExtractResults*     ExtractResults
    );
  /*
  Description:
  Performs the extraction process at time t on the image InIma, thus producing results in ExtractResults which contain the coordinates of the detected markers and other information.

  Arguments:
  MkxHdl         : handler of the targeted CMkx instance
  t              : current time of the extract process
  InIma          : a pointer on the input image

  ExtractResults : 	pointer on a CMkxExtractResults structure used to gather the extraction results and in particular the markers's coordinates together with the skip image facility and the locking flag

  Return value:
  0                      : no error
  MKX_WRN_INCORRECT_TIME : incorrect sequencing (t=0 or should increment)
  MKX_WRN_DARK_IMAGE     : InIma is a fully dark image
  MKX_ERR_CRITICAL_PARA  : critical parameter changed at time != 0

  Remark 1:
  The dimensions of InIma are defined when the current CMkx instance is created. The client should not pass an image array that does not comply with those dimensions.

  Remark 2:
  If MkxExtract is called with an incorrect temporal argument t, the extraction operation on the current image is simply aborted.

  Remark 3:
  Critical parameters cannot be changed without subsequently restarting the process (call with t=0). One can find whether a given parameter is critical or not by consulting MkxSetParamCtx.h (Nat field), or at runtime, through the analysis of the parameter context structure.

  Remark 4:
  When a fully dark image is input to MkxExtract no raw couple is output and a skip condition is produced.
  */

  //////////////////// MkxRefineMarkersAndGetBlobSize ///////////////////////////////////////////////////////////////
  int	 MkxRefineMarkersAndGetBlobSize
    (
    CMkxHdl              MkxHdl,
    short*               InIma,
    int                  x1,
    int                  y1, 
    int                  x2,
    int                  y2,
    float*               x1Refined,
    float*               y1Refined,
    float*               x2Refined,
    float*               y2Refined,
    CMkxAdvancedResults *advancedResults
    );
  /*
  Description:
  Given rough (integer) values for the markers coordinates, the function computes the markers coordinates with a subpixel accuracy and the size of the marker blobs (length and thickness).
  The process performed by this function is included in the MkxExtract function. This function is mainly useful when the markers have been manually edited.

  Arguments:
  MkxHdl         : handler of the targeted CMkx instance
  InIma          : double pointer on the input image
  X1, Y1, X2, Y2 : the rough (integer) values of the markers coordinates

  X1Refined, Y1Refined, X2Refined, Y2Refined: the refined values of the markers coordinates

  AdvancedResults: a pointer on a CMkxAdvancedResults structure.

  Return value:
  0                      : no error
  */

  //////////////////// Utilitary and legacy methods ///////////////////////////////////////////////////////////////

  // Utilitary
  void MkxExtractResultsInit
    (
    CMkxExtractResults *ExtractResults
    );                                 // Init a CMkxExtractResults instance (typically used with MkxRefineMarkersAndGetBlobSize)

  // Clue passing
  void MkxSetClue
    (
    CMkxHdl   MkxHdl,
    CMkxClue *Clue
    );

  // Processing for a whole sequence
  int  MkxConsolidate
    (
    CMkxHdl             MkxHdl,
    int                 NbIma,
    CMkxExtractResults *InOutTab
    );                  // Fill the confidence field, update marker coordinates (polarize) and compute corresponding Cx, Cy, Br, An coordinates for all frames of a sequence
  int  MkxGetSkipFromConfidence
    (
    CMkxHdl                    MkxHdl,
    CMkxSkipFromConfidencePara SkipFromConfPara,
    int                        NbIma,
    CMkxExtractResults        *InOutTab
    );                                                                                             // Update the status field for all frames of a sequence, according to the confidence and SkipFromConfidence parameters
  int  MkxGetDefaultSkipFromConfidencePara
    (
    CMkxSkipFromConfidencePara *defSkipFromConfPara
    );       // Get the default seq params

  //Debug
  void MkxExtractResultsPrint
    (
    CMkxExtractResults *ExtractResults
    );


#ifdef __cplusplus
};
#endif

#endif   //_MKX_H
