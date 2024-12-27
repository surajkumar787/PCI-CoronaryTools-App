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

// ********************************************************************
// Call the DiaLib routine

int OscDiaphragmDetection(COscHdl OscHdl, short *ImOri){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
    int                 IwProc = ExtractProc->IwProcAngio, IhProc = ExtractProc->IhProcAngio, SSFactor = ExtractProc->SSFactor;

	// Input parameters conversion
	CDiaRoiCoord        DiaRoiCoord;
	CDiaDiaphragmParab DiaCoeffIn, DiaCoeffOut;
	int                 Err;

	DiaRoiCoord.XMin = ExtractProc->ShuttersRoiSafe.XMin; DiaRoiCoord.XMax = ExtractProc->ShuttersRoiSafe.XMax;
	DiaRoiCoord.YMin = ExtractProc->ShuttersRoiSafe.YMin; DiaRoiCoord.YMax = ExtractProc->ShuttersRoiSafe.YMax;
	//DiaRoiCoord.XMin = ExtractProc->ShuttersRoi.XMin; DiaRoiCoord.XMax = ExtractProc->ShuttersRoi.XMax;
	//DiaRoiCoord.YMin = ExtractProc->ShuttersRoi.YMin; DiaRoiCoord.YMax = ExtractProc->ShuttersRoi.YMax;
	
	if (ExtractProc->t == 0){
		DiaCoeffIn.Param[0] = 0.f; DiaCoeffIn.Param[1] = 0.f; DiaCoeffIn.Param[2] = 0.f;
	} else {
		DiaCoeffIn.Param[0] = ExtractProc->DiaphragmParams[0]; 
		DiaCoeffIn.Param[1] = ExtractProc->DiaphragmParams[1]; 
		DiaCoeffIn.Param[2] = ExtractProc->DiaphragmParams[2];
	}
	
	// Detection - from DiaLib
    Err = DiaExtractDiaphragm(ExtractProc->DiaHdl, ExtractProc->ImSub, IwProc, IhProc, ImOri, ExtractProc->RotAngio, ExtractProc->AngAngio
							, SSFactor, DiaRoiCoord, ExtractProc->t, DiaCoeffIn, &ExtractProc->detectedDiaphragm, &DiaCoeffOut);
	if (Err){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrPrintErrorReport(ExtractProc->DiaHdl.ErrHdl);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscDiaphragmDetection", OSC_ERR_DIALIB, "Error in DiaExtractDiaphragm");
		return OSC_ERR_DIALIB;
	}

	// Output
	ExtractProc->DiaphragmParams[0] = DiaCoeffOut.Param[0]; 
	ExtractProc->DiaphragmParams[1] = DiaCoeffOut.Param[1]; 
	ExtractProc->DiaphragmParams[2] = DiaCoeffOut.Param[2];

	return 0;
}

// ********************************************************************
int OscDiaphragmTrackingTR(COscHdl OscHdl){
	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
    int                 IwProc = ExtractProc->IwProcFluoro, IhProc = ExtractProc->IhProcFluoro;
    int                 dX = ExtractProc->deltaTabXSub + ExtractProc->OffsetDiffSizeAngioFluoroXSub,
                            dY = ExtractProc->deltaTabYSub + ExtractProc->OffsetDiffSizeAngioFluoroYSub;

	// Input parameters conversion
	CDiaRoiCoord        DiaRoiCoord;
	CDiaDiaphragmParab  DiaCoeffIn, DiaCoeffOut;

	if (ExtractProc->detectedDiaphragm){
		DiaRoiCoord.XMin = ExtractProc->ShuttersRoi.XMin; DiaRoiCoord.XMax = ExtractProc->ShuttersRoi.XMax;
		DiaRoiCoord.YMin = ExtractProc->ShuttersRoi.YMin; DiaRoiCoord.YMax = ExtractProc->ShuttersRoi.YMax;

		if (ExtractProc->t -ExtractProc->NonInitialSkippedImages ==0){
			// Adaptation to panning
			DiaCoeffIn.Param[0] = ExtractProc->DiaphragmParams[0]; 
			DiaCoeffIn.Param[1] = ExtractProc->DiaphragmParams[1] - 2*ExtractProc->DiaphragmParams[0]*dX; 
			DiaCoeffIn.Param[2] = ExtractProc->DiaphragmParams[2] + ExtractProc->DiaphragmParams[0]*dX*dX
								- ExtractProc->DiaphragmParams[1]*dX + dY;
		} else {
			DiaCoeffIn.Param[0] = ExtractProc->DiaphragmParams[0]; 
			DiaCoeffIn.Param[1] = ExtractProc->DiaphragmParams[1]; 
			DiaCoeffIn.Param[2] = ExtractProc->DiaphragmParams[2];
		}

		// Diaphragm extraction
		int Err = DiaExtractDiaphragmIniUpToTR(ExtractProc->DiaHdl, ExtractProc->ImSub, IwProc, IhProc, DiaRoiCoord
									, ExtractProc->t -ExtractProc->NonInitialSkippedImages, DiaCoeffIn, &DiaCoeffOut);
		if (Err){
            ExtractProc->FatalErrorOnPreviousImage = 1;
			ErrPrintErrorReport(ExtractProc->DiaHdl.ErrHdl);
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscDiaphragmDetection", OSC_ERR_DIALIB, "Error in DiaExtractDiaphragm");
			return OSC_ERR_DIALIB;
		}

		// Output parameters conversion
		ExtractProc->DiaphragmParams[0] = DiaCoeffOut.Param[0]; 
		ExtractProc->DiaphragmParams[1] = DiaCoeffOut.Param[1]; 
		ExtractProc->DiaphragmParams[2] = DiaCoeffOut.Param[2];
	}
	return 0;
}

// ********************************************************************
// Zeroes ridges around the diaphragm, provided
// 1- they have pretty much the same orientation
// 2- the image grayvalue increases from the (supposed) diaphragm to the (supposed) lungs

void OscDiaphragmCorrection(COscHdl OscHdl, short *Gau, float *Rdg, float *Dir, int Iw, int Ih)
{
	int					x;
	float				diffAng;
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	float				a = ExtractProc->DiaphragmParams[0], b = ExtractProc->DiaphragmParams[1], c = ExtractProc->DiaphragmParams[2];
	COscRoiCoord		*RoiCoord = &ExtractProc->ShuttersRoiSafe;
	int					thicknessDiaphragmPix = (int)(OSC_DIAPHRAGM_THICKNESS_IM_RATIO * Ih);

	if (!ExtractProc->detectedDiaphragm)
		return;

	// Loop along the diapgram
	for (x=RoiCoord->XMin; x<=RoiCoord->XMax; x++){

		float y = a*(x-Iw/2)*(x-Iw/2)+b*(x-Iw/2)+c;			// Corresponding y
		float y1 = a*(x-Iw/2+1)*(x-Iw/2+1)+b*(x-Iw/2+1)+c;	// Next one (to compute orientation)
		float theta = (float)atan2((float)y1-y,1.f) ;			// Diaphragm orientation

		// y-limit of potential deletion
		int yyMin = (int)(OSC_CLIP(RoiCoord->YMin, y-thicknessDiaphragmPix+.5f,RoiCoord->YMax)); 
		int yyMax = (int)(OSC_CLIP(RoiCoord->YMin, y+thicknessDiaphragmPix+.5f,RoiCoord->YMax));

		int down = 1;

		// Positionning at top of ROI, while jumping potential vessel
		int yy = OSC_MIN(yyMax + 2, RoiCoord->YMax);
		while ((down) && (yy >= yyMax)){

			if ( (Gau[yy*Iw+x] >= Gau[(OSC_MAX(yy-1,RoiCoord->YMin))*Iw+x]) 
				|| (Gau[yy*Iw+x] >= Gau[(OSC_MAX(yy-2,RoiCoord->YMin))*Iw+x]))
				down = 0;

			yy--;
		}
		
		// We go from to to bottom, sa long as the image gets darker
		while ((down == 1) && (yy >= yyMin)){
			diffAng = 2.f*OSC_ABS_F(Dir[yy*Iw+x] - theta);
			while (diffAng > 2*(float)OSC_PI)
				diffAng += - 2*(float)OSC_PI;
			diffAng *= 0.5f;

			// Check that we get darker (point 2)
			if ( (Gau[yy*Iw+x] >= Gau[(OSC_MAX(yy-1,RoiCoord->YMin))*Iw+x]) 
				|| (Gau[yy*Iw+x] >= Gau[(OSC_MAX(yy-2,RoiCoord->YMin))*Iw+x]))
				down = 0;
			else
				// Deletion if local dominant direction compatible with diaphragm
				if (diffAng <= OSC_DIA_DELETION_ANG_THRESHOLD)
					Rdg[yy*Iw+x] = 0 ;
			yy--;
		}
	}
}