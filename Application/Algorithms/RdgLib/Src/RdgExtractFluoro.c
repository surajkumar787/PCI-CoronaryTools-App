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
// Actual Fluoro extraction process 
/*************************************************************************************/
int RdgExtractFluoro(CRdgHdl RdgHdl, int t, short** InIma, CRdgExtractResults* ExtractResults, 
					 CRdgRoiCoord *RoiCoord, // shutters ROI 
					 CRdgRoiCoord *CatheterTipRoiCoord,
					 int CatheterBodyRemovalFlag, CRdgRoiCoord *CatheterBodyRoiCoord, float CatheterBodyDirection, float CatheterBodyAngleRange)
{
	CRdg             *This = (CRdg*)RdgHdl;
	CRdgExtractParam *ExtractParam;
	CRdgExtractProc  *ExtractProc;
	int               IwOri, IhOri, IwProc, IhProc, NbThrPel;
	int               nKernelSigmas;
	int               NbLabels;
	float             NoiseSig = 0, RdgThresh;
	int              *NbPelPerLab;
	float            *CumRdgPerLab;
	int              *NbExtremPerLab;
	float             L1Bg = 0;
	int               significant_motion_flag, stitch_detected_flag;


	CRdgRoiCoord thisCatheterTipRoiCoord;  // Intersection of catheter tip  and shutters ROIs
	CRdgRoiCoord thisCatheterBodyRoiCoord; // Intersection of catheter body and shutters ROIs


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
	}

	ExtractProc->CatheterTipFoundFlag  = FALSE;
	ExtractProc->CatheterBodyFoundFlag = FALSE;

	IwOri  = ExtractProc->IwOri;  IhOri  = ExtractProc->IhOri;
	IwProc = ExtractProc->IwProc; IhProc = ExtractProc->IhProc;


	// Determine the number of scales in ExtractParam->KernelSigmas
	for (nKernelSigmas=0; nKernelSigmas<RDG_CST_MAX_SIGMAS; nKernelSigmas++)
		if (ExtractParam->KernelSigmas[nKernelSigmas]==0)
			break;


	// get ROI coordinates
	ExtractProc->RoiCoord.XMin = RoiCoord->XMin / ExtractProc->SSFactor;
	ExtractProc->RoiCoord.XMax = RoiCoord->XMax / ExtractProc->SSFactor;
	ExtractProc->RoiCoord.YMin = RoiCoord->YMin / ExtractProc->SSFactor;
	ExtractProc->RoiCoord.YMax = RoiCoord->YMax / ExtractProc->SSFactor;

	//
	// get a local catheter tip ROI, which is the intersection 
	// between shutters ROI and catheter tip ROI
	//
	thisCatheterTipRoiCoord.XMin = RDG_MAX(CatheterTipRoiCoord->XMin, RoiCoord->XMin);
	thisCatheterTipRoiCoord.XMax = RDG_MIN(CatheterTipRoiCoord->XMax, RoiCoord->XMax);
	thisCatheterTipRoiCoord.YMin = RDG_MAX(CatheterTipRoiCoord->YMin, RoiCoord->YMin);
	thisCatheterTipRoiCoord.YMax = RDG_MIN(CatheterTipRoiCoord->YMax, RoiCoord->YMax);

	thisCatheterTipRoiCoord.XMin = thisCatheterTipRoiCoord.XMin / ExtractProc->SSFactor;
	thisCatheterTipRoiCoord.XMax = thisCatheterTipRoiCoord.XMax / ExtractProc->SSFactor;
	thisCatheterTipRoiCoord.YMin = thisCatheterTipRoiCoord.YMin / ExtractProc->SSFactor;
	thisCatheterTipRoiCoord.YMax = thisCatheterTipRoiCoord.YMax / ExtractProc->SSFactor;


	//
	// get a local catheter body ROI, which is the intersection 
	// between shutters ROI and catheter body ROI
	if (CatheterBodyRemovalFlag)
	{
		thisCatheterBodyRoiCoord.XMin = RDG_MAX(CatheterBodyRoiCoord->XMin, RoiCoord->XMin);
		thisCatheterBodyRoiCoord.XMax = RDG_MIN(CatheterBodyRoiCoord->XMax, RoiCoord->XMax);
		thisCatheterBodyRoiCoord.YMin = RDG_MAX(CatheterBodyRoiCoord->YMin, RoiCoord->YMin);
		thisCatheterBodyRoiCoord.YMax = RDG_MIN(CatheterBodyRoiCoord->YMax, RoiCoord->YMax);

		thisCatheterBodyRoiCoord.XMin = thisCatheterBodyRoiCoord.XMin / ExtractProc->SSFactor;
		thisCatheterBodyRoiCoord.XMax = thisCatheterBodyRoiCoord.XMax / ExtractProc->SSFactor;
		thisCatheterBodyRoiCoord.YMin = thisCatheterBodyRoiCoord.YMin / ExtractProc->SSFactor;
		thisCatheterBodyRoiCoord.YMax = thisCatheterBodyRoiCoord.YMax / ExtractProc->SSFactor;
	}

	// Possible sub-sampling of the original image
	RdgSubSampleImage(InIma, ExtractProc->ImSub, ExtractProc->ImPel, IwProc, IhProc, ExtractProc->SSFactor);



	////////////////////////////
	//
	// Wire Guide Tip Extraction
	//
	////////////////////////////


	// Separate catheter image from image, by morphological filtering
	// first dilation
	RdgMorphDilateRectSE(ExtractProc->ImSub, 
		ExtractProc->ImDil, IwProc, IhProc, 
		&ExtractProc->RoiCoord, 
		ExtractParam->MaxWireTipThickness, ExtractParam->MaxWireTipThickness, /* sructuring element width and height */
		ExtractParam->MaxWireTipThickness/2 /* active point on width dimension */, ExtractParam->MaxWireTipThickness/2 /* active point on height dimension */);
	// then erosion
	// final result is the closing of image
	RdgMorphErodeRectSE(ExtractProc->ImDil, 
		ExtractProc->ImClo, IwProc, IhProc, 
		&ExtractProc->RoiCoord, 
		ExtractParam->MaxWireTipThickness, ExtractParam->MaxWireTipThickness, // sructuring element width and height */
		ExtractParam->MaxWireTipThickness/2 /* active point on width dimension */, ExtractParam->MaxWireTipThickness/2 /* active point on height dimension */);


#if 0
	// Tip-scale Ridge calculation (with possible normalization)
	RdgEnhanceRidge(ExtractProc->ImClo, 
		ExtractProc->ImRwo, ExtractProc->ImNoE, 
		ExtractProc->ImGau, ExtractProc->ImRdg, 
		ExtractProc->ImDir, ExtractProc->ImNoi, IwProc, IhProc, ExtractProc->DynOri, 
		&ExtractProc->RoiCoord, 
		ExtractParam->NormalizationOriFlag, ExtractParam->NormalizationLFSigma, 
		ExtractParam->RidgeType, ExtractParam->KernelSigmas[0] // only first scale */, 
		ExtractParam->EdgeFromOriFlag, ExtractParam->NoEdgeFactor, 
		ExtractParam->ThresholdType, 
		// ExtractParam->ThinningFlag, // direction image is needed by non-maxima suppression process */
		FALSE, // direction image not needed on closure image */
		&NoiseSig, (ExtractParam->L1ThresholdFlag)?&L1Bg:NULL);
#else
	// Tip-scale Ridge image calculation (with possible normalization)
	RdgEnhanceMultiScaleRidge(ExtractProc->ImClo, 		ExtractProc->ImGau, ExtractProc->ImRdg, 		ExtractProc->ImDir, ExtractProc->ImSca, 
		IwProc, IhProc, ExtractProc->DynOri,		&ExtractProc->RoiCoord, 		ExtractParam->NormalizationOriFlag, ExtractParam->NormalizationLFSigma,
		ExtractParam->RidgeType, 		ExtractParam->KernelSigmas, nKernelSigmas /* 1 */ /* only first scale is taken into account */, ExtractParam->RidgeGamma,
		ExtractParam->EdgeFromOriFlag, ExtractParam->NoEdgeFactor		// ExtractParam->ThinningFlag, /* direction image is needed by non-maxima suppression process */
		);
#endif


#if 1
	// Tip-scale Ridge calculation (with possible normalization)
	RdgEnhanceRidge(ExtractProc->ImSub, 
		ExtractProc->ImRwo, ExtractProc->ImNoE, 
		ExtractProc->ImGau, ExtractProc->ImTipRdg, 
		ExtractProc->ImDir, ExtractProc->ImNoi, IwProc, IhProc, ExtractProc->DynOri, 
		&ExtractProc->RoiCoord, 
		ExtractParam->NormalizationOriFlag, ExtractParam->NormalizationLFSigma, 
		ExtractParam->RidgeType, ExtractParam->KernelSigmas[0] /* only first scale */, 
		ExtractParam->EdgeFromOriFlag, ExtractParam->NoEdgeFactor, 
		ExtractParam->ThresholdType, 
		ExtractParam->ThinningFlag || CatheterBodyRemovalFlag, // direction image is needed by non-maxima suppression or catheter body removal process */
		&NoiseSig, (ExtractParam->L1ThresholdFlag)?&L1Bg:NULL);
#else
	// Tip-scale Ridge image calculation (with possible normalization)
	RdgEnhanceMultiScaleRidge(ExtractProc->ImSub, 
		ExtractProc->ImRwo, ExtractProc->ImNoE, 
		ExtractProc->ImGau, ExtractProc->ImTipRdg, 
		ExtractProc->ImDir, ExtractProc->ImSca, 
		IwProc, IhProc, ExtractProc->DynOri, 
		&ExtractProc->RoiCoord, 
		ExtractParam->NormalizationOriFlag, ExtractParam->NormalizationLFSigma,
		ExtractParam->RidgeType, 
		ExtractParam->KernelSigmas, 1 // only first scale is taken into account */, ExtractParam->RidgeGamma,                     
		ExtractParam->EdgeFromOriFlag, ExtractParam->NoEdgeFactor 
		, ExtractParam->ThinningFlag || CatheterBodyRemovalFlag // direction image is needed by non-maxima suppression or catheter body removal process */
		);
#endif

	// Performs ImTipRdg = ImTipRdg-Ridgeness(ImClo)
	RdgSubtractImageAndKeepPositive(ExtractProc->ImTipRdg, ExtractProc->ImRdg, IwProc, IhProc, &ExtractProc->RoiCoord);

#if RDG_TIMING
	RdgSysTimeMs("RdgTipEnhanceRidge");
	RdgSysTimeMs(0);
#endif

	// Thinning of ridge image
	if(ExtractParam->ThinningFlag)
		RdgSuppressNonMaxima(ExtractProc->ImRwo, 
		ExtractProc->ImTipRdg, ExtractProc->ImDir, IwProc, IhProc, 
		&ExtractProc->RoiCoord);
#if RDG_TIMING
	RdgSysTimeMs("RdgSuppressNonMaxima");
	RdgSysTimeMs(0);
#endif
	// Catheter Body removal
	if (CatheterBodyRemovalFlag)
	{
		RdgRemoveCatheterBody(ExtractProc->ImTipRdg, ExtractProc->ImDir, IwProc, IhProc, &thisCatheterBodyRoiCoord, CatheterBodyDirection, CatheterBodyAngleRange);

#if RDG_TIMING
		RdgSysTimeMs("RdgTipRemoveCatheterBody");
		RdgSysTimeMs(0);
#endif
	}

	// Estimation of background/static objects ridgeness
	RdgBackgroundEstimation(t, ExtractProc->ImTipRdg, ExtractProc->ImTipCircBuffer, IwProc, IhProc, ExtractProc->CircBufferSize, &ExtractProc->RoiCoord, ExtractProc->ImTipBkg, ExtractProc->ImTipFrg);

#if RDG_TIMING
	RdgSysTimeMs("RdgTipBackgroundEstimation");
	RdgSysTimeMs(0);
#endif

	// Threshold of wire guide tip ridge image
	// High threshold

	if (t<=ExtractParam->FreezeThresholdsAt)
	{
		// before thresholds are frozen
		// threshold the top percentile of histogram
		ExtractParam->ThresholdType = RDG_CST_NB_PIX_THR;

		// Target number of pixels in percentage of non-shutter surface
		ExtractParam->NbPixelsKept = (int) (ExtractParam->WireTipThresholdPercent1 / 100 * (ExtractProc->RoiCoord.XMax-ExtractProc->RoiCoord.XMin)*(ExtractProc->RoiCoord.YMax-ExtractProc->RoiCoord.YMin));

		RdgThreshold(ExtractProc->ImTipFrg, ExtractProc->ImTipSee, IwProc, IhProc, 
			&ExtractProc->RoiCoord, 
			NoiseSig, ExtractParam->ThresholdType, 
			ExtractParam->NoiseThreshold, ExtractParam->AbsoluteThreshold,
			ExtractParam->L1ThresholdFlag, L1Bg,
			ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
			&ExtractProc->TresholdTm1,
			&ExtractProc->ThresholdWeightTm1);

		// keep track of absolute threshold
		ExtractProc->TipAbsHighThreshold = RdgThresh;
	}
	else
	{
		// thresholds are frozen
		// use absolute high threshold
		ExtractParam->ThresholdType = RDG_CST_ABSOLUTE_THR;

		RdgThreshold(ExtractProc->ImTipFrg, ExtractProc->ImTipSee, IwProc, IhProc, 
			&ExtractProc->RoiCoord, 
			NoiseSig, ExtractParam->ThresholdType, 
			ExtractParam->NoiseThreshold, ExtractProc->TipAbsHighThreshold,
			ExtractParam->L1ThresholdFlag, L1Bg,
			ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
			&ExtractProc->TresholdTm1,
			&ExtractProc->ThresholdWeightTm1);
	}

	// 
	// make sure some pixels were thresholded
	// 
	if (NbThrPel==0)
	{
		fprintf(stderr, "******* Tip High Threshold divided by 2 ********\n");

		RdgThreshold(ExtractProc->ImTipFrg, ExtractProc->ImTipSee, IwProc, IhProc, 
			&ExtractProc->RoiCoord, 
			NoiseSig, RDG_CST_ABSOLUTE_THR, 
			ExtractParam->NoiseThreshold, RdgThresh/2 /* last absolute threshold divided by 2 */,
			ExtractParam->L1ThresholdFlag, L1Bg,
			ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
			&ExtractProc->TresholdTm1,
			&ExtractProc->ThresholdWeightTm1);
	}

	#if MX_FLAG
		MxPutMatrixUC(ExtractProc->ImTipThr[0], IwProc, IhProc, "RdgThrHigTip");
#endif

	// High labelling
	if (NbThrPel)
	{
		// allocation of label feature's arrays
		NbPelPerLab    = (int*)   malloc((NbThrPel) * sizeof(int)); 
		CumRdgPerLab   = (float*) malloc((NbThrPel) * sizeof(float));
		NbExtremPerLab = (int*)   malloc((NbThrPel) * sizeof(int)); 

		// First labelling to obtain seed image for Tip
		RdgLabelFromSeed(ExtractProc->ImTipSee, ExtractProc->ImTipSee, ExtractProc->ImTipLab, ExtractProc->ImTipFrg, ExtractProc->ImDir, IwProc, IhProc, 
			&ExtractProc->RoiCoord, &NbLabels, NbPelPerLab, CumRdgPerLab, NbExtremPerLab,
			ExtractParam->NbRangePixels1, ExtractParam->DirLabelFlag, 
			ExtractParam->MaxDirDiff1, ExtractProc->SubTipLength);

#if RDG_TIMING
		RdgSysTimeMs("RdgLabel");
		RdgSysTimeMs(0);
#endif
		// Threshold the labels by size
		RdgThresholdLabelsBySize(ExtractProc->ImTipLab, ExtractProc->ImTipSee, 			&ExtractProc->RoiCoord, 
			NbLabels, NbPelPerLab, ExtractParam->MinNbPelPerWireTipLab1, 1);

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
	RdgSysTimeMs("RdgTipThreshold");
	RdgSysTimeMs(0); 
#endif

	// Threshold of wire guide tip ridge image
	// Low threshold

	if (t<=ExtractParam->FreezeThresholdsAt)
	{
		// before thresholds are frozen
		// threshold the top percentile of histogram
		ExtractParam->ThresholdType = RDG_CST_NB_PIX_THR;

		// Target number of pixels in percentage of non-shutter surface
		ExtractParam->NbPixelsKept = (int) (ExtractParam->WireTipThresholdPercent2 / 100 * (ExtractProc->RoiCoord.XMax-ExtractProc->RoiCoord.XMin)*(ExtractProc->RoiCoord.YMax-ExtractProc->RoiCoord.YMin));

		RdgThreshold(ExtractProc->ImTipFrg, ExtractProc->ImTipThr, IwProc, IhProc, 
			&ExtractProc->RoiCoord, 
			NoiseSig, ExtractParam->ThresholdType, 
			ExtractParam->NoiseThreshold, ExtractParam->AbsoluteThreshold,
			ExtractParam->L1ThresholdFlag, L1Bg,
			ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
			&ExtractProc->TresholdTm1,
			&ExtractProc->ThresholdWeightTm1);

		// keep track of minimum of absolute threshold
		ExtractProc->TipAbsLowThreshold = RdgThresh;
	}
	else
	{
		// thresholds are frozen
		// use absolute low threshold
		ExtractParam->ThresholdType = RDG_CST_ABSOLUTE_THR;

		RdgThreshold(ExtractProc->ImTipFrg, ExtractProc->ImTipThr, IwProc, IhProc, 
			&ExtractProc->RoiCoord, 
			NoiseSig, ExtractParam->ThresholdType, 
			ExtractParam->NoiseThreshold, ExtractProc->TipAbsLowThreshold,
			ExtractParam->L1ThresholdFlag, L1Bg,
			ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
			&ExtractProc->TresholdTm1,
			&ExtractProc->ThresholdWeightTm1);
	}

	// 
	// make sure some pixels were thresholded and enforce TipAbsLowThreshold<TipAbsHighThreshold
	// 
	if (NbThrPel==0 || ExtractProc->TipAbsLowThreshold>=ExtractProc->TipAbsHighThreshold)
	{
		fprintf(stderr, "******* Tip Low Threshold divided by 2 ********\n");

		RdgThreshold(ExtractProc->ImTipFrg, ExtractProc->ImTipThr, IwProc, IhProc, 
			&ExtractProc->RoiCoord, 
			NoiseSig, RDG_CST_ABSOLUTE_THR, 
			ExtractParam->NoiseThreshold, RdgThresh/2 /* last absolute threshold divided by 2 */,
			ExtractParam->L1ThresholdFlag, L1Bg,
			ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
			&ExtractProc->TresholdTm1,
			&ExtractProc->ThresholdWeightTm1);
	}

	#if MX_FLAG
		MxPutMatrixUC(ExtractProc->ImTipThr[0], IwProc, IhProc, "RdgThrLowTip");
#endif

	if (NbThrPel)
	{
		// allocation of label feature's arrays
		NbPelPerLab    = (int*)  malloc((NbThrPel) * sizeof(int)); 
		CumRdgPerLab   = (float*)malloc((NbThrPel) * sizeof(float));
		NbExtremPerLab = (int*)  malloc((NbThrPel) * sizeof(int)); 

		// Second labelling to grow from seed image
		RdgLabelFromSeed(ExtractProc->ImTipSee, ExtractProc->ImTipThr, ExtractProc->ImTipLab, ExtractProc->ImTipFrg, ExtractProc->ImDir, IwProc, IhProc, 
			&ExtractProc->RoiCoord, &NbLabels, NbPelPerLab, CumRdgPerLab, NbExtremPerLab,
			ExtractParam->NbRangePixels2, ExtractParam->DirLabelFlag, 
			ExtractParam->MaxDirDiff2, ExtractProc->SubTipLength);
#if RDG_TIMING
		RdgSysTimeMs("RdgLabel");
		RdgSysTimeMs(0);
#endif
#if MX_FLAG
			MxPutMatrixInt(ExtractProc->ImLab[0], IwProc, IhProc, "RdgImLabTip");
#endif

		// Non-linear ridgeness measure
		if (ExtractParam->DoubleBistouFlag)
		{
			RdgAsymetricRidgenessLabels(ExtractProc->ImTipLab, ExtractProc->ImGau, ExtractProc->ImTipRdg, ExtractProc->ImDir, IwProc, IhProc, 
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
		// Remove labels that are not at 3/4 the strength of the strongest one
		RdgThresholdLabels(InIma, ExtractProc->ImSub, ExtractProc->ImPel, ExtractProc->ImTipLab, 			ExtractProc->ImTipRdg, ExtractProc->ImDir, ExtractProc->ImTipThr, 
			IwOri, IhOri, IwProc, IhProc, ExtractProc->SSFactor, &ExtractProc->RoiCoord, &(ExtractProc->PreObjectList), NbLabels, 
			NbPelPerLab, CumRdgPerLab, ExtractParam->NbLabelsKept, 3.0f/4, ExtractParam->ExtraMeasuresFlag);

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
	RdgSysTimeMs("RdgThresholdLabelsTip");
	RdgSysTimeMs(0);
#endif
	// Create mask from labels
	RdgBinaryMaskFromLabels(ExtractProc->ImTipMask, ExtractProc->ImTipLab, IwProc, IhProc, &ExtractProc->RoiCoord);

	//
	// If a global motion has to be computed to remove potential stitches, 
	// it should be done on the tip picture
	//
#if 1
	// Calculate global motion of the image objects 
	// Possible translations found from LabelCentroids from t-1 to t (with rejection if distance too large)
	// For each translation: matching measure = number of overlapping pixels 
	// Best translation validated if enough overlapping pixels sufficiently "ridgy"
	// GlobalMotion is set to zero if best translation invalidated
	RdgCalcGlobalMotion(t, ExtractProc->ImTipLab, ExtractProc->ImLabTm1, ExtractProc->ImTipRdg, IwProc, IhProc, 
		ExtractParam->GlobalMotionFlag, &ExtractProc->RoiCoord, 
		&(ExtractProc->PreObjectList), ExtractProc->CentroidTm1, &(ExtractProc->NbObjTm1), RdgThresh, 
		ExtractParam->ThinningFlag, ExtractParam->FastProfileFlag, &(ExtractProc->GlobalMotion));
#if RDG_TIMING
	RdgSysTimeMs("RdgCalcGlobalMotion");
	RdgSysTimeMs(0);
#endif
	// Determine if objects undergo global motion or zero motion, and for how many frames
	RdgVerifyGlobalMotion(t, ExtractProc->ImTipLab, IwProc, IhProc, &ExtractProc->RoiCoord, 
		&(ExtractProc->PreObjectList), ExtractParam->ThinningFlag, &(ExtractProc->GlobalMotion), 
		ExtractProc->PastGlobalMotion, ExtractProc->ImPastThr, ExtractParam->GlobalMotionThreshold, 
		&significant_motion_flag, &stitch_detected_flag, ExtractProc->ImGMMask);

#if RDG_TIMING
	RdgSysTimeMs("RdgVerifyGlobalMotion");
	RdgSysTimeMs(0);
#endif

	// Accumulate stitch detected flags
	ExtractProc->StitchCumulativeFlag |= stitch_detected_flag;
#endif


	#if MX_FLAG

		printf("MX_FLAG is defined\n");
		MxOpen() ;
		MxSetVisible(1) ;
		MxCommand("cd D:/MatLab/Rdg");

		MxPutMatrixFloat(ExtractProc->ImGau[0], IwProc, IhProc, "RdgGauTip");            
		MxPutMatrixFloat(ExtractProc->ImTipRdg[0], IwProc, IhProc, "RdgRdgTip");            
		// MxPutMatrixUC(ExtractProc->ImGMMask[0], IwProc, IhProc, "RdgMask");            
		// MxPutMatrixShort(ExtractProc->ImSub[0], IwProc, IhProc, "RdgSub");  
		// MxPutMatrixUC(ExtractProc->ImThr[0], IwProc, IhProc, "RdgThr");  
		MxPutMatrixFloat(ExtractProc->ImDir[0], IwProc, IhProc, "RdgDir");  
		MxPutMatrixShort(InIma[0], IwOri, IhOri, "RdgOri");  

		// MxCommand("figure(1); imshow(RdgMask); imagesc(RdgMask)");
		// MxCommand("figure(2); imshow(RdgRdg);  imagesc(RdgRdg)");
		// MxCommand("figure(3); imshow(RdgThr);  imagesc(RdgThr)");
	
#endif



	////////////////////////////
	//
	// Catheter Tip Extraction
	//
	////////////////////////////



	// All scales (catheter+tip) Ridge image calculation (with possible normalization)
	RdgEnhanceMultiScaleRidge(ExtractProc->ImSub, 		ExtractProc->ImGau, ExtractProc->ImRdg, 		ExtractProc->ImDir, ExtractProc->ImSca, 
		IwProc, IhProc, ExtractProc->DynOri,		&ExtractProc->RoiCoord, 		ExtractParam->NormalizationOriFlag, ExtractParam->NormalizationLFSigma,
		ExtractParam->RidgeType, 		ExtractParam->KernelSigmas, nKernelSigmas, ExtractParam->RidgeGamma,
		ExtractParam->EdgeFromOriFlag, ExtractParam->NoEdgeFactor
		);

	// Thinning of ridge image
	if(ExtractParam->ThinningFlag)
		RdgSuppressNonMaxima(ExtractProc->ImRwo, 
		ExtractProc->ImRdg, ExtractProc->ImDir, IwProc, IhProc, 
		&ExtractProc->RoiCoord);

#if RDG_TIMING
	RdgSysTimeMs("RdgSuppressNonMaxima");
	RdgSysTimeMs(0);
	RdgSysTimeMs("RdgEnhanceMultiScaleRidge");
	RdgSysTimeMs(0);
#endif

	// Catheter Body removal
	if (CatheterBodyRemovalFlag)
	{
		RdgRemoveCatheterBody(ExtractProc->ImRdg, ExtractProc->ImDir, IwProc, IhProc, &thisCatheterBodyRoiCoord, CatheterBodyDirection, CatheterBodyAngleRange);
#if RDG_TIMING
		RdgSysTimeMs("RdgRemoveCatheterBody");
		RdgSysTimeMs(0);
#endif
	}

	// Estimation of background/static objects ridgeness
	RdgBackgroundEstimation(t, ExtractProc->ImRdg, ExtractProc->ImCircBuffer, IwProc, IhProc, ExtractProc->CircBufferSize, &ExtractProc->RoiCoord, ExtractProc->ImBkg, ExtractProc->ImFrg);
#if RDG_TIMING
	RdgSysTimeMs("RdgBackgroundEstimation");
	RdgSysTimeMs(0);
#endif

	// Threshold of all scales ridge image
	// High threshold
	// Operate only on the restricted area of thisCatheterTipRoiCoord

	if (t<=ExtractParam->FreezeThresholdsAt)
	{
		// before thresholds are frozen
		// threshold the top percentile of histogram
		ExtractParam->ThresholdType = RDG_CST_NB_PIX_THR;

		// Target number of pixels in percentage of non-shutter surface
		ExtractParam->NbPixelsKept = (int) (ExtractParam->ThresholdPercent1 / 100 * (ExtractProc->RoiCoord.XMax-ExtractProc->RoiCoord.XMin)*(ExtractProc->RoiCoord.YMax-ExtractProc->RoiCoord.YMin));

		RdgThreshold(ExtractProc->ImFrg, ExtractProc->ImSee, IwProc, IhProc, 
			&ExtractProc->RoiCoord, // &thisCatheterTipRoiCoord // ROI around catheter tip, not all non-shutter area */, 
			NoiseSig, ExtractParam->ThresholdType, 
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

		RdgThreshold(ExtractProc->ImFrg, ExtractProc->ImSee, IwProc, IhProc, 
			&ExtractProc->RoiCoord, // &thisCatheterTipRoiCoord // ROI around catheter tip, not all non-shutter area */, 
			NoiseSig, ExtractParam->ThresholdType, 
			ExtractParam->NoiseThreshold, ExtractProc->AbsHighThreshold,
			ExtractParam->L1ThresholdFlag, L1Bg,
			ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
			&ExtractProc->TresholdTm1,
			&ExtractProc->ThresholdWeightTm1);
	}

	// 
	// make sure some pixels were thresholded
	// 
	if (NbThrPel==0)
	{
		fprintf(stderr, "******* Catheter High Threshold divided by 2 ********\n");

		RdgThreshold(ExtractProc->ImFrg, ExtractProc->ImSee, IwProc, IhProc, 
			&ExtractProc->RoiCoord, // &thisCatheterTipRoiCoord // ROI around catheter tip, not all non-shutter area */, 
			NoiseSig, RDG_CST_ABSOLUTE_THR, 
			ExtractParam->NoiseThreshold, RdgThresh/2 /* last absolute threshold divided by 2 */,
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

		// First labelling to obtain seed image
		// Label only in the catheter tip ROI
		RdgLabelFromSeed(ExtractProc->ImSee, ExtractProc->ImSee, ExtractProc->ImLab, ExtractProc->ImFrg, ExtractProc->ImDir, IwProc, IhProc, 
			&thisCatheterTipRoiCoord, // ROI around catheter tip, not all non-shutter area */, // &ExtractProc->RoiCoord, 
			&NbLabels, NbPelPerLab, CumRdgPerLab, NbExtremPerLab, 
			ExtractParam->NbRangePixels1, ExtractParam->DirLabelFlag, 
			ExtractParam->MaxDirDiff1, ExtractProc->SubTipLength);

#if RDG_TIMING
		RdgSysTimeMs("RdgLabel");
		RdgSysTimeMs(0);
#endif
		// Threshold the labels by size
		// Note the ROI larger than the one used during labelling, 
		// to remove all unlabelled thresholded pixels outside of the catheter tip ROI
		RdgThresholdLabelsBySize(ExtractProc->ImLab, ExtractProc->ImSee, 			&ExtractProc->RoiCoord, // &thisCatheterTipRoiCoord // ROI around catheter tip, not all non-shutter area */, 
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

		RdgThreshold(ExtractProc->ImFrg, ExtractProc->ImThr, IwProc, IhProc, 
			&ExtractProc->RoiCoord, 
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

		RdgThreshold(ExtractProc->ImFrg, ExtractProc->ImThr, IwProc, IhProc, 
			&ExtractProc->RoiCoord, 
			NoiseSig, ExtractParam->ThresholdType, 
			ExtractParam->NoiseThreshold, ExtractProc->AbsLowThreshold,
			ExtractParam->L1ThresholdFlag, L1Bg,
			ExtractParam->NbPixelsKept, &NbThrPel, &RdgThresh, 
			&ExtractProc->TresholdTm1,
			&ExtractProc->ThresholdWeightTm1);
	}

	// 
	// make sure some pixels were thresholded
	// 
	if (NbThrPel==0 || ExtractProc->AbsLowThreshold>=ExtractProc->AbsHighThreshold)
	{
		fprintf(stderr, "******* Catheter Low Threshold divided by 2 ********\n");

		RdgThreshold(ExtractProc->ImFrg, ExtractProc->ImThr, IwProc, IhProc, &ExtractProc->RoiCoord, 
			NoiseSig, RDG_CST_ABSOLUTE_THR, 
			ExtractParam->NoiseThreshold, RdgThresh/2, // last absolute threshold divided by 2 */,
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

		// Second labelling to grow from seed image
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
			ExtractProc->ImRdg, ExtractProc->ImDir, ExtractProc->ImThr, IwOri, IhOri, IwProc, IhProc, 
			ExtractProc->SSFactor, 			&ExtractProc->RoiCoord, NULL /* &(ExtractProc->PreObjectList) */, NbLabels, 
			NbPelPerLab, CumRdgPerLab, ExtractParam->NbLabelsKept, 0.1f,			ExtractParam->ExtraMeasuresFlag);

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
	// Create mask from labels and merge tip mask
	RdgBinaryMaskFromLabelsAndMask(ExtractProc->ImMask, ExtractProc->ImLab, ExtractProc->ImTipMask, IwProc, IhProc, &ExtractProc->RoiCoord);


	// Fill extract results structures
	RdgFillFluoroResults(ExtractResults, ExtractProc);

#if RDG_TIMING
	RdgSysTimeMs("RdgFillResults");

#endif


	return 0;
}




/*************************************************************************************/
// RdgFillFluoroResults 
/*************************************************************************************/
void RdgFillFluoroResults(CRdgExtractResults *ExtractResults, CRdgExtractProc *ExtractProc)
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
	ExtractResults->Volatile.Advanced.Clo      = ExtractProc->ImClo;
	ExtractResults->Volatile.Advanced.Rwo      = ExtractProc->ImRwo;
	ExtractResults->Volatile.Advanced.NoE      = ExtractProc->ImNoE;
	ExtractResults->Volatile.Advanced.Noi      = ExtractProc->ImNoi;
	ExtractResults->Volatile.Advanced.Gau      = ExtractProc->ImGau;
	ExtractResults->Volatile.Advanced.Rdg      = ExtractProc->ImRdg;
	ExtractResults->Volatile.Advanced.TipRdg   = ExtractProc->ImTipRdg;
	ExtractResults->Volatile.Advanced.Dir      = ExtractProc->ImDir;
	ExtractResults->Volatile.Advanced.Sca      = ExtractProc->ImSca;
	ExtractResults->Volatile.Advanced.Bkg      = ExtractProc->ImBkg;
	ExtractResults->Volatile.Advanced.Frg      = ExtractProc->ImFrg;
	ExtractResults->Volatile.Advanced.See      = ExtractProc->ImSee;
	ExtractResults->Volatile.Advanced.Thr      = ExtractProc->ImThr;
	ExtractResults->Volatile.Advanced.TipBkg   = ExtractProc->ImTipBkg;
	ExtractResults->Volatile.Advanced.TipFrg   = ExtractProc->ImTipFrg;
	ExtractResults->Volatile.Advanced.TipSee   = ExtractProc->ImTipSee;
	ExtractResults->Volatile.Advanced.TipThr   = ExtractProc->ImTipThr;
	ExtractResults->Volatile.Advanced.Lab      = ExtractProc->ImLab;
	ExtractResults->Volatile.Advanced.LabTm1   = ExtractProc->ImLabTm1;
	ExtractResults->Volatile.Advanced.TipLab   = ExtractProc->ImTipLab;
	ExtractResults->Volatile.Advanced.Msk      = ExtractProc->ImMask;
	ExtractResults->Volatile.Advanced.TipMsk   = ExtractProc->ImTipMask;
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
