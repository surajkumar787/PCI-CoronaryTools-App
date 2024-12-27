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
#include <math.h>

#include <mutex> 


/*************************************************************************************/
// Fluoro Picture Extraction Process
/*************************************************************************************/
int OscFluoroProcessPicture (COscHdl OscHdl, short* InIma, int t, int FrameStatus, COscFluoroPictureResults*  ProcessResults)
{
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	int					skipFrameIndicator;
	int                  ErrorCode, ErrorCodeOut = 0;
	short				*WIm;
	
#ifdef OSC_TIMING
	double t0 = omp_get_wtime(), t1;
#endif
#ifdef OSC_TIMING_DETAIL
	double tAfterSubsample, tAfterDiaDetect, tAfterRdgComputation, tAfterSWRemoval, tAfterBkgComputation
		, tAfterPyramids, tAfterAngiosReg, tAfterViterbi;
#endif

	// *** Initial tests

	// Cope with potential frame skips
	skipFrameIndicator = OscFluoroHandleFluoroSkips(OscHdl, FrameStatus, &t);

	// Initial checks and initializations
	ErrorCode = OscFluoroTestsDataConsistency(OscHdl, InIma, skipFrameIndicator, t);
	WIm = ExtractProc->ImOriWorking;
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscFluoroTestsDataConsistency", "OscFluoroProcessPicture");

	if (skipFrameIndicator){return 0;}

	// *** Foreground ridgeness extraction, deletion of distrubing devices

	// Image subsampling (by minimum)
	ErrorCode = OscSubSampleImageMin(OscHdl, WIm, ExtractProc->IwOriFluoro, ExtractProc->IhOriFluoro
									, ExtractProc->ImSub, ExtractProc->IwProcFluoro, ExtractProc->IhProcFluoro, ExtractProc->SSFactor);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscSubSampleImageMin", "OscFluoroProcessPicture");

#ifdef OSC_TIMING_DETAIL
	tAfterSubsample = omp_get_wtime();
#endif

	// Diaphragm tracking (if detected in angio)
	if (ExtractProc->detectedDiaphragm){
		ErrorCode = OscDiaphragmTrackingTR(OscHdl);
		OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscDiaphragmTrackingTR", "OscFluoroProcessPicture");
	}
#ifdef OSC_TIMING_DETAIL
	tAfterDiaDetect = omp_get_wtime();
#endif

	// Ridge computation (with potential diaphragm correction)
	ErrorCode = OscFluoroUpdateRidgeKernels(OscHdl);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscFluoroUpdateRidgeKernels", "OscFluoroProcessPicture");
	OscRdgMultiScaleRidge(OscHdl, ExtractProc->IwProcFluoro, ExtractProc->IhProcFluoro);
#ifdef OSC_TIMING_DETAIL
	tAfterRdgComputation = omp_get_wtime();
#endif
	
	// Erase potential sternal wires
	ErrorCode = OscRdgSWTrackingAndRemovalFluoro	(OscHdl, WIm);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscRdgSWTrackingAndRemovalFluoro", "OscFluoroProcessPicture");
#ifdef OSC_TIMING_DETAIL
	tAfterSWRemoval = omp_get_wtime();
#endif

	// Compute and substrate background
	OscRdgCircularBackgroundSubtraction(OscHdl);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscRdgCircularBackgroundSubtraction", "OscFluoroProcessPicture");
#ifdef OSC_TIMING_DETAIL
	tAfterBkgComputation = omp_get_wtime();
#endif

	// *** Registrations to angio cycle
	ErrorCode = OscFluoroProcessPicturePyramid(OscHdl);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscFluoroProcessPicturePyramid", "OscFluoroProcessPicture");
#ifdef OSC_TIMING_DETAIL
	tAfterPyramids = omp_get_wtime();
#endif

	ErrorCode = OscMatchFluoroWithEachAngioPhase(OscHdl);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscMatchFluoroWithEachAngioPhase", "OscFluoroProcessPicture");
#ifdef OSC_TIMING_DETAIL
	tAfterAngiosReg = omp_get_wtime();
#endif
	
	// *** Viterbi
	ErrorCode = OscViterbiIteration(OscHdl);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscMatchFluoroWithEachAngioPhase", "OscFluoroProcessPicture");
#ifdef OSC_TIMING_DETAIL
	tAfterViterbi = omp_get_wtime();
#endif

	// *** Motion search range adaptation
	ErrorCode = OscAdaptSearchRange(OscHdl);
	OSC_ERR_REPORT_FROM_CALLER(ErrorCode, ErrorCodeOut, "OscAdaptSearchRange", "OscFluoroProcessPicture");

	// *** Outputs
	OscFillFluoroPictureResults(ProcessResults, ExtractProc, ExtractSequence);

#ifdef OSC_TIMING
	t1 = omp_get_wtime();
	printf("Time to process fluoro image %d: %.1f ms\n", t, 1000.0 * (t1-t0));
#endif
#ifdef OSC_TIMING_DETAIL
	printf("     (Subsampling %.1f DiaDetection %.1f Ridge computation %.1f)\n", 1000.0 * (tAfterSubsample-t0), 1000.0 * (tAfterDiaDetect-tAfterSubsample)
		, 1000.0 * (tAfterRdgComputation-tAfterDiaDetect));
	printf("     (SW removal %.1f Bkg computation %.1f Pyramid compytation %.1f )\n"
		, 1000.0 * (tAfterSWRemoval-tAfterRdgComputation), 1000.0 * (tAfterBkgComputation-tAfterSWRemoval), 1000.0 * (tAfterPyramids-tAfterBkgComputation));
	printf("     (Angios registration %.1f Viterbi %.1f Search range adaptation %.1f)\n"
		, 1000.0 * (tAfterAngiosReg-tAfterPyramids), 1000.0 * (tAfterViterbi-tAfterAngiosReg), 1000.0 * (t1-tAfterViterbi));

#endif

	return ErrorCodeOut;
}

/*************************************************************************************/
// ****** Coping with potentially missing frames

int OscFluoroHandleFluoroSkips(COscHdl OscHdl, int FrameStatus, int *t){
	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

	if (*t == 0){
		ExtractProc->InitialSkipOffset = 0;		// Ongoing series of first frame to skip
		ExtractProc->NonInitialSkippedImages = 0;
        ExtractProc->NbOfSuccessiveSkippedFrames = 0;
	}

	if (FrameStatus == OSC_FLUORO_FRAME){
		*t = *t - ExtractProc->InitialSkipOffset;	// Correct for the initially skipped frames
        ExtractProc->NbOfSuccessiveSkippedFrames = 0;
		return 0;
	}else {

		if (*t == ExtractProc->InitialSkipOffset)
			ExtractProc->InitialSkipOffset ++;			// Ongoing series of first frame to skip
		else {
			//int tViterbi, tViterbiMinus1, k;
			*t = *t - ExtractProc->InitialSkipOffset;	// Correct for the initially skipped frames
			ExtractProc->NonInitialSkippedImages ++;	// Used in background computation

            ExtractProc->NbOfSuccessiveSkippedFrames++;

			//// Adapt Viterbi buffers
			//tViterbi		= ExtractProc->t % OSC_VITERBI_FLUORO_DEPTH;
			//tViterbiMinus1	= (ExtractProc->t-1) % OSC_VITERBI_FLUORO_DEPTH;
			//for (k=ExtractSequence->CardiacCycleStart+1; k<=ExtractSequence->CardiacCycleEnd; k++)
			//	ExtractProc->ViterbiStateScore[tViterbi][k] = ExtractProc->ViterbiStateScore[tViterbiMinus1][k-1];
			//ExtractProc->ViterbiStateScore[tViterbi][ExtractSequence->CardiacCycleStart] = ExtractProc->ViterbiStateScore[tViterbiMinus1][ExtractSequence->CardiacCycleEnd];
		}

		return 1;
	}
}

/*************************************************************************************/
int OscFluoroTestsDataConsistency(COscHdl OscHdl, short* InIma, int skippedImageFlag, int t){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	char                 Reason[128];
	int					Error, ErrorCodeOut = 0;
    
	if (ExtractProc->angioProcessed == 0){
		sprintf_s(Reason, 128,"OscFluoroProcessPicture called without processed matching angio");
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscFluoroTestsDataConsistency", OSC_ERR_ANGIO_UNPROCESSED, Reason);
		return OSC_ERR_ANGIO_UNPROCESSED;
	}

	if (t==0){
        ExtractProc->FatalErrorOnPreviousImage = 0;     // New fluoro -> the fatal error having occurred on the previous fluoro is not revelant anymore
		Error = OscRestoreAngioStatus(OscHdl);
		OSC_ERR_REPORT_FROM_CALLER(Error, ErrorCodeOut, "OscRestoreAngioStatus", "OscFluoroTestsDataConsistency");
	}

    if (ExtractProc->FatalErrorOnPreviousImage){
		sprintf_s(Reason, 128,"OscFluoroTestsDataConsistency: error reported on previous frame");
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscFluoroTestsDataConsistency", OSC_ERR_PREVIOUS_FATAL_ERROR, Reason);
		return OSC_ERR_PREVIOUS_FATAL_ERROR;
    }

	if (t!= ExtractProc->t+1){
		sprintf_s(Reason, 128,"OscFluoroProcessPicture called at non-incremental times %d and %d : Extract aborted", ExtractProc->t, t);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscFluoroTestsDataConsistency", OSC_ERR_INCORRECT_TIME, Reason);
		return OSC_ERR_INCORRECT_TIME;
	}
	

	if ( (ExtractProc->CArmAnglesFluoroGiven) && (ExtractProc->CArmAnglesFluoroGivenRef) && 
		( (ExtractProc->RotFluoro != ExtractProc->RotFluoroRef) || (ExtractProc->AngFluoro != ExtractProc->AngFluoroRef) ) ){
		sprintf_s(Reason, 128,"C-arm angulation changed from (Rot %.3f, Ang %.3f) to (Rot %.3f, Ang %.3f)", ExtractProc->RotFluoro, ExtractProc->AngFluoro
					, ExtractProc->RotFluoroRef, ExtractProc->RotFluoroRef);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscFluoroTestsDataConsistency", OSC_INFO_CARM_MOVED_INSEQ, Reason);
		Error = OSC_INFO_CARM_MOVED_INSEQ;
	}
	if ( (ExtractProc->t == 0) && (ExtractProc->CArmAnglesAngioGiven) && (ExtractProc->CArmAnglesFluoroGiven) ){
		float diff = (ExtractProc->RotFluoro-ExtractProc->RotAngio)*(ExtractProc->RotFluoro-ExtractProc->RotAngio) 
			+ (ExtractProc->AngFluoro-ExtractProc->AngAngio)*(ExtractProc->AngFluoro-ExtractProc->AngAngio);

		if (diff >0) {
			sprintf_s(Reason, 128,"C-arm angulation changed from angio (Rot %.3f, Ang %.3f) to fluoro (Rot %.3f, Ang %.3f)"
				, ExtractProc->RotFluoro, ExtractProc->AngFluoro, ExtractProc->RotFluoroRef, ExtractProc->AngFluoroRef);
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscFluoroTestsDataConsistency", OSC_INFO_CARM_MOVED_ANGIOFLUORO, Reason);
			Error = OSC_INFO_CARM_MOVED_ANGIOFLUORO;
		}
	}
	if (ExtractProc->CArmAnglesFluoroGiven){
		ExtractProc->CArmAnglesFluoroGivenRef = 1; ExtractProc->RotFluoroRef = ExtractProc->RotFluoro; ExtractProc->AngFluoroRef = ExtractProc->AngFluoro;
	}

	if ( (ExtractProc->isoCenterPixSizeRef > 0) && (OSC_ABS_F(ExtractProc->isoCenterPixSize-ExtractProc->isoCenterPixSizeRef) > 0.0001f) ){
		sprintf_s(Reason, 128,"Given pixSizeAtIsocenter has changed from %.3f to %.3f", ExtractProc->isoCenterPixSize, ExtractProc->isoCenterPixSizeRef);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscFluoroTestsDataConsistency", OSC_INFO_PIXSIZE_CHANGED, Reason);
		ErrorCodeOut = OSC_INFO_PIXSIZE_CHANGED;
	}
	if (ExtractProc->isoCenterPixSize <= 0){
		sprintf_s(Reason, 128,"Pixel size at isocenter has not been informed");
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscFluoroTestsDataConsistency", OSC_ERR_SYSTEM_INFO_MISSING, Reason);
		return OSC_ERR_SYSTEM_INFO_MISSING;
	} else {
		ExtractProc->isoCenterPixSizeRef = ExtractProc->isoCenterPixSize;
	}
	

	if ((ExtractProc->ShuttersRoiFullScale.XMin < 0) || (ExtractProc->ShuttersRoiFullScale.XMax >= ExtractProc->IwOriFluoro) 
		|| (ExtractProc->ShuttersRoiFullScale.YMin < 0) || (ExtractProc->ShuttersRoiFullScale.YMax >= ExtractProc->IhOriFluoro)){
		sprintf_s(Reason, 128,"Informed shutter [%d,%d]x[%d,%d] incompatible with image size [0,%d]x[0,%d]", ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax
			, ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax, ExtractProc->IwOriFluoro-1, ExtractProc->IhOriFluoro-1);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscFluoroTestsDataConsistency", OSC_ERR_INCORRECT_SHUTTERINFO, Reason);
		return OSC_ERR_INCORRECT_SHUTTERINFO;
	}
	if ((ExtractProc->ShuttersRoiFullScale.XMin >= ExtractProc->ShuttersRoiFullScale.XMax ) || (ExtractProc->ShuttersRoiFullScale.YMin >= ExtractProc->ShuttersRoiFullScale.YMax)){
		sprintf_s(Reason, 128,"Incoherent shutter info: min > max (xmin %d xmax %d, ymin %d ymax %d)"
			, ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax, ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscFluoroTestsDataConsistency", OSC_ERR_INCORRECT_SHUTTERINFO, Reason);
		return OSC_ERR_INCORRECT_SHUTTERINFO;
	}
    if (ExtractProc->ShuttersRoiFullScale.XMax - ExtractProc->ShuttersRoiFullScale.XMin < OSC_MIN_PIXSIZE_EACHDIMENSION
        || ExtractProc->ShuttersRoiFullScale.YMax - ExtractProc->ShuttersRoiFullScale.YMin < OSC_MIN_PIXSIZE_EACHDIMENSION){
        sprintf_s(Reason, 128,"Informed shutter [%d,%d]x[%d,%d] results in a too small image amplitude in one of the direction at least", ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax
			, ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscFluoroTestsDataConsistency", OSC_ERR_INCORRECT_SHUTTERINFO, Reason);
		return OSC_ERR_INCORRECT_SHUTTERINFO;
    }

	if ( (ExtractProc->ShuttersRoiPosGiven) &&
			( (ExtractProc->ShuttersRoiFullScale.XMin != ExtractProc->ShuttersRoiFullScaleRef.XMin) || (ExtractProc->ShuttersRoiFullScale.XMax != ExtractProc->ShuttersRoiFullScaleRef.XMax)
			|| (ExtractProc->ShuttersRoiFullScale.YMin != ExtractProc->ShuttersRoiFullScaleRef.YMin) || (ExtractProc->ShuttersRoiFullScale.XMax != ExtractProc->ShuttersRoiFullScaleRef.YMax) ) ){	// Shutter position was already given
		sprintf_s(Reason, 128,"Shutter position modified from [%d,%d]x[%d,%d] to [%d,%d]w[%d,%d]", ExtractProc->ShuttersRoiFullScale.XMin, ExtractProc->ShuttersRoiFullScale.XMax
				, ExtractProc->ShuttersRoiFullScale.YMin, ExtractProc->ShuttersRoiFullScale.YMax
				, ExtractProc->ShuttersRoiFullScaleRef.XMin, ExtractProc->ShuttersRoiFullScaleRef.XMax
				, ExtractProc->ShuttersRoiFullScaleRef.YMin, ExtractProc->ShuttersRoiFullScaleRef.YMax);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscFluoroTestsDataConsistency", OSC_INFO_SHUTTERINFO_CHANGE, Reason);
		ErrorCodeOut = OSC_INFO_SHUTTERINFO_CHANGE;
	}
	ExtractProc->t = t;

	ExtractProc->ShuttersRoiFullScaleRef.XMin = ExtractProc->ShuttersRoiFullScale.XMin;
	ExtractProc->ShuttersRoiFullScaleRef.XMax = ExtractProc->ShuttersRoiFullScale.XMax;
	ExtractProc->ShuttersRoiFullScaleRef.YMin = ExtractProc->ShuttersRoiFullScale.YMin;
	ExtractProc->ShuttersRoiFullScaleRef.YMax = ExtractProc->ShuttersRoiFullScale.YMax;

	if (t==0){
		ExtractProc->correctedMotionRangeX = 0; ExtractProc->correctedMotionRangeY = 0;
        ExtractProc->AngFluoro0 = ExtractProc->AngFluoro; ExtractProc->RotFluoro0 = ExtractProc->RotFluoro;
	}

	// Potential image panning
    if (skippedImageFlag == 0){
	    //if ( ( (ExtractProc->IwOriRaw == ExtractProc->IwOri) && (ExtractProc->IhOriRaw == ExtractProc->IhOri) ) )
		   // ExtractProc->ImOriWorking = InIma;
	    //else {
		    int x, y;
		    // padding
		    for (y=0; y<ExtractProc->IhOriRawFluoro; y++){
		        //	memcpy(&(ExtractProc->ImOriWorking[y*ExtractProc->IwOri]), &(InIma[y*ExtractProc->IwOriRaw])
		        //				, ExtractProc->IwOriRaw*sizeof(short));
			    for (x=0; x<ExtractProc->IwOriRawFluoro; x++)
				    ExtractProc->ImOriWorking[y*ExtractProc->IwOriFluoro+x] = InIma[y*ExtractProc->IwOriRawFluoro + x];
			    for (x=ExtractProc->IwOriRawFluoro; x<ExtractProc->IwOriFluoro; x++)
				    ExtractProc->ImOriWorking[y*ExtractProc->IwOriFluoro+x] = InIma[y*ExtractProc->IwOriRawFluoro + ExtractProc->IwOriRawFluoro-1];
		    }
		    for (y=ExtractProc->IhOriRawFluoro; y<ExtractProc->IhOriFluoro; y++){
			    //memcpy(&(ExtractProc->ImOriWorking[y*ExtractProc->IwOri]), &(InIma[(ExtractProc->IhOriRaw-1)*ExtractProc->IwOriRaw])
			    //			, ExtractProc->IwOriRaw*sizeof(short));
			    for (x=0; x<ExtractProc->IwOriRawFluoro; x++)
				    ExtractProc->ImOriWorking[y*ExtractProc->IwOriFluoro+x] = InIma[(ExtractProc->IhOriRawFluoro-1)*ExtractProc->IwOriRawFluoro+x];
			    for (x=ExtractProc->IwOriRawFluoro; x<ExtractProc->IwOriFluoro; x++)
				    ExtractProc->ImOriWorking[y*ExtractProc->IwOriFluoro+x] = InIma[ExtractProc->IhOriRawFluoro*ExtractProc->IwOriRawFluoro-1];
		    }
	    //}
    }
	
	// Table
	if ( (ExtractProc->tableInfoFromtheStart) && (ExtractProc->TableInformationGiven > 0) ){

		if (ExtractProc->TableInformationGiven == 2){

			// table position given in MM - conversion to pixels
			float dx = ExtractProc->CurrentPhysicalTablePosXMM;
			float dy = ExtractProc->CurrentPhysicalTablePosYMM;
			float dz = ExtractProc->CurrentPhysicalTablePosZMM;
			float cA = (float)cos(OSC_PI*ExtractProc->AngFluoro0/180), sA = (float)sin(OSC_PI*ExtractProc->AngFluoro0/180)
				, cR = (float)cos(OSC_PI*ExtractProc->RotFluoro0/180), sR = (float)sin(OSC_PI*ExtractProc->RotFluoro0/180);

			ExtractProc->CurrentTablePosXMMPlane = (-cR*dx + sR*dz);
			ExtractProc->CurrentTablePosYMMPlane = ( sA*sR*dx + cA*dy +sA*cR*dz);
            
		    ExtractProc->deltaTabXSub = (int) OSC_RND( (ExtractProc->CurrentTablePosXMMPlane-ExtractProc->OrigTablePosXMMPlane) 
                                                        / (ExtractProc->isoCenterPixSize*ExtractProc->SSFactor));
		    ExtractProc->deltaTabYSub = (int) OSC_RND( (ExtractProc->CurrentTablePosYMMPlane-ExtractProc->OrigTablePosYMMPlane) 
                                                        / (ExtractProc->isoCenterPixSize*ExtractProc->SSFactor));
		} else {
            // Conversion already done
            ExtractProc->deltaTabXSub = (ExtractProc->CurrentTablePosXPix - ExtractProc->OrigTablePosXPix) / ExtractProc->SSFactor;
            ExtractProc->deltaTabYSub = (ExtractProc->CurrentTablePosYPix - ExtractProc->OrigTablePosYPix) / ExtractProc->SSFactor;
        }


	} else {
		ExtractProc->deltaTabXSub = 0; ExtractProc->deltaTabYSub = 0;
	}

	// ROIs
	ExtractProc->ShuttersRoi.XMin = ExtractProc->ShuttersRoiFullScale.XMin / ExtractProc->SSFactor;
	ExtractProc->ShuttersRoi.XMax = ExtractProc->ShuttersRoiFullScale.XMax / ExtractProc->SSFactor;
	ExtractProc->ShuttersRoi.YMin = ExtractProc->ShuttersRoiFullScale.YMin / ExtractProc->SSFactor;
	ExtractProc->ShuttersRoi.YMax = ExtractProc->ShuttersRoiFullScale.YMax / ExtractProc->SSFactor;
	
	ExtractProc->ShuttersRoiSafe.XMin 
		= OSC_MIN(ExtractProc->ShuttersRoi.XMin + OSC_SHUTTERS_SAFETY_MARGIN / ExtractProc->SSFactor, ExtractProc->IwProcFluoro-1);
	ExtractProc->ShuttersRoiSafe.XMax 
		= OSC_MAX(ExtractProc->ShuttersRoi.XMax - OSC_SHUTTERS_SAFETY_MARGIN / ExtractProc->SSFactor, 0);
	ExtractProc->ShuttersRoiSafe.YMin 
		= OSC_MIN(ExtractProc->ShuttersRoi.YMin + OSC_SHUTTERS_SAFETY_MARGIN / ExtractProc->SSFactor, ExtractProc->IhProcFluoro-1);
	ExtractProc->ShuttersRoiSafe.YMax 
		= OSC_MAX(ExtractProc->ShuttersRoi.YMax - OSC_SHUTTERS_SAFETY_MARGIN / ExtractProc->SSFactor,0);
	
	ExtractProc->ShuttersRoiSafeFullScale.XMin = ExtractProc->SSFactor * ExtractProc->ShuttersRoiSafe.XMin;
	ExtractProc->ShuttersRoiSafeFullScale.XMax = ExtractProc->SSFactor * ExtractProc->ShuttersRoiSafe.XMax;
	ExtractProc->ShuttersRoiSafeFullScale.YMin = ExtractProc->SSFactor * ExtractProc->ShuttersRoiSafe.YMin;
	ExtractProc->ShuttersRoiSafeFullScale.YMax = ExtractProc->SSFactor * ExtractProc->ShuttersRoiSafe.YMax;

	// Motion estimation amplitude
	ExtractProc->topLevelSearchRangeX = OSC_FLUORO_MAXRV_X;
	ExtractProc->topLevelSearchRangeY = OSC_FLUORO_MAXRV_Y;

	return ErrorCodeOut;
}

/*************************************************************************************/
int OscFluoroUpdateRidgeKernels(COscHdl OscHdl){	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

    ExtractProc->KernelSigmas[0] = OSC_FLUORO_KERNEL0 / (ExtractProc->SSFactor * ExtractProc->isoCenterPixSize);
    ExtractProc->KernelSigmas[1] = OSC_FLUORO_KERNEL1 / (ExtractProc->SSFactor * ExtractProc->isoCenterPixSize);

	ExtractProc->nKernelSigs	= 2;
	ExtractProc->gammaRdg		= OSC_RIDGEGAMMAFLUORO;

	return 0;
}

// *************************************************************************
int OscFluoroProcessPicturePyramid(COscHdl OscHdl){
	
    static float cosTab[OSC_TABULATE_SINCOS_TABSIZE], sinTab[OSC_TABULATE_SINCOS_TABSIZE];

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);

    float sincosTabFactor = (float)(OSC_TABULATE_SINCOS_TABSIZE / (2*OSC_PI));
    int i, idx, t = ExtractProc->t, W = ExtractProc->PyrWFluoro[0], H = ExtractProc->PyrHFluoro[0];
	float RdgMax = 0.f;

	short (*ImDirRot)[2] = ExtractProc->ImFluoroDirPyr1D[0];
	float *ImDir = ExtractProc->ImDir, *ImFrg = ExtractProc->ImFrg;

    if (t == 0)
        for (i = 0; i < OSC_TABULATE_SINCOS_TABSIZE; i++){
            cosTab[i] = cosf(i*((float)(2*OSC_PI/OSC_TABULATE_SINCOS_TABSIZE)));
            sinTab[i] = sinf(i*((float)(2*OSC_PI/OSC_TABULATE_SINCOS_TABSIZE)));
        }

	// compute ImDirX = Rdg * cos(ImDir); ImDirY = Rdg * sin(ImDir);
	for(i=0; i<W * H; i++) 
		RdgMax = OSC_MAX(RdgMax, ImFrg[i]);
		
	for(i=0; i<W * H; i++) {
		float value = ImFrg[i]/ RdgMax;
		if (value <= 0) { ImDirRot[i][0] = 0; ImDirRot[i][1] = 0; continue; }

        idx = (int)OSC_CLIP(0, (ImDir[i] < 0 ? ImDir[i] + 2*OSC_PI : ImDir[i]) * sincosTabFactor, OSC_TABULATE_SINCOS_TABSIZE - 1);
		ImDirRot[i][0] = OSC_CLIP_SHORT(32768 * cosTab[idx] * value);	//ImDirRot[i][0] = (short)OSC_CLIP_SHORT(32768 * value * cosf(ImDir[i]));
        ImDirRot[i][1] = OSC_CLIP_SHORT(32768 * sinTab[idx] * value);	//ImDirRot[i][1] = (short)OSC_CLIP_SHORT(32768 * value * sinf(ImDir[i]));
	}

	// Multi-resolution decomposition into Gaussian pyramid of directions
	OscMrGaussianDecompPyrShort2(ExtractProc->ImFluoroDirPyr1D, ExtractSequence->PyrNbBands, ExtractProc->PyrWFluoro, ExtractProc->PyrHFluoro);

	return 0;
}

// *************************************************************************
int OscMatchFluoroWithEachAngioPhase(COscHdl OscHdl){

    COsc                *This = (COsc*)OscHdl.Pv;
    COscExtractProc     *ExtractProc = &(This->ExtractProc);
	std::mutex          mutex;

	ExtractProc->pool->execute([&mutex,&OscHdl](int threadIndex, int threadCount)
	{
	    COsc                *This = (COsc*)OscHdl.Pv;
	    COscExtractProc     *ExtractProc = &(This->ExtractProc);
	    COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
        int                 tViterbi = ExtractProc->t % OSC_VITERBI_FLUORO_DEPTH;
#ifdef OSC_ADAPT_SEARCH_RANGE_TO_FRAME_RATE
            float ratioReferenceFrameRate = ExtractProc->frameRateFluoro / 15.f;
#endif

        int cycleLength = ExtractSequence->CardiacCycleEnd-ExtractSequence->CardiacCycleStart+1;

        threadCount = OSC_MIN(threadCount, cycleLength);

        if (threadIndex < cycleLength) {
            int firstIndice = ExtractSequence->CardiacCycleStart + OSC_RND((float)threadIndex * cycleLength / threadCount);
            int lastIndice = ExtractSequence->CardiacCycleStart + OSC_RND((float)(threadIndex+1) * cycleLength / threadCount);

            //printf("Pool thread %d of %d (firstIndice %d lastIndice %d) \n", threadIndex, threadCount, firstIndice, lastIndice); 

	        for (int k = firstIndice; k< lastIndice; k++){
			        float score;
			        int dXFinal, dYFinal, topLevelSearchRangeX, topLevelSearchRangeY, PyrNbBands, dXIni, dYIni;//, l;
			        short (**PyrDirAngioK)[2], (**PyrDirFluoro)[2];
			        int *PyrW, *PyrH, *PyrWFluoro, *PyrHFluoro;
			        COscRoiCoord *ShuttersRoi;

			        {
				        PyrDirAngioK = ExtractSequence->ExtractPictures[k].ImPyrDir1D;
				        PyrDirFluoro = ExtractProc->ImFluoroDirPyr1D;
				        ShuttersRoi = &ExtractProc->ShuttersRoi;
#ifdef OSC_ADAPT_SEARCH_RANGE_TO_FRAME_RATE
				        topLevelSearchRangeX = (int)OSC_RND(ratioReferenceFrameRate*ExtractProc->topLevelSearchRangeX);
				        topLevelSearchRangeY = (int)OSC_RND(ratioReferenceFrameRate*ExtractProc->topLevelSearchRangeY);
#else
				        topLevelSearchRangeX = ExtractProc->topLevelSearchRangeX;
				        topLevelSearchRangeY = ExtractProc->topLevelSearchRangeY;
#endif
				        PyrNbBands = ExtractSequence->PyrNbBands;
				        PyrW = ExtractSequence->PyrW;
				        PyrH = ExtractSequence->PyrH;
                        PyrWFluoro = ExtractProc->PyrWFluoro;
                        PyrHFluoro = ExtractProc->PyrHFluoro;
				        dXIni = ExtractProc->correctedMotionRangeX/ExtractProc->SSFactor							// Potential shift to adapt search range
						        - ExtractProc->deltaTabXSub + ExtractSequence->ExtractPictures[k].deltaTabXSub;		// Table panning
				        dYIni = ExtractProc->correctedMotionRangeY/ExtractProc->SSFactor
						        - ExtractProc->deltaTabYSub + ExtractSequence->ExtractPictures[k].deltaTabYSub;
			        }

			        score = 
				        OscMultiResGlobalTranslationEstimationScalarProduct(PyrDirAngioK, PyrDirFluoro, ShuttersRoi
                                                                            , PyrW, PyrH, PyrWFluoro, PyrHFluoro, dXIni, dYIni
																	        , topLevelSearchRangeX, topLevelSearchRangeY, PyrNbBands
                                                                            , &dXFinal, &dYFinal);
			
		            std::unique_lock<std::mutex> lock(mutex,std::defer_lock);
		
		            lock.lock();
			                {
				                ExtractProc->FluoroToAngioScore[k]			= score;
				                ExtractProc->FluoroToAngioDX[tViterbi][k]	= dXFinal;
				                ExtractProc->FluoroToAngioDY[tViterbi][k]	= dYFinal;

			                }
		            lock.unlock();
            }
        }
	});
/*
    {
        FILE * fid = fopen("d://Temp//logCrade.txt", "a");
        int                 tViterbi = ExtractProc->t % OSC_VITERBI_FLUORO_DEPTH;

        fprintf(fid, "%d Corrected motions %d %d - Tab fluoro %d %d\n", ExtractProc->t
            ,ExtractProc->correctedMotionRangeX,ExtractProc->correctedMotionRangeY, ExtractProc->deltaTabXSub, ExtractProc->deltaTabYSub);
        for (int k = ExtractSequence->CardiacCycleStart; k<=ExtractSequence->CardiacCycleEnd; k++){
            fprintf(fid, "%d *** %.2f %d %d (%d %d)\n", ExtractProc->t, ExtractProc->FluoroToAngioScore[k], ExtractProc->FluoroToAngioDX[tViterbi][k]
            , ExtractProc->FluoroToAngioDY[tViterbi][k], ExtractSequence->ExtractPictures[k].deltaTabXSub, ExtractSequence->ExtractPictures[k].deltaTabYSub);
        }
        fclose(fid);

        if (ExtractProc->t == 0){
            FILE * fichierTempX = fopen("d://Temp//fichierTempX.raw","wb")
                , * fichierTempY = fopen("d://Temp//fichierTempY.raw","wb");
            short * AngioX = (short*)malloc(ExtractProc->IwProcAngio*ExtractProc->IhProcAngio*sizeof(short));
            short * AngioY = (short*)malloc(ExtractProc->IwProcAngio*ExtractProc->IhProcAngio*sizeof(short));

            for (int k = ExtractSequence->CardiacCycleStart; k<=ExtractSequence->CardiacCycleEnd; k++){
                short (**PyrDirAngioK)[2] = ExtractSequence->ExtractPictures[k].ImPyrDir1D;
                for (int p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++){
                    AngioX[p] = PyrDirAngioK[0][p][0]; AngioY[p] = PyrDirAngioK[0][p][1];
                }
                fwrite(AngioX, ExtractProc->IwProcAngio*ExtractProc->IhProcAngio, sizeof(short), fichierTempX);
                fwrite(AngioY, ExtractProc->IwProcAngio*ExtractProc->IhProcAngio, sizeof(short), fichierTempY);
            }
            free(AngioX); free(AngioY);
            fclose(fichierTempX); fclose(fichierTempY);
         }
    }*/

    return 0;
}



// *************************************************************************
int OscAdaptSearchRange(COscHdl OscHdl){
	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	int					i, t = ExtractProc->t, tViterbi = t % OSC_VITERBI_FLUORO_DEPTH, sumDec;
	int					dX = ExtractProc->SSFactor*ExtractProc->FluoroToAngioDX[tViterbi][ExtractProc->SelectedPhase];
	int					dY = ExtractProc->SSFactor*ExtractProc->FluoroToAngioDY[tViterbi][ExtractProc->SelectedPhase];

	// *** Search range at full res
	int dXIni = ExtractProc->correctedMotionRangeX/ExtractProc->SSFactor
			- ExtractProc->deltaTabXSub + ExtractSequence->ExtractPictures[ExtractProc->SelectedPhase].deltaTabXSub;
	int dYIni = ExtractProc->correctedMotionRangeY/ExtractProc->SSFactor
			- ExtractProc->deltaTabYSub + ExtractSequence->ExtractPictures[ExtractProc->SelectedPhase].deltaTabYSub;
	int dXIniLowRes	 =	 dXIni >> ExtractSequence->PyrNbBands;
	int dYIniLowRes	 =	 dYIni >> ExtractSequence->PyrNbBands;
	int LimitXPlus = (int)(pow(2.f, ExtractSequence->PyrNbBands) * (dXIniLowRes+ExtractProc->topLevelSearchRangeX+1) -1) * ExtractProc->SSFactor;
	int LimitXMinus= (int)(pow(2.f, ExtractSequence->PyrNbBands) * (dXIniLowRes-ExtractProc->topLevelSearchRangeX-1) +1) * ExtractProc->SSFactor;
	int LimitYPlus = (int)(pow(2.f, ExtractSequence->PyrNbBands) * (dYIniLowRes+ExtractProc->topLevelSearchRangeY+1) -1) * ExtractProc->SSFactor;
	int LimitYMinus= (int)(pow(2.f, ExtractSequence->PyrNbBands) * (dYIniLowRes-ExtractProc->topLevelSearchRangeY-1) +1) * ExtractProc->SSFactor;

	// *** Update the "reach limit" array

	// Shifting the previous values
	if (t >= OSC_MOTION_ANALYSIS_DEPTH)
		for (i = 1; i<= OSC_MOTION_ANALYSIS_DEPTH-1; i++){
			ExtractProc->analyzedMotionsX[i-1] = ExtractProc->analyzedMotionsX[i];
			ExtractProc->analyzedMotionsY[i-1] = ExtractProc->analyzedMotionsY[i];
		}

	// dX close to the limits?
	ExtractProc->analyzedMotionsX[OSC_MIN(t, OSC_MOTION_ANALYSIS_DEPTH-1)] = 0;
	if (dX > LimitXPlus-OSC_MOTION_ANALYSIS_MARGIN)
		ExtractProc->analyzedMotionsX[OSC_MIN(t, OSC_MOTION_ANALYSIS_DEPTH-1)] = -1;
	if (dX < LimitXMinus+OSC_MOTION_ANALYSIS_MARGIN)
		ExtractProc->analyzedMotionsX[OSC_MIN(t, OSC_MOTION_ANALYSIS_DEPTH-1)] = 1;

	// dY close to the limits?
	ExtractProc->analyzedMotionsY[OSC_MIN(t, OSC_MOTION_ANALYSIS_DEPTH-1)] = 0;
	if (dY > LimitYPlus	-OSC_MOTION_ANALYSIS_MARGIN)
		ExtractProc->analyzedMotionsY[OSC_MIN(t, OSC_MOTION_ANALYSIS_DEPTH-1)] = -1;
	if (dY < LimitYMinus+OSC_MOTION_ANALYSIS_MARGIN)
		ExtractProc->analyzedMotionsY[OSC_MIN(t, OSC_MOTION_ANALYSIS_DEPTH-1)] = 1;

	// *** Decision

	// in X
	sumDec = 0;
	for (i = 0; i<= OSC_MIN(t, OSC_MOTION_ANALYSIS_DEPTH-1); i++)
		sumDec += ExtractProc->analyzedMotionsX[i];
	if (sumDec <= -OSC_MOTION_ANALYSIS_NB_FOR_ADAPT){
		ExtractProc->correctedMotionRangeX += OSC_MOTION_ANALYSIS_UPDATE_STEP;
		OSC_MEMZERO(ExtractProc->analyzedMotionsX, OSC_MOTION_ANALYSIS_DEPTH);
	}
	if (sumDec >= OSC_MOTION_ANALYSIS_NB_FOR_ADAPT){
		ExtractProc->correctedMotionRangeX -= OSC_MOTION_ANALYSIS_UPDATE_STEP;
		OSC_MEMZERO(ExtractProc->analyzedMotionsX, OSC_MOTION_ANALYSIS_DEPTH);
	}

	// In Y
	sumDec = 0;
	for (i = 0; i<= OSC_MIN(t, OSC_MOTION_ANALYSIS_DEPTH-1); i++)
		sumDec += ExtractProc->analyzedMotionsY[i];
	if (sumDec <= -OSC_MOTION_ANALYSIS_NB_FOR_ADAPT){
		ExtractProc->correctedMotionRangeY += OSC_MOTION_ANALYSIS_UPDATE_STEP;
		OSC_MEMZERO(ExtractProc->analyzedMotionsY, OSC_MOTION_ANALYSIS_DEPTH);
	}
	if (sumDec >= OSC_MOTION_ANALYSIS_NB_FOR_ADAPT){
		ExtractProc->correctedMotionRangeY -= OSC_MOTION_ANALYSIS_UPDATE_STEP;
		OSC_MEMZERO(ExtractProc->analyzedMotionsY, OSC_MOTION_ANALYSIS_DEPTH);
	}

	return 0;
}

/*************************************************************************************/
// Updates advanced volatile results, for intermediate results study
/*************************************************************************************/
void OscFillFluoroPictureResults(COscFluoroPictureResults *ExtractResults, COscExtractProc *ExtractProc, COscExtractSequence *ExtractSequence)
{
	int tViterbi								= ExtractProc->t % OSC_VITERBI_FLUORO_DEPTH;

	ExtractResults->selectedAngioIndice							= ExtractProc->SelectedPhase;
	ExtractResults->dXSelectedAngio								= -ExtractProc->SSFactor*ExtractProc->FluoroToAngioDX[tViterbi][ExtractProc->SelectedPhase] + (ExtractProc->IwOriAngio-ExtractProc->IwOriFluoro)/2;
	ExtractResults->dYSelectedAngio								= -ExtractProc->SSFactor*ExtractProc->FluoroToAngioDY[tViterbi][ExtractProc->SelectedPhase] + (ExtractProc->IwOriAngio-ExtractProc->IwOriFluoro)/2;
	ExtractResults->Advanced.Volatile.Ovr						= ExtractSequence->ExtractPictures[ExtractResults->selectedAngioIndice].ImOverlayMask;

	// advanced information
	ExtractResults->Advanced.IwOri				        		= ExtractProc->IwOriRawFluoro;
	ExtractResults->Advanced.IhOri				        		= ExtractProc->IhOriRawFluoro;
	ExtractResults->Advanced.IwOriInternal   	        		= ExtractProc->IwOriFluoro;
	ExtractResults->Advanced.IhOriInternal   	        		= ExtractProc->IhOriFluoro;
	ExtractResults->Advanced.IwProc				        		= ExtractProc->IwProcFluoro;
	ExtractResults->Advanced.IhProc			        			= ExtractProc->IhProcFluoro;
	ExtractResults->Advanced.SSFactor							= ExtractProc->SSFactor;
	ExtractResults->Advanced.Volatile.Sub						= ExtractProc->ImSub;
	ExtractResults->Advanced.Volatile.Rdg						= ExtractProc->ImRdg;
	ExtractResults->Advanced.Volatile.Dir						= ExtractProc->ImDir;
	ExtractResults->Advanced.Volatile.Bkg						= ExtractProc->ImBkg;
	ExtractResults->Advanced.Volatile.Frg						= ExtractProc->ImFrg;

	// Sternal wire
	ExtractResults->Advanced.Volatile.SWRes						= &ExtractProc->SWRes;
	
	// Diaphragm (upsampled)
	ExtractResults->Advanced.DiaDetected						= ExtractProc->detectedDiaphragm;
	ExtractResults->Advanced.DiaParam.a							= ExtractProc->DiaphragmParams[0] / ExtractProc->SSFactor;
	ExtractResults->Advanced.DiaParam.b							= ExtractProc->DiaphragmParams[1];
	ExtractResults->Advanced.DiaParam.c							= ExtractProc->DiaphragmParams[2] * ExtractProc->SSFactor;
}