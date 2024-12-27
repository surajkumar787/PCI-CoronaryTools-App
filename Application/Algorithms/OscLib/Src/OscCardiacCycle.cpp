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

//#include "MxLab.h"

/*************************************************************************************/
int OscHeartCycleSelection(COscHdl OscHdl, int *IndexOfCycleStart, int *IndexOfCycleEnd){
	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc     = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);

	int					k, l, m, cardiacCycleLength, ErrorCodeOut = 0;
	float				*diagSum, *d1;
	float				bestDiagMatchingScore;

	int					CardiacCycleMinFrames	= (int)ceil(ExtractProc->frameRateAngio*60.f / OSC_MAX_CARDIACFREQ);
	int					CardiacCycleMaxFrames	= (int)ceil(ExtractProc->frameRateAngio*60.f / OSC_MIN_CARDIACFREQ);
	int					CardiacCycleIntegration	= (int) (CardiacCycleMinFrames+CardiacCycleMaxFrames)/2;
    int                 IwProc = ExtractProc->IwProcAngio, IhProc = ExtractProc->IhProcAngio;

    if (ExtractSequence->nExtractPictures < 2*CardiacCycleIntegration){
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscHeartCycleSelection", OSC_ERR_SHORT_ANGIOSEQ, "Angio too short for integration"); 
		return OSC_ERR_SHORT_ANGIOSEQ;
    }

   // *** Cardiac cycle length
   if (IhProc * IwProc > CardiacCycleMaxFrames){
	   diagSum = ExtractProc->BufsF[0]; d1 = ExtractProc->BufsF[1];
   } else {
	    OSC_MALLOC(diagSum, (CardiacCycleMaxFrames+1)); OSC_MALLOC(d1, (CardiacCycleMaxFrames+1)); 
   }
   OSC_MEMZERO(diagSum, (CardiacCycleMaxFrames+1));

	// * Loop on potential cardiac cycles length
   for (k = CardiacCycleMinFrames; k<= CardiacCycleMaxFrames; k++){

	   if (ExtractSequence->nExtractPictures-k-CardiacCycleIntegration <=0)
		   continue;			// Not enough angio frames to consider that possible cycle length
	   
		// Are the frames in [l, l+CardiacCycleIntegration] well registered with those k frames further?
	   for (l=0; l<ExtractSequence->nExtractPictures-k-CardiacCycleIntegration; l++){

		   float sum = 0;
		   for (m=l; m<l+CardiacCycleIntegration; m++)
			   sum += ExtractSequence->MatchingScores[m][m+k];
		   diagSum[k] = OSC_MAX(diagSum[k], sum);
	   }
   }

   // * Heart cycle matching score analysis

   // keep only local maxima, not neighboring zero (untested) values
	OSC_MEMCPY(d1, diagSum, (CardiacCycleMaxFrames+1));
	for (k=CardiacCycleMinFrames; k<=OSC_MIN(CardiacCycleMaxFrames, ExtractSequence->nExtractPictures-1); k++)
		if (k==0 || k==OSC_MIN(CardiacCycleMaxFrames, ExtractSequence->nExtractPictures-1) || d1[k-1]==0 || d1[k+1]==0 || d1[k]<d1[k-1] || d1[k]<d1[k+1])
			diagSum[k] = 0;

	// Best heart cycle length
	bestDiagMatchingScore = 0.f; cardiacCycleLength = -1;
	for (k=CardiacCycleMinFrames; k<=OSC_MIN(CardiacCycleMaxFrames, ExtractSequence->nExtractPictures-1); k++)
		if (bestDiagMatchingScore < diagSum[k]){
			bestDiagMatchingScore = diagSum[k]; cardiacCycleLength = k;
		}

	if (cardiacCycleLength == -1){		
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscHeartCycleSelection", OSC_ERR_NO_CARDIAC_CYCLE, "Could not extract cardiac cycle");
		return OSC_ERR_NO_CARDIAC_CYCLE;
	}

	// *** Heart cycle selection
    float *indicators;
    COscRoiCoord *RoiCoord = &ExtractProc->ShuttersRoiSafe;
    if (IhProc * IwProc > ExtractSequence->nExtractPictures){
	   indicators = ExtractProc->BufsF[0];
   } else {
	   OSC_MALLOC(indicators, (ExtractSequence->nExtractPictures+1));
   }

    // Compute indicator
    for (k=0; k<ExtractSequence->nExtractPictures; k++){
        COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);

        indicators[k] = 0;
        for (int y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
		    for (int x=RoiCoord->XMin; x<=RoiCoord->XMax; x++)
                if (ExtractPicture->ImLabS[y*IwProc+x] >= 2)
                    indicators[k] += ((float)ExtractPicture->ImFrgUC[y*IwProc+x]*(ExtractPicture->maxFrg-ExtractPicture->minFrg)
                                            + ExtractPicture->minFrg);
    }
    //{
    //    MxOpen(); MxSetVisible(1);
    //    MxPutVectorFloat(indicators, ExtractSequence->nExtractPictures, "indicator");
    //    MxCommand("figure(1); subplot('Position',[0.05,0.05,0.4,0.25]); plot(indicator/max(indicator(:)),'b-'); hold on;");
    //}

    // Best injected cycle
    float bestInjCycle = -FLT_MAX;
    int   indiceBeginBestInjCycle = -1;
    for (k=0; k<ExtractSequence->nExtractPictures-cardiacCycleLength-1; k++){
        float cycleIndicatorValue = 0.f;
        for (int i=k; i<=k+cardiacCycleLength; i++)
            cycleIndicatorValue += indicators[i];
        if (cycleIndicatorValue > bestInjCycle){
            bestInjCycle = cycleIndicatorValue; 
            indiceBeginBestInjCycle = k;
        }
    }

    // Time of contrast uptake
    int indiceContrastUptake = 0;
    for (k=indiceBeginBestInjCycle; k>=1; k--){
        if (indicators[k-1] < OSC_HCSELECT_COEFF_UPTAKE * indicators[indiceBeginBestInjCycle]){
                indiceContrastUptake = k; break;
        }
    }

    // Computing of the reference boosting mask
    unsigned char * MaskIn          = ExtractProc->BufsUC[0];
    unsigned char * Buffer          = (unsigned char *)ExtractProc->BufsF[1];
    unsigned char * MaskProxBoost   = (unsigned char *)ExtractProc->BufsF[2];

    memset(MaskIn, 0, IwProc*IhProc*sizeof(unsigned char));
    for (int p=0; p<IwProc*IhProc; p++)
        if (ExtractSequence->ExtractPictures[indiceContrastUptake].ImLabS[p] > 1)
            MaskIn[p] = 1;

    int HalfSizeMaskDilatationPix = OSC_RND( OSC_HCSELECT_HALFSIZE_MASK_DILATATION_MM / (ExtractProc->isoCenterPixSize * ExtractProc->SSFactor) );

    OscDilationRect(MaskIn, IwProc, IhProc, &ExtractProc->ShuttersRoi, HalfSizeMaskDilatationPix
                    , HalfSizeMaskDilatationPix, Buffer, MaskProxBoost);

    // Computing new indicators
    for (k=0; k<ExtractSequence->nExtractPictures; k++){
        COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);

        indicators[k] = 0;
        for (int y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
		    for (int x=RoiCoord->XMin; x<=RoiCoord->XMax; x++)
                if (ExtractPicture->ImLabS[y*IwProc+x] >= 2){
                    indicators[k] += ((float)ExtractPicture->ImFrgUC[y*IwProc+x]*(ExtractPicture->maxFrg-ExtractPicture->minFrg)
                                            + ExtractPicture->minFrg)
                                    * (1.f+ (OSC_HCSELECT_COEFF_BOOST_PROX-1.f)*MaskProxBoost[y*IwProc+x]);
                }
    }

    // Selecting most injected cycle wrt that criterion
    *IndexOfCycleStart = -1;
    bestInjCycle = -FLT_MAX;
    for (k=0; k<ExtractSequence->nExtractPictures-cardiacCycleLength-1; k++){
        float cycleIndicatorValue = 0.f;
        for (int i=k; i<=k+cardiacCycleLength; i++)
            cycleIndicatorValue += indicators[i];
        if (cycleIndicatorValue > bestInjCycle){
            bestInjCycle = cycleIndicatorValue; 
            *IndexOfCycleStart = k;
        }
    }
    *IndexOfCycleEnd        = *IndexOfCycleStart+cardiacCycleLength;

    //{
    //    MxOpen(); MxSetVisible(1);
    //    MxPutVectorFloat(indicators, ExtractSequence->nExtractPictures, "indicatorBoosted");
    //    MxPutInt((long)indiceBeginBestInjCycle,"indiceBeginBestInjCycle");
    //    MxPutInt((long)indiceContrastUptake,"indiceContrastUptake");
    //    MxPutInt((long)*IndexOfCycleStart,"indiceStart");
    //    MxPutInt((long)*IndexOfCycleEnd,"indiceEnd");
    //    MxPutMatrixUC(MaskProxBoost, IwProc, IhProc, "MaskProxBoost");
    //    MxPutMatrixUC(MaskIn, IwProc, IhProc, "Mask");
    //    MxPutMatrixUC(ExtractSequence->ExtractPictures[indiceContrastUptake].ImFrgUC, IwProc, IhProc, "ImFrgUCInj");

    //    MxCommand("plot(indicatorBoosted/max(indicatorBoosted(:)),'m-');");
    //    MxCommand("plot([indiceBeginBestInjCycle,indiceBeginBestInjCycle],[0,1],'b--'); plot([indiceContrastUptake,indiceContrastUptake],[0,1],'b-');");
    //    MxCommand("plot([indiceStart,indiceStart],[0,1],'r-'); plot([indiceEnd,indiceEnd],[0,1],'r-'); hold off;");
    //    MxCommand("ImTmp = double(Mask.*ImFrgUCInj); ImTmp = ImTmp / max(ImTmp(:));");
    //    MxCommand("figure(1); subplot('Position',[0.5,0,0.5,1]); imagesc(50*double(MaskProxBoost)' + 200*ImTmp'); colormap gray; axis image; axis off;");
    //}
    
	// Finalizing
   if (IhProc * IwProc <= CardiacCycleMaxFrames){
	   free(indicators);
   } 

    if (IhProc * IwProc <= CardiacCycleMaxFrames){
	   free(diagSum); free(d1);
   } 

    return ErrorCodeOut; 
}