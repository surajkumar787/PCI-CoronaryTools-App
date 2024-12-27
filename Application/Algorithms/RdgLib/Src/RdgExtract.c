// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <stdio.h> 
#include <string.h> 
#include <time.h> 
#include <windows.h> 
#include <RdgDev.h>

#include <assert.h>
//#include <MxLab.h>

/*************************************************************************************/
// Actual extraction process 
/*************************************************************************************/
int RdgExtract(CRdgHdl RdgHdl, int t, short** InIma, CRdgExtractResults* ExtractResults, CRdgRoiCoord *RoiCoord)
{
	CRdg             *This = (CRdg*)RdgHdl;
	CRdgExtractParam *ExtractParam;
	CRdgExtractProc  *ExtractProc;
	int              IwOri, IhOri, IwProc, IhProc, NbThrPel;
	int              NbLabels;
	float            NoiseSig = 0, RdgThresh;
	int              *NbPelPerLab;
	float            *CumRdgPerLab;
	int              *NbExtremPerLab;
	float            L1Bg = 0;
	int              significant_motion_flag, stitch_detected_flag;

#if RDG_TIMING
	RdgSysTimeMs(0);
#endif
	ExtractParam = &(This->ExtractParam);
	ExtractProc  = &(This->ExtractProc);


	// Time management
	// return RDG_WRN_INCORRECT_TIME if there is a time gap, or parameters changed, and t!=0
	if((t != ExtractProc->ExtractTime + 1 || ExtractProc->NewParameters) && t != 0)
	{
		return RDG_WRN_INCORRECT_TIME;
	}
	ExtractProc->ExtractTime = t;

	// if t==0, (re)set a few things
	if (t==0)
	{
		ExtractProc->NewParameters        = FALSE;
		ExtractProc->StitchCumulativeFlag = FALSE; 
		ExtractProc->TresholdTm1          = 0;
		ExtractProc->ThresholdWeightTm1   = 0;
	}

	IwOri  = ExtractProc->IwOri;  IhOri  = ExtractProc->IhOri;
	IwProc = ExtractProc->IwProc; IhProc = ExtractProc->IhProc;

	// get ROI coordinates
	ExtractProc->RoiCoord.XMin = RoiCoord->XMin / ExtractProc->SSFactor;
	ExtractProc->RoiCoord.XMax = RoiCoord->XMax / ExtractProc->SSFactor;
	ExtractProc->RoiCoord.YMin = RoiCoord->YMin / ExtractProc->SSFactor;
	ExtractProc->RoiCoord.YMax = RoiCoord->YMax / ExtractProc->SSFactor;


	// Possible sub-sampling of the original image
	RdgSubSampleImage(InIma, ExtractProc->ImSub, ExtractProc->ImPel, IwProc, IhProc, ExtractProc->SSFactor);

	// Ridge image calculation (with possible normalization)
	RdgEnhanceRidge(ExtractProc->ImSub, 
		ExtractProc->ImRwo, ExtractProc->ImNoE, 
		ExtractProc->ImGau, ExtractProc->ImRdg, 
		ExtractProc->ImDir, ExtractProc->ImNoi, IwProc, IhProc, ExtractProc->DynOri, 
		&ExtractProc->RoiCoord, 
		ExtractParam->NormalizationOriFlag, ExtractParam->NormalizationLFSigma, 
		ExtractParam->RidgeType, ExtractParam->KernelSigmas[0], 
		ExtractParam->EdgeFromOriFlag, ExtractParam->NoEdgeFactor, 
		ExtractParam->ThresholdType, 
		ExtractParam->ThinningFlag, /* direction image is needed by non-maxima suppression process */
		&NoiseSig, (ExtractParam->L1ThresholdFlag)?&L1Bg:NULL);

#if RDG_TIMING
	RdgSysTimeMs("RdgEnhanceRidge");
	RdgSysTimeMs(0);
#endif


	// Thinning of ridge image
	if(ExtractParam->ThinningFlag)
		RdgSuppressNonMaxima(
		ExtractProc->ImRwo, 
		ExtractProc->ImRdg, ExtractProc->ImDir, IwProc, IhProc, 
		&ExtractProc->RoiCoord);

#if RDG_TIMING
	RdgSysTimeMs("RdgSuppressNonMaxima");
	RdgSysTimeMs(0);
#endif

	// Threshold of ridge image
	RdgThreshold(ExtractProc->ImRdg, ExtractProc->ImThr, IwProc, IhProc, &ExtractProc->RoiCoord, 
		NoiseSig, ExtractParam->ThresholdType, 
		ExtractParam->NoiseThreshold, ExtractParam->AbsoluteThreshold,
		ExtractParam->L1ThresholdFlag, L1Bg,
		ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
		&ExtractProc->TresholdTm1,
		&ExtractProc->ThresholdWeightTm1);

#if RDG_TIMING
	RdgSysTimeMs("RdgThreshold");
	RdgSysTimeMs(0);
#endif

	if (NbThrPel)
	{
		// allocation of label feature's arrays
		NbPelPerLab    = (int*)   malloc((NbThrPel) * sizeof(int)); 
		CumRdgPerLab   = (float*) malloc((NbThrPel) * sizeof(float));
		NbExtremPerLab = (int*)   malloc((NbThrPel) * sizeof(int)); 

		// Labelling
		RdgLabelFromSeed(ExtractProc->ImThr, ExtractProc->ImThr, ExtractProc->ImLab, ExtractProc->ImRdg, ExtractProc->ImDir, IwProc, IhProc, 
			&ExtractProc->RoiCoord, &NbLabels, NbPelPerLab, CumRdgPerLab, NbExtremPerLab,
			ExtractParam->NbRangePixels, ExtractParam->DirLabelFlag, 
			(float) ExtractParam->MaxDirDiff, ExtractProc->SubTipLength);

#if RDG_TIMING
		RdgSysTimeMs("RdgLabel");
		RdgSysTimeMs(0);
#endif

		// Non-linear ridgeness measure
		if (ExtractParam->DoubleBistouFlag)
		{
			RdgAsymetricRidgenessLabels(ExtractProc->ImLab, ExtractProc->ImGau, ExtractProc->ImRdg, ExtractProc->ImDir, IwProc, IhProc, 
				ExtractParam->KernelSigmas[0], RdgThresh, &ExtractProc->RoiCoord, NbLabels, NbPelPerLab, CumRdgPerLab, 
				ExtractParam->NbRangePixels, ExtractProc->SubTipLength, 
				!(ExtractParam->NormalizationOriFlag), 
				ExtractProc->RdgFilterNorm);

#if RDG_TIMING
			RdgSysTimeMs("RdgDoubleBistou");
			RdgSysTimeMs(0);
#endif
		}

		// Threshold of labels
		RdgThresholdLabels(InIma, ExtractProc->ImSub, ExtractProc->ImPel, ExtractProc->ImLab, 
			ExtractProc->ImRdg, ExtractProc->ImDir, ExtractProc->ImThr, IwOri, IhOri, IwProc, IhProc, 			ExtractProc->SSFactor, 
			&ExtractProc->RoiCoord, &(ExtractProc->PreObjectList), NbLabels, 			NbPelPerLab, CumRdgPerLab, ExtractParam->NbLabelsKept, 0.1f, 			ExtractParam->ExtraMeasuresFlag);

		// freeing of label feature's arrays
		free(NbPelPerLab);
		free(CumRdgPerLab);
		free(NbExtremPerLab);
	}
	else
	{
		// (NbThrPel == 0), no object can be found
		NbLabels = 0;
		RdgClearObjectList(&ExtractProc->PreObjectList);
		fflush(stderr);
	}

#if RDG_TIMING
	RdgSysTimeMs("RdgThresholdLabels");
	RdgSysTimeMs(0);
#endif

	// Calculate global motion of the image objects 
	// Possible translations found from LabelCentroids from t-1 to t (with rejection if distance too large)
	// For each translation: matching measure = number of overlapping pixels 
	// Best translation validated if enough overlapping pixels sufficiently "ridgy"
	// GlobalMotion is set to zero if best translation invalidated
	RdgCalcGlobalMotion(t, ExtractProc->ImLab, ExtractProc->ImLabTm1, ExtractProc->ImRdg, IwProc, IhProc, 
		ExtractParam->GlobalMotionFlag, &ExtractProc->RoiCoord, 
		&(ExtractProc->PreObjectList), ExtractProc->CentroidTm1, &(ExtractProc->NbObjTm1), RdgThresh, 
		ExtractParam->ThinningFlag, ExtractParam->FastProfileFlag, &(ExtractProc->GlobalMotion));

#if RDG_TIMING
	RdgSysTimeMs("RdgCalcGlobalMotion");
	RdgSysTimeMs(0);
#endif
	// Determine if objects undergo global motion or zero motion, and for how many frames
	RdgVerifyGlobalMotion(t, ExtractProc->ImLab, IwProc, IhProc,&ExtractProc->RoiCoord, 
		&(ExtractProc->PreObjectList), ExtractParam->ThinningFlag, &(ExtractProc->GlobalMotion), 
		ExtractProc->PastGlobalMotion, ExtractProc->ImPastThr, ExtractParam->GlobalMotionThreshold, 
		&significant_motion_flag, &stitch_detected_flag, ExtractProc->ImGMMask);

#if RDG_TIMING
	RdgSysTimeMs("RdgVerifyGlobalMotion");
	RdgSysTimeMs(0);
#endif
	// Accumulate stitch detected flags
	ExtractProc->StitchCumulativeFlag |= stitch_detected_flag;

	// Fill extract results structures
	RdgFillResults(ExtractResults, ExtractProc);

#if RDG_TIMING
	RdgSysTimeMs("RdgFillResults");
#endif

#if MX_FLAG

	printf("MX_FLAG is defined\n");
	MxOpen() ;
	MxSetVisible(1) ;
	MxCommand("cd D:/MatLab/Rdg");

	// MxPutMatrixFloat(ExtractProc->ImRdg[0], IwProc, IhProc, "RdgRdg");            
	MxPutMatrixUC(ExtractProc->ImGMMask[0], IwProc, IhProc, "RdgMask");            
	// MxPutMatrixShort(ExtractProc->ImSub[0], IwProc, IhProc, "RdgSub");  
	// MxPutMatrixUC(ExtractProc->ImThr[0], IwProc, IhProc, "RdgThr");  
	// MxPutMatrixFloat(ExtractProc->ImDir[0], IwProc, IhProc, "RdgDir");  
	MxPutMatrixShort(InIma[0], IwOri, IhOri, "RdgOri");  

	MxCommand("figure(1); imshow(RdgMask); imagesc(RdgMask)");
	// MxCommand("figure(2); imshow(RdgRdg);  imagesc(RdgRdg)");
	// MxCommand("figure(3); imshow(RdgThr);  imagesc(RdgThr)");

#if 0
	{
		int CX[100], CY[100];
		int k, l;

		MxCommand(" figure(2); hold off; RdgOri = RdgOri*0; RdgOri(1,1) = 255; Show(RdgOri); hold on ; ") ;

		for (k=0; k<ExtractResults->Volatile.ObjectList->NbObject; k++)
		{
			int X[1000], Y[1000];

			for (l=0; l<ExtractResults->Volatile.ObjectList->Objects[k].CenterLinePelList.NbPel; l++)
			{
				X[l] = ExtractResults->Volatile.ObjectList->Objects[k].CenterLinePelList.CenterLinePels[l].CenterLinePel.X;
				Y[l] = ExtractResults->Volatile.ObjectList->Objects[k].CenterLinePelList.CenterLinePels[l].CenterLinePel.Y;
			}

			CX[k] = (int) ExtractResults->Volatile.ObjectList->Objects[k].ObjPelList.Centroid.X;
			CY[k] = (int) ExtractResults->Volatile.ObjectList->Objects[k].ObjPelList.Centroid.Y;

			MxPutVectorInt(X,ExtractResults->Volatile.ObjectList->Objects[k].CenterLinePelList.NbPel,"X") ;
			MxPutVectorInt(Y,ExtractResults->Volatile.ObjectList->Objects[k].CenterLinePelList.NbPel,"Y") ;

			if (ExtractResults->Volatile.ObjectList->Objects[k].Measures.RigidMotion)
				MxCommand(" figure(2); scatter(X+1,Y+1,1,'r') ; ") ;
			else
				MxCommand(" figure(2); scatter(X+1,Y+1,1,'g') ; ") ;
		}

		MxPutVectorInt(CX,ExtractResults->Volatile.ObjectList->NbObject,"CX");
		MxPutVectorInt(CY,ExtractResults->Volatile.ObjectList->NbObject,"CY");

		MxCommand(" figure(2); plot(2*CX+1,2*CY+1,'y+') ; ") ;

		MxCommand(" figure(2); hold off; ");
	}
#endif

#endif

return 0;
}



/*************************************************************************************/
// RdgFillResults 
/*************************************************************************************/
void RdgFillResults(CRdgExtractResults *ExtractResults, CRdgExtractProc *ExtractProc)
{
	// set all fields to zero
	memset(ExtractResults, 0, sizeof(ExtractResults[0]));

	// global motion of objects
	ExtractResults->GlobalMotion = ExtractProc->GlobalMotion;

	// stitch detected flag
	ExtractResults->StitchDetectedFlag = ExtractProc->StitchCumulativeFlag;

	// advanced information
	ExtractResults->Volatile.Advanced.IwProc   = ExtractProc->IwProc;
	ExtractResults->Volatile.Advanced.IhProc   = ExtractProc->IhProc;
	ExtractResults->Volatile.Advanced.SSFactor = ExtractProc->SSFactor;
	ExtractResults->Volatile.Advanced.Sub      = ExtractProc->ImSub;
	ExtractResults->Volatile.Advanced.Pel      = ExtractProc->ImPel;
	ExtractResults->Volatile.Advanced.Rwo      = ExtractProc->ImRwo;
	ExtractResults->Volatile.Advanced.NoE      = ExtractProc->ImNoE;
	ExtractResults->Volatile.Advanced.Noi      = ExtractProc->ImNoi;
	ExtractResults->Volatile.Advanced.Gau      = ExtractProc->ImGau;
	ExtractResults->Volatile.Advanced.Rdg      = ExtractProc->ImRdg;
	ExtractResults->Volatile.Advanced.Dir      = ExtractProc->ImDir;
	ExtractResults->Volatile.Advanced.Thr      = ExtractProc->ImThr;
	ExtractResults->Volatile.Advanced.Lab      = ExtractProc->ImLab;
	ExtractResults->Volatile.Advanced.LabTm1   = ExtractProc->ImLabTm1;
	ExtractResults->Volatile.Advanced.GMMask   = ExtractProc->ImGMMask;

	// filling object list with the result of the output tracker
	ExtractResults->Volatile.ObjectList = &(ExtractProc->PreObjectList);

	return;
}
