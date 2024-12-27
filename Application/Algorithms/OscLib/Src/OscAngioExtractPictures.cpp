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


#include <stdio.h>
#include <OscDev.h>

//#define OSC_TIMING 
//#define OSC_TIMING_DETAIL
//#define OSC_MXLAB_SHOW_SUBRES

#ifdef OSC_MXLAB_SHOW_SUBRES
#include "MxLab.h"
#endif

/*************************************************************************************/
// Angio Picture Extraction Process
// 
/*************************************************************************************/
int OscAngioProcessPicture (COscHdl OscHdl, short* InIma, int t, COscAngioPictureResults*  ProcessResults)
{
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	int                  ErrorCode, ErrorCodeOut = 0, IwOri, IhOri, IwProc, IhProc;
	short				*WIm;

#ifdef OSC_TIMING
	double t0 = omp_get_wtime(), t1;
#endif
#ifdef OSC_TIMING_DETAIL
	double tAfterSubsample, tAfterDiaDetec, tAfterRidge, tAfterSWRemoval, tAfterCBDDetection
			, tAfterSWDetection, tAfterBkgComputation, tAfterHysteresisComputation;
#endif


	// *** Initial tests
	ErrorCode = OscAngioTestsDataConsistency(OscHdl, InIma, t);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscAngioTestsDataConsistency", "OscAngioProcessPicture");
    if (ErrorCode == OSC_WNG_TOO_LONG_ANGIO)
        return ErrorCode;

	// *** 

	// Image subsampling (by minimum)
	WIm = ExtractProc->ImOriWorking;
	IwOri = ExtractProc->IwOriAngio; IhOri = ExtractProc->IhOriAngio; IwProc = ExtractProc->IwProcAngio; IhProc = ExtractProc->IhProcAngio;
	ErrorCode = OscSubSampleImageMin(OscHdl, WIm, IwOri, IhOri, ExtractProc->ImSub, IwProc, IhProc, ExtractProc->SSFactor);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscSubSampleImageMin", "OscAngioProcessPicture");

#ifdef OSC_TIMING_DETAIL
	tAfterSubsample = omp_get_wtime();
#endif

	// Catheter body detection and subtraction
	ErrorCode =  OscCatheterBodyDetectionAndSubtraction(OscHdl); 
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscCatheterBodyDetectionAndSubtraction", "OscAngioProcessPicture");
#ifdef OSC_TIMING_DETAIL
	tAfterCBDDetection = omp_get_wtime();
#endif

	// Potential diaphragm detection
	ErrorCode = OscDiaphragmDetection(OscHdl, WIm);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscDiaphragmDetectionAngio", "OscAngioProcessPicture");
	
#ifdef OSC_TIMING_DETAIL
	tAfterDiaDetec = omp_get_wtime();
#endif

	// Sternal wire detection
	ErrorCode = OscSternalWireDetection(OscHdl, WIm);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscSternalWireDetection", "OscAngioProcessPicture");
	
#ifdef OSC_TIMING_DETAIL
	tAfterSWDetection = omp_get_wtime();
#endif

	// Ridges filtering (with potential diaphragm correction)
	OscRdgMultiScaleRidge(OscHdl, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio);

#ifdef OSC_TIMING_DETAIL
	tAfterRidge = omp_get_wtime();
#endif

	//// Track and erase potential sternal wires
	OscRdgSWRemoval(OscHdl, WIm, 1);
	
#ifdef OSC_TIMING_DETAIL
	tAfterSWRemoval = omp_get_wtime();
#endif
    
	// Background subtraction
	OscRdgInitialWarpedBackgroundSubtraction(OscHdl);
	
#ifdef OSC_TIMING_DETAIL
	tAfterBkgComputation = omp_get_wtime();
#endif
    
	// Hysteresis thresholding
	ErrorCode = OscRdgAngioHysteresisLabeling(OscHdl);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscRdgAngioHysteresisLabeling", "OscAngioProcessPicture");

#ifdef OSC_MXLAB_SHOW_SUBRES
    MxOpen(); MxSetVisible(1); MxCommand("figure(1);");
    MxPutMatrixShort(InIma, ExtractProc->IwOriRawAngio, ExtractProc->IhOriRawAngio, "InIma");
    MxCommand("subplot('Position',[0,0.5,0.25,0.5]); imagesc(InIma'); colormap gray; axis image; axis off;");
    MxPutMatrixShort(ExtractProc->ImOriWorking, ExtractProc->IwOriAngio, ExtractProc->IhOriAngio, "WIm");
    MxCommand("subplot('Position',[0.25,0.5,0.25,0.5]); imagesc(WIm'); colormap gray; axis image; axis off;");
    MxPutMatrixShort(ExtractProc->ImSub, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, "ImSub");
    MxCommand("subplot('Position',[0.5,0.5,0.25,0.5]); imagesc(ImSub'); colormap gray; axis image; axis off;");
    MxPutMatrixFloat(ExtractProc->ImRdg, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, "ImRdg");
    MxCommand("subplot('Position',[0.75,0.5,0.25,0.5]); imagesc(ImRdg'); colormap gray; axis image; axis off;");
    MxPutMatrixFloat(ExtractProc->ImFrg, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, "ImFrg");
    MxCommand("subplot('Position',[0,0,0.25,0.5]); imagesc(ImFrg'); colormap gray; axis image; axis off;");
    MxPutMatrixUC(ExtractProc->ImSee, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, "ImSee");
    MxCommand("subplot('Position',[0.25,0,0.25,0.5]); imagesc(ImSee'); colormap gray; axis image; axis off;");
    MxPutMatrixUC(ExtractProc->ImThr, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, "ImThr");
    MxCommand("subplot('Position',[0.5,0,0.25,0.5]); imagesc(ImThr'); colormap gray; axis image; axis off;");
    MxPutMatrixUC(ExtractProc->ImMask, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, "ImMask");
    MxCommand("subplot('Position',[0.75,0,0.25,0.5]); imagesc(ImMask'); colormap gray; axis image; axis off;");
#endif
	
#ifdef OSC_TIMING_DETAIL
	tAfterHysteresisComputation = omp_get_wtime();
#endif

	// *** Clean vasculature
	ErrorCode = OscRdgCleanVasculatureImage(OscHdl);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscRdgCleanVasculatureImage", "OscAngioProcessPicture");

	// *** Copy results
	ErrorCode = OscStoreExtractedPicture(This);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscStoreExtractedPicture", "OscAngioProcessPicture");

    ExtractProc->lastDeltaXSubAngio = ExtractProc->deltaTabXSub;
    ExtractProc->lastDeltaYSubAngio = ExtractProc->deltaTabYSub;

	OscFillAngioPictureResults(ProcessResults, ExtractProc);

#ifdef OSC_TIMING
	t1 = omp_get_wtime();
	printf("Time to process angio image %d: %.1f ms\n", t, 1000.0 * (t1-t0));
#endif
#ifdef OSC_TIMING_DETAIL
	printf("     (Subsampling %.1f CBD detection %.1f DiaDetection %.1f  SW detection %.1f )\n", 1000.0 * (tAfterSubsample-t0), 1000.0 * (tAfterCBDDetection-tAfterSubsample)
		, 1000.0 * (tAfterDiaDetec-tAfterCBDDetection), 1000.0 * (tAfterSWDetection-tAfterDiaDetec));
	printf("     (Ridge computation %.1f SW removal %.1f )\n", 1000 * (tAfterRidge-tAfterSWDetection)
		, 1000.0 * (tAfterSWRemoval-tAfterRidge));
	printf("     (Bkg computation %.1f Hysteresis %.1f Vasc cleaning %.1f)\n"
		, 1000.0 * (tAfterBkgComputation-tAfterSWDetection), 1000.0 * (tAfterHysteresisComputation-tAfterBkgComputation), 1000.0 * (t1-tAfterHysteresisComputation));
#endif

	return ErrorCodeOut;
}

/*************************************************************************************/

int OscAngioTestsDataConsistency(COscHdl OscHdl, short* InIma, int t){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	char                 Reason[128];
	int					Err = 0;

    if (t == 0)
        ExtractProc->FatalErrorOnPreviousImage = 0;

    if (ExtractProc->FatalErrorOnPreviousImage){
		sprintf_s(Reason, 128,"OscAngioProcessPicture: error reported on previous frame");
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioTestsDataConsistency", OSC_ERR_PREVIOUS_FATAL_ERROR, Reason);
		return OSC_ERR_PREVIOUS_FATAL_ERROR;
    }
    
	if (t >= ExtractProc->maxAngioLengthAlloc){
		sprintf_s(Reason, 128,"Angio sequence is longer than the max allowed length %d", ExtractProc->maxAngioLengthAlloc);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioTestsDataConsistency", OSC_WNG_TOO_LONG_ANGIO, Reason);
		return OSC_WNG_TOO_LONG_ANGIO;
	}

	if (t!= ExtractProc->t+1){
		sprintf_s(Reason, 128,"OscAngioProcessPicture called at non-incremental times %d and %d : Extract aborted", ExtractProc->t, t);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioTestsDataConsistency", OSC_ERR_INCORRECT_TIME, Reason);
		return OSC_ERR_INCORRECT_TIME;
	}


	if ( (t>0) && (ExtractProc->CArmAnglesAngioGiven) && (ExtractProc->CArmAnglesAngioGivenRef) && 
		( (ExtractProc->RotAngio != ExtractProc->RotAngioRef) || (ExtractProc->AngAngio != ExtractProc->AngAngioRef) ) ){
		sprintf_s(Reason, 128,"C-arm angulation changed from (Rot %.3f, Ang %.3f) to (Rot %.3f, Ang %.3f)", ExtractProc->RotAngio, ExtractProc->AngAngio
					, ExtractProc->RotAngioRef, ExtractProc->AngAngioRef);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioTestsDataConsistency", OSC_INFO_CARM_MOVED_INSEQ, Reason);
		Err = OSC_INFO_CARM_MOVED_INSEQ;
	}
	if (ExtractProc->CArmAnglesAngioGiven){
		ExtractProc->CArmAnglesAngioGivenRef = 1; ExtractProc->RotAngioRef = ExtractProc->RotAngio; ExtractProc->AngAngioRef = ExtractProc->AngAngio;
	}

	if ( (t > 0) && (OSC_ABS_F(ExtractProc->isoCenterPixSize-ExtractProc->isoCenterPixSizeRef) > 0.0001f) ){
		sprintf_s(Reason, 128,"Given pixSizeAtIsocenter has changed from %.3f to %.3f", ExtractProc->isoCenterPixSize, ExtractProc->isoCenterPixSizeRef);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioTestsDataConsistency", OSC_INFO_PIXSIZE_CHANGED, Reason);
		Err = OSC_INFO_PIXSIZE_CHANGED;
	}
	if (ExtractProc->isoCenterPixSize <= 0){
		sprintf_s(Reason, 128,"Pixel size at isocenter has not been informed");
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioTestsDataConsistency", OSC_ERR_SYSTEM_INFO_MISSING, Reason);
		return OSC_ERR_SYSTEM_INFO_MISSING;
	} else {
		ExtractProc->isoCenterPixSizeRef = ExtractProc->isoCenterPixSize;
	}

	if ((ExtractProc->ShuttersRoiFullScale.XMin < 0) || (ExtractProc->ShuttersRoiFullScale.XMax >= ExtractProc->IwOriAngio) 
		|| (ExtractProc->ShuttersRoiFullScale.YMin < 0) || (ExtractProc->ShuttersRoiFullScale.YMax >= ExtractProc->IhOriAngio)){
		sprintf_s(Reason, 128,"Informed shutter [%d,%d]x[%d,%d] incompatible with image size [0,%d]x[0,%d]", ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax
			, ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax, ExtractProc->IwOriAngio-1, ExtractProc->IhOriAngio-1);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioTestsDataConsistency", OSC_ERR_INCORRECT_SHUTTERINFO, Reason);
		return OSC_ERR_INCORRECT_SHUTTERINFO;
	}
	if ((ExtractProc->ShuttersRoiFullScale.XMin >= ExtractProc->ShuttersRoiFullScale.XMax ) || (ExtractProc->ShuttersRoiFullScale.YMin >= ExtractProc->ShuttersRoiFullScale.YMax)){
		sprintf_s(Reason, 128,"Incoherent shutter info: min > max (xmin %d xmax %d, ymin %d ymax %d)"
			, ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax, ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioTestsDataConsistency", OSC_ERR_INCORRECT_SHUTTERINFO, Reason);
		return OSC_ERR_INCORRECT_SHUTTERINFO;
	}
    if (ExtractProc->ShuttersRoiFullScale.XMax - ExtractProc->ShuttersRoiFullScale.XMin < OSC_MIN_PIXSIZE_EACHDIMENSION
        || ExtractProc->ShuttersRoiFullScale.YMax - ExtractProc->ShuttersRoiFullScale.YMin < OSC_MIN_PIXSIZE_EACHDIMENSION){
        sprintf_s(Reason, 128,"Informed shutter [%d,%d]x[%d,%d] results in a too small image amplitude in one of the direction at least", ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax
			, ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioTestsDataConsistency", OSC_ERR_INCORRECT_SHUTTERINFO, Reason);
		return OSC_ERR_INCORRECT_SHUTTERINFO;
    }

	if ( (ExtractProc->ShuttersRoiPosGiven) &&
			( (ExtractProc->ShuttersRoiFullScale.XMin != ExtractProc->ShuttersRoiFullScaleRef.XMin) || (ExtractProc->ShuttersRoiFullScale.XMax != ExtractProc->ShuttersRoiFullScaleRef.XMax)
			|| (ExtractProc->ShuttersRoiFullScale.YMin != ExtractProc->ShuttersRoiFullScaleRef.YMin) || (ExtractProc->ShuttersRoiFullScale.XMax != ExtractProc->ShuttersRoiFullScaleRef.YMax) ) ){	// Shutter position was already given
		sprintf_s(Reason, 128,"Shutter position modified from [%d,%d]x[%d,%d] to [%d,%d]w[%d,%d]", ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax
				, ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax
				, ExtractProc->ShuttersRoiFullScaleRef.XMin, ExtractProc->ShuttersRoiFullScaleRef.XMax
				, ExtractProc->ShuttersRoiFullScaleRef.YMin, ExtractProc->ShuttersRoiFullScaleRef.YMax);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioTestsDataConsistency", OSC_INFO_SHUTTERINFO_CHANGE, Reason);
		Err = OSC_INFO_SHUTTERINFO_CHANGE;
	}
	ExtractProc->ShuttersRoiFullScaleRef.XMin = ExtractProc->ShuttersRoiFullScale.XMin;
	ExtractProc->ShuttersRoiFullScaleRef.XMax = ExtractProc->ShuttersRoiFullScale.XMax;
	ExtractProc->ShuttersRoiFullScaleRef.YMin = ExtractProc->ShuttersRoiFullScale.YMin;
	ExtractProc->ShuttersRoiFullScaleRef.YMax = ExtractProc->ShuttersRoiFullScale.YMax;
    
	ExtractProc->t = t;
    
	// Inits / allocations at t=0
	if (t==0){

		if (ExtractSequence->PyrNbBands != OSC_NB_PYR){
			//// Need to allocate memory with buffers of the correct size
            ExtractProc->FatalErrorOnPreviousImage = 1;
		    sprintf_s(Reason, 128,"Nb pyramid levels (%d to %d) has been changed after OscHdl has been created. Please reallocate memory with new parameters\n"
                        , ExtractSequence->PyrNbBands, OSC_NB_PYR);
		    ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioTestsDataConsistency", OSC_ERR_CRITICAL_PARA, Reason);
		    return OSC_ERR_CRITICAL_PARA;
		}

		ExtractProc->detectedDiaphragm	= 0;
		ExtractProc->detectedCatheter	= 0;

        ExtractProc->AngAngio0 = ExtractProc->AngAngio; ExtractProc->RotAngio0 = ExtractProc->RotAngio;
	}

	// *** Inits at any time
	ExtractProc->angioProcessed = 0;

	// Potential image panning
/*	if ( (ExtractProc->IwOriRaw == ExtractProc->IwOri) && (ExtractProc->IhOriRaw == ExtractProc->IhOri) )
		ExtractProc->ImOriWorking = InIma;
	else*/ 
    {
		int x, y;
		// padding
		for (y=0; y<ExtractProc->IhOriRawAngio; y++){
		//	memcpy(&(ExtractProc->ImOriWorking[y*ExtractProc->IwOri]), &(InIma[y*ExtractProc->IwOriRaw])
		//				, ExtractProc->IwOriRaw*sizeof(short));
			for (x=0; x<ExtractProc->IwOriRawAngio; x++)
				ExtractProc->ImOriWorking[y*ExtractProc->IwOriAngio+x] = InIma[y*ExtractProc->IwOriRawAngio + x];
			for (x=ExtractProc->IwOriRawAngio; x<ExtractProc->IwOriAngio; x++)
				ExtractProc->ImOriWorking[y*ExtractProc->IwOriAngio+x] = InIma[y*ExtractProc->IwOriRawAngio + ExtractProc->IwOriRawAngio-1];
		}
		for (y=ExtractProc->IhOriRawAngio; y<ExtractProc->IhOriAngio; y++){
			//memcpy(&(ExtractProc->ImOriWorking[y*ExtractProc->IwOri]), &(InIma[(ExtractProc->IhOriRaw-1)*ExtractProc->IwOriRaw])
			//			, ExtractProc->IwOriRaw*sizeof(short));
			for (x=0; x<ExtractProc->IwOriRawAngio; x++)
				ExtractProc->ImOriWorking[y*ExtractProc->IwOriAngio+x] = InIma[(ExtractProc->IhOriRawAngio-1)*ExtractProc->IwOriRawAngio+x];
			for (x=ExtractProc->IwOriRawAngio; x<ExtractProc->IwOriAngio; x++)
				ExtractProc->ImOriWorking[y*ExtractProc->IwOriAngio+x] = InIma[ExtractProc->IhOriRawAngio*ExtractProc->IwOriRawAngio-1];
		}
	}

	// Table
	if ( ((t==0) || (ExtractProc->tableInfoFromtheStart)) && (ExtractProc->TableInformationGiven > 0) ){

		if (ExtractProc->TableInformationGiven == 2){
			// table position given in MM - conversion to pixels
			float dx = ExtractProc->CurrentPhysicalTablePosXMM;
			float dy = ExtractProc->CurrentPhysicalTablePosYMM;
			float dz = ExtractProc->CurrentPhysicalTablePosZMM;
			float cA = (float)cos(OSC_PI*ExtractProc->AngAngio0/180), sA = (float)sin(OSC_PI*ExtractProc->AngAngio0/180)
				, cR = (float)cos(OSC_PI*ExtractProc->RotAngio0/180), sR = (float)sin(OSC_PI*ExtractProc->RotAngio0/180);

			ExtractProc->CurrentTablePosXMMPlane =  (-cR*dx + sR*dz);
			ExtractProc->CurrentTablePosYMMPlane = ( sA*sR*dx + cA*dy +sA*cR*dz);
            
		    if (t == 0){
			    ExtractProc->tableInfoFromtheStart = 1;
			    ExtractProc->OrigTablePosXMMPlane = ExtractProc->CurrentTablePosXMMPlane;
			    ExtractProc->OrigTablePosYMMPlane = ExtractProc->CurrentTablePosYMMPlane;
		    }
		    ExtractProc->deltaTabXSub = (int) OSC_RND( (ExtractProc->CurrentTablePosXMMPlane-ExtractProc->OrigTablePosXMMPlane)
                                                            /(ExtractProc->isoCenterPixSize*ExtractProc->SSFactor));
		    ExtractProc->deltaTabYSub = (int) OSC_RND((ExtractProc->CurrentTablePosYMMPlane-ExtractProc->OrigTablePosYMMPlane)
                                                            /(ExtractProc->isoCenterPixSize*ExtractProc->SSFactor));

		} else {
            // Conversion already done
            if (t == 0){
			    ExtractProc->tableInfoFromtheStart = 1;
			    ExtractProc->OrigTablePosXPix = ExtractProc->CurrentTablePosXPix;
			    ExtractProc->OrigTablePosYPix = ExtractProc->CurrentTablePosYPix;
		    }

            ExtractProc->deltaTabXSub = (ExtractProc->CurrentTablePosXPix - ExtractProc->OrigTablePosXPix) / ExtractProc->SSFactor;
            ExtractProc->deltaTabYSub = (ExtractProc->CurrentTablePosYPix - ExtractProc->OrigTablePosYPix) / ExtractProc->SSFactor;
        }


	} else {
		ExtractProc->deltaTabXSub = 0; ExtractProc->deltaTabYSub = 0;
		if (t == 0)
			ExtractProc->tableInfoFromtheStart = -1;
	}

	// ROIs
	ExtractProc->ShuttersRoi.XMin = ExtractProc->ShuttersRoiFullScale.XMin / ExtractProc->SSFactor;
	ExtractProc->ShuttersRoi.XMax = ExtractProc->ShuttersRoiFullScale.XMax / ExtractProc->SSFactor;
	ExtractProc->ShuttersRoi.YMin = ExtractProc->ShuttersRoiFullScale.YMin / ExtractProc->SSFactor;
	ExtractProc->ShuttersRoi.YMax = ExtractProc->ShuttersRoiFullScale.YMax / ExtractProc->SSFactor;
	
	ExtractProc->ShuttersRoiSafe.XMin 
		= OSC_MIN(ExtractProc->ShuttersRoi.XMin + OSC_SHUTTERS_SAFETY_MARGIN / ExtractProc->SSFactor, ExtractProc->IwProcAngio-1);
	ExtractProc->ShuttersRoiSafe.XMax 
		= OSC_MAX(ExtractProc->ShuttersRoi.XMax - OSC_SHUTTERS_SAFETY_MARGIN / ExtractProc->SSFactor, 0);
	ExtractProc->ShuttersRoiSafe.YMin 
		= OSC_MIN(ExtractProc->ShuttersRoi.YMin + OSC_SHUTTERS_SAFETY_MARGIN / ExtractProc->SSFactor, ExtractProc->IhProcAngio-1);
	ExtractProc->ShuttersRoiSafe.YMax 
		= OSC_MAX(ExtractProc->ShuttersRoi.YMax - OSC_SHUTTERS_SAFETY_MARGIN / ExtractProc->SSFactor,0);

	ExtractProc->ShuttersRoiSafeFullScale.XMin = ExtractProc->SSFactor * ExtractProc->ShuttersRoiSafe.XMin;
	ExtractProc->ShuttersRoiSafeFullScale.XMax = ExtractProc->SSFactor * ExtractProc->ShuttersRoiSafe.XMax;
	ExtractProc->ShuttersRoiSafeFullScale.YMin = ExtractProc->SSFactor * ExtractProc->ShuttersRoiSafe.YMin;
	ExtractProc->ShuttersRoiSafeFullScale.YMax = ExtractProc->SSFactor * ExtractProc->ShuttersRoiSafe.YMax;

	// Rdgs
    ExtractProc->KernelSigmas[0] = OSC_ANGIO_KERNEL0 / (ExtractProc->SSFactor * ExtractProc->isoCenterPixSize);
    ExtractProc->KernelSigmas[1] = OSC_ANGIO_KERNEL1 / (ExtractProc->SSFactor * ExtractProc->isoCenterPixSize);
    ExtractProc->KernelSigmas[2] = OSC_ANGIO_KERNEL2 / (ExtractProc->SSFactor * ExtractProc->isoCenterPixSize);

	ExtractProc->nKernelSigs = 3;
	ExtractProc->gammaRdg	= OSC_RIDGEGAMMAANGIO;

	return Err;
}

/*************************************************************************************/
// Alloc and stores the extracted picture into the ExtractSequence object
/*************************************************************************************/
int OscStoreExtractedPicture(COsc *This)
{
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	COscExtractPicture  *ExtractPic = &ExtractSequence->ExtractPictures[ExtractProc->t];
	int                 Err = 0, p;
    float               coeff;

	// Copy images useful for OscAngioSequence processing
    ExtractPic->minFrg = FLT_MAX; ExtractPic->maxFrg = -FLT_MAX;
    for (p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++){
        ExtractPic->ImDirUC[p] = OSC_MIN(255, OSC_MAX(0, (int)OSC_RND(128.f+254.f*ExtractProc->ImDir[p] / OSC_PI)));
        ExtractPic->ImLabS[p] = (short)OSC_MIN(ExtractProc->ImLab[p],32767);

        ExtractPic->minFrg = OSC_MIN(ExtractProc->ImFrg[p], ExtractPic->minFrg);
        ExtractPic->maxFrg = OSC_MAX(ExtractProc->ImFrg[p], ExtractPic->maxFrg);
    }
    if (ExtractPic->minFrg < ExtractPic->maxFrg)
        coeff = 1.f/(ExtractPic->maxFrg-ExtractPic->minFrg);
    else
        coeff = 0.f;

    for (p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++)
        ExtractPic->ImFrgUC[p] = OSC_MIN(255, OSC_MAX(0, (int)OSC_RND(255*coeff*(ExtractProc->ImFrg[p]-ExtractPic->minFrg))));


	ExtractPic->deltaTabXSub = ExtractProc->deltaTabXSub;
	ExtractPic->deltaTabYSub = ExtractProc->deltaTabYSub;

	ExtractSequence->nExtractPictures = ExtractProc->t;

	return Err;
}


/*************************************************************************************/
// Updates advanced volatile results, for intermediate results study
/*************************************************************************************/
void OscFillAngioPictureResults(COscAngioPictureResults *ExtractResults, COscExtractProc *ExtractProc)
{
	// advanced information
	ExtractResults->Advanced.IwOri							= ExtractProc->IwOriRawAngio;
	ExtractResults->Advanced.IhOri							= ExtractProc->IhOriRawAngio;
	ExtractResults->Advanced.IwOriInternal					= ExtractProc->IwOriAngio;
	ExtractResults->Advanced.IhOriInternal					= ExtractProc->IhOriAngio;
	ExtractResults->Advanced.IwProc							= ExtractProc->IwProcAngio;
	ExtractResults->Advanced.IhProc							= ExtractProc->IhProcAngio;
	ExtractResults->Advanced.SSFactor						= ExtractProc->SSFactor;
	ExtractResults->Advanced.Volatile.Sub					= ExtractProc->ImSub;
	ExtractResults->Advanced.Volatile.Rdg				    = ExtractProc->ImRdg;
	ExtractResults->Advanced.Volatile.Dir			        = ExtractProc->ImDir;
	ExtractResults->Advanced.Volatile.Bkg					= ExtractProc->ImBkg;
	ExtractResults->Advanced.Volatile.Frg					= ExtractProc->ImFrg;
	ExtractResults->Advanced.Volatile.See					= ExtractProc->ImSee;
	ExtractResults->Advanced.Volatile.Thr					= ExtractProc->ImThr;
	ExtractResults->Advanced.Volatile.Lab					= ExtractProc->ImLab;
	ExtractResults->Advanced.Volatile.Mask					= ExtractProc->ImMask;

	// Catheter body
	ExtractResults->Advanced.CatheterBodyDetected			= ExtractProc->detectedCatheter;
	ExtractResults->Advanced.Volatile.CBDRes				= &ExtractProc->CBDRes;

	// Sternal wire
	ExtractResults->Advanced.Volatile.SWRes					= &ExtractProc->SWRes;

	// Diaphragm (upsampled)
	ExtractResults->Advanced.DiaDetected					= ExtractProc->detectedDiaphragm;
	ExtractResults->Advanced.DiaParam.a						= ExtractProc->DiaphragmParams[0] / ExtractProc->SSFactor;
	ExtractResults->Advanced.DiaParam.b						= ExtractProc->DiaphragmParams[1];
	ExtractResults->Advanced.DiaParam.c						= ExtractProc->DiaphragmParams[2] * ExtractProc->SSFactor;

    ExtractResults->Advanced.minRdgThresholdClean           = ExtractProc->minRdgThresholdClean;
}
