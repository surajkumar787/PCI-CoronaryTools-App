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

#include <OscDev.h>

#define DOWN(dim) (((dim) % 2) == 0 ? ((dim) >> 1) : ((dim) >> 1) + 1)

int OscSetAngioSequenceParameters(COscHdl OscHdl, int IwAngio, int IhAngio, float frameRateAngio, int SSFactor, IThreadPool *pool){
    
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
    COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
    char                 Reason[256];
    int                 ErrorCodeOut = 0, Level;
    int					pyrFactor = (int)pow(2.0, OSC_NB_PYR);

    int                 IwOriAngio = (int)floor( ((double)IwAngio / (SSFactor * pyrFactor))) * SSFactor * pyrFactor; 
    int                 IhOriAngio = (int)floor( ((double)IhAngio / (SSFactor * pyrFactor))) * SSFactor * pyrFactor; 
    int                 IwProcAngio = IwOriAngio / SSFactor;
    int                 IhProcAngio = IhOriAngio / SSFactor;
    int                 nbProc;

    if ( (IwAngio>ExtractProc->IwOriRawAlloc) || (IhAngio>ExtractProc->IhOriRawAlloc) 
        || (IwProcAngio>ExtractProc->IwProcAlloc)|| (IhProcAngio>ExtractProc->IhProcAlloc)){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		sprintf_s(Reason, 256,"Image dimension in angiohigher than max allowed by allocation:\nFull res (%d, %d) vs (%d,%d)\nSub res (%d, %d) vs (%d,%d)\n"
                    , IwAngio, IhAngio, ExtractProc->IwOriRawAlloc, ExtractProc->IhOriRawAlloc
                    , IwProcAngio, IhProcAngio, ExtractProc->IwProcAlloc, ExtractProc->IhProcAlloc);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscSetSequenceParameters", OSC_ERR_IMSIZEEXCEEDSALLOCATION, Reason);
		return OSC_ERR_IMSIZEEXCEEDSALLOCATION;
    }

    pool->execute([&nbProc](int threadIndex, int threadCount){ nbProc = threadCount;});  // Get number of threads

    if ( (nbProc > ExtractProc->nbProcMaxAlloc) || (OSC_NB_PYR > ExtractProc->pyrLevelMaxAlloc)
        || (frameRateAngio > ExtractProc->maxFrameRateAlloc)){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		sprintf_s(Reason, 256,"Allocated memory insufficient: expecting %d proc max (needed %d), expecting %d pyr levels (needed %d), expecting frame rate %.1f (needed %.1f in angio)\n"
                    , ExtractProc->nbProcMaxAlloc, nbProc, ExtractProc->pyrLevelMaxAlloc, OSC_NB_PYR
                    , ExtractProc->maxFrameRateAlloc, frameRateAngio);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscSetSequenceParameters", OSC_ERR_IMSIZEEXCEEDSALLOCATION, Reason);
		return OSC_ERR_IMSIZEEXCEEDSALLOCATION;
    }

    // Setting dimensions
	ExtractProc->IwOriRawAngio = IwAngio; ExtractProc->IhOriRawAngio = IhAngio;
	ExtractProc->IwOriAngio = IwOriAngio; 
	ExtractProc->IhOriAngio = IhOriAngio; 
	ExtractProc->IwProcAngio = IwProcAngio; 
	ExtractProc->IhProcAngio = IhProcAngio; 
    ExtractProc->SSFactor = SSFactor;
    ExtractProc->frameRateAngio = frameRateAngio;
    ExtractProc->CircBufferSize = (int)ceil(OSC_INTEGRATION_TIME_CIRC_BKG * ExtractProc->frameRateAngio);
    ExtractProc->NbProc = nbProc;
    ExtractProc->OffsetDiffSizeAngioFluoroXSub = 0;
    ExtractProc->OffsetDiffSizeAngioFluoroYSub = 0;

    if (ExtractProc->pyrLevelMaxAlloc < OSC_NB_PYR){
        int Err =  OscFreePyramidDim(This, ExtractSequence->PyrW, ExtractSequence->PyrH);
	    OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscFreePyramidDim", "OscSetSequenceParameters");

        ExtractProc->pyrLevelMaxAlloc = OSC_NB_PYR;
	    Err = OscGetPyramidDim(This, ExtractProc->pyrLevelMaxAlloc, IwProcAngio, IhProcAngio, &ExtractSequence->PyrW, &ExtractSequence->PyrH);
	    OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscGetPyramidDim", "OscSetSequenceParameters");
    }
    ExtractSequence->PyrNbBands = OSC_NB_PYR;
    ExtractSequence->PyrW[0] = IwProcAngio; ExtractSequence->PyrH[0] = IhProcAngio;
	for(Level = 1; Level <= ExtractSequence->PyrNbBands; Level++){
		ExtractSequence->PyrW[Level] = DOWN(ExtractSequence->PyrW[Level-1]);
		ExtractSequence->PyrH[Level] = DOWN(ExtractSequence->PyrH[Level-1]);
	}

    if (ExtractProc->ShuttersRoiPosGiven == 0){
	    ExtractProc->ShuttersRoiFullScale.XMin = 0; ExtractProc->ShuttersRoiFullScale.XMax = IwOriAngio-1;
	    ExtractProc->ShuttersRoiFullScale.YMin = 0; ExtractProc->ShuttersRoiFullScale.YMax = IhOriAngio-1;
    }

    OscReInitializeInternalParameters(&OscHdl);
    
	ExtractProc->TableInformationGiven				= 0;
	ExtractProc->CurrentTablePosXMMPlane			= 0;
	ExtractProc->CurrentTablePosYMMPlane			= 0;
	ExtractProc->CurrentPhysicalTablePosXMM			= 0;
	ExtractProc->CurrentPhysicalTablePosYMM			= 0;
	ExtractProc->CurrentPhysicalTablePosZMM			= 0;

    ExtractProc->nbThresholdsForMedianComputation   = (int)ceil(frameRateAngio * OSC_CLEAN_IMAGE_WISE_CAUSAL_THRESHOLD_CHECK_DURATION);
    
    ExtractProc->pool = pool;

    ExtractProc->tLimitForSWExtraction              = (int)ceil(frameRateAngio * OSC_SW_EXTRACTION_DURATION_S);
    
    if (SWSetThreadpool(&ExtractProc->SWHdl, pool) < 0){
        sprintf_s(Reason, 128, "SWSetThreadpool: Number of thread in pool above the max number of cores allocated for");
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscSetAngioSequenceParameters", OSC_ERR_SWLIB, Reason);
		return OSC_ERR_SWLIB;
    }

    return ErrorCodeOut;
}


int OscSetFluoroSequenceParameters(COscHdl OscHdl, int IwFluoro, int IhFluoro, float frameRateFluoro, IThreadPool *pool){
    
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
    COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
    char                 Reason[256];
    int                 ErrorCodeOut = 0, Level;
    int					pyrFactor = (int)pow(2.0, OSC_NB_PYR);

    int                 IwOriFluoro = (int)floor( ((double)IwFluoro / (ExtractProc->SSFactor * pyrFactor))) * ExtractProc->SSFactor * pyrFactor; 
    int                 IhOriFluoro = (int)floor( ((double)IhFluoro / (ExtractProc->SSFactor * pyrFactor))) * ExtractProc->SSFactor * pyrFactor; 
    int                 IwProcFluoro = IwOriFluoro / ExtractProc->SSFactor;
    int                 IhProcFluoro = IhOriFluoro / ExtractProc->SSFactor;
    int                 nbProc;
    
    pool->execute([&nbProc](int threadIndex, int threadCount){nbProc = threadCount;});  // Get number of threads

    //printf("Allocated info: Full res (%d, %d), Sub res (%d, %d), procMax %d pyrLevelMax %d frameRateMax %d \n"
    //    , ExtractProc->IwOriRawAlloc, ExtractProc->IhOriRawAlloc, ExtractProc->IwProcAlloc, ExtractProc->IhProcAlloc
    //    , ExtractProc->nbProcMaxAlloc, ExtractProc->pyrLevelMaxAlloc, ExtractProc->maxFrameRateAlloc);

    if ( (IwFluoro>ExtractProc->IwOriRawAlloc) || (IhFluoro>ExtractProc->IhOriRawAlloc) 
        || (IwProcFluoro>ExtractProc->IwProcAlloc)|| (IhProcFluoro>ExtractProc->IhProcAlloc)){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		sprintf_s(Reason, 256,"Image dimension higher than max allowed by allocation:\nFull res (%d, %d) vs (%d,%d)\nSub res (%d, %d) vs (%d,%d)\n"
                    , IwFluoro, IhFluoro, ExtractProc->IwOriRawAlloc, ExtractProc->IhOriRawAlloc
                    , IwProcFluoro, IhProcFluoro, ExtractProc->IwProcAlloc, ExtractProc->IhProcAlloc);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscSetSequenceParameters", OSC_ERR_IMSIZEEXCEEDSALLOCATION, Reason);
		return OSC_ERR_IMSIZEEXCEEDSALLOCATION;
    }
    if ( (nbProc > ExtractProc->nbProcMaxAlloc) || (OSC_NB_PYR > ExtractProc->pyrLevelMaxAlloc)
        || (frameRateFluoro > ExtractProc->maxFrameRateAlloc)  ){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		sprintf_s(Reason, 256,"Allocated memory insufficient: expecting %d proc max (needed %d), expecting %d pyr levels (needed %d), expecting frame rate %.1f (%.1f in fluoro)\n"
                    , ExtractProc->nbProcMaxAlloc, nbProc, ExtractProc->pyrLevelMaxAlloc, OSC_NB_PYR
                    , ExtractProc->maxFrameRateAlloc, frameRateFluoro);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscSetSequenceParameters", OSC_ERR_IMSIZEEXCEEDSALLOCATION, Reason);
		return OSC_ERR_IMSIZEEXCEEDSALLOCATION;
    }

    // Setting dimensions
	ExtractProc->IwOriRawFluoro = IwFluoro; ExtractProc->IhOriRawFluoro = IhFluoro;
	ExtractProc->IwOriFluoro = IwOriFluoro; 
	ExtractProc->IhOriFluoro = IhOriFluoro; 
	ExtractProc->IwProcFluoro = IwProcFluoro; 
	ExtractProc->IhProcFluoro = IhProcFluoro; 
    ExtractProc->frameRateFluoro = frameRateFluoro;
    ExtractProc->CircBufferSize = (int)ceil(OSC_INTEGRATION_TIME_CIRC_BKG * ExtractProc->frameRateAngio);
    ExtractProc->NbProc = nbProc;
    ExtractProc->OffsetDiffSizeAngioFluoroXSub = (ExtractProc->IwOriAngio-IwOriFluoro)/(2*ExtractProc->SSFactor);
    ExtractProc->OffsetDiffSizeAngioFluoroYSub = (ExtractProc->IhOriAngio-IhOriFluoro)/(2*ExtractProc->SSFactor);

    if (ExtractProc->pyrLevelMaxAlloc < OSC_NB_PYR){
        int Err =  OscFreePyramidDim(This, ExtractProc->PyrWFluoro, ExtractProc->PyrHFluoro);
	    OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscFreePyramidDim", "OscSetSequenceParameters");

        ExtractProc->pyrLevelMaxAlloc = OSC_NB_PYR;
	    Err = OscGetPyramidDim(This, ExtractProc->pyrLevelMaxAlloc, IwProcFluoro, IhProcFluoro, &ExtractProc->PyrWFluoro, &ExtractProc->PyrHFluoro);
	    OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscGetPyramidDim", "OscSetSequenceParameters");
    }
    ExtractSequence->PyrNbBands = OSC_NB_PYR;
    ExtractProc->PyrWFluoro[0] = IwProcFluoro; ExtractProc->PyrHFluoro[0] = IhProcFluoro;
	for(Level = 1; Level <= ExtractSequence->PyrNbBands; Level++){
		ExtractProc->PyrWFluoro[Level] = DOWN(ExtractProc->PyrWFluoro[Level-1]);
		ExtractProc->PyrHFluoro[Level] = DOWN(ExtractProc->PyrHFluoro[Level-1]);
	}

    if (ExtractProc->ShuttersRoiPosGiven == 0){
	    ExtractProc->ShuttersRoiFullScale.XMin = 0; ExtractProc->ShuttersRoiFullScale.XMax = IwOriFluoro-1;
	    ExtractProc->ShuttersRoiFullScale.YMin = 0; ExtractProc->ShuttersRoiFullScale.YMax = IhOriFluoro-1;
    }
    
	ExtractProc->TableInformationGiven				= 0;
	ExtractProc->CurrentTablePosXMMPlane			= 0;
	ExtractProc->CurrentTablePosYMMPlane			= 0;
	ExtractProc->CurrentPhysicalTablePosXMM			= 0;
	ExtractProc->CurrentPhysicalTablePosYMM			= 0;
	ExtractProc->CurrentPhysicalTablePosZMM			= 0;

    ExtractProc->pool = pool;
    
    if (SWSetThreadpool(&ExtractProc->SWHdl, pool) < 0){
        sprintf_s(Reason, 128, "SWSetThreadpool: Number of thread in pool above the max number of cores allocated for");
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscSetFluoroSequenceParameters", OSC_ERR_SWLIB, Reason);
		return OSC_ERR_SWLIB;
    }

    return ErrorCodeOut;
}

/*************************************************************************************/
// Convenience function to set the shutters ROI
/*************************************************************************************/
int OscSetShuttersRoi(COscHdl OscHdl, COscRoiCoord RoiCoord)
{	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

	ExtractProc->ShuttersRoiFullScale.XMin = OSC_MAX(RoiCoord.XMin, 0);
	ExtractProc->ShuttersRoiFullScale.XMax = OSC_MIN(RoiCoord.XMax, ExtractProc->IwOriAngio-1);
	ExtractProc->ShuttersRoiFullScale.YMin = OSC_MAX(RoiCoord.YMin, 0);
	ExtractProc->ShuttersRoiFullScale.YMax = OSC_MIN(RoiCoord.YMax, ExtractProc->IhOriAngio-1);

	return 0;
}


/*************************************************************************************/
// Set C-arm geometry
/*************************************************************************************/

int OscSetCurrentGeometryAngio	(COscHdl OscHdl, float Rot, float Ang, float pixSizeAtIsocenter){
	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

	ExtractProc->CArmAnglesAngioGiven = 1; ExtractProc->RotAngio = Rot; ExtractProc->AngAngio = Ang;
	ExtractProc->isoCenterPixSize = pixSizeAtIsocenter;

	return 0;
}

int OscSetCurrentGeometryFluoro	(COscHdl OscHdl, float Rot, float Ang, float pixSizeAtIsocenter){
	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

	ExtractProc->CArmAnglesFluoroGiven = 1; ExtractProc->RotFluoro = Rot; ExtractProc->AngFluoro = Ang;
	ExtractProc->isoCenterPixSize = pixSizeAtIsocenter;

	return 0;
}

/*************************************************************************************/
// Set table apparent position
/*************************************************************************************/
int OscSetTableApparentPositionMM	(COscHdl OscHdl, float TablePosXMM, float TablePosYMM, float TablePosZMM){
	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

	if (ExtractProc->TableInformationGiven >= 0)
		ExtractProc->TableInformationGiven				= 2;
	ExtractProc->CurrentPhysicalTablePosXMM			= TablePosXMM;
	ExtractProc->CurrentPhysicalTablePosYMM			= TablePosYMM;
	ExtractProc->CurrentPhysicalTablePosZMM			= TablePosZMM;

	return 0;
}

int OscSetTableApparentPositionPix	(COscHdl OscHdl, int TablePosXPix, int TablePosYPix){
	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

	if (ExtractProc->TableInformationGiven >= 0)
		ExtractProc->TableInformationGiven				= 1;
	ExtractProc->CurrentTablePosXPix	    			= TablePosXPix;
	ExtractProc->CurrentTablePosYPix    				= TablePosYPix;

	return 0;
}

int GetCurrentSystemInformationsAngio(COscHdl OscHdl, char systemInformations[512]){
    
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

    if (ExtractProc->TableInformationGiven == 0)
        sprintf_s(systemInformations, 512, "System info:\nRot %.2f Ang %.2f\npixSizeIso %.3f\nShutter: [%d,%d]x[%d,%d]\nNo table panning info\n"
                    , ExtractProc->RotAngio, ExtractProc->AngAngio, ExtractProc->isoCenterPixSize
                    , ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax
                    , ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax);

    if (ExtractProc->TableInformationGiven == 1)
        sprintf_s(systemInformations, 512, "System info:\nRot %.2f Ang %.2f\npixSizeIso %.3f\nShutter: [%d,%d]x[%d,%d]\nTable panning in mm %.2f %.2f\n"
                    , ExtractProc->RotAngio, ExtractProc->AngAngio, ExtractProc->isoCenterPixSize
                    , ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax
                    , ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax
                    , ExtractProc->CurrentTablePosXMMPlane, ExtractProc->CurrentTablePosYMMPlane);
    
    if (ExtractProc->TableInformationGiven == 2)
        sprintf_s(systemInformations, 512, "System info:\nRot %.2f Ang %.2f\npixSizeIso %.3f\nShutter: [%d,%d]x[%d,%d]\nTable panning in mm %.2f %.2f %.2f\n"
                    , ExtractProc->RotAngio, ExtractProc->AngAngio, ExtractProc->isoCenterPixSize
                    , ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax
                    , ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax
                    , ExtractProc->CurrentPhysicalTablePosXMM, ExtractProc->CurrentPhysicalTablePosYMM
                    , ExtractProc->CurrentPhysicalTablePosZMM);

    return 0;
}

int GetCurrentSystemInformationsFluoro(COscHdl OscHdl, char systemInformations[512]){
    
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

    if (ExtractProc->TableInformationGiven == 0)
        sprintf_s(systemInformations, 512, "System info fluoro:\nRot %.2f Ang %.2f\npixSizeIso %.3f\nShutter: [%d,%d]x[%d,%d]\nNo table panning info\n"
                    , ExtractProc->RotFluoro, ExtractProc->AngFluoro, ExtractProc->isoCenterPixSize
                    , ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax
                    , ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax);

    if (ExtractProc->TableInformationGiven == 1)
        sprintf_s(systemInformations, 512, "System info fluoro:\nRot %.2f Ang %.2f\npixSizeIso %.3f\nShutter: [%d,%d]x[%d,%d]\nTable panning in pix %.2f %.2f\n"
                    , ExtractProc->RotFluoro, ExtractProc->AngFluoro, ExtractProc->isoCenterPixSize
                    , ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax
                    , ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax
                    , ExtractProc->CurrentTablePosXMMPlane, ExtractProc->CurrentTablePosYMMPlane);
    
    if (ExtractProc->TableInformationGiven == 2)
        sprintf_s(systemInformations, 512, "System info fluoro:\nRot %.2f Ang %.2f\npixSizeIso %.3f\nShutter: [%d,%d]x[%d,%d]\nTable panning in mm %.2f %.2f %.2f\n"
                    , ExtractProc->RotFluoro, ExtractProc->AngFluoro, ExtractProc->isoCenterPixSize
                    , ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax
                    , ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax
                    , ExtractProc->CurrentPhysicalTablePosXMM, ExtractProc->CurrentPhysicalTablePosYMM
                    , ExtractProc->CurrentPhysicalTablePosZMM);

    return 0;
}

void OscSetInjectionDetectionThreshold(COscHdl OscHdl, float injectionCoeffThreshold){
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);

    ExtractSequence->injectionCoefficientThreshold = injectionCoeffThreshold;
}
