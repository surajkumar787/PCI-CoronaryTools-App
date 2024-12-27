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

#include <DiaLibDev.h>
#include <DiaErrorMessages.h>


// **************************************************************************************
// Diaphragm extraction in Angios
// **************************************************************************************

int DiaExtractDiaphragm(CDiaHdl DiaHdl, short *Im, int Iw, int Ih, short *ImFull, float Rot, float Ang
						 , int SSFactor, CDiaRoiCoord RoiCoord, int t
						, CDiaDiaphragmParab coeffIn, int *detectedDiaphragm
						, CDiaDiaphragmParab *coeffOut){

	CDia			*This = (CDia*)DiaHdl.Pv;
	CDiaProc		*DiaProc = &(This->DiaProc);
	int             Err;
	CDiaDiaphragmParab coeffTmpIn, coeffTmpOut;
	coeffOut->Param[0] = 0; coeffOut->Param[1] = 0; coeffOut->Param[2] = 0;

    {
        int IwOri = Iw*SSFactor;

        if ( (IwOri > DiaProc->IwOriAlloc) || (Iw > DiaProc->IwSS1Alloc) || (Ih > DiaProc->IhSS1Alloc) ){
            char Reason [128];
		    sprintf_s(Reason, 128, "Image size above allocated sizes: full proc %d vs %d, subsampled1 (%d,%d) vs (%d,%d)\n"
                        , IwOri, DiaProc->IwOriAlloc, Iw, DiaProc->IwSS1Alloc, Ih, DiaProc->IhSS1Alloc);
		    ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaExtractDiaphragm", DIA_ERR_IMSIZEABOVEALLOCATED, Reason);
		    return DIA_ERR_IMSIZEABOVEALLOCATED;
	    }

       DiaProc->IwSS1 = Iw; DiaProc->IhSS1 = Ih;
       DiaProc->IwSS2 = Iw /EXTRA_SSFACTOR_HOUGH; DiaProc->IhSS2 = Ih /EXTRA_SSFACTOR_HOUGH;
    }

	if (t == 0){
		float minHisto, maxHisto;
		float gradThres;
		CDiaDiaphragmParabs extractedCoeffs;
        int k;

		// *** Initialization: Hough parabol extraction

		// Gradient computation
		int i;
		CDiaRoiCoord RoiCoordHere;

		RoiCoordHere.XMin = RoiCoord.XMin / EXTRA_SSFACTOR_HOUGH; RoiCoordHere.XMax = RoiCoord.XMax / EXTRA_SSFACTOR_HOUGH;
		RoiCoordHere.YMin = RoiCoord.YMin / EXTRA_SSFACTOR_HOUGH; RoiCoordHere.YMax = RoiCoord.YMax / EXTRA_SSFACTOR_HOUGH;

		Err = DiaSubSampleImage(DiaHdl, Im, DiaProc->ImSubSS2, Iw, Ih, DiaProc->IwSS2, DiaProc->IhSS2, EXTRA_SSFACTOR_HOUGH);
		if (Err){
			ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaExtractDiaphragm", "DiaSubSampleImage", 0, Err);
			return Err;
		}
        Err = DiaGradientComputation(DiaHdl, DiaProc->ImSubSS2, DiaProc->gradImSS2, DiaProc->IwSS2, DiaProc->IhSS2, RoiCoordHere, SIGMA_GRADIENT * DiaProc->IwSS2 / 1024);
		if (Err){
			ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaExtractDiaphragm", "DiaGradientComputation", 0, Err);
			return Err;
		}

		// Threshold computation
		Err = DiaHistoConstruction(DiaHdl, DiaProc->gradImSS2, DiaProc->IwSS2, DiaProc->IhSS2, RoiCoordHere, NB_BINS_PERCENTILE, DiaProc->hist, &minHisto, &maxHisto);
		if (Err){
			ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaExtractDiaphragm", "DiaHistoConstruction", 0, Err);
			return Err;
		}
		gradThres = DiaPercentile(DiaProc->IwSS2, DiaProc->IhSS2, RoiCoordHere, DiaProc->hist, NB_BINS_PERCENTILE, minHisto, maxHisto, GRAD_PERCENTILE);

		// Hough extraction
		Err = DiaHoughParabolExtraction(DiaHdl, DiaProc->gradImSS2, DiaProc->IwSS2, DiaProc->IhSS2, RoiCoordHere, gradThres, NB_CELLS_PER_DIM_HOUGHA, NB_CELLS_PER_DIM_HOUGH
										, NB_CELLS_PER_DIM_HOUGH, PERCENT_CELLS_ZEROED, NB_EXTRACTED_DIAPHRAGM_LIMIT
					 					, SEUILMIN_MERGEPARAB * DiaProc->IwSS2, &extractedCoeffs);
		if (Err){
			ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaExtractDiaphragm", "DiaHoughParabolExtraction", 0, Err);
			return Err;
		}

		#ifdef DISPLAY_INT
			MxOpen();
			MxSetVisible(1);
			MxDisPlaySetBackgroundImageF(gradImSS2, IwSS2, IhSS2, "gradImSS2");
			MxCommand("figure(2); subplot(1,1,1); imagesc(gradImSS2);  colormap gray; axis off; axis image; title('Im');");
			MxPutDouble((double)IwSS2, "IwSS2"); MxPutDouble((double)IhSS2, "IhSS2"); 
			MxPutDouble((double)RoiCoordHere.XMin, "xMinR"); MxPutDouble((double)RoiCoordHere.XMax, "xMaxR"); 
			MxPutDouble((double)RoiCoordHere.YMin, "yMinR"); MxPutDouble((double)RoiCoordHere.YMax, "yMaxR"); 

			for (i = 0; i<extractedCoeffs.nbExtractedDiaphragm; i++){
				MxPutDouble((double)extractedCoeffs.Param[i][0], "a");
				MxPutDouble((double)extractedCoeffs.Param[i][1], "b"); 
				MxPutDouble((double)extractedCoeffs.Param[i][2], "c");
				MxCommand("figure(2); x = [xMinR:xMaxR]; y = min(yMaxR,max(yMinR,a*(x-IwSS2/2).*(x-IwSS2/2)+b*(x-IwSS2/2)+c)); subplot(1,1,1); hold on; plot(1+x,1+y,'y'); hold off;");
			}
		#endif

		for (i=0; i<extractedCoeffs.nbExtractedDiaphragm; i++){
			extractedCoeffs.Param[i][0] /= EXTRA_SSFACTOR_HOUGH;
			extractedCoeffs.Param[i][2] *= EXTRA_SSFACTOR_HOUGH;
		}

		// *** Diaphragm position update
		for (k=0; k<extractedCoeffs.nbExtractedDiaphragm; k++){
			coeffTmpIn.Param[0] = extractedCoeffs.Param[k][0]; 
			coeffTmpIn.Param[1] = extractedCoeffs.Param[k][1]; 
			coeffTmpIn.Param[2] = extractedCoeffs.Param[k][2];
			Err = DiaParabolicContrastTracking(DiaHdl, Im, Iw, Ih, RoiCoord, DELTA_TRACK * Iw / 1024, TRACK_STEPMAX * Iw / 1024
									   , NB_ITERATIONS_TRACK, coeffTmpIn, &coeffTmpOut);
			if (Err){
				ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaExtractDiaphragm", "DiaParabolicContrastTracking", 0, Err);
				return Err;
			}
			extractedCoeffs.Param[k][0] = coeffTmpOut.Param[0];
			extractedCoeffs.Param[k][1] = coeffTmpOut.Param[1];
			extractedCoeffs.Param[k][2] = coeffTmpOut.Param[2];
		}

		// *** Representative diaphram extraction over the tLimit first images
		Err = DiaClassif(DiaHdl, ImFull, SSFactor*Iw, SSFactor, RoiCoord, &extractedCoeffs
							, SHIFT_CONT * SSFactor * Iw / 1024, Rot, Ang, coeffOut, detectedDiaphragm);
		if (Err){
			ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaExtractDiaphragm", "DiaClassif", 0, Err);
			return Err;
		}
	} else if (*detectedDiaphragm == 1) {
			// *** Diaphragm position update
			Err = DiaParabolicContrastTracking(DiaHdl, Im, Iw, Ih, RoiCoord, DELTA_TRACK * Iw / 1024, TRACK_STEPMAX * Iw / 1024
									   , NB_ITERATIONS_TRACK, coeffIn, coeffOut);
			if (Err){
				ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaExtractDiaphragm", "DiaParabolicContrastTracking", 0, Err);
				return Err;
			}
		}

	#ifdef DISPLAY_INT
		MxOpen();
		MxSetVisible(1);
		MxDisPlaySetBackgroundImageS(Im, Iw, Ih, "Im");
		MxCommand("figure(1); subplot(1,1,1); imagesc(Im);  colormap gray; axis off; axis image; title('Im');");
		MxPutDouble((double)Iw, "Iw"); MxPutDouble((double)Ih, "Ih"); 
		MxPutDouble((double)RoiCoord.XMin, "xMin"); MxPutDouble((double)RoiCoord.XMax, "xMax"); 
		MxPutDouble((double)RoiCoord.YMin, "yMin"); MxPutDouble((double)RoiCoord.YMax, "yMax"); 
		MxPutDouble((double)coeffOut->Param[0], "a");
		MxPutDouble((double)coeffOut->Param[1], "b"); MxPutDouble((double)coeffOut->Param[2], "c");
		
		if (*detectedDiaphragm == -1)
			MxCommand("figure(1); x = [xMin:xMax]; y = min(yMax,max(yMin,a*(x-Iw/2).*(x-Iw/2)+b*(x-Iw/2)+c)); subplot(1,1,1); hold on; plot(1+x,1+y,'g'); hold off;");
		if (*detectedDiaphragm == 0)
			MxCommand("figure(1); x = [xMin:xMax]; y = min(yMax,max(yMin,a*(x-Iw/2).*(x-Iw/2)+b*(x-Iw/2)+c)); subplot(1,1,1); hold on; plot(1+x,1+y,'b'); hold off;");
		if (*detectedDiaphragm == 1)
			MxCommand("figure(1); x = [xMin:xMax]; y = min(yMax,max(yMin,a*(x-Iw/2).*(x-Iw/2)+b*(x-Iw/2)+c)); subplot(1,1,1); hold on; plot(1+x,1+y,'r'); hold off;");
		//MxCommand("pause(1);");	
	#endif

	return 0;
}

// **************************************************************************************
// Diaphragm extraction in Fluoros
// **************************************************************************************

int DiaExtractDiaphragmIniUpToTR(CDiaHdl DiaHdl, short *Im, int Iw, int Ih, CDiaRoiCoord RoiCoord, int t,
							 CDiaDiaphragmParab coeffIn, CDiaDiaphragmParab *coeffOut){

	CDia			*This = (CDia*)DiaHdl.Pv;
	CDiaProc		*DiaProc = &(This->DiaProc);
	int             Err;
    
    {
        if ( (Iw > DiaProc->IwSS1Alloc) || (Ih > DiaProc->IhSS1Alloc) ){
            char            Reason[128];
		    sprintf_s(Reason, 128, "Image size above allocated sizes: subsampled1 (%d,%d) vs (%d,%d)\n"
                        , Iw, DiaProc->IwSS1Alloc, Ih, DiaProc->IhSS1Alloc);
		    ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaExtractDiaphragm", DIA_ERR_IMSIZEABOVEALLOCATED, Reason);
		    return DIA_ERR_IMSIZEABOVEALLOCATED;
	    }

       DiaProc->IwSS1 = Iw; DiaProc->IhSS1 = Ih;
       DiaProc->IwSS2 = Iw /EXTRA_SSFACTOR_HOUGH; DiaProc->IhSS2 = Ih /EXTRA_SSFACTOR_HOUGH;
    }

	if (t == 0){
		// *** Hough parabol extraction

		// Gradient computation
		float minHisto, maxHisto, gradThres;
		CDiaRoiCoord RoiCoordHere;

		RoiCoordHere.XMin = RoiCoord.XMin / EXTRA_SSFACTOR_HOUGH; RoiCoordHere.XMax = RoiCoord.XMax / EXTRA_SSFACTOR_HOUGH;
		RoiCoordHere.YMin = RoiCoord.YMin / EXTRA_SSFACTOR_HOUGH; RoiCoordHere.YMax = RoiCoord.YMax / EXTRA_SSFACTOR_HOUGH;

		Err = DiaSubSampleImage(DiaHdl, Im, DiaProc->ImSubSS2, Iw, Ih, DiaProc->IwSS2, DiaProc->IhSS2, EXTRA_SSFACTOR_HOUGH);
		if (Err){
			ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaExtractDiaphragmIniUpToTR", "DiaSubSampleImage", 0, Err);
			return Err;
		}

        Err = DiaGradientComputation(DiaHdl, DiaProc->ImSubSS2, DiaProc->gradImSS2, DiaProc->IwSS2, DiaProc->IhSS2, RoiCoordHere, SIGMA_GRADIENT * DiaProc->IwSS2 / 1024);
		if (Err){
			ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaExtractDiaphragm", "DiaGradientComputation", 0, Err);
			return Err;
		}

		// Threshold computation - for subsequent percentile thresholding	
		Err = DiaHistoConstruction(DiaHdl,DiaProc->gradImSS2, DiaProc->IwSS2, DiaProc->IhSS2, RoiCoordHere, NB_BINS_PERCENTILE, DiaProc->hist, &minHisto, &maxHisto);
		if (Err){
			ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaExtractDiaphragmIniUpToTR", "DiaHistoConstruction", 0, Err);
			return Err;
		}
		gradThres = DiaPercentile(DiaProc->IwSS2, DiaProc->IhSS2, RoiCoordHere, DiaProc->hist, NB_BINS_PERCENTILE, minHisto, maxHisto, GRAD_PERCENTILE);

		// 1D Hough extraction
		Err = DiaHoughParabolExtraction1D(DiaHdl, DiaProc->gradImSS2, DiaProc->IwSS2, DiaProc->IhSS2, RoiCoordHere, gradThres, NB_CELLS_PER_DIM_HOUGH
									, coeffIn.Param[0] * EXTRA_SSFACTOR_HOUGH, coeffIn.Param[1], coeffOut);
		if (Err){
			ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaExtractDiaphragmIniUpToTR", "DiaHoughParabolExtraction1D", 0, Err);
			return Err;
		}

		coeffOut->Param[0] /= EXTRA_SSFACTOR_HOUGH;
		coeffOut->Param[2] *= EXTRA_SSFACTOR_HOUGH;

		coeffIn.Param[0] = coeffOut->Param[0]; coeffIn.Param[1] = coeffOut->Param[1]; coeffIn.Param[2] = coeffOut->Param[2];
	}

	// *** Diaphragm position update
	Err = DiaParabolicContrastTracking(DiaHdl, Im, Iw, Ih, RoiCoord, DELTA_TRACK * Iw / 1024, TRACK_STEPMAX * Iw / 1024
								  , NB_ITERATIONS_TRACK, coeffIn, coeffOut);		
	if (Err){
		ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaExtractDiaphragmIniUpToTR", "DiaParabolicContrastTracking", 0, Err);
		return Err;
	}

	return 0;
}