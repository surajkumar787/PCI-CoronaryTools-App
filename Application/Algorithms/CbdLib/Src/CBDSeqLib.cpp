//Copyright (c)2012 Koninklijke Philips Electronics N.V.,
//All Rights Reserved.
//
//This source code and any compilation or derivative thereof is the
//proprietary information of Koninklijke Philips Electronics N.V.
//and is confidential in nature.
//Under no circumstances is this software to be combined with any Open Source
//Software in any way or placed under an Open Source License of any type 
//without the express written permission of Koninklijke Philips 
//Electronics N.V.

// *************************************************************
// *************************************************************
// ************ SEQUENCE-BASED PART OF THE LIBRARY *************
// *************************************************************
// *************************************************************

#include "CBDSeqDev.h"

//#define EVALUATE_TIMING

// ************************************************************************************
// ***************************** Library management  **********************************
// ************************************************************************************

int CBDSeqCreate(CCBDHdlSeq * PtrCBDSeq, CCBDParamsSeq * Params){

	CCBDSeq *ThisSeq = (CCBDSeq*)malloc(sizeof(CCBDSeq));

	// CBD
	CCBD *This = (CCBD*)malloc(sizeof(CCBD));
	This->detectedTrack = NULL;
	for (int j=0; j<CBD_MAX_NB_EXTRACTED_TRACKS; j++)
		This->extractedTracks[j] = NULL;
	ThisSeq->CBD = This;

	// Default values
	This->SSFactor = 0; This->IwSub = 0; This->IhSub = 0; This->nbSeedsPerSide = 0;
	This->ImSub				= NULL;
	This->SaliencyMap		= NULL;
	This->Rdg				= NULL;
	This->Vect				= NULL;
	This->selectedIndices	= NULL;
	This->seedsX			= NULL;
	This->seedsY			= NULL;
	This->bestTrack		= NULL;
	This->scoreTracks		= NULL;
	This->orientation		= NULL;
	This->length			= NULL;
	This->contrast			= NULL;
	This->homogeneity		= NULL;

	This->lt				= NULL;
	This->ltstopcriterionTop= NULL;
	This->ltstopcriterionBottom= NULL;
	This->ltstopcriterionLeft= NULL;
	This->ltstopcriterionRight= NULL;

	// CBDSeq
	ThisSeq->nbImProcessed = -1;
	ThisSeq->detectedCathSeq = 0;
	ThisSeq->ResIms = (CCBDResults *)malloc(Params->NbImMax * sizeof(CCBDResults));

	PtrCBDSeq->Pv = (void*)ThisSeq;

	return 0;
}

int CBDSeqDelete(CCBDHdlSeq PtrCBDSeq, CCBDParamsSeq * Params){
	
	CCBDSeq * ThisSeq = (CCBDSeq *)PtrCBDSeq.Pv;
	CCBD * This = (CCBD *)ThisSeq->CBD;

	// CBD
	delete This->ltstopcriterionTop;
	delete This->ltstopcriterionBottom;
	delete This->ltstopcriterionLeft;
	delete This->ltstopcriterionRight;
	delete This->lt;

	if (This->detectedTrack != NULL){
		free(This->detectedTrack->X); free(This->detectedTrack->Y);
		free(This->detectedTrack); This->detectedTrack = NULL;
	}

	for (int i=0; i<CBD_MAX_NB_EXTRACTED_TRACKS; i++)
		if (This->extractedTracks[i] != NULL){
			free(This->extractedTracks[i]->X); free(This->extractedTracks[i]->Y);
			free(This->extractedTracks[i]); This->extractedTracks[i] = NULL;
		}
	free(This);
	
	// CBDSeq
	for (int i=0; i<Params->NbImMax; i++)
		if (ThisSeq->ResIms[i].detectedCath){
			free(ThisSeq->ResIms[i].detectedTrack->X);
			free(ThisSeq->ResIms[i].detectedTrack->Y);
			free(ThisSeq->ResIms[i].detectedTrack);
		}
	free(ThisSeq->ResIms);
	free(ThisSeq);

	return 0;
}

int CDBDefaultParametersSeq(CCBDParamsSeq * Params){

	Params->rateImDetectionForSeriesDetection	= 2.8f/5.f;
	Params->NbImMax								= 5;

	return 0;
}

// ************************************************************************************
// ********************* Catheter detection from one sequence  ************************
// ************************************************************************************

int CatheterBodyDetectionSequence(short * ImIn, int t, int Iw, int Ih, float Rot, float Ang
								, int SSFactor, CCDBSequenceType CDBSequenceType
								, CCBDHdlSeq * PtrCBDHdlSeq, CCBDResultsSequence * CBDResultsSequence){

	int x, y, Err;
	float *Rdg, * SaliencyMap;
	CCBDParams Params;
	int ShutterPos[4], ShutterPos2[4];
	int IwSub = Iw/SSFactor, IhSub = Ih/SSFactor;
	CCBDSeq * ThisSeq = (CCBDSeq*)PtrCBDHdlSeq->Pv;
	CCBD * This = ThisSeq->CBD;;
	CCBDParamsSeq ParamsSeq;

	CCBDHdl CBDHdlHere;
	CBDHdlHere.Pv = (void*)(ThisSeq->CBD);

	short * ImSub = (short*)malloc(IwSub*IhSub*sizeof(short));	
	if (ImSub == NULL){
		ErrStartErrorReportFromLeaf(This->ErrHdl, "CatheterBodyDetectionSequence", CBD_ERR_MEM_ALLOC
									, "Allocation of ImSub impossible");
		return CBD_ERR_MEM_ALLOC;
	}

	// *** Initial subsample
	CBDSubSampleImage(ImIn, ImSub, Iw, Ih, IwSub, IhSub, SSFactor);

	// *** Parameters initialization
	CDBDefaultParametersSeq(&ParamsSeq);
	if (t==0) {
		CDBDefaultParameters(&Params, IwSub, CDBSequenceType);

		CBDCopyParams(Params, &ThisSeq->CBD->Params);
	} else
		CBDCopyParams(ThisSeq->CBD->Params, &Params);

	int marginRdg = (int) ceil(2*Params.KernelRdg);
	#ifdef EVALUATE_TIMING
		CBDSysTimeMs(0);
	#endif

	// *** Check on the time
	if (t >= ParamsSeq.NbImMax) // Decision has already been made
		return 0;

	if (t != ThisSeq->nbImProcessed +1)
		return -1;
	else
		ThisSeq->nbImProcessed ++;


	// *** Line tracker initialization

	// Shutter and image allocation
	if (t == 0){
		Err = CBDExtractShutters(This, ImIn, Iw, Ih, ShutterPos);
		if(Err)
		{
			ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterBodyDetection", "CBDExtractShutters", 0, Err);
			return Err;
		}

		ShutterPos[0] = ShutterPos[0] / SSFactor; ShutterPos[1] = ShutterPos[1] / SSFactor;
		ShutterPos[2] = ShutterPos[2] / SSFactor; ShutterPos[3] = ShutterPos[3] / SSFactor;

		ThisSeq->ShutterPos[0] = ShutterPos[0]; ThisSeq->ShutterPos[1] = ShutterPos[1];
		ThisSeq->ShutterPos[2] = ShutterPos[2]; ThisSeq->ShutterPos[3] = ShutterPos[3];
	}else {
		ShutterPos[0] = ThisSeq->ShutterPos[0]; ShutterPos[1] = ThisSeq->ShutterPos[1];
		ShutterPos[2] = ThisSeq->ShutterPos[2]; ShutterPos[3] = ThisSeq->ShutterPos[3];
	}
		
	if (t== 0){
		This->ImPotential = (float*)calloc(IwSub*IhSub, sizeof(float));
		CBDTrackerInitialization(This->ImPotential, IwSub, IhSub, ShutterPos, &Params, &CBDHdlHere);
	}

	SaliencyMap = This->ImPotential;

	#ifdef EVALUATE_TIMING
		printf("Initialization (TIMING = %d ms)\n", CBDSysTimeMs("time"));
		CBDSysTimeMs(0);
	#endif

	// *** Saliency map computation
	Rdg = (float*)calloc(IwSub*IhSub, sizeof(float));
	Err = CathBDtRdgMonoscaleComputation(ImSub, IwSub, IhSub, ShutterPos, Params.KernelRdg, Rdg, This);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterBodyDetectionSequence", "CathBDtRdgMonoscaleComputation", 0, Err);
		return Err;
	}

	ShutterPos2[0] = (int)(CBD_MAX(ShutterPos[0], Params.trackerRingRadius+1) + marginRdg);
	ShutterPos2[1] = (int)(CBD_MIN(ShutterPos[1], IwSub-1-Params.trackerRingRadius-1) - marginRdg);
	ShutterPos2[2] = (int)(CBD_MAX(ShutterPos[2], Params.trackerRingRadius+1) + marginRdg);
	ShutterPos2[3] = (int)(CBD_MIN(ShutterPos[3], IhSub-1-Params.trackerRingRadius-1) - marginRdg);

	Err = CathBDtImAdjust(This, Rdg, IwSub, IhSub, ShutterPos2, 1-Params.percentileL, 1-Params.percentileH, SaliencyMap);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterBodyDetectionSequence", "CathBDtImAdjust", 0, Err);
		return Err;
	}
	free(Rdg);
	
	for (y=0; y<IhSub; y++)
		for (x=0; x<ShutterPos2[0]-1; x++)
			SaliencyMap[y*IwSub+x] = 0;
	for (y=0; y<IhSub; y++)
		for (x=ShutterPos2[1]+1; x<IwSub; x++)
			SaliencyMap[y*IwSub+x] = 0;
	for (y=0; y<ShutterPos2[2]-1; y++)
		for (x=0; x<IwSub; x++)
			SaliencyMap[y*IwSub+x] = 0;
	for (y=ShutterPos2[3]+1; y<IhSub; y++)
		for (x=0; x<IwSub; x++)
			SaliencyMap[y*IwSub+x] = 0;

	#ifdef EVALUATE_TIMING
		printf("Saliency image computation (TIMING = %d ms)\n", CBDSysTimeMs("time"));
		CBDSysTimeMs(0);
	#endif


	// *** Tracking
	CatheterTracking(Rot, Ang, IwSub, IhSub, &CBDHdlHere, &CBDResultsSequence->CBDResultsLastIm);
	CBDCopyResults(&CBDResultsSequence->CBDResultsLastIm, &(ThisSeq->ResIms[t]));

	if (SSFactor != 1){
		// Detected cath
		if (ThisSeq->ResIms[t].detectedCath){
			for (int j=0; j<ThisSeq->ResIms[t].detectedTrack->nbElts; j++){
				ThisSeq->ResIms[t].detectedTrack->X[j] *= SSFactor;
				ThisSeq->ResIms[t].detectedTrack->Y[j] *= SSFactor;
			}
		}

		// Tracks
		for (int i=0; i<ThisSeq->ResIms[t].Advanced.nbTracks; i++){
			for (int j=0; j<ThisSeq->ResIms[t].Advanced.extractedTracks[i]->nbElts; j++){
				ThisSeq->ResIms[t].Advanced.extractedTracks[i]->X[j] *= SSFactor;
				ThisSeq->ResIms[t].Advanced.extractedTracks[i]->Y[j] *= SSFactor;
			}
		}
	}

	#ifdef EVALUATE_TIMING
		printf("Tracking (TIMING = %d ms)\n", CBDSysTimeMs("time"));
	#endif

	// Free
	free(ImSub);

	// ***** Decision
	if (t == ParamsSeq.NbImMax-1){
		CatheterBodyDecisionMultipleFrames(&ParamsSeq, ThisSeq->ResIms, ParamsSeq.NbImMax, CBDResultsSequence);
		ThisSeq->detectedCathSeq = CBDResultsSequence->detectedCathSeq;
		free(SaliencyMap); 
	}


	return 0;
}

// ************************************************************************************
// ************************* Decision over multiple frames  ***************************
// ************************************************************************************

int CatheterBodyDecisionMultipleFrames(CCBDParamsSeq * Params, CCBDResults * PtrCBDDetectionsListIn, int nbIms
										, CCBDResultsSequence * PtrCBDDetectionOut){

	// * Decision
	int nbDetect = 0;
	for (int frame=0; frame<nbIms; frame++)
		if (PtrCBDDetectionsListIn[frame].detectedCath == 1)
			nbDetect ++;

	if (nbDetect < Params->rateImDetectionForSeriesDetection * nbIms){
		PtrCBDDetectionOut->detectedCathSeq = 0;
		return 0;
	} else {
		PtrCBDDetectionOut->detectedCathSeq = 1;

		// * Representant

		// Distance inter-candidates
		float	* distToOthers = (float	*) malloc(nbIms*sizeof(float));
		int		* nbOthers = (int *) malloc(nbIms*sizeof(int));

		for (int frame1=0; frame1<nbIms; frame1++){
			distToOthers[frame1] = 0; nbOthers[frame1] = 0;
		}

		for (int frame1=0; frame1<nbIms; frame1++)
			if (PtrCBDDetectionsListIn[frame1].detectedCath == 1){

				for (int frame2=0; frame2<nbIms; frame2++)
					if ( (frame2 !=frame1) && (PtrCBDDetectionsListIn[frame2].detectedCath == 1) ){

						// Distance between the detected track over both images

						float distTracks = 0;
						int nbPtDistTracks = 0;
		
						for (int i=0; i< PtrCBDDetectionsListIn[frame1].detectedTrack->nbElts; i++){

							float distPt = FLT_MAX, distPtTest;

							for (int j=0; j< PtrCBDDetectionsListIn[frame2].detectedTrack->nbElts; j++){
								
								distPtTest = (PtrCBDDetectionsListIn[frame1].detectedTrack->X[i] - PtrCBDDetectionsListIn[frame2].detectedTrack->X[j])
											* (PtrCBDDetectionsListIn[frame1].detectedTrack->X[i] - PtrCBDDetectionsListIn[frame2].detectedTrack->X[j])
											+ (PtrCBDDetectionsListIn[frame1].detectedTrack->Y[i] - PtrCBDDetectionsListIn[frame2].detectedTrack->Y[j])
											* (PtrCBDDetectionsListIn[frame1].detectedTrack->Y[i] - PtrCBDDetectionsListIn[frame2].detectedTrack->Y[j]);

								if (distPtTest < distPt)
									distPt = distPtTest;
							}

							distTracks += sqrt(distPt);
							nbPtDistTracks ++;
						}

						distToOthers[frame1] = distTracks / nbPtDistTracks;
						nbOthers[frame1] ++;
					}
			}

			// Choice
			int indiceChoice = 0; float minDist = FLT_MAX;

			for (int frame=0; frame<nbIms; frame++)
				if (PtrCBDDetectionsListIn[frame].detectedCath == 1)
					if (distToOthers[frame] / nbOthers[frame] < minDist){
						minDist = distToOthers[frame] / nbOthers[frame];
						indiceChoice = frame;
					}

			PtrCBDDetectionOut->detectedTrackSeq	= PtrCBDDetectionsListIn[indiceChoice].detectedTrack;
			PtrCBDDetectionOut->selectedIndice		= indiceChoice;

			free(distToOthers); free(nbOthers);
	}

	return 0;
}