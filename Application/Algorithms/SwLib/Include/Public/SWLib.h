#include "IThreadPool.h"

#pragma once

//#define SW_TIMING

/** Library handle. */
typedef struct 
{ 
	void*   Pv;     /** Access to private material. */
} CSWHdl; 

/** ROI structure.*/
typedef struct
{
	int XMin, YMin, XMax, YMax;

}CSWRoiCoord;

/** Advanced result structure.*/
typedef struct{
    unsigned char   *SWMaskFinalTight, *SWMaskFinalLoose;               /** Final mask, loose and tight versions.*/
    unsigned char   *SWMaskTight, *SWMaskLoose;                         /** Instantaneous masks, loose and tight versions.*/

    float           medianRdg;                                          /** Median of the positive ridges.*/
    float           *Rdg;                                               /** Ridge-filtered image.*/

    int             nbLabsBeforeConsolidation;                          /** Nb of regions before geometrical constraints.*/
    int             *nbPixPerLabBeforeConsolidation;                    /** Nb pixels of each region before geometrical constraints.*/
    float           *xBaryRegionBeforeCons, *yBaryRegionBeforeCons,     /** Barycenters of the different regions.*/
                    *vDir1RegionBeforeCons, *vDir2RegionBeforeCons;     /** Associated inertia axes.*/
    float           *xMin1BeforeCons, *xMax1BeforeCons
                    , *yMin2BeforeCons, *yMax2BeforeCons;               /** Limits of the region reprojection over inertia axes.*/
    unsigned char   *validationStatus;                                  /** Result of the geometrical assessment. Over 10 accepted, below refused.*/

#ifdef SW_TIMING
    double          durationRdg, durationFrameBased
                    , durationTemporalMin, durationConsolidation;       /** Computation time at different instants (measured if macro SW_TIMING defined)*/
#endif
}CSWResAdvanced;

/** Result structure.*/
typedef struct
{
    unsigned char   *SWMask;                                            /** Current sternal wire mask (definitive after first frames). Can be full of zeros if no SW detected.*/
    int             nbPixMask;                                          /** Nb of pixels positive in the mask.*/
    int             Iw, Ih;                                             /** Mask image size.*/

    CSWResAdvanced advanced;
}CSWRes;

#ifdef __cplusplus
extern "C" {
#endif

	#ifdef StW_DLLEXPORT
	#undef StW_DLLEXPORT
	#endif // ifdef StW_DLLEXPORT

	#ifdef SWLIB_EXPORTS
	#define StW_DLLEXPORT __declspec(dllexport)
	#else
	#define StW_DLLEXPORT
	#endif

    // ****** Life management

	/** Routine creating the library handle from maximum image size and nb of cores. the internal allocations are performed in this function.*/
    StW_DLLEXPORT int SWCreate(CSWHdl *Hdl, int IwMax, int IhMax, int nbCoresMax);
    /** Setting the threadpool necessary for the multicore processing.*/
    StW_DLLEXPORT int SWSetThreadpool(CSWHdl *Hdl, IThreadPool *pool);
    /** Deleting the handle. */
    StW_DLLEXPORT void SWDelete(CSWHdl *hdl);
    /** Routine adapting the internal results to a new image size. Necessary in particular when translating the SW mask estimated on an angio image to a corresponding fluoro image of different size.*/
    StW_DLLEXPORT int SWTranslateMaskToDifferentImageSize(CSWHdl *Hdl, int Iw, int Ih);
    /** Setting a mask for the outside, to use the tracking capabilities. For restoring SW tracking after result was loaded and handle deleted.*/
    StW_DLLEXPORT int SWRestoreStatusForTracking(CSWHdl *Hdl, int Iw, int Ih, unsigned char *NewMaskIn);
    /** Returns the version info.*/
    StW_DLLEXPORT int SWGetVersionInfo(CSWHdl Hdl, char** VersionInfo);
    
    // ****** SW extraction

    /** Sternal wire extraction. Mask computation and consolidation over the first few frames, tracking then.*/
    StW_DLLEXPORT int SWExtract(CSWHdl *Hdl, const short* Ori, int Iw, int Ih, CSWRoiCoord* RoiCoord, int t, float pixSize, bool consolidate, CSWRes * SWRes);

    // ****** SW tracking

    /** Sternal wire tracking, default search range (adapted to SW motion).*/
    StW_DLLEXPORT int SWTracking(CSWHdl *Hdl, const short* Ori, int Iw, int Ih, CSWRoiCoord* RoiCoord, int t, float pixSize, CSWRes * SWRes);
    /** Sternal wire tracking handling tracking, search range (adapted to SW motion).*/
    StW_DLLEXPORT int SWTrackingWithPanningInfo(CSWHdl *Hdl, const short* Ori, int Iw, int Ih, CSWRoiCoord* RoiCoord, int t, float pixSize, float dXPanningMM, float dYPanningMM, CSWRes * SWRes);
    /** Mask repositionning. Boils down to tracking with a search range set by the user. Needed when translating from angio detection to corresponding fluoro.*/
    StW_DLLEXPORT int SWRepositionMask(CSWHdl *Hdl, const short* Ori, int Iw, int Ih, CSWRoiCoord* RoiCoord, float pixSize
                                        , float dXPanningMM, float dYPanningMM, float searchRangeXMM, float searchRangeYMM, CSWRes * SWRes);
    
    // ****** Utility functions

    /** Input image subsample. By local minimum. */
    StW_DLLEXPORT void SWSubSample(short *ImIn, int IwIn, int IhIn, int SSFactor, short *ImOut);
    /** Image-based estimation of the shutter position. */
    StW_DLLEXPORT int SWExtractShutters(short* InIma, int Iw, int Ih, CSWRoiCoord *RoiCoord);
    /** Returns the estimated motion. For results analysis.*/
    StW_DLLEXPORT void SWGetEstimatedMotion(CSWHdl *Hdl, int *tx, int *ty);

#ifdef __cplusplus
};
#endif
