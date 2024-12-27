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

#ifdef OSC_MXLAB_SHOW_SUBRES
#include "MxLab.h"
#endif

// ********************************************************************
// Call the DiaLib routine

int OscSternalWireDetection(COscHdl OscHdl, short *ImOri){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
    char                Reason[128];
    int                 Err = 0;

    if (ExtractProc->t == 0){
        ExtractProc->SWRes.nbPixMask = 0;
        ExtractProc->deltaTabXSubPrev = 0;
        ExtractProc->deltaTabYSubPrev = 0;
    }

    if (ExtractProc->t <= ExtractProc->tLimitForSWExtraction){
        CSWRoiCoord RoiCoord;
        RoiCoord.XMin = ExtractProc->ShuttersRoiFullScale.XMin;
        RoiCoord.XMax = ExtractProc->ShuttersRoiFullScale.XMax;
        RoiCoord.YMin = ExtractProc->ShuttersRoiFullScale.YMin;
        RoiCoord.YMax = ExtractProc->ShuttersRoiFullScale.YMax;

        Err = SWExtract(&ExtractProc->SWHdl, (const short*)ImOri, ExtractProc->IwOriAngio, ExtractProc->IhOriAngio, &RoiCoord, ExtractProc->t, ExtractProc->isoCenterPixSize, true, &ExtractProc->SWRes);
        if (Err == -1){
		    sprintf_s(Reason, 128,"Image size not compatible with allocated image sizes in SWLib");
		    ErrStartErrorReportFromLeaf(This->ErrHdl, "SWExtract", OSC_ERR_SWLIB, Reason);
		    return OSC_ERR_SWLIB;
        }
        if (Err == -2){
		    sprintf_s(Reason, 128,"Image size has changed wrt to previous extraction times %d", ExtractProc->t);
		    ErrStartErrorReportFromLeaf(This->ErrHdl, "SWExtract", OSC_ERR_SWLIB, Reason);
		    return OSC_ERR_SWLIB;
        }
    } else {
        if (ExtractProc->SWRes.nbPixMask > 0){
            CSWRoiCoord RoiCoord;
            RoiCoord.XMin = ExtractProc->ShuttersRoiFullScale.XMin;
            RoiCoord.XMax = ExtractProc->ShuttersRoiFullScale.XMax;
            RoiCoord.YMin = ExtractProc->ShuttersRoiFullScale.YMin;
            RoiCoord.YMax = ExtractProc->ShuttersRoiFullScale.YMax;
            
            float deltaXMM = 0.f, deltaYMM = 0.f;
            if ( (ExtractProc->tableInfoFromtheStart) && (ExtractProc->TableInformationGiven > 0) ){
                deltaXMM = (ExtractProc->deltaTabXSub-ExtractProc->lastDeltaXSubAngio) * ExtractProc->SSFactor * ExtractProc->isoCenterPixSize;
                deltaYMM = (ExtractProc->deltaTabYSub-ExtractProc->lastDeltaYSubAngio) * ExtractProc->SSFactor * ExtractProc->isoCenterPixSize;
            }

            Err = SWTrackingWithPanningInfo(&ExtractProc->SWHdl, (const short*)ImOri, ExtractProc->IwOriAngio, ExtractProc->IhOriAngio, &RoiCoord
                                        , ExtractProc->t, ExtractProc->isoCenterPixSize, deltaXMM, deltaYMM, &ExtractProc->SWRes);
            if (Err == -1){
		        sprintf_s(Reason, 128,"Image size not compatible with allocated image sizes in SWLib");
		        ErrStartErrorReportFromLeaf(This->ErrHdl, "SWTracking", OSC_ERR_SWLIB, Reason);
		        return OSC_ERR_SWLIB;
            }
            if (Err == -2){
		        sprintf_s(Reason, 128,"Image size has changed before tracking");
		        ErrStartErrorReportFromLeaf(This->ErrHdl, "SWTracking", OSC_ERR_SWLIB, Reason);
		        return OSC_ERR_SWLIB;
            }
        }
    }
    ExtractProc->deltaTabXSubPrev = ExtractProc->deltaTabXSub;
    ExtractProc->deltaTabYSubPrev = ExtractProc->deltaTabYSub;

	return 0;
}

// Stiches removal
void OscRdgSWRemoval(COscHdl OscHdl, short *ImOri, unsigned char angioFlag){

	COsc             *This = (COsc*)OscHdl.Pv;
	COscExtractProc  *ExtractProc = &(This->ExtractProc);
	int				SSFactor = ExtractProc->SSFactor, IwOri, IhOri, IwProc;

    if (angioFlag){
        IwOri = ExtractProc->IwOriAngio; IhOri = ExtractProc->IhOriAngio;
        IwProc = ExtractProc->IwProcAngio;
    } else {
        IwOri = ExtractProc->IwOriFluoro; IhOri = ExtractProc->IhOriFluoro;
        IwProc = ExtractProc->IwProcFluoro;
    }

	if (ExtractProc->SWRes.nbPixMask > 0){
		int x, y, xx, yy, xMin, xMax, yMin, yMax;
        
		// Deletion
		xMin = OSC_MAX(ExtractProc->ShuttersRoiSafe.XMin, OSC_SW_SIZE_DIL);
		xMax = OSC_MIN(ExtractProc->ShuttersRoiSafe.XMax, (IwOri-1)/SSFactor-OSC_SW_SIZE_DIL);
		yMin = OSC_MAX(ExtractProc->ShuttersRoiSafe.YMin, OSC_SW_SIZE_DIL);
		yMax = OSC_MIN(ExtractProc->ShuttersRoiSafe.YMax, (IhOri-1)/SSFactor-OSC_SW_SIZE_DIL);
        
	    for (y=yMin; y<=yMax; y++)
		    for (x=xMin; x<=xMax; x++)

			    for (yy = SSFactor*(y-OSC_SW_SIZE_DIL); yy<=SSFactor*(y+OSC_SW_SIZE_DIL); yy++)
				    for (xx = SSFactor*(x-OSC_SW_SIZE_DIL); xx<=SSFactor*(x+OSC_SW_SIZE_DIL); xx++)
                        if (ExtractProc->SWRes.SWMask[yy*IwOri+xx]){
						    ExtractProc->ImRdg[y*IwProc+x]		= 0;
					    }
	}
}


int OscRdgSWTrackingAndRemovalFluoro(COscHdl OscHdl, short *ImOri){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
    int                 Err = 0;

    if (ExtractProc->SWRes.nbPixMask > 0){
        
        char Reason[128];
        CSWRoiCoord RoiCoord;
        RoiCoord.XMin = ExtractProc->ShuttersRoiFullScale.XMin;
        RoiCoord.XMax = ExtractProc->ShuttersRoiFullScale.XMax;
        RoiCoord.YMin = ExtractProc->ShuttersRoiFullScale.YMin;
        RoiCoord.YMax = ExtractProc->ShuttersRoiFullScale.YMax;

        if (ExtractProc->t == 0){
            if ( (ExtractProc->IwOriAngio != ExtractProc->IwOriFluoro) || (ExtractProc->IhOriAngio != ExtractProc->IhOriFluoro) ){
                Err = SWTranslateMaskToDifferentImageSize(&ExtractProc->SWHdl, ExtractProc->IwOriFluoro, ExtractProc->IhOriFluoro);
                if (Err == -1){
		            sprintf_s(Reason, 128,"New fluoro size not compatible with allocated image sizes in SWLib");
		            ErrStartErrorReportFromLeaf(This->ErrHdl, "SWTranslateMaskToDifferentImageSize", OSC_ERR_SWLIB, Reason);
		            return OSC_ERR_SWLIB;
                }
            }

            float deltaXMM = 0.f, deltaYMM = 0.f;
            if ( (ExtractProc->tableInfoFromtheStart) && (ExtractProc->TableInformationGiven > 0) ){
                deltaXMM = (ExtractProc->deltaTabXSub-ExtractProc->lastDeltaXSubAngio) * ExtractProc->SSFactor * ExtractProc->isoCenterPixSize;
                deltaYMM = (ExtractProc->deltaTabYSub-ExtractProc->lastDeltaYSubAngio) * ExtractProc->SSFactor * ExtractProc->isoCenterPixSize;
            }

            Err = SWRepositionMask(&ExtractProc->SWHdl, (const short*)ImOri, ExtractProc->IwOriFluoro, ExtractProc->IhOriFluoro, &RoiCoord, ExtractProc->isoCenterPixSize
                                   , deltaXMM, deltaYMM, OSC_SW_MV_REINIT_SEARCH_RANGE_X_MM, OSC_SW_MV_REINIT_SEARCH_RANGE_Y_MM, &ExtractProc->SWRes);
            if (Err == -1){
		        sprintf_s(Reason, 128,"Image size not compatible with allocated image sizes in SWLib");
		        ErrStartErrorReportFromLeaf(This->ErrHdl, "SWRepositionMask", OSC_ERR_SWLIB, Reason);
		        return OSC_ERR_SWLIB;
            }
            if (Err == -2){
		        sprintf_s(Reason, 128,"Image size has changed before tracking");
		        ErrStartErrorReportFromLeaf(This->ErrHdl, "SWRepositionMask", OSC_ERR_SWLIB, Reason);
		        return OSC_ERR_SWLIB;
            }
        } else {

            float deltaXMM = 0.f, deltaYMM = 0.f;
            if ( (ExtractProc->tableInfoFromtheStart) && (ExtractProc->TableInformationGiven > 0) ){
                deltaXMM = (ExtractProc->deltaTabXSub-ExtractProc->deltaTabXSubPrev) * ExtractProc->SSFactor * ExtractProc->isoCenterPixSize;
                deltaYMM = (ExtractProc->deltaTabYSub-ExtractProc->deltaTabYSubPrev) * ExtractProc->SSFactor * ExtractProc->isoCenterPixSize;
            }

            Err = SWTrackingWithPanningInfo(&ExtractProc->SWHdl, (const short*)ImOri, ExtractProc->IwOriFluoro, ExtractProc->IhOriFluoro, &RoiCoord, ExtractProc->t
                                            , ExtractProc->isoCenterPixSize, deltaXMM, deltaYMM, &ExtractProc->SWRes);
            if (Err == -1){
		        sprintf_s(Reason, 128,"Image size not compatible with allocated image sizes in SWLib");
		        ErrStartErrorReportFromLeaf(This->ErrHdl, "SWTracking", OSC_ERR_SWLIB, Reason);
		        return OSC_ERR_SWLIB;
            }
            if (Err == -2){
		        sprintf_s(Reason, 128,"Image size has changed before tracking");
		        ErrStartErrorReportFromLeaf(This->ErrHdl, "SWTracking", OSC_ERR_SWLIB, Reason);
		        return OSC_ERR_SWLIB;
            }
        }
        OscRdgSWRemoval(OscHdl, ImOri, 0);

        ExtractProc->deltaTabXSubPrev = ExtractProc->deltaTabXSub;
        ExtractProc->deltaTabYSubPrev = ExtractProc->deltaTabYSub;
    }
    return Err;
}
