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
//#include "vld.h"

//#define MXLAB_FOR_CBD
//#define OSC_TIMING_CBD

#ifdef MXLAB_FOR_CBD
#include "Mxlab.h"
#endif

// ************************************************************************
// Rescale the ridge image, call the CBDLib routine, and compute the output

int OscCatheterBodyDetectionAndSubtraction(COscHdl OscHdl){
    
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	int					Err, ErrorCodeOut = 0;
    bool                imPotAlreadyComputed = false;

#ifdef MXLAB_FOR_CBD
    short * ImSubBeforeSub = (short*)malloc(ExtractProc->IwProcAngio*ExtractProc->IhProcAngio*sizeof(short));
    memcpy(ImSubBeforeSub, ExtractProc->ImSub, ExtractProc->IwProcAngio*ExtractProc->IhProcAngio*sizeof(short));
#endif
    
#ifdef OSC_TIMING_CBD
	double t0 = omp_get_wtime(), t1, t2, t3 , t4;
#endif

    if (ExtractProc->t == 0){
        ExtractProc->tCBDDetectionFrequency = (int)OSC_MAX(1,OSC_RND(ExtractProc->frameRateAngio * OSC_CBD_FREQUENCY_REDETECTION_CBD_ANGIO_S));
        ExtractProc->detectedCatheter = 0;//ExtractProc->CBDRes.detectedCath = 0;
    }
    
    if ( (ExtractProc->detectedCatheter == 0) && (ExtractProc->t % ExtractProc->tCBDDetectionFrequency == 0) ){
        // Detection
        Err = OscCatheterBodyDetection(OscHdl);
        OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscCatheterBodyDetection", "OscCatheterBodyDetectionAndSubtraction");

        imPotAlreadyComputed = true;
        ExtractProc->detectedCatheter = ExtractProc->CBDRes.detectedCath;
    }

#ifdef OSC_TIMING_CBD
	t1 = omp_get_wtime(); t2 = t1; t3 = t1; t4 = t1;
#endif
    
    if ( (ExtractProc->detectedCatheter) &&(!imPotAlreadyComputed) ){
        Err = OscComputePotentialImage(OscHdl);
        OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscComputePotentialImage", "OscCatheterBodyDetectionAndSubtraction");
    }
#ifdef OSC_TIMING_CBD
	t2 = omp_get_wtime();
#endif

    // Temporal tracking and tube parameters estimation
    int ShutterPos[4];
    ShutterPos[0] = ExtractProc->ShuttersRoiSafe.XMin; ShutterPos[1] = ExtractProc->ShuttersRoiSafe.XMax;
    ShutterPos[2] = ExtractProc->ShuttersRoiSafe.YMin; ShutterPos[3] = ExtractProc->ShuttersRoiSafe.YMax;
    if (ExtractProc->detectedCatheter){
        Err = CBDTrackAndComputeTubeModel(ExtractProc->ImSub, ExtractProc->ImCathPotential, ExtractProc->IwProcAngio
            , ExtractProc->IhProcAngio, ShutterPos, ExtractProc->isoCenterPixSize * ExtractProc->SSFactor
            , &ExtractProc->CBDHdl, &ExtractProc->CBDRes);
        OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "CBDTrackAndComputeTubeModel", "OscCatheterBodyDetectionAndSubtraction");
        
        ExtractProc->detectedCatheter = ExtractProc->CBDRes.detectedCath;   // In case the track is abandonned
    }

#ifdef OSC_TIMING_CBD
	t3 = omp_get_wtime();
#endif

    // Subtraction
    if (ExtractProc->detectedCatheter){
        Err = CBDSubtractCBDTube(ExtractProc->ImSub, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, ShutterPos
            , ExtractProc->isoCenterPixSize * ExtractProc->SSFactor, &ExtractProc->CBDHdl, &ExtractProc->CBDRes
            , ExtractProc->ImSub);
        OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "CBDSubtractCBDTube", "OscCatheterBodyDetectionAndSubtraction");
    }

#ifdef OSC_TIMING_CBD
	t4 = omp_get_wtime();
#endif

    // *** Display
#ifdef MXLAB_FOR_CBD
    if (ExtractProc->detectedCatheter){
        int yMin = ExtractProc->ShuttersRoiSafe.YMin, yMax = ExtractProc->ShuttersRoiSafe.YMax;

        int nbEltsToDisp = (int)ceil(yMax-yMin+1);
        float *xDisp = (float*)malloc(nbEltsToDisp*sizeof(float));
        for (int i=0; i<nbEltsToDisp; i++)
            xDisp[i] = CBDGetCBDCenterlineX(&ExtractProc->CBDHdl, &ExtractProc->CBDRes, (float)yMin+i);

        MxOpen(); MxSetVisible(1);
        MxPutMatrixShort(ImSubBeforeSub, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, "ImSub");
        MxPutVectorFloat(ExtractProc->CBDRes.detectedTrack->X, ExtractProc->CBDRes.detectedTrack->nbElts, "XCP");
        MxPutVectorFloat(ExtractProc->CBDRes.detectedTrack->Y, ExtractProc->CBDRes.detectedTrack->nbElts, "YCP");
        MxPutFloat((float)yMin, "yMin"); MxPutFloat((float)yMax, "yMax"); 
        MxPutFloat(CBDGetRadius(&ExtractProc->CBDHdl, &ExtractProc->CBDRes)/(ExtractProc->isoCenterPixSize * ExtractProc->SSFactor), "R");
        MxPutVectorFloat(xDisp, nbEltsToDisp, "xDisp");
        MxPutMatrixShort(ExtractProc->ImSub, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, "ImSubWithoutCBD");

        MxPutInt(ExtractProc->ShuttersRoiSafe.XMin, "xMinShut");
        MxPutInt(ExtractProc->ShuttersRoiSafe.XMax, "xMaxShut");
        MxPutInt(ExtractProc->ShuttersRoiSafe.YMin, "yMinShut");
        MxPutInt(ExtractProc->ShuttersRoiSafe.YMax, "yMaxShut");

        MxCommand("figure(1); subplot('Position',[0,0,0.33,1]); imagesc(ImSub(xMinShut:xMaxShut,yMinShut:yMaxShut)'); colormap gray; axis image; axis off;");
        MxCommand("hold on; plot(1+xDisp-R-double(xMinShut),1+(yMin:yMax)-double(yMinShut),'y-');");
        MxCommand("plot(1+xDisp+R-double(xMinShut),1+(yMin:yMax)-double(yMinShut),'y-');");
        if (ExtractProc->t == 0)
            MxCommand("plot(1+XCP-double(xMinShut),1+YCP-double(yMinShut),'bx');");
        MxCommand("hold off;");
        MxCommand("figure(1); subplot('Position',[0.335,0,0.33,1]); imagesc(ImSub(xMinShut:xMaxShut,yMinShut:yMaxShut)'); colormap gray; axis image; axis off;");
        MxCommand("figure(1); subplot('Position',[0.67,0,0.33,1]); imagesc(ImSubWithoutCBD(xMinShut:xMaxShut,yMinShut:yMaxShut)'); colormap gray; axis image; axis off;");

        free(xDisp);
    }
    free(ImSubBeforeSub);
#endif
    
#ifdef OSC_TIMING_CBD
    printf("Time to detect: %.1f ms, to compute potential image %.1f ms, to track and compute tube model %.1f ms, to subtract %.1f ms\n"
            , 1000.0 * (t1-t0), 1000.0 * (t2-t1), 1000.0 * (t3-t2), 1000.0 * (t4-t3));
#endif

    return 0;
}

// ******************************* Detection *****************************************
int OscCatheterBodyDetection(COscHdl OscHdl){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
    int					Iw = ExtractProc->IwProcAngio, Ih = ExtractProc->IhProcAngio;
	int					Err;

    // *** Initializations
    Err = CDBDefaultParameters(&This->ExtractProc.CBDParams, Iw, CBD_ANGIO);
    if (Err){
        ExtractProc->FatalErrorOnPreviousImage = 1;
	    ErrPrintErrorReport(ExtractProc->CBDHdl.ErrHdl);
	    ErrStartErrorReportFromLeaf(This->ErrHdl, "OscCatheterBodyDetection", OSC_ERR_CBDLIB, "Error in CBDHdl");
	    return OSC_ERR_CBDLIB;
    }

    // *** Build potential image
    OscComputePotentialImage(OscHdl);

    // *** Call library to track and identify a possible catheter body
    int ShutterPosInit[4];
    ShutterPosInit[0] = ExtractProc->ShuttersRoi.XMin; ShutterPosInit[1] = ExtractProc->ShuttersRoi.XMax;
    ShutterPosInit[2] = ExtractProc->ShuttersRoi.YMin; ShutterPosInit[3] = ExtractProc->ShuttersRoi.YMax;
    CBDTrackerInitialization(ExtractProc->ImCathPotential, Iw, Ih, ShutterPosInit, &ExtractProc->CBDParams, &ExtractProc->CBDHdl);

    Err = CatheterTracking(ExtractProc->RotAngio, ExtractProc->AngAngio, Iw, Ih, &ExtractProc->CBDHdl, &ExtractProc->CBDRes);
    if (Err){
        ExtractProc->FatalErrorOnPreviousImage = 1;
	    ErrPrintErrorReport(ExtractProc->CBDHdl.ErrHdl);
	    ErrStartErrorReportFromLeaf(This->ErrHdl, "OscCatheterBodyDetection", OSC_ERR_CBDLIB, "Error in CBDHdl");
	    return OSC_ERR_CBDLIB;
    }
    return 0;
}

// ******************************* *****************************************
int OscComputePotentialImage(COscHdl OscHdl){
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	float				*localRdg, *localDir, *localTmpRdg;
	short				*localGau;
	float				sigmaRdg;
    int					Iw = ExtractProc->IwProcAngio, Ih = ExtractProc->IhProcAngio;
	COscRoiCoord		ShutterPos;
	int					Err, ErrorCodeOut = 0;

    ShutterPos.XMin = (int)(OSC_MAX(ExtractProc->ShuttersRoiSafe.XMin, ExtractProc->CBDParams.trackerRingRadius+1) );
    ShutterPos.XMax = (int)(OSC_MIN(ExtractProc->ShuttersRoiSafe.XMax, Iw-1-ExtractProc->CBDParams.trackerRingRadius-1) );
    ShutterPos.YMin = (int)(OSC_MAX(ExtractProc->ShuttersRoiSafe.YMin, ExtractProc->CBDParams.trackerRingRadius+1) );
    ShutterPos.YMax = (int)(OSC_MIN(ExtractProc->ShuttersRoiSafe.YMax, Ih-1-ExtractProc->CBDParams.trackerRingRadius-1) );

    // *** Build potential image

    // Based on proper scale ridge filtering
    localRdg = ExtractProc->BufsF[0]; localDir = ExtractProc->BufsF[1]; localTmpRdg = ExtractProc->BufsF[2]; localGau = ExtractProc->BufsS[0];
    sigmaRdg = ExtractProc->KernelSigmas[OSC_MIN(1,ExtractProc->nKernelSigs)];
    OscRdgMonoScaleRidge(ExtractProc->ImSub, localGau, localRdg, localDir, Iw, Ih, &ExtractProc->ShuttersRoi, sigmaRdg, localTmpRdg);

    // Normalize potential image
    Err = OscImDynamicsAdjust(This, localRdg, Iw, Ih, &ShutterPos, 1-ExtractProc->CBDParams.percentileL, 1-ExtractProc->CBDParams.percentileH
 						    , ExtractProc->BufsF[1], ExtractProc->ImCathPotential);
    OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscImDynamicsAdjust", "OscCatheterBodyDetection");

    return Err;
}

bool OscGetEstimatedCatheterBodyLimitsX(COscHdl OscHdl, float y, float *xCT, float *R){
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

    if (ExtractProc->detectedCatheter){

        *xCT = ExtractProc->SSFactor * CBDGetCBDCenterlineX(&ExtractProc->CBDHdl, &ExtractProc->CBDRes, y / ExtractProc->SSFactor);
        *R = CBDGetRadius(&ExtractProc->CBDHdl, &ExtractProc->CBDRes) / ExtractProc->isoCenterPixSize;
        return true;
    } else
        return false;
}

