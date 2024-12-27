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


//
// Library developer-level umbrella include 
//
#ifndef		_OSCDEV_H
#define		_OSCDEV_H

#include <Osc.h>

#include "string.h"
#include <omp.h>
//#include "vld.h"

#define OSC_USE_MULTITHREADING

#ifdef DISPLAY_INT
	#define MX_USE_DLL_AS_STATIC 1
	#include "MxLab.h"
	#define OSC_MX_LAB
#endif

#define OSC_USE_SSE_RDG

#define OSC_VERSION_WRITE_HDL_TO_DISK 1


//===================================================================================================
// MACROS
static __forceinline int OSC_ABS(int a) { return (((a) >= 0)   ? (a): (-(a))); }

#define OSC_SQ(a)            ((a)*(a))
#define OSC_SIGN(a)          (((a) >= 0)   ? (1): (-(1)))
#define OSC_ABS_F(a)		 (float)fabs(a)
#define OSC_MIN(a,b)         (((a) <= (b)) ? (a):   (b) )
#define OSC_MAX(a,b)         (((a) >= (b)) ? (a):   (b) )
#define OSC_CLIP(a,b,c)      OSC_MIN(OSC_MAX((a), (b)), (c))
#define OSC_CLIP_SHORT(a)    ((short)OSC_CLIP(-32768, (a), 32767))
#define OSC_RND(a)           (((a) >= 0.0) ? (int)((a)+0.5) : (int)((a)-0.5))
#define OSC_DIVCEIL(a, b)    (((a) + (b)-1) / (b))



#define	OSC_CST_MIN_IMADYN		        4
#define	OSC_CST_MAX_IMADYN		        16
#define OSC_CST_PARA_VEC_LENGTH         4   // param vectors have 4 components
#define OSC_MIN_PIXSIZE_EACHDIMENSION   64  // in pixels
#define OSC_RIDGEGAMMAANGIO     1.f
#define OSC_ANGIO_KERNEL0       .52f
#define OSC_ANGIO_KERNEL1       1.04f
#define OSC_ANGIO_KERNEL2       1.56f
#define OSC_RIDGEGAMMAFLUORO    1.f
#define OSC_FLUORO_KERNEL0      .39f
#define OSC_FLUORO_KERNEL1      1.56f
#define OSC_NB_PYR              3
#define OSC_ANGIO_MAXRV         2
#define OSC_FLUORO_MAXRV_X      2
#define OSC_FLUORO_MAXRV_Y      4

#define OSC_DEFAULT_SSFACTOR			2
#define OSC_DEFAULT_NBPROC				3
#define OSC_MIN_CARDIACFREQ				45				// In beats per minute
#define OSC_MAX_CARDIACFREQ				135				// In beats per minute

#define OSC_PI							3.14159265358979323846
#define OSC_EPSILON						0.000001f

#define OSC_RDG_BORDER_MIN_ABS			2				// Absolute margin for rdg computation
#define OSC_RDG_BORDER_MIN_FACTOR		2.f				// Relative margin for rdg computation
#define OSC_RDG_LAMBDA					0.33f			// Lambda for ridges computation

#define OSC_DIA_DELETION_ANG_THRESHOLD 0.78f			// Pi/4 - Do not zero ridges oriented differently from the diaphragm
#define OSC_DIAPHRAGM_THICKNESS_IM_RATIO 0.0391f		// Corresponds to 40 pixel on a 1024x1024 image

#define OSC_CBD_FREQUENCY_REDETECTION_CBD_ANGIO_S 0.5f

#define OSC_SW_EXTRACTION_DURATION_S    0.33f
#define OSC_SW_SIZE_DIL					1
#define OSC_SW_MV_REINIT_SEARCH_RANGE_X_MM	1.f				// Search range in X for SW tracking reinitialization (first fluoro)
#define OSC_SW_MV_REINIT_SEARCH_RANGE_Y_MM	3.f				// Search range in Y for SW tracking reinitialization (first fluoro)

#define OSC_FREEZE_THRESHOLDS_AT		5				// In frames
#define OSC_BKGANGIO_DURATION_BEFORE_MOTION	1			// In seconds
#define OSC_BKGANGIO_NB_BLOCKS			4				// Nb of blocks (in each direction) of the block-Matching compensation applied to the background in angio
#define	OSC_BKGANGIO_BM_AMPLITUDE		4				// Amplitude of the Block-Matching of the warping of the background in angio
#define OSC_INTEGRATION_TIME_CIRC_BKG	1.f				// In seconds

#define OSC_ANGIO_THRPERCENT_H			0.01f
#define OSC_ANGIO_THRPERCENT_L			0.04f
#define OSC_ANGIO_NBRANGEPIX_H			3				// In pixels
#define OSC_ANGIO_MAXDIRDIFF_H			10				// In degrees
#define OSC_ANGIO_NBMINPIX_H			200				// In pixels
#define OSC_ANGIO_NBRANGEPIX_L			2				// In pixels
#define OSC_CUMRDG_THRES_RATIO			0.1f//0.1f
#define OSC_CUMRDG_PEN_SURF				600				// In pixels, full res
#define OSC_MAXLABELSNB					1000//10

// Discard cycles with flashs (no proximal boost)
#define OSC_THRES_FLASH_LOW                                     0.35f
#define OSC_THRES_FLASH_HIGH                                    0.35f

// Clean parameters for threshold adaptation
#define OSC_CLEAN_RANGE_LABELING		                        3	// In pixels
#define OSC_CLEAN_LIM_DIR				                        10	// In degrees
#define OSC_CLEAN_MAXFILSIZE			                        5	// In pixels
#define OSC_CLEAN_INICOEFFTHRES			                        .666667f
#define OSC_CLEAN_COEFFTHRESSTEP		                        .05f
#define OSC_CLEAN_THRSIZEADDED			                        .05f
#define OSC_CLEAN_COEFFSTDFORTHRES_SEQ                          1.f
#define OSC_CLEAN_IMAGE_WISE_CAUSAL_THRESHOLD_CHECK_DURATION    0.5f    // Temporal adaptation
#define OSC_CLEAN_MAXIMAGES_FORANGIOSTORAGE                     1000

// Clean parameters for clustering
//#define OSC_CLEAN_DEBUG

#define OSC_CLEAN_NBREGIONS_BYLENGTH                            10
#define OSC_CLEAN_DISTFORCLUSTERING_MM                          3.f
#define OSC_CLEAN_MARGIN_CONTACT_BORDER_PIX                     2
#define OSC_CLEAN_MAXDISTONSAMEBORDER_MM                        40
#define OSC_CLEAN_MINDISTTOINCLUDEBRANCHWITHBORDERCONTACT_MM    15

// Clean parameters temporal consolidation
#define OSC_CLEAN_DILSIZE_TEMPCONS_MM                           5
#define OSC_CLEAN_REFERENCE_RATIO_COVERED                       0.75f
#define OSC_CLEAN_RATIO_COVERED_TO_REMOVE                       0.5f

// Heart cycle choice with proximal boost
#define OSC_SELECT_HEARTCYCLE_WITH_PROXIMAL_BOOST               // Cycle selection methods
#define OSC_HCSELECT_COEFF_UPTAKE                   0.33f       // Coeff to identify inflation begin wrt max CA presence
#define OSC_HCSELECT_HALFSIZE_MASK_DILATATION_MM    6.5f        // Size of the inflow mask dilatation (to boost prox indicators)
#define OSC_HCSELECT_COEFF_BOOST_PROX               2.f         // Boosting coefficient

// Unsufficient angio injection detection
#define OSC_MEANRDGCYCLE_THRES_NOINJECTION			320 // MM2      0.030518f // 2000 on 256x256 (after SSFactor)
#define OSC_MEANRDGCYCLE_THRES_UNSUFFIENTINJECTION	468 // MM2      0.04576f // 3000 on 256x256 (after SSFactor)

#define OSC_TABULATE_SINCOS_TABSIZE		1024

#define OSC_VITERBI_FLUORO_DEPTH		2

#define OSC_MC_COEFF_FOR_PANN_CORR		0.33f				// Coefficient applied to enlarge Y search range in case of lage panning

#define OSC_VITERBI_OBSERVATION_SCORE			1.f				// Max observation score (Viterbi)
#define OSC_VITERBI_TEMPORAL_TRANSITION_SCORE	1.f				// Max temporal transition score (Viterbi)
#define OSC_VITERBI_SPATIAL_TRANSITION_SCORE	1.f				// Max temporal spatial score (Viterbi)
#define OSC_VITERBI_SPATIAL_LIMIT1				1.f				// Spatial Viterbi Coefficient Limit 1 (to compute the spatial constraint)
#define OSC_VITERBI_SPATIAL_VAL1				0.95f			// Spatial Viterbi Coefficient Value 1 (to compute the spatial constraint)
#define OSC_VITERBI_SPATIAL_LIMIT2				1.5f			// Spatial Viterbi Coefficient Limit 2 (to compute the spatial constraint)
#define OSC_VITERBI_SPATIAL_VAL2				0.66f			// Spatial Viterbi Coefficient Value 2 (to compute the spatial constraint)
#define OSC_VITERBI_SPATIAL_LIMIT3				1.75f			// Spatial Viterbi Coefficient Limit 3 (to compute the spatial constraint)

#define OSC_MOTION_ANALYSIS_DEPTH				10				// Nb of frames
#define OSC_MOTION_ANALYSIS_NB_FOR_ADAPT		5				// Nb of frames
#define OSC_MOTION_ANALYSIS_MARGIN				16				// In pixels at full resolution
#define OSC_MOTION_ANALYSIS_UPDATE_STEP			8				// In pixels at full resolution

// Catheter injection ROI detection
#define OSC_CATHINJ_MEDIANSIZE_INJINDFILTERING 5                // Size of the temporal median to filter injection indicator (cumulated ridges)
#define OSC_CATHINJ_SLOPERATIOLIMIT 0.1                         // Limit on the slope to set the injection begin and end. Starting from the max slope, we progress and stop when the slope is smaller than slopeRatioLimit times the max slope
#define OSC_CATHINJ_NBJOKERS_SEARCHINGFORINJBEGIN 2             // Number of successive points not satisfying the condition that can be met and ignored.

#define OSC_CATHINJ_DELTAFROMINJDETECTION 0                     // Non-injected frames used to compute candidates ROI are taken OSC_CATHINJ_DELTAFROMINJDETECTION before the detected injection time (>0-->robustness to injection estimation. But some cathinj may be too faint before the CA arrives)
#define OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION 6            // Number of considered non injected frames to define candidate ROIs

#define OSC_CATHINJ_MARGINTOINCLUDELAB 25                       // Margin (in pixel) to consider a label close to the border (shutter)
#define OSC_CATHINJ_MARGINTOEXTENDTOOTHERLAB 10                 // Distance (in pixels) to merge one label to an existing ROI
#define OSC_CATHINJ_RATIOCUMRIDGECLEANING 0.3f                  // Regions with cumulated ridge < OSC_CATHINJ_RATIOCUMRIDGECLEANING x maxCumRdg are not considered

#define OSC_CATHINJ_MAXNBOFCANDIDATEROI 100                     // Max number of candidate ROI (allocation limit)
#define OSC_CATHINJ_MEANSHIFT_NBINIT 100                        // Nb of random init tried in the clustering process
#define OSC_CATHINJ_MEANSHIFT_NBITERATIONOPTIM 20               // Nb of optimization iterations performed from each initialization

#define OSC_CATHINJ_MAXRATIODIMROIVSSHUT 0.6f                   // Max dimension of representative ROI wrt to imaging ROI
#define OSC_CATHINJ_MINRATIOCOVERAGE 0.35f                      // Minimum coverage (sum surface ROIs in the cluster)/surface tested representative ROI, to consider the representative ROI validated
#define OSC_CATHINJ_MAXMISSED 0.3f                              // Maximum surface ROI in the cluster missed (relative to the total surface f the ROIs in the cluster)
#define OSC_WEIGHT_COVERAGEVSMISSED 1.f                         // Secondary score to optimize is coverage - OSC_WEIGHT_COVERAGEVSMISSED x missed
#define OSC_CATHINJ_MAXNBREGIONINLAB 15                         // Maximum number of ROIs in the cluster. If there are more, the OSC_CATHINJ_MAXNBREGIONINLAB largest are kept.

#define OSC_CATHINJROI_MEDIANSIZE_CUMRDG 5                      // Size of the median imposed to the injection indicator
#define OSC_CATHINJ_THRESHOLDINJSURE 2.5f                       // Threshold to be sure the injection has began
#define OSC_CATHINJ_RATIOMAXTHRESHOLDINJSURE 0.25f              // Completed with the ratio of the max observed indicator. ie, threshold = max(OSC_CATHINJ_THRESHOLDINJSURE, OSC_CATHINJ_RATIOMAXTHRESHOLDINJSURE x maxIndicator)
#define OSC_CATHINJ_THRESHOLDINJBEGIN 1.f                       // Injection considered finished if under that value
#define OSC_CATHINJ_RATIOSLOPEINTAKE 0.25f                      // AND the local indicator slope is under OSC_CATHINJ_RATIOSLOPEINTAKE x max Slope
#define OSC_CATHINJ_TOLERANCEFORFIRSTINTAKE 2                   // ROIs whose injection has been detected at the same time up to OSC_CATHINJ_TOLERANCEFORFIRSTINTAKE frames are considered simultaneaous
#define OSC_CATHINJ_NBFRAMESTOSTUDYAFTERINI 6                   // Number of instants considered after the first detected injeciton (on the different indicators) to compute the score of each ROI
#define OSC_CATHINJ_FACTORFORJUMPINTAKE 2                       // If the initial jump of a ROI is OSC_CATHINJ_FACTORFORJUMPINTAKE x the max slope on the first injected ROI, it is considered an initial slope - and attributed a penalty
#define OSC_CATHINJ_RATIOFORJUMPAFTERINTAKE 0.7                 // If the injection indicator drops by this factor (after intake), it is considered a jump after intake
#define OSC_CATHINJ_DELTARDGFOREQUIVALENTSCORE 2                // At any time instant, the ROIs having an indicator close to the max indicator at that time (up to OSC_CATHINJ_DELTARDGFOREQUIVALENTSCORE) receive a bonus
#define OSC_CATHINJ_PTSFORFIRSTINTAKE 3                         // Score for being the first ROI to have an intake (up to OSC_CATHINJ_TOLERANCEFORFIRSTINTAKE)
#define OSC_CATHINJ_PTSFORMOSTINJATTIMET 1                      // Score for begin the max indicator at time t (up to OSC_CATHINJ_DELTARDGFOREQUIVALENTSCORE)
#define OSC_CATHINJ_PTSFORJUMPAFTERINTAKE -1                    // Penalty for jump after intake
#define OSC_CATHINJ_PTSFORJUMPATTHEBEGINING -3                  // Penalty for jump at intake
#define OSC_CATHINJ_DELTASCORETOMAXFORSELECTION 0               // Regions with the best score up to OSC_CATHINJ_DELTASCORETOMAXFORSELECTION are finally selected

#define OSC_CATHINJ_MINDISTTOMERGEROI_MM 0                      // Min distance to merge two (final) ROIs

//======================================================================================
// CONSTANTS

#define OSC_ALLOCA(Dest, Size)  *((void **)&(Dest)) = alloca((Size) * sizeof(Dest[0]))
#define OSC_CALLOC(Dest, Size)  *((void **)&(Dest)) = calloc((Size), sizeof(Dest[0]))
#define OSC_MALLOC(Dest, Size)  *((void **)&(Dest)) = malloc((Size) * sizeof(Dest[0]))
#define OSC_MALLOC16(Dest, Size) *((void **)&(Dest)) = _aligned_malloc((Size) * sizeof(Dest[0]), 16)
#define OSC_MEMCPY(Dest, Src, Size) memcpy((Dest), (Src), (Size)*sizeof((Dest)[0]))
#define OSC_MEMZERO(Dest, Size) memset((Dest), 0, (Size)*sizeof((Dest)[0]))

#define OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, nameSubFunc, nameFunc) if(Err){int Fatal; ErrAppendErrorReportFromCaller(This->ErrHdl, nameFunc, nameSubFunc, 0, Err); ErrIsFatal(OscHdl.ErrHdl, Err, &Fatal); if (Fatal){This->ExtractProc.FatalErrorOnPreviousImage = 1; return Err; } else {ErrorCodeOut = Err;} }


#define OSC_SHUTTERS_SAFETY_MARGIN 8    // (At full resolution)

//===================================================================================================
// STRUCTURES

// Parameter nature
enum  {
	CBD_SUBTRACTION_CLASSIC = 0,
    CBD_SUBTRACTION_MODEL
};

// Advanced diaphragm result structure
typedef struct
{
	int nbExtractedDiaphragm;
	float Param[NB_EXTRACTED_DIAPHRAGM_MAX][3];
	float meanEnergy[NB_EXTRACTED_DIAPHRAGM_MAX];
	float brighnessUnder[NB_EXTRACTED_DIAPHRAGM_MAX];
}
COscDiaphragmParabs;	// To search for the diaphragm

typedef struct
{
	// Data and system dimensions
	float   		frameRateAngio, frameRateFluoro, maxFrameRateAlloc;
	int				IwOriRawAlloc, IhOriRawAlloc, IwProcAlloc, IhProcAlloc, nbProcMaxAlloc, pyrLevelMaxAlloc, maxAngioLengthAlloc;
    int             IwOriRawAngio, IhOriRawAngio, IwOriAngio, IhOriAngio, IwOriRawFluoro, IhOriRawFluoro, IwOriFluoro, IhOriFluoro, Id;
    int             OffsetDiffSizeAngioFluoroXSub, OffsetDiffSizeAngioFluoroYSub;
	float			isoCenterPixSize;
	float			RotAngio, AngAngio, RotFluoro, AngFluoro, RotAngio0, AngAngio0, RotFluoro0, AngFluoro0;
	unsigned char	CArmAnglesAngioGiven, CArmAnglesFluoroGiven, ShuttersRoiPosGiven;
	int				t;
	unsigned char	angioProcessed;
	COscRoiCoord	ShuttersRoiFullScale, ShuttersRoiSafeFullScale;
	COscRoiCoord	ShuttersRoi, ShuttersRoiSafe;
	COscRoiCoord	ShuttersRoiFullScaleRef;
	unsigned char	CArmAnglesAngioGivenRef, CArmAnglesFluoroGivenRef;
	float			RotAngioRef, AngAngioRef, RotFluoroRef, AngFluoroRef;
	float			isoCenterPixSizeRef;
    unsigned char   FatalErrorOnPreviousImage;

    IThreadPool     *pool;

	// Image buffers
	float			**BufsF;
	short			**BufsS;
    unsigned char   **BufsUC;
    int             **BufsI;

	// Table pos
	int				TableInformationGiven;
	float			CurrentPhysicalTablePosXMM, CurrentPhysicalTablePosYMM, CurrentPhysicalTablePosZMM;
	int				tableInfoFromtheStart;
	float			OrigTablePosXMMPlane, OrigTablePosYMMPlane;
	float			CurrentTablePosXMMPlane, CurrentTablePosYMMPlane;
	int			    OrigTablePosXPix, OrigTablePosYPix;
	int			    CurrentTablePosXPix, CurrentTablePosYPix;
	int				deltaTabXSub, deltaTabYSub, deltaTabXSubPrev, deltaTabYSubPrev;
    int             lastDeltaXSubAngio, lastDeltaYSubAngio;
	
	short			*ImOriWorking;

	int				NbProc;
	int				SSFactor;
	int				IwProcAngio, IhProcAngio, IwProcFluoro, IhProcFluoro;
	short			*ImSub;

	float			gammaRdg, KernelSigmas[OSC_CST_MAX_SIGMAS];
	int				nKernelSigs;
	float			*ImRdg, *ImDir;
    int             IwRdg, IhRdg;

	// Diaphragm
	CDiaHdl			DiaHdl;
	unsigned char	DiaHdlCreatedFlag;
	int				detectedDiaphragm;
	float			DiaphragmParams[3];	// Parabola coefficients
	
	// Catheter body
	unsigned char	detectedCatheter;
	CCBDHdl			CBDHdl;				// Handle for catheter body detection
	CCBDParams		CBDParams;		// Parameters for the catheter body detection
	CCBDResults		CBDRes;
	unsigned char	CBDLibCreatedFlag;
	float			*ImCathPotential;
    int             tCBDDetectionFrequency;


	// Sternal wires
	unsigned char	SWHdlCreatedFlag;
	CSWHdl			SWHdl;
	CSWRes		    SWRes;
    int             tLimitForSWExtraction;
    int             SwNbPixStored;
    unsigned char * MaskSWStored;

	// Background
	float			*ImBkg, *ImFrg;
	int				initialDeltaTabCircBackX, initialDeltaTabCircBackY;
	int				CircBufferSize;
	float			** ImCircBuffer;

	// Thresholding
	float			thresFrgL, thresFrgH;
	unsigned char	*ImSee, *ImThr, *ImMask;
	int				*ImLab;

	// For initial allocation
	int				CardiacCycleMaxFrames;
	unsigned char   **BuffOverlayMask;
	short			(***BuffPyrDir1D)[2];

	// Management of potential fluoro skips
	int             InitialSkipOffset, NonInitialSkippedImages, NbOfSuccessiveSkippedFrames;

    int				*PyrWFluoro, *PyrHFluoro;
	short			(**ImFluoroDirPyr1D)[2];
	int				topLevelSearchRangeX, topLevelSearchRangeY;
	
	float			*FluoroToAngioScore;
	int				*FluoroToAngioDX[OSC_VITERBI_FLUORO_DEPTH], *FluoroToAngioDY[OSC_VITERBI_FLUORO_DEPTH];
	
	float			**ViterbiStateScore;
	int				SelectedPhase;

	int				correctedMotionRangeX, correctedMotionRangeY;
	short			analyzedMotionsX[OSC_MOTION_ANALYSIS_DEPTH], analyzedMotionsY[OSC_MOTION_ANALYSIS_DEPTH];

	// Angio reference store
	float			DiaphragmParamsRefAngio[3];
	unsigned char	RefAngioResStored;


    // Cath inj ROI detection
    float           *BufVectF1, *BufVectF2, *BufStack1, *BufStack2;
    unsigned char   *BufVectUC;
    int             *BufVectI1, *BufVectI2;
    COscRoiCoord    CathInjROICandidatesPerFrame[OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION][OSC_CATHINJ_MAXNBOFCANDIDATEROI];
    float           CathInjROICandidatesPerFrameCumRdg[OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION][OSC_CATHINJ_MAXNBOFCANDIDATEROI];
    int             nbROICandidatesPerFrame[OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION];
    float           *cumRdgRegions[NB_EXTRACTED_CATHINJROI_MAX], *cumRdgRegionsFiltered[NB_EXTRACTED_CATHINJROI_MAX];
    int             tInjBeginEstimated;
    unsigned char   selectedFinalROIs[NB_EXTRACTED_CATHINJROI_MAX];
    COscRoiCoord    FinalROICandidates[OSC_CATHINJ_MAXNBOFCANDIDATEROI];
    int             NbFinalCandidateROI;

    // To optimize thresholding stability
    float           minRdgThresholdClean;
    int             nbThresholdsForMedianComputation;
    float           storedThresholds[OSC_CLEAN_MAXIMAGES_FORANGIOSTORAGE];
    float           *storedThresholdsBuf1, *storedThresholdsBuf2;

}COscExtractProc;

typedef struct
{
	// Filled at OscAngioProcessPicture() call
    float           minFrg, maxFrg;
	unsigned char	*ImFrgUC;
	unsigned char  	*ImDirUC;
    short           *ImLabS;

	// Filled during OscAngioProcessSequence() call
	short			**ImPyr1D;
	short			(**ImPyrDir1D)[2];	// Gaussian Pyramid of direction image over x and rotated
	unsigned char   *ImOverlayMask;		// Overlay mask, normalized
	
	int					deltaTabXSub, deltaTabYSub;
}COscExtractPicture;


typedef struct
{
	COscExtractPicture  *ExtractPictures; // Arrays of stored pictures
	int					nExtractPictures;                         // number of stored pictures
	int					*PyrW, *PyrH, PyrNbBands;

	float				**MatchingScores;
	int					**dXAngioToAngio, **dYAngioToAngio;

	int					CardiacCycleStart, CardiacCycleEnd;

    int                 nbSelectedCathInjROI;
    COscRoiCoord        SelectedCathInjROI[NB_EXTRACTED_CATHINJROI_MAX];

    float               meanInjectedSurfaceMM2, normalizedInjectionCoefficient;
    float               injectionCoefficientThreshold;
    
    // Cleaning by temporal consolidation
    int             maxImageNbCycle;
    short           **storeCycleShort;
    
#ifdef OSC_CLEAN_DEBUG
    int             **ImLabOverSeq;
    short           **ImLabOverSeqAfterRdgThres;
    short           **ImLabLargestRegions;
    short           **ImLabAfterClustering;
#endif
    
    //int             SSCumX, SSCumY, nbLabCumulatedCycle;
    //unsigned char   MapCumulatedCycle[OSC_CONSOLIDATE_CYCLE_IW*OSC_CONSOLIDATE_CYCLE_IH];
    //short           LabCumulatedCycle[OSC_CONSOLIDATE_CYCLE_IW*OSC_CONSOLIDATE_CYCLE_IH];
    //short           CumulatedIndicatorPerLab[OSC_CONSOLIDATE_CYCLE_IW*OSC_CONSOLIDATE_CYCLE_IH];
    //unsigned char   MaskCumulatedCycle[OSC_CONSOLIDATE_CYCLE_IW*OSC_CONSOLIDATE_CYCLE_IH];
    //float           **LabNbElts;
}COscExtractSequence;


typedef struct
{
	CErrHdl             ErrHdl;          // Handle for error management
	CPrmHdl             PrmHdl;          // Parameter handle

	char               *VersionInfo;     // Library's version-related info
	char               *VersionInfoCopy; // Copy of VersionInfo returned, as a pointer, to client

	CPrmPara            ExtractParam;    // External parameters
	COscExtractProc     ExtractProc;     // Internal variables and images for the extraction process
	COscExtractSequence ExtractSequence; // Internal storage for processed angio pictures

}COsc;


//===================================================================================================
// METHODS

#ifdef __cplusplus
extern "C" {
#endif

// *** Library
void OscDefaultInternalParameters(COscHdl* PtOscHdl);                                           // Osc.c
void OscReInitializeInternalParameters(COscHdl* PtOscHdl);                                      // Osc.c
int OscSetVersion(COsc *This);																	// Osc.c
int OscSetParaCtx(CPrmHdl P);																	// OscSetParaCtx
int OscSetExtractProcFromExternalParam(COsc *This, int LockCriticalParamsFlag);					// OscParam

// *** Angio processing
int OscAngioTestsDataConsistency(COscHdl OscHdl, short* InIma, int t);							// OscAngioExtractPicture
int OscStoreExtractedPicture(COsc *This);														// OscAngioExtractPicture
void OscFillAngioPictureResults(COscAngioPictureResults *ExtractResults
                                , COscExtractProc *ExtractProc);	                            // OscAngioExtractPicture

// *** Fluoro processing
int OscFluoroTestsDataConsistency(COscHdl OscHdl, short* InIma, int skippedImage, int t);       //OscFluoroExtractPicture

int OscSubSampleImageMin(COscHdl OscHdl, short *ImIn, int IwIn, int IhIn
						, short *ImOut, int IwOut, int IhOut, int SSFactor);					// OscUtilsImage

// *** Rdg computation
void OscRdgMultiScaleRidge(COscHdl OscHdl, int Iw, int Ih);										// OscRdgComputation
int OscRdgComputation(short *Ori, short *GauBuf, float *Rdg, float *Dir, int Iw, int Ih
						, COscRoiCoord *RoiCoord, float KernelSig);								// OscRdgComputation
int OscRdgMonoScaleRidge(short *Ori, short *Gau, float *Rdg, float *Dir, int Iw, int Ih,
							COscRoiCoord *RoiCoord, float KernelSig, float *TmpRdg);			// OscRdgComputation
void OscRdgAnisoFilter(float *Rdg, float *Dir, int Iw, int Ih, float *TmpRdg);					// OscRdgComputation

// *** Rdg computation SSE
void OscRdgMonoScaleRidgeSSE(short * Ori, short * Gau, float *Rdg, float *Dir, int Iw, int Ih,
                             COscRoiCoord *RoiCoord, float KernelSig);

// *** Diaphragm
int OscDiaphragmDetection(COscHdl OscHdl, short *ImOri);									// OscDiaphragm.c
int OscDiaphragmTrackingTR(COscHdl OscHdl);													// OscDiaphragm.c
void OscDiaphragmCorrection(COscHdl OscHdl, short *Gau, float *Rdg, float *Dir
                            , int Iw, int Ih);				                                // OscDiaphragm.c

// *** Catheter bodies
int OscCatheterBodyDetectionAndSubtraction(COscHdl OscHdl);

int OscCatheterBodyDetection(COscHdl OscHdl);
int OscComputePotentialImage(COscHdl OscHdl);

// *** OscUtilsImage
int OscSubSampleImageMin(COscHdl OscHdl, short *ImIn, int IwIn, int IhIn, short *ImOut, int IwOut, int IhOut, int SSFactor);
int OscImDynamicsAdjust(COsc* This, float * Im, int Iw, int Ih, COscRoiCoord *ShutterPos
					, float percentileL, float percentileH, float * Buf, float * ImRescaled);
int OscPercentileComputation(COsc *This, float *Im, int Iw, int Ih, COscRoiCoord *RoiCoord, float percentileIn, float *thresholdOut);
int OscPercentileComputationMask(COsc *This, float *Im, unsigned char *Mask, int Iw, int Ih, COscRoiCoord *RoiCoord, float percentileIn, float *thresholdOut);
int OscPercentileComputationShort(COsc *This, short *Im, unsigned char *Mask, int Iw, int Ih, COscRoiCoord *RoiCoord, float percentileIn, float *thresholdOut);
void OscDilationRect(unsigned char * ImIn, int Iw, int Ih, COscRoiCoord *RoiCoord, int halfSizeX, int halfSizeY
					, unsigned char *Buf, unsigned char *ImOut);
int OscMedian(COsc* This, float * stack1, float * stack2, int x0, int pos, int xEnd, float *medVal);

// *** Sternal wires
int OscSternalWireDetection(COscHdl OscHdl, short *ImOri);			                            // OscSternalWires.c
void OscRdgSWRemoval(COscHdl OscHdl, short *ImOri, unsigned char angioFlag);	                // OscSternalWires.c
int OscRdgSWTrackingAndRemovalFluoro(COscHdl OscHdl, short *ImOri);	                                // OscSternalWires.c

// *** Background
void OscRdgCircularBackgroundSubtraction(COscHdl OscHdl);										// OscRdgBackground.c
void OscRdgInitialWarpedBackgroundSubtraction(COscHdl OscHdl);									// OscRdgBackground.c
void OscTemporalMinimum(float *Im, float *Back, int Iw, int Ih, int t, COscRoiCoord *RoiCoord
						, int TabX, int TabY);													// OscRdgBackground.c

// *** Thresholding and hysteresis
int OscRdgAngioHysteresisLabeling(COscHdl OscHdl);												// OscRdgThresholding.c

int OscRdgThresholdLabelsByCumRdg(int *Lab, unsigned char *Thr, int Iw, int Ih, COscRoiCoord *RoiCoord
								, int NbLabels, int *NbPelPerLab, float *CumRdgPerLab, float RidgenessThresholdRatio);				// OscRdgThresholding.c
void OscRdgThresholdLabelsBySize(int *Lab, unsigned char *Thr, int IwProc, int IhProc, COscRoiCoord *RoiCoord
								 , int NbLabels, int *NbPelPerLab, int MinNbPelPerLab, int KeepLargestLabel);						// OscRdgThresholding.c

int OscRdgDirectionalLabelingFromSeed(COscHdl OscHdl, const unsigned char *SeedIn, const unsigned char *ThrIn
									, int NbRangePix, float MaxDirInDiff
									, int *LabOut, int *NbLabelsOut, int *NbPelPerLabOut, float *CumRdgOut);						// OscRdgThresholding.c
int OscRdgDirectionalLabelingFromSeedDetailed(COscHdl OscHdl, const unsigned char *SeedIn, const unsigned char *ThrIn
											, float *RdgIn, float *DirIn, int Iw, int Ih, int NbRangePix, float MaxDirInDiff
                                            , short *stack_x, short *stack_y, short *stack_x0, short *stack_y0, short *nbIterOnMaskIn
											, int *LabOut, int *NbLabelsOut, int *NbPelPerLabOut, float *CumRdgOut);				// OscRdgThresholding.c
int OscRdgLabelingFromSeed(COscHdl OscHdl, const unsigned char *SeedIn, const unsigned char *ThrIn
							, int NbRangePix, short *stack_x, short *stack_y, short *stack_x0, short *stack_y0, short *nbIterOnMaskIn
                            , int *LabOut, int *NbLabelsOut, int *NbPelPerLabOut, float *CumRdgOut);				// OscRdgThresholding.c

// *** Motion estimation
float OscMultiResGlobalTranslationEstimationScalarProduct(short (** PyrDir)[2],	short (** PyrDirRef)[2], COscRoiCoord *RoiCoordRef
														, int *PyrW, int *PyrH, int *PyrWRef, int *PyrHRef
                                                        , int dXIn, int dYIn, int maxDeltaX, int maxDeltaY, int maxRes
                                                        , int *dXOut, int *dYOut);	                                                // OscMotionEstimation
float OscMultiResGlobalTranslationEstimationProduct(short ** Pyr, short ** PyrRef, int Iw, int Ih, COscRoiCoord *RoiCoord
													, int dXIn, int dYIn, int maxDeltaX, int maxDeltaY, int maxRes
													, int *dXOut, int *dYOut);														// OscMotionEstimation
void OscBlockMatchingCompensation(float * ImRef, float * ImEst, int Iw, int Ih, COscRoiCoord *ROICoord
								, int nbBlocsX, int nbBlocsY, int maxDelta, int TRXGlobal, int TRYGlobal, float * WImEst);			// OscMotionEstimation

// *** Pyramids
int OscGetPyramidDim(COsc *This, int NbBand, int Iw, int Ih, int **PtPyW, int **PtPyH);			// OscPyramid.c
int OscFreePyramidDim(COsc *This, int *PtPyW, int *PtPyH);										// OscPyramid.c
int OscAllocatePyramid(COsc *This, short ***PtPyr, int *PyW, int *PyH, int NbBand);				// OscPyramid.c
int OscFreePyramid(COsc *This, short **Pyr, int NbBand);										// OscPyramid.c
void OscMrGaussianDecompPyr(short **Pyr, int NbBand, int *PyW, int *PyH);						// OscPyramid.c
int OscAllocatePyramidGeneric(COsc *This, void ***PtPyr, int nb_bytes, int *PyW, int *PyH, int NbBand);// OscPyramid.c
int OscFreePyramidGeneric(COsc *This, void **Pyr, int NbBand);									// OscPyramid.c
void OscMrGaussianDecompPyrShort2(short (**Pyr)[2], int NbBand, int *PyW, int *PyH);			// OscPyramid.c

// *** OscAngioSequence
int OscRegisterAngioImagesOnEachOther(COscHdl OscHdl);											// OscAngioProcessSequence.c
int OscHeartCycleSelection(COscHdl OscHdl, int *IndexOfCycleStart, int *IndexOfCycleEnd);		// OscCardiacCycle.c
int OscComputeOverlay(COscHdl OscHdl, int t);													// OscAngioProcessSequence.c
int OscFullResolutionCycleMotionEstimation(COscHdl OscHdl);										// OscAngioProcessSequence.c
float OscComputeInjectionIndicator(COscHdl OscHdl, int CardiacCycleStart, int CardiacCycleEnd); // OscAngioProcessSequence.c
int OscInjectionQualityEstimation(COscHdl OscHdl);												// OscAngioProcessSequence.c
void OscFillAngioSequenceResults(COscAngioSequenceResults *SequenceResults
                                 , COscExtractProc *ExtractProc, COscExtractSequence *ExtractSequence);						// OscAngioProcessSequence.c

//int OscConsolidateSelectedHeartCycleTemporally(COscHdl OscHdl);
//void OscSubFunc4NeighborLabeling(unsigned char *Map, int Iw, int Ih, unsigned char threshold, int x, int y, short currLab, short *Lab);

// *** OscCleanVasculature
int OscRdgCleanVasculatureImage(COscHdl OscHdl);								    // OscCleanVasculature.c
int OscClean_OverSegmentAndStats(COscHdl OscHdl, unsigned char *ImMaskIn, float *ImFrgIn, float *ImDirIn, int Iw, int Ih, float *CumRdgTmp, float *ImFrgFiltTmp
                                 , short *stack_x, short *stack_y, short *stack_x0, short *stack_y0, short *nbIterOnMaskIn
                                  , int *ImLabOut, int *nbLabelsOut, int *nbPixPerLab, float *meanRdgOut);
float OscClean_ComputeMeanRdgThresholdImageWise(int *nbPixPerLab, float *meanRdgPerLab, int nbLabels);
void OscImMaxFiltPerLab(int * ImLab, float * Rdg, float * RdgFilt, int Iw, int Ih, int sizeFilt);// OscCleanVasculature.c
int OscRdgCleanVasculatureSequenceConsolidation(COscHdl OscHdl, int firstFrame, int lastFrame);
int OscClean_RemoveShortestLabels(short *ImLabIn, int Iw, int Ih, int *nbPixPerLab, int nbLabelsIn, int nbLabelsToKeep
                                  , short *ImLabOut, int *nbPixByLabelsOut);
int OscClean_ClusteringByDistance(short *ImLabIn, int Iw, int Ih, COscRoiCoord *RoiCoord, int *lengthRegionsIn, int nbLabsIn, float maxDistPix
                                  , unsigned char *BufMask, unsigned char *BufMask2, unsigned char *BufMask3, short *ImClustOut);
void OscClean_ComputeFinalMapFromClusters(short *ImClust, int Iw, int Ih, COscRoiCoord *RoiCoord, int nbClusters
                                          , float pixSize, float minDistBorderToMerge, float minLengthToMergeIfSameBorder
                                          , short *ImLabFinal);

// *** OscFluoroExtractPictures
int OscFluoroHandleFluoroSkips(COscHdl OscHdl, int FrameStatus, int *t);						// OscFluoroExtractPictures.c
int OscFluoroTestsDataConsistency(COscHdl OscHdl, short* InIma, int skipFrameFlag, int t);		// OscFluoroExtractPictures.c
int OscFluoroUpdateRidgeKernels(COscHdl OscHdl);												// OscFluoroExtractPictures.c
int OscFluoroProcessPicturePyramid(COscHdl OscHdl);											    // OscFluoroExtractPictures.c
int OscMatchFluoroWithEachAngioPhase(COscHdl OscHdl);											// OscFluoroExtractPictures.c
int OscAdaptSearchRange(COscHdl OscHdl);														// OscFluoroExtractPictures.c
void OscFillFluoroPictureResults(COscFluoroPictureResults *ExtractResults
								 , COscExtractProc *ExtractProc, COscExtractSequence *ExtractSequence);// OscFluoroExtractPictures.c

// *** OscViterbi.c
int OscViterbiIteration(COscHdl OscHdl);														// OscViterbi.c
float OscViterbiSpatialTransScore(COscHdl OscHdl, int dX, int dY
								  , float d1, float v1, float d2, float v2, float d3);			// OscViterbi.c

// *** OscMemoryManagement.c
int OscStoreRefAngioStatus(COscHdl OscHdl);
int OscRestoreAngioStatus(COscHdl OscHdl);

// *** OscInjectionCatheter
int OscCatheterInjectionCatheterROIDetection(COscHdl OscHdl);
int OscDetermineInflowBegin(COscHdl OscHdl, int *tInjBegin, int *tInjEnd);
int OscFindSlopeLimits(float *injInd, int In, unsigned char* alreadyTaken, int *injBegin, int *injEnd);
void OscGetCandidateCathInjROIsFrameByFrame(COscHdl OscHdl, int tInjBegin);
int OscGetRepresentativeCathInjCandidatesROI(COscHdl OscHdl, int tInjBegin);
int OscKMeansBarycenterBoxes(float *xs, float *ys, float *ws, int nbIn, int nbParticles, int nbInitsKMeans, int nbIterationOptimizations
                              , float *xBary, float *yBary, short *bufS, int *bufI, float *xOut, float *yOut, short *labelParticle);
int OscCathInjROICompatibleWithAngulation(COscRoiCoord *ShutterPos, int marginShut, int xMinROI, int xMaxROI, int yMinROI, int yMaxROI
                                          , int topLeft, int topRight, int rightBottom, int rightTop
                                          , int leftBottom, int leftTop, float Rot, float Ang);
void OscRemoveSmallestRegionslabel(COscRoiCoord *listROIs, short *labelParticle, int nrROI, int nbLabs, int maxNrRegions
                                   , int *bufI, short *bufUC);
int OscCathInjProposeBestROIForLabel(int lab, COscRoiCoord * listROIs, short *labelParticle, int nbROIIn, int nbFrames, COscRoiCoord *ShutterCoord
                                     , COscRoiCoord * bestROIsLab, float *bestSurfaceCoveredRatioOut, float *bestMissedSurfaceRatioOut
                                     , float *bestCompromiseOut);
void OscComputeCumulRidgeTouchingROIs(COscHdl OscHdl, int tInjBegin, int tInjEnd, float ** cumRdgs);
int OscProcessCumRdgToSelectInjROI(COscHdl OscHdl, int tInjBegin, int tInjEnd, float ** cumRdgs, float **cumRdgsFiltered
                                    , float *Buf1, float *Buf2, unsigned char *selectedROIsOut);

#ifdef __cplusplus
};
#endif

#endif   //_OSCDEV_H
