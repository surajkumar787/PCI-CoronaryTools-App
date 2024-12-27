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

#define OSC_ADAPT_SPATIALFILTERINGVALUES_TOFRAMERATE

/*************************************************************************************/
// Convenience function to set the shutters ROI
/*************************************************************************************/
int OscViterbiIteration(COscHdl OscHdl)
{	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	int					k, CardiacCycleStart = ExtractSequence->CardiacCycleStart
						, CardiacCycleEnd = ExtractSequence->CardiacCycleEnd, CardiacCycleLength = CardiacCycleEnd-CardiacCycleStart+1;
	float				maxFluoroMatchingsScore, bestScore, ratioFrameRate = ExtractProc->frameRateAngio / ExtractProc->frameRateFluoro;

	int tViterbi		= ExtractProc->t % OSC_VITERBI_FLUORO_DEPTH;
	int tViterbiMinus1	= (ExtractProc->t-1) % OSC_VITERBI_FLUORO_DEPTH;

    int nbOfSuccessiveSkippedFrames = ExtractProc->NbOfSuccessiveSkippedFrames;

    while (nbOfSuccessiveSkippedFrames > CardiacCycleLength)
        nbOfSuccessiveSkippedFrames -= CardiacCycleLength;

	// *** Compute Viterbi state score
	maxFluoroMatchingsScore = ExtractProc->FluoroToAngioScore[CardiacCycleStart];
	for (k=CardiacCycleStart+1; k<=CardiacCycleEnd; k++)
		maxFluoroMatchingsScore = OSC_MAX(maxFluoroMatchingsScore, ExtractProc->FluoroToAngioScore[k]);

	OSC_MEMZERO(ExtractProc->ViterbiStateScore[tViterbi], CardiacCycleEnd+1);

	if (ExtractProc->t == 0){
		// * Observation score only
		if (maxFluoroMatchingsScore > 0)
			for (k=CardiacCycleStart; k<=CardiacCycleEnd; k++)
				ExtractProc->ViterbiStateScore[tViterbi][k] = OSC_VITERBI_OBSERVATION_SCORE * ExtractProc->FluoroToAngioScore[k]/maxFluoroMatchingsScore;
	}else {
		// Coefficients defining the piecewise linear cost function (for spatial transition term)
#ifdef OSC_ADAPT_SPATIALFILTERINGVALUES_TOFRAMERATE
		float l1 = (15/ExtractProc->frameRateFluoro) * OSC_VITERBI_SPATIAL_LIMIT1 / (ExtractProc->isoCenterPixSize * ExtractProc->SSFactor);
		float l2 = (15/ExtractProc->frameRateFluoro) * OSC_VITERBI_SPATIAL_LIMIT2 / (ExtractProc->isoCenterPixSize * ExtractProc->SSFactor);
		float l3 = (15/ExtractProc->frameRateFluoro) * OSC_VITERBI_SPATIAL_LIMIT3 / (ExtractProc->isoCenterPixSize * ExtractProc->SSFactor);
        // Reference frame rate 15
#else
		float l1 = OSC_VITERBI_SPATIAL_LIMIT1 / (ExtractProc->isoCenterPixSize * ExtractProc->SSFactor);
		float l2 = OSC_VITERBI_SPATIAL_LIMIT2 / (ExtractProc->isoCenterPixSize * ExtractProc->SSFactor);
		float l3 = OSC_VITERBI_SPATIAL_LIMIT3 / (ExtractProc->isoCenterPixSize * ExtractProc->SSFactor);
#endif
		float v1 = OSC_VITERBI_SPATIAL_VAL1;
		float v2 = OSC_VITERBI_SPATIAL_VAL2;

		for (int k1=CardiacCycleStart; k1<=CardiacCycleEnd; k1++){
            float observationScore, transitionScore, delta;

			// * Observation score
			if (maxFluoroMatchingsScore > 0)
				observationScore = OSC_VITERBI_OBSERVATION_SCORE * ExtractProc->FluoroToAngioScore[k1]/maxFluoroMatchingsScore;
			else
				observationScore = 0.f;
			
			// Exploring the phases it can originate from
			for (int k2=CardiacCycleStart; k2<=CardiacCycleEnd; k2++){

                delta = k2+ratioFrameRate*(nbOfSuccessiveSkippedFrames+1)-k1;
                while (delta >= CardiacCycleLength)
                    delta -= CardiacCycleLength;
                while (delta <= -CardiacCycleLength)
                    delta += CardiacCycleLength;

				transitionScore = (float) (OSC_VITERBI_TEMPORAL_TRANSITION_SCORE*exp(-OSC_SQ((float)delta/ratioFrameRate)/2));
                        // Division by ratioFrameRate to ensure given cost per transition type *in the world of fluoro images*

				// * Spatial constraint
				// Motion composition: dptFtm1Ft = dptFtm1A2 + dptA2A1 - dptFtA1
				int dX = ExtractProc->FluoroToAngioDX[tViterbiMinus1][k2] + ExtractSequence->dXAngioToAngio[k2][k1] - ExtractProc->FluoroToAngioDX[tViterbi][k1];
				int dY = ExtractProc->FluoroToAngioDY[tViterbiMinus1][k2] + ExtractSequence->dYAngioToAngio[k2][k1] - ExtractProc->FluoroToAngioDY[tViterbi][k1];

				// Piecewise cost function
				transitionScore += (float) (OSC_VITERBI_SPATIAL_TRANSITION_SCORE *OscViterbiSpatialTransScore(OscHdl, dX, dY, l1, v1, l2, v2, l3));

				// * Update corresponding state
				ExtractProc->ViterbiStateScore[tViterbi][k1] = OSC_MAX(ExtractProc->ViterbiStateScore[tViterbi][k1]
																		, observationScore + ExtractProc->ViterbiStateScore[tViterbiMinus1][k2] + transitionScore);
			}
		}
	}
	
	
	// *** Select highest Viterbi state score
	bestScore = ExtractProc->ViterbiStateScore[tViterbi][CardiacCycleStart]; ExtractProc->SelectedPhase = CardiacCycleStart;
	for (k=CardiacCycleStart+1; k<=CardiacCycleEnd; k++)
		if (ExtractProc->ViterbiStateScore[tViterbi][k] > bestScore){
			bestScore = ExtractProc->ViterbiStateScore[tViterbi][k];
			ExtractProc->SelectedPhase = k;
		}

	return 0;
}

// ************************************************************************************
float OscViterbiSpatialTransScore(COscHdl OscHdl, int dX, int dY, float d1, float v1, float d2, float v2, float d3){

	float alpha, beta, diff = (float)sqrt((float)(OSC_SQ(dX)+OSC_SQ(dY)));

	if (diff < d1){
		alpha = (v1-1) / d1;
		beta = v1 - alpha*d1;
		return (alpha * diff + beta);
	}else {
		if (diff < d2){
			alpha = (v2-v1) / (d2-d1);
			beta = v1 - alpha*d1;
			return  (alpha * diff + beta);
		} else {
			if (diff < d3){
				alpha = v2 / (d2-d3);
				beta = v2 - alpha*d2;
				return (alpha * diff + beta);
			} else
				return 0.f;
		}
	}
}