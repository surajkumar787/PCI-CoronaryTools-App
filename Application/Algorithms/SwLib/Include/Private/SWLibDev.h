#pragma once
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <float.h> 

#include "SWMkxGaussian.h"
#include "SWLib.h"

//#include "vld.h"


#define SW_LIB_FREE(a)              if(a){free(a);a=0;}
#define SW_LIB_ABS(a)               (((a) >= 0)   ? (a): (-(a)))
#define SW_LIB_ABS_F(a)		        (float)fabs(a)
#define SW_LIB_MIN(a,b)             (((a) <= (b)) ? (a):   (b) )
#define SW_LIB_MAX(a,b)             (((a) >= (b)) ? (a):   (b) )
#define SW_LIB_RND(a)               (((a) >= 0.0) ? (int)((a)+0.5) : (int)((a)-0.5))

#define SW_LIB_MEMZERO(Dest, Size)  memset((Dest), 0, (Size)*sizeof((Dest)[0]))
#define SW_LIB_CALLOC(Dest, Size)  *((void **)&(Dest)) = calloc((Size), sizeof(Dest[0]))
#define SW_LIB_MALLOC(Dest, Size)  *((void **)&(Dest)) = malloc((Size) * sizeof(Dest[0]))
#define SW_LIB_MEMCPY(Dest, Src, Size) memcpy((Dest), (Src), (Size)*sizeof((Dest)[0]))
#define SW_LIB_SQ(a)            ((a)*(a))

#define SW_LIB_PI               3.14159265358979323846f

// Parameters
#define SW_GAMMA                                2.f
#define SW_SIGMAS                               {0.06f, 0.12f, 0.24f, 0.36f, 0.48f, 0.6f, 0.72f, 0.84f, 0.96f, 1.08f, 1.2f, 1.32f, 1.44f, 1.56f, 1.68f, 1.8f}
#define SW_NB_WIDTH                             12
#define SW_SEARCH_RANGE_MM                      0.6f
#define SW_WID_LOW_TIGHT_MM                     0.36f
#define SW_WID_HIGH_TIGHT_MM                    0.72f
#define SW_WID_LOW_LOOSE_MM                     0.24f
#define SW_WID_HIGH_LOOSE_MM                    1.28f
#define SW_LAMBDA_THRES                         200
#define SW_COEFF_LAMBDA_THRES                   2.5f
#define SW_HISTO_NB_BINS                        100
#define SW_COEFF_AFTER_VERTI_WEIGHT             0.7f
#define SW_VERTI_WEIGHT_SIGMA_MM                50.f
#define SW_RANGE_SAME_REGION_MM                 1.f
#define SW_MAX_NB_REGIONS                       1000    // For pre-allocating
#define SW_LAB_REGION_FOR_EFFICIENT_REGION_MERGING 100  // For two-step connected components (more efficient)
#define SW_DIST_TO_TOUCH_BORDER_PIX_MM          2.5f
#define SW_LIB_MIN_SIZE_X_MM                    8.f
#define SW_LIB_MIN_SIZE_Y_MM                    2.f
#define SW_LIB_MIN_SURFACE_BORDER_1_MM2         7.5f
#define SW_LIB_MIN_SURFACE_BORDER_2_MM2         2.5f
#define SW_LIB_MIN_SURFACE_1_MM2                7.5f
#define SW_LIB_MIN_SURFACE_2_MM2                2.5f
#define SW_MIN_RATIO_EIGENVALUES                0.12f
#define SW_FINAL_MASK_MAX_DIST_TO_LOOSE_MM      1.5f
#define SW_MARGIN_SHUTTERS_LOOSE                8       // For 1024 images
#define SW_SIGMA_TRACKING                       0.54f


#define SW_VERSION_MAJOR    3
#define SW_VERSION_MINOR    1
#define SW_VERSION_PATCH    2

//#define SW_VERSION_INFO "V.%d.%d.%d: New algo: strong ridges of proper width selection, temporal min, geometrical constraint on resulting connected components." // 3.0.0
//#define SW_VERSION_INFO "V.%d.%d.%d: Bug correction in SWTranslateMaskToDifferentImageSize, adapted tracking function for exploiting table panning info." // 3.1.0
//#define SW_VERSION_INFO "V.%d.%d.%d: Pragma once, removed some include MxLab." // 3.1.1
#define SW_VERSION_INFO "V.%d.%d.%d: Bug correction: when no motion could be estimated in GlobalTranslation." // 3.1.2

typedef struct
{
    // Sequence parameters
    int             IwAlloc, IhAlloc, Iw, Ih;
    CSWRoiCoord     Roi;
    float           pixSize;
    int             t;
    int             nbPixMask;
    
	char            *VersionInfo;     // Library's version-related info
	char            *VersionInfoCopy; // Copy of VersionInfo returned, as a pointer, to client

    // Algo parameters
    int             SearchRange;
    float           WidLowTight, WidHighTight, WidLowLoose, WidHighLoose;
    float           VertiSigmaPix;
    float           TabSig[16];
    int             RangeSameRegion;
    int             MinDistSWX, MinDistSWY;
    float           MinSurfaceTouchesBorder_1, MinSurfaceTouchesBorder_2;
    float           MinSurface_1, MinSurface_2;
    int             MarginShutters;
    float           medianRdg, RdgThres;
    int             dilSizeMaxDistToLoose;
    float           sigmaTracking;
    int             DistToTouchBorder;

    // Internal images
    float           *fOri;
    float           *Ctr;
    float           *Wid;
    float           *VertiWeights;
    float           *EnergyTight;
    float           *EnergyLoose;
    unsigned char   *MaskLiveTight, *MaskRegTight, *MaskSeqTight;
    unsigned char   *MaskLiveLoose, *MaskRegLoose, *MaskSeqLoose;
    unsigned char   *MaskTightFinal, *MaskLooseFinal;

    int             nbCoresMax;
    float           **WidCores;
    float           **CtrCores;
    float           **WkBuf1Cores;
    float           **WkBuf2Cores;

    int             histoBuf[SW_HISTO_NB_BINS];

    int             estTx, estTy;

    short           *WkBuf1S, *WkBuf2S, *WkBuf3S, *WkBuf4S;
    int             *WkBufI1, *WkBufI2, *WkBufI3, *WkBufI4;
    unsigned char   *WkBufUC1, *WkBufUC2;

    int             nbLabsConsolidate;

    int             WVectI1[SW_MAX_NB_REGIONS], WVectI2[SW_MAX_NB_REGIONS], WVectI3[SW_MAX_NB_REGIONS], WVectI4[SW_MAX_NB_REGIONS];
    float           WVectF1[SW_MAX_NB_REGIONS], WVectF2[SW_MAX_NB_REGIONS], WVectF3[SW_MAX_NB_REGIONS], WVectF4[SW_MAX_NB_REGIONS]
                    , WVectF5[SW_MAX_NB_REGIONS], WVectF6[SW_MAX_NB_REGIONS], WVectF7[SW_MAX_NB_REGIONS], WVectF8[SW_MAX_NB_REGIONS]
                    , WVectF9[SW_MAX_NB_REGIONS]; 
    float           WVectFDoubleLength1[2*SW_MAX_NB_REGIONS], WVectFDoubleLength2[2*SW_MAX_NB_REGIONS];
    unsigned char   WVectUC[SW_MAX_NB_REGIONS]; 
    bool            WMatrixBool[SW_LAB_REGION_FOR_EFFICIENT_REGION_MERGING*SW_LAB_REGION_FOR_EFFICIENT_REGION_MERGING];
    
    IThreadPool     *pool;
    
#ifdef SW_TIMING
    double durationRdg, durationFrameBased, durationTemporalMin, durationConsolidation;
#endif
}CSW;

// SWLib
void SWUpdateParameters(CSWHdl *Hdl, int Iw, int Ih, CSWRoiCoord* RoiCoord, int t, float pixSize);
int SWSetVersion(CSW* hdl);

// SWLibProcess
int SWExtractFrameBased(CSWHdl *Hdl, const short* Ori);
int SWExtractTemporalConsolidationMin(CSWHdl *Hdl);
int SWConsolidate(CSWHdl *Hdl);

// SWLibComponentsAnalysis
void SWGeometricalConstraintsOnConnectedComponents(CSWHdl *Hdl, const int *LabLoose, int nbLabs, int * LabTight, int *nbPixPerLab, unsigned char * valid);
void SWTransferLabelToTightAndComputeLimitsAndBary(CSWHdl *Hdl, const int *LabLoose, int nbLabs, int * LabTight, int *nbPixPerLab
                                                   , int *xMinLab, int*xMaxLab, int*yMinLab, int*yMaxLab, float *xBary, float*yBary);
void SWComputeRegionExtensionInPrincipalDirections(CSWHdl *Hdl, const int *LabTight, int nbLabs
                                                   , const int *xMinLab, const int*xMaxLab, const int* yMinLab, const int*yMaxLab, const float *xBary, const float*yBary, const int*nbPixPerLab
                                                   , float *xMinEig1, float *xMaxEig1, float *yMinEig2, float *yMaxEig2);

// SWLibTracking
int SWTrack(CSWHdl *Hdl, const short* Ori, int Iw, int Ih, CSWRoiCoord* RoiCoord, int dXInit, int dYInit, int searchRangeXPix, int searchRangeYPix, CSWRes * SWRes);
unsigned char SWEstimateGlobalTranslation(unsigned char* Maptm1, float* Energyt, int Iw, int Ih, int xMin, int xMax, int yMin, int yMax, int dXInit, int dYInit
                                        , int SearchRangePixX, int SearchRangePixY, IThreadPool *pool, int *TxPix, int* TyPix);

//SWLibSubfunctions
int MultiscaleHessianNoDir(float* fOri, int Iw, int Ih, CSWRoiCoord *RoiCoord, const float* TabSig, int nbSigmas, float Gamma, float* Wid, float* Ctr, float *wkBuf1, float *wkBuf2);
int HessianNoDir(float* fGau, int Iw, int Ih, CSWRoiCoord *RoiCoord, float* lambda1);
int VertiWeight(float* Rt, int Iw, int Ih, float* weight, float sig);
int ImDilate(unsigned char *ImIn, int Iw, int Ih, int xMin, int xMax, int yMin, int yMax, unsigned char* ImOut, int strelSize, unsigned char* buf);
int SWConnectedComponents(const unsigned char *MaskIn, int Iw, int Ih, int xMin, int xMax, int yMin, int yMax, int NbRangePix
                          , short *stack_x, short *stack_y, short *stack_x0, short *stack_y0, int *LabOut, int *nbPelPerLab, int *NbLabelsOut);
void SWConnectedComponentsAndDistance(CSWHdl *Hdl, int *LabOut, int *nbPixPerLab, int *NbLabelsOut);
void RelabelRecursive(int ind, int newLab, int maxInd, const bool*merge, const int *nbPixPerLabPrev, int *newLabs, int *nbPixPerLab);
float SWMedianPosRidgeByHisto(float *Rdg, int Iw, int Ih, int xMin, int xMax, int yMin, int yMax, int *histBuf, int nbBins);

// SWRdgComputationSSE
int MultiscaleHessianNoDirSSEFloat(float* Ori, int Iw, int Ih, CSWRoiCoord *RoiCoord, const float* TabSig, int nbSigmas, float Gamma, float* Wid, float* Ctr, float *wkBuf1, float *wkBuf2);
void SWHessianNoDirSSEFloat(float * Ori, float * Gau, float *Rdg, int Iw, int Ih, CSWRoiCoord *RoiCoord, float KernelSig);
int MultiscaleHessianNoDirSSEShort(short* Ori, int Iw, int Ih, CSWRoiCoord *RoiCoord, const float* TabSig, int nbSigmas, float Gamma, float* Wid, float* Ctr, float *wkBuf1, float *wkBuf2);
void SWHessianNoDirSSEShort(short * Ori, short * Gau, float *Rdg, int Iw, int Ih, CSWRoiCoord *RoiCoord, float KernelSig);


