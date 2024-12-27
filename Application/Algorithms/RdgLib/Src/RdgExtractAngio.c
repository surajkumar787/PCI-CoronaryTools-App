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
#include <float.h>
#include <RdgDev.h>

#include <assert.h>
//#include <MxLab.h>

/*************************************************************************************/
// Actual Angio extraction process 
/*************************************************************************************/
int RdgExtractAngio(CRdgHdl RdgHdl, int t, short** InIma, CRdgExtractResults* ExtractResults, CRdgRoiCoord *RoiCoord, 
					int CatheterBodyRemovalFlag, CRdgRoiCoord *CatheterBodyRoiCoord, float CatheterBodyDirection, float CatheterBodyAngleRange, 
					int ExtractCatheterROIsFlag)
{
	CRdg             *This = (CRdg*)RdgHdl;
	CRdgExtractParam *ExtractParam;
	CRdgExtractProc  *ExtractProc;
	int              IwOri, IhOri, IwProc, IhProc, NbThrPel;
	int              nKernelSigmas;
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

		ExtractProc->AbsHighThreshold = FLT_MAX;
		ExtractProc->AbsLowThreshold  = FLT_MAX;

		ExtractProc->CatheterTipLargestSize  = 0;
		ExtractProc->CatheterBodyLargestSize = 0;
	}

	ExtractProc->CatheterTipFoundFlag  = FALSE;
	ExtractProc->CatheterBodyFoundFlag = FALSE;

	IwOri  = ExtractProc->IwOri;  IhOri  = ExtractProc->IhOri;
	IwProc = ExtractProc->IwProc; IhProc = ExtractProc->IhProc;


	// get ROI coordinates
	ExtractProc->RoiCoord.XMin = RoiCoord->XMin / ExtractProc->SSFactor;
	ExtractProc->RoiCoord.XMax = RoiCoord->XMax / ExtractProc->SSFactor;
	ExtractProc->RoiCoord.YMin = RoiCoord->YMin / ExtractProc->SSFactor;
	ExtractProc->RoiCoord.YMax = RoiCoord->YMax / ExtractProc->SSFactor;


	// Determine the number of scales in ExtractParam->KernelSigmas
	for (nKernelSigmas=0; nKernelSigmas<RDG_CST_MAX_SIGMAS; nKernelSigmas++)
		if (ExtractParam->KernelSigmas[nKernelSigmas]==0)
			break;

	// Possible sub-sampling of the original image
	RdgSubSampleImage(InIma, ExtractProc->ImSub, ExtractProc->ImPel, IwProc, IhProc, ExtractProc->SSFactor);

	// Multiscale Ridge image calculation (with possible normalization)
	RdgEnhanceMultiScaleRidge(ExtractProc->ImSub, 
		ExtractProc->ImGau, ExtractProc->ImRdg, 
		ExtractProc->ImDir, ExtractProc->ImSca, 
		IwProc, IhProc, ExtractProc->DynOri, 
		&ExtractProc->RoiCoord, 
		ExtractParam->NormalizationOriFlag, ExtractParam->NormalizationLFSigma,
		ExtractParam->RidgeType, 
		ExtractParam->KernelSigmas, nKernelSigmas, ExtractParam->RidgeGamma, 
		ExtractParam->EdgeFromOriFlag, ExtractParam->NoEdgeFactor
		);

#if RDG_TIMING
	RdgSysTimeMs("RdgEnhanceRidge");
	RdgSysTimeMs(0);
#endif

	// Thinning of ridge image
	if(ExtractParam->ThinningFlag)
		RdgSuppressNonMaxima(ExtractProc->ImRwo, 
		ExtractProc->ImRdg, ExtractProc->ImDir, IwProc, IhProc, 
		&ExtractProc->RoiCoord);

#if RDG_TIMING
	RdgSysTimeMs("RdgSuppressNonMaxima");
	RdgSysTimeMs(0);
#endif

	// Catheter Body removal
	if (CatheterBodyRemovalFlag)
	{
		RdgRemoveCatheterBody(ExtractProc->ImRdg, ExtractProc->ImDir, IwProc, IhProc, CatheterBodyRoiCoord, CatheterBodyDirection, CatheterBodyAngleRange);

#if RDG_TIMING
		RdgSysTimeMs("RdgRemoveCatheterBody");
		RdgSysTimeMs(0);
#endif
	}

	// Estimation of background/static objects ridgeness
	if (ExtractParam->BackgroundEstimationFlag)
		RdgBackgroundEstimation(t, ExtractProc->ImRdg, ExtractProc->ImCircBuffer, IwProc, IhProc, ExtractProc->CircBufferSize, &ExtractProc->RoiCoord, ExtractProc->ImBkg, ExtractProc->ImFrg);
	else
		memcpy(ExtractProc->ImFrg[0], ExtractProc->ImRdg[0], IwProc*IhProc*sizeof(ExtractProc->ImFrg[0][0]));

#if RDG_TIMING 
	RdgSysTimeMs("RdgBackgroundEstimation");
	RdgSysTimeMs(0);
#endif

	// Threshold of ridge image
	// High threshold

	if (t<=ExtractParam->FreezeThresholdsAt)
	{
		// before thresholds are frozen
		// threshold the top percentile of histogram
		ExtractParam->ThresholdType = RDG_CST_NB_PIX_THR;

		// Target number of pixels in percentage of non-shutter surface
		ExtractParam->NbPixelsKept = (int) (ExtractParam->ThresholdPercent1 / 100 * (ExtractProc->RoiCoord.XMax-ExtractProc->RoiCoord.XMin)*(ExtractProc->RoiCoord.YMax-ExtractProc->RoiCoord.YMin));

		RdgThreshold(ExtractProc->ImFrg, ExtractProc->ImSee, IwProc, IhProc, &ExtractProc->RoiCoord, 
			NoiseSig, RDG_CST_NB_PIX_THR, 
			ExtractParam->NoiseThreshold, ExtractParam->AbsoluteThreshold,
			ExtractParam->L1ThresholdFlag, L1Bg,
			ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
			&ExtractProc->TresholdTm1,
			&ExtractProc->ThresholdWeightTm1);

		// keep track of absolute threshold
		ExtractProc->AbsHighThreshold = RdgThresh;
	}
	else
	{
		// thresholds are frozen
		// use absolute high threshold
		ExtractParam->ThresholdType = RDG_CST_ABSOLUTE_THR;

		RdgThreshold(ExtractProc->ImFrg, ExtractProc->ImSee, IwProc, IhProc, &ExtractProc->RoiCoord, 
			NoiseSig, ExtractParam->ThresholdType, 
			ExtractParam->NoiseThreshold, ExtractProc->AbsHighThreshold,
			ExtractParam->L1ThresholdFlag, L1Bg,
			ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
			&ExtractProc->TresholdTm1,
			&ExtractProc->ThresholdWeightTm1);
	}

#if MX_FLAG
	MxPutMatrixUC(ExtractProc->ImThr[0], IwProc, IhProc, "RdgThrHig");
#endif

	// High labelling
	if (NbThrPel)
	{
		// allocation of label feature's arrays
		NbPelPerLab    = (int*)  malloc((NbThrPel) * sizeof(int)); 
		CumRdgPerLab   = (float*)malloc((NbThrPel) * sizeof(float));
		NbExtremPerLab = (int*)  malloc((NbThrPel) * sizeof(int)); 

		// Labelling
		RdgLabelFromSeed(ExtractProc->ImSee, ExtractProc->ImSee, ExtractProc->ImLab, ExtractProc->ImFrg, ExtractProc->ImDir, IwProc, IhProc, 
			&ExtractProc->RoiCoord, &NbLabels, NbPelPerLab, CumRdgPerLab, NbExtremPerLab,
			ExtractParam->NbRangePixels1, ExtractParam->DirLabelFlag, 
			ExtractParam->MaxDirDiff1, ExtractProc->SubTipLength);

#if RDG_TIMING 
		RdgSysTimeMs("RdgLabel"); 
		RdgSysTimeMs(0);
#endif
		// Threshold the labels by size
		RdgThresholdLabelsBySize(ExtractProc->ImLab, ExtractProc->ImSee, 			&ExtractProc->RoiCoord, 
			NbLabels, NbPelPerLab, ExtractParam->MinNbPelPerLab1, 1);

		// freeing of label feature's arrays
		free(NbPelPerLab);
		free(CumRdgPerLab);
		free(NbExtremPerLab);
	}
	else
	{
		// (NbThrPel == 0), no object can be found
		NbLabels = 0;
		fflush(stderr);
	}


#if RDG_TIMING 
	RdgSysTimeMs("RdgThreshold");
	RdgSysTimeMs(0); 
#endif

	// Threshold of ridge image
	// Low threshold

	if (t<=ExtractParam->FreezeThresholdsAt)
	{
		// before thresholds are frozen
		// threshold the top percentile of histogram
		ExtractParam->ThresholdType = RDG_CST_NB_PIX_THR;

		// Target number of pixels in percentage of non-shutter surface
		ExtractParam->NbPixelsKept = (int) (ExtractParam->ThresholdPercent2 / 100 * (ExtractProc->RoiCoord.XMax-ExtractProc->RoiCoord.XMin)*(ExtractProc->RoiCoord.YMax-ExtractProc->RoiCoord.YMin));

		RdgThreshold(ExtractProc->ImFrg, ExtractProc->ImThr, IwProc, IhProc, &ExtractProc->RoiCoord, 
			NoiseSig, ExtractParam->ThresholdType, 
			ExtractParam->NoiseThreshold, ExtractParam->AbsoluteThreshold,
			ExtractParam->L1ThresholdFlag, L1Bg,
			ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
			&ExtractProc->TresholdTm1,
			&ExtractProc->ThresholdWeightTm1);

		// keep track of minimum of absolute threshold
		ExtractProc->AbsLowThreshold = RdgThresh;
	}
	else
	{
		// thresholds are frozen
		// use absolute low threshold
		ExtractParam->ThresholdType = RDG_CST_ABSOLUTE_THR;

		RdgThreshold(ExtractProc->ImFrg, ExtractProc->ImThr, IwProc, IhProc, &ExtractProc->RoiCoord, 
			NoiseSig, ExtractParam->ThresholdType, 
			ExtractParam->NoiseThreshold, ExtractProc->AbsLowThreshold,
			ExtractParam->L1ThresholdFlag, L1Bg,
			ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
			&ExtractProc->TresholdTm1,
			&ExtractProc->ThresholdWeightTm1);
	}

	#if MX_FLAG
		MxPutMatrixUC(ExtractProc->ImThr[0], IwProc, IhProc, "RdgThrLow");
#endif

	if (NbThrPel)
	{
		// allocation of label feature's arrays
		NbPelPerLab    = (int*)  malloc((NbThrPel) * sizeof(int)); 
		CumRdgPerLab   = (float*)malloc((NbThrPel) * sizeof(float));
		NbExtremPerLab = (int*)  malloc((NbThrPel) * sizeof(int)); 

		// Labelling
		RdgLabelFromSeed(ExtractProc->ImSee, ExtractProc->ImThr, ExtractProc->ImLab, ExtractProc->ImFrg, ExtractProc->ImDir, IwProc, IhProc, 
			&ExtractProc->RoiCoord, &NbLabels, NbPelPerLab, CumRdgPerLab, NbExtremPerLab, 
			ExtractParam->NbRangePixels2, ExtractParam->DirLabelFlag, 
			ExtractParam->MaxDirDiff2, ExtractProc->SubTipLength);

#if RDG_TIMING 
		RdgSysTimeMs("RdgLabel");
		RdgSysTimeMs(0);
#endif


		#if MX_FLAG
			MxPutMatrixInt(ExtractProc->ImLab[0], IwProc, IhProc, "RdgImLab");
#endif

		// Non-linear ridgeness measure
		if (ExtractParam->DoubleBistouFlag)
		{
			RdgAsymetricRidgenessLabels(ExtractProc->ImLab, ExtractProc->ImGau, ExtractProc->ImRdg, ExtractProc->ImDir, IwProc, IhProc, 
				ExtractParam->KernelSigmas[0], RdgThresh, &ExtractProc->RoiCoord, NbLabels, NbPelPerLab, CumRdgPerLab, 
				ExtractParam->NbRangePixels2, ExtractProc->SubTipLength, 
				!(ExtractParam->NormalizationOriFlag), 
				ExtractProc->RdgFilterNorm);

#if RDG_TIMING 
			RdgSysTimeMs("RdgDoubleBistou");
			RdgSysTimeMs(0);
#endif
		}

		// Threshold of labels
		RdgThresholdLabels(InIma, ExtractProc->ImSub, ExtractProc->ImPel, ExtractProc->ImLab, 
			ExtractProc->ImRdg, ExtractProc->ImDir, ExtractProc->ImThr, IwOri, IhOri, IwProc, IhProc,			ExtractProc->SSFactor, 
			&ExtractProc->RoiCoord, NULL /* &(ExtractProc->PreObjectList) */, NbLabels, 
			NbPelPerLab, CumRdgPerLab, ExtractParam->NbLabelsKept, 0.1f, 			ExtractParam->ExtraMeasuresFlag);

#if RDG_TIMING 
		RdgSysTimeMs("RdgThresholdLabels");
		RdgSysTimeMs(0);
#endif
		if (ExtractCatheterROIsFlag)
		{
			// Determine catheter end and/or extra catheter body
			// Need to have a larger size than the ones found previously
			RdgGetCathetersBoundingBoxes(ExtractProc->ImLab, IwProc, IhProc, &ExtractProc->RoiCoord, NbLabels, NbPelPerLab, 5 /* margin in pixels */, 
				&ExtractProc->CatheterTipFoundFlag,  &ExtractProc->CatheterTipLargestSize,  &ExtractProc->CatheterTipROI, 
				&ExtractProc->CatheterBodyFoundFlag, &ExtractProc->CatheterBodyLargestSize, &ExtractProc->CatheterBodyROI, &ExtractProc->CatheterBodyDir);

#if 0 // suppressed because now integrated in RdgGetCathetersBoundingBoxes() through tensor method
			// If extra catheter body, get median orientation of thresholded pels in this ROI
			if (ExtractProc->CatheterBodyFoundFlag)
			{
				RdgGetMedianDirection(ExtractProc->ImDir, ExtractProc->ImLab, IwProc, IhProc, &ExtractProc->CatheterBodyROI, &ExtractProc->CatheterBodyDir);
			}
#endif

#if RDG_TIMING 
			RdgSysTimeMs("RdgGetCathetersBoundingBoxes");
			RdgSysTimeMs(0);
#endif
		}

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

#if RDG_TIMING 
		RdgSysTimeMs("RdgThresholdLabels");
		RdgSysTimeMs(0);
#endif
	}

	// Create mask from labels
	RdgBinaryMaskFromLabels(ExtractProc->ImMask, ExtractProc->ImLab, IwProc, IhProc, &ExtractProc->RoiCoord);

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
	RdgVerifyGlobalMotion(t, ExtractProc->ImLab, IwProc, IhProc,  &ExtractProc->RoiCoord, 
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
	RdgFillAngioResults(ExtractResults, ExtractProc);


#if RDG_TIMING 
	RdgSysTimeMs("RdgFillResults");
#endif                


	return 0;
}




/*************************************************************************************/
// RdgFillAngioResults 
/*************************************************************************************/
void RdgFillAngioResults(CRdgExtractResults *ExtractResults, CRdgExtractProc *ExtractProc)
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
	ExtractResults->Volatile.Advanced.Sca      = ExtractProc->ImSca;
	ExtractResults->Volatile.Advanced.Bkg      = ExtractProc->ImBkg;
	ExtractResults->Volatile.Advanced.Frg      = ExtractProc->ImFrg;
	ExtractResults->Volatile.Advanced.TipBkg   = ExtractProc->ImTipBkg;
	ExtractResults->Volatile.Advanced.TipFrg   = ExtractProc->ImTipFrg;
	ExtractResults->Volatile.Advanced.See      = ExtractProc->ImSee;
	ExtractResults->Volatile.Advanced.Thr      = ExtractProc->ImThr;
	ExtractResults->Volatile.Advanced.Lab      = ExtractProc->ImLab;
	ExtractResults->Volatile.Advanced.LabTm1   = ExtractProc->ImLabTm1;
	ExtractResults->Volatile.Advanced.Msk      = ExtractProc->ImMask;
	ExtractResults->Volatile.Advanced.GMMask   = ExtractProc->ImGMMask;

	// filling object list with the result of the output tracker
	ExtractResults->Volatile.ObjectList = &(ExtractProc->PreObjectList);

	// Catheter tip and body bounding boxes
	ExtractResults->CatheterTipFoundFlag  = ExtractProc->CatheterTipFoundFlag;
	ExtractResults->CatheterTipROI        = ExtractProc->CatheterTipROI;
	ExtractResults->CatheterBodyFoundFlag = ExtractProc->CatheterBodyFoundFlag;
	ExtractResults->CatheterBodyDir       = ExtractProc->CatheterBodyDir;
	ExtractResults->CatheterBodyROI       = ExtractProc->CatheterBodyROI;

	return;
}
