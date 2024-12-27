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


// Client-level umbrella include with public method prototypes

#ifndef		_OSC_H
#define		_OSC_H

#include <ErrLib.h>
#include <PrmLib.h>

#include <OscErrorCodes.h>
#include <OscParamCodes.h>
#include <CBDLib.h>
#include <DiaLib.h>
#include <SWLib.h>

#include "IThreadPool.h"

//****************************************************************************
// CONSTANTS

#define OSC_VERSION_MAJOR               3
#define OSC_VERSION_MINOR               9
#define OSC_VERSION_PATCH               2

#define OSC_CST_MAX_SIGMAS		        16	// Rdg maximum number of scales for multi-level ridgeness
#define NB_EXTRACTED_DIAPHRAGM_MAX      10
#define NB_EXTRACTED_CATHINJROI_MAX     4

/** Status of the shutter extraction commodity function */
enum {
	OSC_SHUTEXT_OK = 0,					/** Processing Ok */
	OSC_SHUTEXT_WARNING_LIM_ABOVE_HALF, /** (At least) one of the components was estimated on the wrong half of the image */
	OSC_SHUTEXT_ERR_LIM					/** (At least) one of the shutter position had max and min confused */
};

/** Constants to use for defining whether the frame needs to be processed or skiped */
enum {
	OSC_FLUORO_FRAME = 0,
	OSC_FLUORO_SKIP_FRAME
};

//****************************************************************************
// STRUCTURES

/** Library handle */
typedef struct 
{ 
	CErrHdl ErrHdl; /** Error-library handle associated to library */
	CPrmHdl PrmHdl; /** Parameter-library handle associated to library */
	void*   Pv;     /** Access to private material */
} COscHdl; 

/** ROI structure */
typedef struct
{
	int XMin, YMin, XMax, YMax;

}COscRoiCoord;

/** Diaphragm result structure */
typedef struct
{
	int	  diaphragmDetected;		/** Diaphragm detected (-1 (no decision yet) / 0 / 1) */
	float a, b, c;                  /** Diaphragm parabola parameters */
}
COscDiaphragmParab;

/** Volatile result structure. Read only.
*/
typedef struct
{
	short				*Sub;                   //* Internal subsampled image */
	float				*Rdg, *Dir, *Bkg, *Frg; //* Internal ridge, direction, background, foreground images */
	unsigned char		*See,*Thr,*Mask;        //* High-thresholded, low thresholded, hyteresis thresholded images */
	int					*Lab;                   //* Connected components image */
	unsigned char		*Ovr;                   //* Overlay image image */
	CCBDResults			*CBDRes;                //* Result structure of the extracted catheter body */ 
	CSWRes			    *SWRes;                 //* Result structure of the extracted sternal wires */ 

    float				**AngiosMatchingScores; //* Angio matching scores, after respective registrations. */
    int                 **AngiosMatchingdX;     // Corresponding motion vectors in X
    int                 **AngiosMatchingdY;     // Corresponding motion vectors in Y
    
    int                 **ImLabOverSeg;         //* Directional oversegmented images. */
    short               **ImLabOverSeqAfterRdgThres, **ImLabLargestRegions, **ImLabAfterClustering;         //* Cleaned regions after thresholding, removal of small regions, clustering analysis. */

}COscVolatile;

/** Advanced result structure.
*/
typedef struct{
	int					IwOri, IhOri                        //* Input image size. */
                        , IwOriInternal, IhOriInternal      //* Internal input image size. Extensions of IwOri and IhOri scaling well with chosen pyramid levels and subsapling factor. */
                        , IwProc, IhProc                    //* Subsampled image sizes */
                        , SSFactor;                         //* Subsample factor */

	// Catheter ROIs
	unsigned char     CatheterBodyDetected;                 //* Flag to indicate whether catheter body has been detected. */

	// Diaphragm 
	unsigned char		DiaDetected;                        //* Flag to indicate whether catheter body has been detected. */
	COscDiaphragmParab	DiaParam;				            //* Parameters of the detected diaphragm (if any). */

    // CathInj
    int             nbDetectedCathInjROI;                   //* Number of detected catheter injection ROI. */
    COscRoiCoord    DetectedCathInjROI[NB_EXTRACTED_CATHINJROI_MAX];    //* Coordinated of the detected catheter injection ROIs. */
    int             tInjBeginEstimated;                     //* Estimated injection time instant. */

    // Log cleaning
    float           minRdgThresholdClean;                   //* Threshold on region mean ridge used to clean vessel maps. */
	
	COscVolatile Volatile;                                  //* Volatile extraction results (read-only) */
}COscAdvanced;

/** AngioPicture result structure.
*/
typedef struct{
	int               Status;                               //* Extraction status //
	COscAdvanced      Advanced;					            //* Advanced data	//
}COscAngioPictureResults;

/** AngioSequence result structure.
*/
typedef struct{
	int             CardiacCycleStart;                      //* Selected heart cycle start //
	int             CardiacCycleEnd;                        //* Selected heart cycle end //

    unsigned char   detectionDubious;                       //* Confidence in catheter injection ROI detection. 0 if confident, 1 if detection unlikely. //
    unsigned char   detectedContactWithBorder[4];           //* Border where an catheter injection ROI was detected. Indice 0: left, 1: top; 2 right; 3 bottom. //
    
    float           meanInjectedSurfaceMM2;                 //* Mean surface covered by the injected vessels, over the selected heart cycle. //
    float           normalizedInjectionCoefficient;         //* Normalized version of the mean surface covered by the injected vessels. Initially meant to be thresholded at 0.5.//

    COscAdvanced      Advanced;					            //* Advanced result data //

}COscAngioSequenceResults;

/** Fluoro picture result structure.
*/
typedef struct{
	int		selectedAngioIndice;                            // * Indice of the angio image matched with the current fluoro image. //
	int		dXSelectedAngio, dYSelectedAngio;               // * Spatial shift between current fluoro image and the matching angio. //

	COscAdvanced      Advanced;					            //* Advanced result data //	

}COscFluoroPictureResults;



//===================================================================================================
// METHODS

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OSC_DLLEXPORT
#undef OSC_DLLEXPORT
#endif // ifdef DLLEXPORT 

#ifdef OSCLIB_EXPORTS
#define OSC_DLLEXPORT __declspec(dllexport)
#else
#define OSC_DLLEXPORT
#endif


    // ****** Life management

	/** Routine creating the library handle. 
    Performs all the future needed internal allocation, that is why max image size, max internal subsampled image size, max frame rate, max angio duration and max number of cores are needed.
    Also checks that the image dynamics are acceptable.*/
	OSC_DLLEXPORT int OscCreate                     (COscHdl* PtOscHdl, int ImaDyn, int ImaWidthAlloc, int ImaHeightAlloc
                                                    , int ImaWidthSubAlloc, int ImaHeightSubAlloc, float maxFrameRate, float maxAngioDuration, int maxNbProc);
    /** Routine deleting the handle */
	OSC_DLLEXPORT int OscDelete						(COscHdl OscHdl);
	/** Routine returning the version of the library. */
	OSC_DLLEXPORT int OscGetVersionNumbers			(int* Major, int* Minor, int* Patch); 
	/** Routine returning the information attached to the latest version. */
	OSC_DLLEXPORT int OscGetVersionInfo				(COscHdl OscHdl, char** VersionInfo);

	// ******* Memory management

    /** Routine desallocating the internal memory. Was meant to be used only after handle has been saved to memory (with OscSaveToMemory). */
	OSC_DLLEXPORT int OscMemoryDesallocation		(COscHdl OscHdl);
    /** Routine allocating the internal memory, using the already defined maximum sequence and processing characteristics. Was meant to be used before handle is loaded from memory (with OscLoadToMemory). */
	OSC_DLLEXPORT int OscMemoryAllocation			(COscHdl OscHdl);
    /** Routine allocating the internal memory, using specific defined maximum sequence and processing characteristics. Was meant to be used before handle is loaded from memory (with OscLoadToMemory). */
    OSC_DLLEXPORT int OscMemoryAllocationGivenSize  (COscHdl OscHdl, int IwFull, int IhFull, int IwSub, int IhSub
                                                    , float maxFrameRate, float maxAngioDuration, int nbProcMax, int nbPyrLevelsMax);
    /** Save handle information to memory. Is meant to be used after OscAngioSequence processing. */
    OSC_DLLEXPORT int OscSaveToMemory               (COscHdl OscHdl, void* memOut);
    /** Load handle information from memory. Is meant to be used before OscFluoroPicture processing. */
    OSC_DLLEXPORT int OscLoadFromMemory             (COscHdl OscHdl, void* memIn);
    /** Computes memory size needed to store handle. For allocation before using OscSaveToMemory. */
    OSC_DLLEXPORT long OscComputeMemorySizeForSave  (COscHdl OscHdl);

	// *******  System info management

    /** To set the characteristics of the angio sequence to process (image size, frame rate), the processing subsample factor, and the threadpool for the 
    multiprocessing computations (defining in particular the number of cores). It is checked that these parameters are compatible with the maximum values
    expected by the handle.*/
    OSC_DLLEXPORT int OscSetAngioSequenceParameters(COscHdl OscHdl, int IwAngio, int IhAngio, float frameRateAngio, int SSFactor, IThreadPool *pool);
    /** To set the characteristics of the angio sequence to process (image size, frame rate), and the threadpool for the multiprocessing computations 
    (defining in particular the number of cores). The subsampled factor has to be the same than the one used in angio. It is checked that these parameters 
    are compatible with the maximum values expected by the handle.*/
    OSC_DLLEXPORT int OscSetFluoroSequenceParameters(COscHdl OscHdl, int IwFluoro, int IhFluoro, float frameRateFluoro, IThreadPool *pool);
    /** To set the useful part of the image defined by the shutters.*/
	OSC_DLLEXPORT int OscSetShuttersRoi				(COscHdl OscHdl, COscRoiCoord RoiCoord);
    /** To set the geometry used during the acquisition of the angio image (C-arm angulations, pixel size at isocenter).*/
	OSC_DLLEXPORT int OscSetCurrentGeometryAngio	(COscHdl OscHdl, float Rot, float Ang, float pixSizeAtIsocenter);
    /** To set the geometry used during the acquisition of the fluoro image (C-arm angulations, pixel size at isocenter).*/
	OSC_DLLEXPORT int OscSetCurrentGeometryFluoro	(COscHdl OscHdl, float Rot, float Ang, float pixSizeAtIsocenter);
    /** To set the physical table position used during the acquisition of the angio image (in millimeters).*/
	OSC_DLLEXPORT int OscSetTableApparentPositionMM	(COscHdl OscHdl, float TablePosXMM, float TablePosYMM, float TablePosZMM);
    /** To set the table position used during the acquisition of the angio image in the space of the image (in pixels). The conversion from the C-arm geometry
    to the image space is performed by the client in that case.*/
	OSC_DLLEXPORT int OscSetTableApparentPositionPix(COscHdl OscHdl, int TablePosXPix, int TablePosYPix);
    /** Returns the angio system informations (shutter position, C-arm angles, iso pixelsize, table position) as a string. For regression tests.*/
    OSC_DLLEXPORT int GetCurrentSystemInformationsAngio(COscHdl OscHdl, char systemInformations[512]);
    /** Returns the fluoro system informations (shutter position, C-arm angles, iso pixelsize, table position) as a string. For regression tests.*/
    OSC_DLLEXPORT int GetCurrentSystemInformationsFluoro(COscHdl OscHdl, char systemInformations[512]);

	// *******   Processing
    
    /** Routine called to process one further angio image.*/
	OSC_DLLEXPORT int OscAngioProcessPicture		(COscHdl OscHdl, short* InIma, int t, COscAngioPictureResults*  ProcessResults);
    /** Routine called to process the angio sequence, once every angio image have been processed by OscAngioProcessPicture.*/
	OSC_DLLEXPORT int OscAngioProcessSequence		(COscHdl OscHdl, COscAngioSequenceResults* ProcessResults);
    /** Routine called to process one fluoro image, after a matching angio sequence has been processed.*/
	OSC_DLLEXPORT int OscFluoroProcessPicture		(COscHdl OscHdl, short* InIma, int t, int FrameStatus, COscFluoroPictureResults*  ProcessResults);

	// *******   Displaying/Monitoring
    
    /** Returns the subsampled overlay (mask times ridge foreground) associated to the angio image that matches the current fluoro image. 
    No spatial alignment is performed on the overlay. The pointer VesselIm has to be previously allocated by the client (at least IwProc times IhProc).*/
    OSC_DLLEXPORT int OscAngioPicGetCurrentVesselImage  (COscHdl OscHdl, float *VesselIm);
    /** Returns the subsampled overlay (mask times ridge foreground) associated to the angio image that matches the current fluoro image. 
    Performs a spatial motion compensation in order to cancel the global motion that went on between the angio images AngioIndex and RegisterIndex.
    This is to display the selected heart cycle (and its overlay).*/
	OSC_DLLEXPORT int OscAngioSeqGetRegisteredOverlay	(COscHdl OscHdl, int AngioIndex, int RegisterIndex, unsigned char *OutOverlayIma);
    /** Returns the overlay associated to a given angio frame of the selected heart cycle.*/
	OSC_DLLEXPORT unsigned char* OscAngioSeqGetOverlayAssociatedToAngioFrame(COscHdl OscHdl, int t);
    /** Returns the overlay size, and the associated subsample factor (for allocations performed by the client).*/
    OSC_DLLEXPORT void OscAngioGetOvrSize(COscHdl OscHdl, int *IwOvr, int *IhOvr, int *SSFactor);
    /** Returns the overlay of the angio frame associated to the current fluoro image. Also returns the image characteristics (size and subsample factor).*/
    OSC_DLLEXPORT unsigned char * OscFluoroGetOverlay(COscHdl OscHdl, int *IwOvr, int *IhOvr, int *SSFactor);
    /** Returns the overlay of the angio frame associated to the current fluoro image, and motion compensated it.*/
	OSC_DLLEXPORT int OscFluoroGetCurrentWarpedOverlay	(COscHdl OscHdl, COscFluoroPictureResults *ExtractResults, unsigned char *OvrW);
    /** Returns the x position and the radius of the detected catheter body associated to y (if any).*/
    OSC_DLLEXPORT bool OscGetEstimatedCatheterBodyLimitsX(COscHdl OscHdl, float y, float *xCT, float *R);

	// *******   Provided as a convenience function

    /** Shutters position estimation. Potentially to be used by the client in case he does not have that information from the system.*/
	OSC_DLLEXPORT int OscExtractShutters			(short * InIma, int Iw, int Ih, COscRoiCoord *RoiCoord);
    /** Overlays the roadmap associated to the last processed fluoro image (InIma). The client may prefer to compute his own output from the subsampled 
    overlay returned by one of the other functions.*/
	OSC_DLLEXPORT int OscComputeMedisysOverlay		(COscHdl OscHdl, const short* InIma, short *ImOut);
    /** Sets the threshold on the injection quality indicator that will trigger the "no injection" and "bad injection" warnings. It is set to 0.5 by default. 
    The client may chose to ignore those, and make its decision based on the normalizedInjectionCoefficient from the COscAngioSequenceResults result structure.*/
    OSC_DLLEXPORT void OscSetInjectionDetectionThreshold(COscHdl OscHdl, float injectionCoeffThreshold);

#ifdef __cplusplus
};
#endif

#endif   //_OSC_H
