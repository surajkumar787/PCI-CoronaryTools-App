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

#include "CBDDev.h"
#include "CBDVersion.h"
#include "CBDErrorMessages.h"

//#define EVALUATE_TIMING


// ************************************************************************************
// ***************************** Library management  **********************************
// ************************************************************************************
int CBDCreate(CCBDHdl* PtrCBDHdl){

	CCBD *This = (CCBD*)malloc(sizeof(CCBD));

    if (This == NULL){
        printf("CBDCreate: could not This");
        return -1;
    }

	PtrCBDHdl->Pv = (void*)This;


    memset(This, 0, sizeof(*This));

	// Init error management material
	ErrCreate(&(This->ErrHdl));
	ErrAdd(This->ErrHdl, CBDErrorArray, CBD_ERR_LAST-CBD_ERR_FIRST-1);
	PtrCBDHdl->ErrHdl = This->ErrHdl;

	// Default values
	This->IwAlloc = 0; This->IhAlloc = 0; This->IwSubAlloc = 0; This->IhSubAlloc = 0;
    This->IwSub = 0; This->IhSub = 0;
	This->SSFactor = 0; This->nbSeedsPerSide = 0;
	This->ImSub				= NULL;
	This->SaliencyMap		= NULL;
	This->Rdg				= NULL;
	This->Vect				= NULL;
	This->selectedIndices	= NULL;
	This->seedsX			= NULL;
	This->seedsY			= NULL;
	This->bestTrack		    = NULL;
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

    This->detectedTrack = NULL;
	for (int i=0; i<CBD_MAX_NB_EXTRACTED_TRACKS; i++)
		This->extractedTracks[i] = NULL;

	return 0;
}

int CBDMemoryAllocation(CCBDHdl CBDHdl, int IwAlloc, int IhAlloc, int SSFactor, int nbSeedsPerSide){
	CCBD * This = (CCBD *)CBDHdl.Pv;
    This->SSFactor = SSFactor; 
    return CBDMemoryAllocationImSize(CBDHdl, IwAlloc, IhAlloc, IwAlloc/SSFactor, IhAlloc/SSFactor, nbSeedsPerSide);
}

int CBDMemoryAllocationImSize(CCBDHdl CBDHdl, int IwAlloc, int IhAlloc, int IwSubAlloc, int IhSubAlloc, int nbSeedsPerSide){
	CCBD * This = (CCBD *)CBDHdl.Pv;

	if ( (This->IwAlloc != IwAlloc) || (This->IhAlloc != IhAlloc)){

		if (This->Vect != NULL){free(This->Vect);}
		This->Vect = (float*)malloc(CBD_MAX(IwAlloc, IhAlloc)*sizeof(float));
		if (This->Vect == NULL){
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC, "Allocation of Vect impossible");
			return CBD_ERR_MEM_ALLOC;
		}
		This->IwAlloc = IwAlloc; This->IhAlloc = IhAlloc;

		if ( (This->IwSubAlloc != IwSubAlloc) || (This->IhSubAlloc != IhSubAlloc)){

			if (This->ImSub != NULL)
				free(This->ImSub);
			This->ImSub = (short*)malloc(IwSubAlloc*IhSubAlloc*sizeof(short));
			if (This->ImSub == NULL){
				ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC
											, "Allocation of ImSub impossible");
				return CBD_ERR_MEM_ALLOC;
			}

			if (This->SaliencyMap != NULL)
				free(This->SaliencyMap);
			This->SaliencyMap = (float*)calloc(IwSubAlloc*IhSubAlloc, sizeof(float));
			if (This->SaliencyMap == NULL){
				ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC
											, "Allocation of SaliencyMap impossible");
				return CBD_ERR_MEM_ALLOC;
			}

			if (This->Rdg != NULL)
				free(This->Rdg);
			This->Rdg = (float*)calloc(IwSubAlloc*IhSubAlloc, sizeof(float));
			if (This->Rdg == NULL){
				ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC
											, "Allocation of Rdg impossible");
				return CBD_ERR_MEM_ALLOC;
			}

            This->IwSubAlloc = IwSubAlloc; This->IhSubAlloc = IhSubAlloc;
            This->IwSub = IwSubAlloc; This->IhSub = IhSubAlloc;
		}
	}

	if (This->nbSeedsPerSide != nbSeedsPerSide){
		if (This->selectedIndices != NULL){free(This->selectedIndices);}
		This->selectedIndices = (int*) malloc(nbSeedsPerSide * sizeof(int));
		if (This->selectedIndices == NULL){
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC
										, "Allocation of selectedIndices impossible");
			return CBD_ERR_MEM_ALLOC;
		}
		if (This->seedsX != NULL){free(This->seedsX);}
		This->seedsX = (int*) malloc(nbSeedsPerSide * sizeof(int));
		if (This->seedsX == NULL){
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC
										, "Allocation of seedsX impossible");
			return CBD_ERR_MEM_ALLOC;
		}
		if (This->seedsY != NULL){free(This->seedsY);}
		This->seedsY = (int*) malloc(nbSeedsPerSide * sizeof(int));
		if (This->seedsY == NULL){
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC
										, "Allocation of seedsY impossible");
			return CBD_ERR_MEM_ALLOC;
		}
		if (This->bestTrack != NULL){
			for (int i=0; i<4*This->nbSeedsPerSide; i++){
				if (This->bestTrack[i] !=  NULL){free(This->bestTrack[i]);}
			}
			free(This->bestTrack);
		}
		This->bestTrack = (Lt::LtTrack **) malloc(4*nbSeedsPerSide * sizeof(Lt::LtTrack*));
		if (This->bestTrack == NULL){
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC
										, "Allocation of bestTrack impossible");
			return CBD_ERR_MEM_ALLOC;
		}
		for (int i=0; i<4*nbSeedsPerSide; i++)
			This->bestTrack[i] = NULL;
		
		if (This->scoreTracks != NULL){free(This->scoreTracks);}
		This->scoreTracks = (float*)malloc(4*nbSeedsPerSide*sizeof(float));
		if (This->scoreTracks == NULL){
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC
										, "Allocation of scoreTracks impossible");
			return CBD_ERR_MEM_ALLOC;
		}
		if (This->orientation != NULL){free(This->orientation);}
		This->orientation = (float*)malloc(4*nbSeedsPerSide*sizeof(float));
		if (This->orientation == NULL){
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC
										, "Allocation of orientation impossible");
			return CBD_ERR_MEM_ALLOC;
		}
		if (This->length != NULL){free(This->length);}
		This->length = (float*)malloc(4*nbSeedsPerSide*sizeof(float));
		if (This->length == NULL){
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC
										, "Allocation of length impossible");
			return CBD_ERR_MEM_ALLOC;
		}
		if (This->contrast != NULL){free(This->contrast);}
		This->contrast = (float*)malloc(4*nbSeedsPerSide*sizeof(float));
		if (This->contrast == NULL){
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC
										, "Allocation of contrast impossible");
			return CBD_ERR_MEM_ALLOC;
		}
		if (This->homogeneity != NULL){free(This->homogeneity);}
		This->homogeneity = (float*)malloc(4*nbSeedsPerSide*sizeof(float));
		if (This->homogeneity == NULL){
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMemoryAllocation", CBD_ERR_MEM_ALLOC
										, "Allocation of homogeneity impossible");
			return CBD_ERR_MEM_ALLOC;
		}
		This->nbSeedsPerSide = nbSeedsPerSide;
	}


	return 0;
}

/*************************************************************************************/
// Set version info material
/*************************************************************************************/
int CBDSetVersion(CCBD *This)
{
	int  NbChar;
	char *Format;

	NbChar = (int)(strlen(CBD_VERSION_HEADER) + strlen(CBD_VERSION_INFO) + strlen(CBD_CONFIG) + 16);

	This->VersionInfo = (char *)malloc(NbChar * sizeof(char));
	This->VersionInfoCopy = (char *)malloc(NbChar * sizeof(char));
	Format = (char *)malloc(NbChar * sizeof(char));
    if (Format == NULL || This->VersionInfo == NULL || This->VersionInfoCopy == NULL){
        if (Format != NULL){ free(Format);}
        if (This->VersionInfo != NULL){ free(This->VersionInfo);}
        if (This->VersionInfoCopy != NULL){ free(This->VersionInfoCopy);}
        return -1;
    }

	sprintf_s(Format, NbChar, "%s%s", CBD_VERSION_HEADER, CBD_VERSION_INFO);
	sprintf_s(This->VersionInfo, NbChar, Format, CBD_CONFIG, CBD_VERSION_MAJOR, CBD_VERSION_MINOR, CBD_VERSION_PATCH);

	free(Format);
	return 0;
}

/*************************************************************************************/
// Return current SWLib version info
/*************************************************************************************/
int CBDGetVersionInfo(CCBDHdl CBDHdl, char** VersionInfo)
{
	CCBD * This = (CCBD *)CBDHdl.Pv;
	int NbChar = (int)(strlen(CBD_VERSION_HEADER) + strlen(CBD_VERSION_INFO) + strlen(CBD_CONFIG) + 16);
	strcpy_s(This->VersionInfoCopy, NbChar, This->VersionInfo);
	*VersionInfo = This->VersionInfoCopy;
	return 0;
}

/*************************************************************************************/
// Return current OscLib version numbers
/*************************************************************************************/
int CBDGetVersionNumbers(int* Major, int* Minor, int* Patch)
{
	*Major = CBD_VERSION_MAJOR; *Minor = CBD_VERSION_MINOR; *Patch = CBD_VERSION_PATCH;
	return 0;
}


int CBDDelete(CCBDHdl CBDHdl){

	CCBD * This = (CCBD *)CBDHdl.Pv;

	if (This->ImSub != NULL)			{ free(This->ImSub); This->ImSub = NULL;}
	if (This->SaliencyMap != NULL)		{ free(This->SaliencyMap); This->SaliencyMap = NULL;}
	if (This->Rdg != NULL)				{ free(This->Rdg); This->Rdg = NULL;}
	if (This->Vect != NULL)				{ free(This->Vect); This->Vect = NULL;}
	if (This->selectedIndices != NULL)	{ free(This->selectedIndices); This->selectedIndices = NULL;}
	if (This->seedsX != NULL)			{ free(This->seedsX); This->seedsX = NULL;}
	if (This->seedsY != NULL)			{ free(This->seedsY); This->seedsY = NULL;}
	if (This->bestTrack	!= NULL)		{ free(This->bestTrack); This->bestTrack = NULL;}
	if (This->scoreTracks != NULL)		{ free(This->scoreTracks); This->scoreTracks = NULL;}
	if (This->orientation != NULL)		{ free(This->orientation); This->orientation = NULL;}
	if (This->length != NULL)			{ free(This->length); This->length = NULL;}
	if (This->contrast != NULL)			{ free(This->contrast); This->contrast = NULL;}
	if (This->homogeneity != NULL)		{ free(This->homogeneity); This->homogeneity = NULL;}

	This->IwAlloc = 0; This->IhAlloc = 0; This->IwSub = 0; This->IhSub = 0;
	This->SSFactor = 0; This->nbSeedsPerSide = 0;
	

    if (This->ltstopcriterionTop)
	    delete This->ltstopcriterionTop;
    if (This->ltstopcriterionBottom)
	    delete This->ltstopcriterionBottom;
    if (This->ltstopcriterionLeft)
	    delete This->ltstopcriterionLeft;
    if (This->ltstopcriterionRight)
	    delete This->ltstopcriterionRight;
    if (This->lt)
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

	ErrDelete(This->ErrHdl);

	free(This);

	return 0;
}

int CDBDefaultParameters(CCBDParams * Params, int Iw, CCDBSequenceType CDBSequenceType){

	Params->KernelRdg				= 4.f * Iw / 512;// Ridge kernel size for saliency map
	Params->nbSeedsPerSide			= 3;			// Number of seeds per investigated side
	Params->trackerMaxNbTracks		= 5;			// Maximum track number to be kept at each iteration
	Params->trackerSectorOut		= 4*_PI/3;		// Opposite sector left out
	Params->trackerRingRadius		= 10.f * Iw / 512;// Also named speed
	Params->DThetaIrregArg0			= _PI/9;		// Highest DThetha producing zero-irregularity
	Params->DThetaIrregArg1			= _PI/2;		// Lowest DThetha producing zero-irregularity
	Params->NbStepForAngleF			= 5;			// Number of steps over which angle filtering is achieved
	Params->AngleLimitForVirtual	= _PI/9;		// Angle limit to introduce virtual
	Params->maxNbVirtual			= 5;

	Params->limitShortLong			= 200 * Iw / 512;
	if (CDBSequenceType == CBD_ANGIO){
		Params->percentileL				= .1f;			// Low percentile thresholding the ridge image
		Params->percentileH				= .99f;			// High percentile thresholding the ridge image

		Params->contrastCriteriumShort = 0.65f;//0.2f;
		Params->homogenityCriteriumShort = 0.5f;//0.18f;
		Params->contrastCriteriumLong  = 0.55f;//0.23f;
		Params->homogenityCriteriumLong = 0.35f;//0.18f;
	} else {
		Params->percentileL				= .0f;			// Low percentile thresholding the ridge image
		Params->percentileH				= 1.f;			// High percentile thresholding the ridge image

		Params->contrastCriteriumShort = 0.15f;
		Params->homogenityCriteriumShort = 0.6f;
		Params->contrastCriteriumLong  = 0.18f;
		Params->homogenityCriteriumLong = 0.6f;
	}

	return 0;
}

// ************************************************************************************
// *********************** Catheter detection from one image  *************************
// ************************************************************************************

int CatheterBodyDetection(short * ImIn, int Iw, int Ih, float Rot, float Ang
						  , int SSFactor, CCDBSequenceType CDBSequenceType, CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults){

	int x, y, Err;
	CCBDParams Params;
	int ShutterPos[4], ShutterPos2[4];
	CCBD * This = (CCBD*)PtrCBDHdl->Pv;
	
	// *** Parameters initialization
    if ( (Iw > This->IwAlloc) || (Ih > This->IhAlloc) || (Iw/SSFactor > This->IwSubAlloc) || (Ih/SSFactor > This->IhSubAlloc) ){
        char Reason[128];
		sprintf_s(Reason, 128,"Image size exceeds allocation:\nOri (%d,%d) vs (%d,%d)\nSub (%d,%d) vs (%d,%d)\n"
                    , Iw, This->IwAlloc, Ih, This->IhAlloc
                    , Iw/SSFactor, This->IwSubAlloc, Ih/SSFactor, This->IhSubAlloc);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "CatheterBodyDetection", CBD_ERR_SIZEEXCEEDSALLOCATION, Reason);
		return CBD_ERR_SIZEEXCEEDSALLOCATION;
    }
    This->Iw = Iw; This->Ih = Ih; This->IwSub = Iw/SSFactor; This->IhSub = Ih/SSFactor; This->SSFactor = SSFactor;
	CDBDefaultParameters(&Params, Iw/SSFactor, CDBSequenceType);

	//// *** Allocate internal images, if necessary
	//Err = CBDMemoryAllocation(*PtrCBDHdl, Iw, Ih, SSFactor, Params.nbSeedsPerSide);
	//if (Err){ ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterBodyDetection", "CBDMemoryAllocation", 0, Err); return Err; }


	// *** Initial subsample
	CBDSubSampleImage(ImIn, This->ImSub, This->Iw, This->Ih, This->IwSub, This->IhSub, This->SSFactor);

	//Params.KernelRdg = ;	// VA test

	int marginRdg = (int) ceil(2*Params.KernelRdg);
	#ifdef EVALUATE_TIMING
		CBDSysTimeMs(0);
	#endif

	// *** Line tracker initialization

	// Shutter and image allocation
	Err = CBDExtractShutters(This, ImIn, Iw, Ih, ShutterPos);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterBodyDetection", "CBDExtractShutters", 0, Err);
		return Err;
	}
	ShutterPos[0] = ShutterPos[0] / SSFactor; ShutterPos[1] = ShutterPos[1] / SSFactor;
	ShutterPos[2] = ShutterPos[2] / SSFactor; ShutterPos[3] = ShutterPos[3] / SSFactor;

	CBDTrackerInitialization(This->SaliencyMap, This->IwSub, This->IhSub, ShutterPos, &Params, PtrCBDHdl);

	#ifdef EVALUATE_TIMING
		printf("Initialization (TIMING = %d ms)\n", CBDSysTimeMs("time"));
		CBDSysTimeMs(0);
	#endif

	// *** Saliency map computation
	Err = CathBDtRdgMonoscaleComputation(This->ImSub, This->IwSub, This->IhSub, ShutterPos, Params.KernelRdg, This->Rdg, This);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterBodyDetection", "CathBDtRdgMonoscaleComputation", 0, Err);
		return Err;
	}

	ShutterPos2[0] = (int)(CBD_MAX(ShutterPos[0], Params.trackerRingRadius+1) + marginRdg);
	ShutterPos2[1] = (int)(CBD_MIN(ShutterPos[1], This->IwSub-1-Params.trackerRingRadius-1) - marginRdg);
	ShutterPos2[2] = (int)(CBD_MAX(ShutterPos[2], Params.trackerRingRadius+1) + marginRdg);
	ShutterPos2[3] = (int)(CBD_MIN(ShutterPos[3], This->IhSub-1-Params.trackerRingRadius-1) - marginRdg);

	Err = CathBDtImAdjust(This, This->Rdg, This->IwSub, This->IhSub, ShutterPos2, 1-Params.percentileL, 1-Params.percentileH, This->SaliencyMap);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterBodyDetection", "CathBDtImAdjust", 0, Err);
		return Err;
	}
	
	for (y=0; y<This->IhSub; y++)
		for (x=0; x<ShutterPos2[0]-1; x++)
			This->SaliencyMap[y*This->IwSub+x] = 0;
	for (y=0; y<This->IhSub; y++)
		for (x=ShutterPos2[1]+1; x<This->IwSub; x++)
			This->SaliencyMap[y*This->IwSub+x] = 0;
	for (y=0; y<ShutterPos2[2]-1; y++)
		for (x=0; x<This->IwSub; x++)
			This->SaliencyMap[y*This->IwSub+x] = 0;
	for (y=ShutterPos2[3]+1; y<This->IhSub; y++)
		for (x=0; x<This->IwSub; x++)
			This->SaliencyMap[y*This->IwSub+x] = 0;

	#ifdef EVALUATE_TIMING
		printf("Saliency image computation (TIMING = %d ms)\n", CBDSysTimeMs("time"));
		CBDSysTimeMs(0);
	#endif

	// *** Tracking
	Err = CatheterTracking(Rot, Ang, This->IwSub, This->IhSub, PtrCBDHdl, PtrCBDResults);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterBodyDetection", "CatheterTracking", 0, Err);
		return Err;
	}


	if (SSFactor != 1){
		// Detected cath
		if (PtrCBDResults->detectedCath){
			for (int j=0; j<PtrCBDResults->detectedTrack->nbElts; j++){
				PtrCBDResults->detectedTrack->X[j] *= SSFactor;
				PtrCBDResults->detectedTrack->Y[j] *= SSFactor;
			}
		}

		// Tracks
		for (int i=0; i<PtrCBDResults->Advanced.nbTracks; i++){
			for (int j=0; j<PtrCBDResults->Advanced.extractedTracks[i]->nbElts; j++){
				PtrCBDResults->Advanced.extractedTracks[i]->X[j] *= SSFactor;
				PtrCBDResults->Advanced.extractedTracks[i]->Y[j] *= SSFactor;
			}
		}
	}

	#ifdef EVALUATE_TIMING
		printf("Tracking (TIMING = %d ms)\n", CBDSysTimeMs("time"));
	#endif

	return 0;
}

// ********************************************************************************************

int CatheterBodyDetectionFromPot(float * ImPot, int Iw, int Ih, float Rot, float Ang
								  , int ShutterPos[4], int angioSeqFlag
								  , CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults){

	int x, y, Err;
	CCBDParams Params;
	int ShutterPos2[4];
	CCDBSequenceType CDBSequenceType;
	CCBD * This = (CCBD *)PtrCBDHdl->Pv;

	// *** Handle initialization

	// *** Parameters initialization
	if (angioSeqFlag == 1)
		CDBSequenceType = CBD_ANGIO;
	else
		CDBSequenceType = CBD_FLUORO;
    if ( (Iw > This->IwAlloc) || (Ih > This->IhAlloc)  ){
        char Reason[128];
		sprintf_s(Reason, 128,"Image size exceeds allocation:\nOri (%d,%d) vs (%d,%d)\n"
                    , Iw, This->IwAlloc, Ih, This->IhAlloc);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "CatheterBodyDetectionFromPot", CBD_ERR_SIZEEXCEEDSALLOCATION, Reason);
		return CBD_ERR_SIZEEXCEEDSALLOCATION;
    }
	CDBDefaultParameters(&Params, Iw, CDBSequenceType);
    This->Iw = Iw; This->Ih = Ih;

	//// *** Allocate internal images, if necessary
	//Err = CBDMemoryAllocation(*PtrCBDHdl, Iw, Ih, 1, Params.nbSeedsPerSide);
	//if (Err){ ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterBodyDetectionFromPot", "CBDMemoryAllocation", 0, Err); return Err; }

	int marginRdg = (int) ceil(2*Params.KernelRdg);

	// *** Line tracker initialization

	// Shutter and image allocation
	CBDTrackerInitialization(This->SaliencyMap, Iw, Ih, ShutterPos, &Params, PtrCBDHdl);

	// *** Saliency map computation
	ShutterPos2[0] = (int)(CBD_MAX(ShutterPos[0], Params.trackerRingRadius+1) + marginRdg);
	ShutterPos2[1] = (int)(CBD_MIN(ShutterPos[1], Iw-1-Params.trackerRingRadius-1) - marginRdg);
	ShutterPos2[2] = (int)(CBD_MAX(ShutterPos[2], Params.trackerRingRadius+1) + marginRdg);
	ShutterPos2[3] = (int)(CBD_MIN(ShutterPos[3], Ih-1-Params.trackerRingRadius-1) - marginRdg);

	//CathBDtImAdjust(ImPot, Iw, Ih, ShutterPos2, 1-Params.percentileL, 1-Params.percentileH, SaliencyMap);
	memcpy(This->SaliencyMap, ImPot, Iw*Ih*sizeof(float));
	
	for (y=0; y<Ih; y++)
		for (x=0; x<ShutterPos2[0]-1; x++)
			This->SaliencyMap[y*Iw+x] = 0;
	for (y=0; y<Ih; y++)
		for (x=ShutterPos2[1]+1; x<Iw; x++)
			This->SaliencyMap[y*Iw+x] = 0;
	for (y=0; y<ShutterPos2[2]-1; y++)
		for (x=0; x<Iw; x++)
			This->SaliencyMap[y*Iw+x] = 0;
	for (y=ShutterPos2[3]+1; y<Ih; y++)
		for (x=0; x<Iw; x++)
			This->SaliencyMap[y*Iw+x] = 0;

	// *** Tracking
	Err = CatheterTracking(Rot, Ang, Iw, Ih, PtrCBDHdl, PtrCBDResults);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterBodyDetectionFromPot", "CatheterTracking", 0, Err);
		return Err;
	}

	return 0;
}

// ********************************************************************************************

int CatheterBodyDetectionFromPotAndParam(float * ImPot, int Iw, int Ih, float Rot, float Ang
									  , int ShutterPos[4], CCBDParams Params
									  , CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults){

	int x, y, Err;
	int ShutterPos2[4];
	CCBD * This = (CCBD *)PtrCBDHdl->Pv;

	// *** Parameters initialization
	int marginRdg = (int) ceil(2*Params.KernelRdg);

	// *** Allocate internal images, if necessary
	Err = CBDMemoryAllocation(*PtrCBDHdl, Iw, Ih, 1, Params.nbSeedsPerSide);
	if (Err){ ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterBodyDetectionFromPotAndParam", "CBDMemoryAllocation", 0, Err); return Err; }

	// *** Line tracker initialization

	// Shutter and image allocation
	CBDTrackerInitialization(This->SaliencyMap, Iw, Ih, ShutterPos, &Params, PtrCBDHdl);

	// *** Saliency map computation
	ShutterPos2[0] = (int)(CBD_MAX(ShutterPos[0], Params.trackerRingRadius+1) + marginRdg);
	ShutterPos2[1] = (int)(CBD_MIN(ShutterPos[1], Iw-1-Params.trackerRingRadius-1) - marginRdg);
	ShutterPos2[2] = (int)(CBD_MAX(ShutterPos[2], Params.trackerRingRadius+1) + marginRdg);
	ShutterPos2[3] = (int)(CBD_MIN(ShutterPos[3], Ih-1-Params.trackerRingRadius-1) - marginRdg);

	//CathBDtImAdjust(ImPot, Iw, Ih, ShutterPos2, 1-Params.percentileL, 1-Params.percentileH, SaliencyMap);
	memcpy(This->SaliencyMap, ImPot, Iw*Ih*sizeof(float));
	
	for (y=0; y<Ih; y++)
		for (x=0; x<ShutterPos2[0]-1; x++)
			This->SaliencyMap[y*Iw+x] = 0;
	for (y=0; y<Ih; y++)
		for (x=ShutterPos2[1]+1; x<Iw; x++)
			This->SaliencyMap[y*Iw+x] = 0;
	for (y=0; y<ShutterPos2[2]-1; y++)
		for (x=0; x<Iw; x++)
			This->SaliencyMap[y*Iw+x] = 0;
	for (y=ShutterPos2[3]+1; y<Ih; y++)
		for (x=0; x<Iw; x++)
			This->SaliencyMap[y*Iw+x] = 0;

	// *** Tracking
	Err = CatheterTracking(Rot, Ang, Iw, Ih, PtrCBDHdl, PtrCBDResults);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterBodyDetectionFromPotAndParam", "CatheterTracking", 0, Err);
		return Err;
	}

	return 0;
}


// ************************************************************************************
// *************************** Initializing tracker ***********************************
// ************************************************************************************
void CBDTrackerInitialization(float * ImPotentialIn, int Iw, int Ih, int ShutterPos[4], CCBDParams * Params, CCBDHdl * PtrCBDHdl){

	int sizeIm[2];
	sizeIm[0] = Iw; sizeIm[1] = Ih;

	CCBD * This = (CCBD*)PtrCBDHdl->Pv;

	This->ImPotential = ImPotentialIn;
	This->Iw = Iw;
	This->Ih = Ih;

	// *****************************************************
	// ***************** Parameters ************************
	// *****************************************************
	int marginRdg = 0;//(int) ceil(2*Params->KernelRdg);

	This->ShutterPos[0] = (int)(CBD_MAX(ShutterPos[0], Params->trackerRingRadius+1) + marginRdg);
	This->ShutterPos[1] = (int)(CBD_MIN(ShutterPos[1], This->Iw-1-Params->trackerRingRadius-1) - marginRdg);
	This->ShutterPos[2] = (int)(CBD_MAX(ShutterPos[2], Params->trackerRingRadius+1) + marginRdg);
	This->ShutterPos[3] = (int)(CBD_MIN(ShutterPos[3], This->Ih-1-Params->trackerRingRadius-1) - marginRdg);

	CBDCopyParams(*Params, &This->Params);

	// *****************************************************
	// *************** Line tracker setting ****************
	// *****************************************************
	int nbringsamples = (int)ceil(2*_PI*Params->trackerRingRadius *2);

	// *** Tracker allocation
    if (This->lt)
        delete This->lt;
	This->lt = new Lt::LtTracker<TreeType>();
	
	// *** Track extension
	typedef Lt::LtSamplerRingLinear<float> RingSamplerType;

	Lt::LtExtendTrackOnRingFromFeatureIm<TreeType,RingSamplerType>* ltextend =
		new Lt::LtExtendTrackOnRingFromFeatureIm<TreeType,RingSamplerType>();
	#ifdef VIRTUAL_VINCENT
		ltextend->CreateRingSampler(This->ImPotential , sizeIm[0] , sizeIm, Params->trackerRingRadius, nbringsamples, Params->AngleLimitForVirtual);
	#else
		ltextend->CreateRingSampler(This->ImPotential , sizeIm[0] , sizeIm, Params->trackerRingRadius, nbringsamples);
	#endif
	ltextend->m_IrregLocScaleA = 0.5f * Params->trackerSectorOut;
	ltextend->m_IrregLocScaleB = 0.5f * Params->trackerSectorOut;
	This->lt->AddExtensionFunc(ltextend);

	#ifdef VIRTUAL_VINCENT
		Lt::LtExtendTrackAddVirtual<TreeType, Lt::details::EvalScalarImFunc>* ltextendvirtual = 
			new Lt::LtExtendTrackAddVirtual<TreeType, Lt::details::EvalScalarImFunc>(Params->maxNbVirtual);
	#else
		Lt::LtExtendTrackAddVirtual<TreeType, Lt::details::EvalScalarImFunc>* ltextendvirtual = 
			new Lt::LtExtendTrackAddVirtual<TreeType, Lt::details::EvalScalarImFunc>();
	#endif
	ltextendvirtual->m_FeatureFunc.SetFeatureIm(This->ImPotential, sizeIm[0],sizeIm);
	This->lt->AddExtensionFunc(ltextendvirtual);

	// * Add features that must be computed (involved in the track score)
	Lt::LtComputeFeatureLinInterScalarIm<TreeType>* ltcomputeinterscalarim = new Lt::LtComputeFeatureLinInterScalarIm<TreeType>();
	ltcomputeinterscalarim->SetFeatureIm(This->ImPotential , sizeIm[0] , sizeIm);
	This->lt->AddFeatureFunc(ltcomputeinterscalarim);

	Lt::LtComputeFeatureLength<TreeType>* ltcomputelength = new Lt::LtComputeFeatureLength<TreeType>();
	This->lt->AddFeatureFunc(ltcomputelength);

	Lt::LtComputeFeatureLocalSmoothness<TreeType>* ltcomputelocalsmoothness = new Lt::LtComputeFeatureLocalSmoothness<TreeType>();
	ltcomputelocalsmoothness->m_IrregA = Params->DThetaIrregArg0;
	ltcomputelocalsmoothness->m_IrregB = Params->DThetaIrregArg1;
	This->lt->AddFeatureFunc(ltcomputelocalsmoothness);

	Lt::LtComputeFeatureFilteredGlobalSmoothness<TreeType>* ltcomputefilteredglobalsmoothness 
		= new Lt::LtComputeFeatureFilteredGlobalSmoothness<TreeType>();
	ltcomputefilteredglobalsmoothness->m_Scale = Params->NbStepForAngleF;
	ltcomputefilteredglobalsmoothness->m_IrregA = Params->DThetaIrregArg0;
	ltcomputefilteredglobalsmoothness->m_IrregB = Params->DThetaIrregArg1;
	This->lt->AddFeatureFunc(ltcomputefilteredglobalsmoothness);

	// * Set how the tracks are pruned at each iteration
	Lt::LtPruningTracksScore<TreeType>* ltpruningscore = new Lt::LtPruningTracksScore<TreeType>();
	ltpruningscore->m_MaxTracks = Params->trackerMaxNbTracks;
	This->lt->AddPruningFunc(ltpruningscore);

	// * Prepare stop functions
	float margin = Params->trackerRingRadius+2;
	
	float roiTop[4] = {(float)This->ShutterPos[0]+margin,margin
					,(float)This->ShutterPos[1]-margin,(float)This->ShutterPos[3]-margin};
    if (This->ltstopcriterionTop)
        delete This->ltstopcriterionTop;
	This->ltstopcriterionTop = new Lt::LtStopTracksOnBorders<TreeType>(roiTop);

	float roiBottom[4] = {(float)This->ShutterPos[0]+margin,(float)This->ShutterPos[2]+margin
					,(float)This->ShutterPos[1]-margin,(float)(This->Ih-margin)};
    if (This->ltstopcriterionBottom)
        delete This->ltstopcriterionBottom;
	This->ltstopcriterionBottom = new Lt::LtStopTracksOnBorders<TreeType>(roiBottom);

	float roiLeft[4] = {margin,(float)This->ShutterPos[2]+margin
					,(float)This->ShutterPos[1]-margin,(float)This->ShutterPos[3]-margin};
    if (This->ltstopcriterionLeft)
        delete This->ltstopcriterionLeft;
	This->ltstopcriterionLeft = new Lt::LtStopTracksOnBorders<TreeType>(roiLeft);

	float roiRight[4] = {(float)This->ShutterPos[0]+margin,(float)This->ShutterPos[2]+margin
					, (float)(This->Iw-margin),(float)This->ShutterPos[3]-margin};
    if (This->ltstopcriterionRight)
        delete This->ltstopcriterionRight;
	This->ltstopcriterionRight = new Lt::LtStopTracksOnBorders<TreeType>(roiRight);
}

// Allows to change the shutter position
void CBDUpdateStopCriteria(int ShutterPos[4], CCBDParams * Params, CCBDHdl * PtrCBDHdl){

	CCBD * This = (CCBD*)PtrCBDHdl->Pv;

	This->ShutterPos[0] = ShutterPos[0]; This->ShutterPos[1] = ShutterPos[1];
	This->ShutterPos[2] = ShutterPos[2]; This->ShutterPos[3] = ShutterPos[3];

	// * Prepare stop functions
	float margin = Params->trackerRingRadius+2;

	ShutterPos[0] = (int)(CBD_MAX(ShutterPos[0], Params->trackerRingRadius+1));
	ShutterPos[1] = (int)(CBD_MIN(ShutterPos[1], This->Iw-1-Params->trackerRingRadius-1));
	ShutterPos[2] = (int)(CBD_MAX(ShutterPos[2], Params->trackerRingRadius+1));
	ShutterPos[3] = (int)(CBD_MIN(ShutterPos[3], This->Ih-1-Params->trackerRingRadius-1));

	delete[] This->ltstopcriterionTop;
	delete[] This->ltstopcriterionBottom;
	delete[] This->ltstopcriterionLeft;
	delete[] This->ltstopcriterionRight;
	
	float roiTop[4] = {(float)ShutterPos[0]+margin,margin
					,(float)ShutterPos[1]-margin,(float)ShutterPos[3]-margin};
	This->ltstopcriterionTop = new Lt::LtStopTracksOnBorders<TreeType>(roiTop);

	float roiBottom[4] = {(float)ShutterPos[0]+margin,(float)ShutterPos[2]+margin
					,(float)ShutterPos[1]-margin,(float)(This->Ih-margin)};
	This->ltstopcriterionBottom = new Lt::LtStopTracksOnBorders<TreeType>(roiBottom);

	float roiLeft[4] = {margin,(float)ShutterPos[2]+margin
					,(float)ShutterPos[1]-margin,(float)ShutterPos[3]-margin};
	This->ltstopcriterionLeft = new Lt::LtStopTracksOnBorders<TreeType>(roiLeft);

	float roiRight[4] = {(float)ShutterPos[0]+margin,(float)ShutterPos[2]+margin
					, (float)(This->Iw-margin),(float)ShutterPos[3]-margin};
	This->ltstopcriterionRight = new Lt::LtStopTracksOnBorders<TreeType>(roiRight);
}

// ************************************************************************************
// ************ Tracking catheter from the inialized line tracker *********************
// ************************************************************************************

// Lauch line tracker from 3 maximum of each border
// Criterion to perform detection: cumulated ridgeness, homogeneity, orientation and 
// position wrt angulation

int CatheterTracking(float Rot, float Ang, int Iw, int Ih, CCBDHdl * PtrCBDHdl, CCBDResults *CBDResults){
	int x,y,nbSeeds;

	CCBD *This = (CCBD*)PtrCBDHdl->Pv;
	CCBDParams * Params = &(This->Params);
	int ShutterPosHere[4];

	char Reason[128];
        
    if ( (Iw > This->IwAlloc) || (Ih > This->IhAlloc)  ){
		sprintf_s(Reason, 128,"Image size exceeds allocation:\nOri (%d,%d) vs (%d,%d)\n"
                    , Iw, This->IwAlloc, Ih, This->IhAlloc);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "CatheterBodyDetectionFromPot", CBD_ERR_SIZEEXCEEDSALLOCATION, Reason);
		return CBD_ERR_SIZEEXCEEDSALLOCATION;
    }
    This->Iw = Iw; This->Ih = Ih;
	//Err = CBDMemoryAllocation(*PtrCBDHdl, This->Iw, This->Ih, This->SSFactor, Params->nbSeedsPerSide);
	//if (Err){ ErrAppendErrorReportFromCaller(This->ErrHdl, "CatheterTracking", "CBDMemoryAllocation", 0, Err); return Err; }

	// *****************************************************
	// ***************** Parameters ************************

	int marginRdg = (int) ceil(2*Params->KernelRdg);
	int margin = (int)ceil(Params->trackerRingRadius)+2;

	ShutterPosHere[0] = (int)(CBD_MAX(This->ShutterPos[0], Params->trackerRingRadius+1) + marginRdg);
	ShutterPosHere[1] = (int)(CBD_MIN(This->ShutterPos[1], This->Iw-1-Params->trackerRingRadius-1) - marginRdg);
	ShutterPosHere[2] = (int)(CBD_MAX(This->ShutterPos[2], Params->trackerRingRadius+1) + marginRdg);
	ShutterPosHere[3] = (int)(CBD_MIN(This->ShutterPos[3], This->Ih-1-Params->trackerRingRadius-1) - marginRdg);

	for (y=0; y<This->Ih; y++)
		for (x=0; x<ShutterPosHere[0]-1; x++)
			This->ImPotential[y*This->Iw+x] = 0;
	for (y=0; y<This->Ih; y++)
		for (x=ShutterPosHere[1]+1; x<This->Iw; x++)
			This->ImPotential[y*This->Iw+x] = 0;
	for (y=0; y<ShutterPosHere[2]-1; y++)
		for (x=0; x<This->Iw; x++)
			This->ImPotential[y*This->Iw+x] = 0;
	for (y=ShutterPosHere[3]+1; y<This->Ih; y++)
		for (x=0; x<This->Iw; x++)
			This->ImPotential[y*This->Iw+x] = 0;

	// *** Display de merde à Olivier
	int sizeIm[2];
	sizeIm[0] = This->Iw; sizeIm[1] = This->Ih;
	This->lt->MxDisPlaySetBackgroundImage(This->ImPotential, sizeIm[0] , sizeIm);

	// *****************************************************
	// ********************* Initializations ***************
	// *****************************************************

	//int configAngulation;

	//if ((Rot >= 10) && (Ang < 15))
	//	configAngulation = 1;
	//else{
	//	if ((Rot >= 10) && (Ang >= 15))
	//		configAngulation = 2;
	//	else {
	//		if ((Rot < -20) && (Ang < 15))
	//			configAngulation = 3;
	//		else {
	//			if ((Rot < 10) && (Ang >= 15))
	//				configAngulation = 4;
	//			else
	//				configAngulation = 5;
	//		}
	//	}
	//} 

	for (int j=0; j<4*Params->nbSeedsPerSide; j++)
		This->bestTrack[j] = new Lt::LtTrack();

	int nbTracks = 0;  

	// *****************************************************
	// *************** Exploring the 4 borders *************
	// *****************************************************

	if (ShutterPosHere[0] +2*margin < ShutterPosHere[1]){

		// ***************
		// *** Top side
		This->lt->AddStopFunc(This->ltstopcriterionTop);

		// Strongest local maxima	
		for (x=ShutterPosHere[0]+margin; x<= ShutterPosHere[1]-margin; x++){
			This->Vect[x-ShutterPosHere[0]-margin] 
				= This->ImPotential[ShutterPosHere[2] * This->Iw + x];
		}
		LocalMaxima(This->Vect, ShutterPosHere[1]-ShutterPosHere[0]+1-2*margin, Params->nbSeedsPerSide, This->selectedIndices);

		// Geometry constraint
		nbSeeds = 0;
		for (int i=0; i<Params->nbSeedsPerSide; i++){
			x = This->selectedIndices[i] + ShutterPosHere[0]+margin; 
			y = ShutterPosHere[2];

            // Test always top
			This->seedsX[nbSeeds] = x; This->seedsY[nbSeeds] = y; nbSeeds ++;	
		}

		// Tracking
		for (int i=0; i<nbSeeds; i++){
			This->lt->AddSeed((float)This->seedsX[i],(float)This->seedsY[i]);
			This->lt->Track();

			This->lt->GetBestTrackAndScores(This->bestTrack[nbTracks], &This->scoreTracks[nbTracks], &This->length[nbTracks]);

			ComputeTrackIndicators(This->bestTrack[nbTracks], Params->trackerRingRadius, Params->NbStepForAngleF
								, Params->DThetaIrregArg0, Params->DThetaIrregArg1, This->ImPotential, This->Iw, This->Ih
								, &This->orientation[nbTracks], &This->contrast[nbTracks], &This->homogeneity[nbTracks]);

			This->lt->Reset();
			nbTracks++;
		}

		// ***************
		// *** Bottom side
		This->lt->RemoveStopFuncs(); This->lt->AddStopFunc(This->ltstopcriterionBottom);

		// Strongest local maxima
		for (x=ShutterPosHere[0]+margin; x<= ShutterPosHere[1]-margin; x++)
			This->Vect[x-ShutterPosHere[0]-margin] 
				= This->ImPotential[ShutterPosHere[3] * This->Iw + x];
		LocalMaxima(This->Vect, ShutterPosHere[1]-ShutterPosHere[0]+1-2*margin, Params->nbSeedsPerSide, This->selectedIndices);

		// Geometry constraint
		nbSeeds = 0;
		for (int i=0; i<Params->nbSeedsPerSide; i++){
			x = This->selectedIndices[i] + ShutterPosHere[0]+margin; 
			y = ShutterPosHere[3];

            // Test always bottom
            This->seedsX[nbSeeds] = x; This->seedsY[nbSeeds] = y; nbSeeds ++;
		}
		// Tracking
		for (int i=0; i<nbSeeds; i++){
			This->lt->AddSeed((float)This->seedsX[i],(float)This->seedsY[i]);
			This->lt->Track();

			This->lt->GetBestTrackAndScores(This->bestTrack[nbTracks], &This->scoreTracks[nbTracks], &This->length[nbTracks]);

			ComputeTrackIndicators(This->bestTrack[nbTracks], Params->trackerRingRadius, Params->NbStepForAngleF
								, Params->DThetaIrregArg0, Params->DThetaIrregArg1, This->ImPotential, This->Iw, This->Ih
								, &This->orientation[nbTracks], &This->contrast[nbTracks], &This->homogeneity[nbTracks]);

			This->lt->Reset();
			nbTracks++;
		}
	}

	if (ShutterPosHere[2] +2*margin < ShutterPosHere[3]){
		// ***************
		// *** Left side
		This->lt->RemoveStopFuncs(); This->lt->AddStopFunc(This->ltstopcriterionLeft);

		// Strongest local maxima
		for (y=ShutterPosHere[2]+margin; y<= ShutterPosHere[3]-margin; y++)
			This->Vect[y-ShutterPosHere[2]-margin] 
				= This->ImPotential[y* This->Iw + ShutterPosHere[0]];
		LocalMaxima(This->Vect, ShutterPosHere[3]-ShutterPosHere[2]+1-2*margin, Params->nbSeedsPerSide, This->selectedIndices);

		// Geometry constraint
		nbSeeds = 0;
		for (int i=0; i<Params->nbSeedsPerSide; i++){
			x = ShutterPosHere[0];
			y = This->selectedIndices[i] + ShutterPosHere[2]+margin;

            This->seedsX[nbSeeds] = x; This->seedsY[nbSeeds] = y; nbSeeds ++;
		}

		// Tracking
		for (int i=0; i<nbSeeds; i++){
			This->lt->AddSeed((float)This->seedsX[i],(float)This->seedsY[i]);
			This->lt->Track();

			This->lt->GetBestTrackAndScores(This->bestTrack[nbTracks], &This->scoreTracks[nbTracks], &This->length[nbTracks]);

			ComputeTrackIndicators(This->bestTrack[nbTracks], Params->trackerRingRadius, Params->NbStepForAngleF
								, Params->DThetaIrregArg0, Params->DThetaIrregArg1, This->ImPotential, This->Iw, This->Ih
								, &This->orientation[nbTracks], &This->contrast[nbTracks], &This->homogeneity[nbTracks]);

			This->lt->Reset();
			nbTracks++;
		}

		// ***************
		// *** Right side
		This->lt->RemoveStopFuncs(); This->lt->AddStopFunc(This->ltstopcriterionRight);

		// Strongest local maxima
		for (y=ShutterPosHere[2]+margin; y<= ShutterPosHere[3]-margin; y++)
			This->Vect[y-ShutterPosHere[2]-margin] 
				= This->ImPotential[y* This->Iw + ShutterPosHere[1]];
		LocalMaxima(This->Vect, ShutterPosHere[3]-ShutterPosHere[2]+1-2*margin, Params->nbSeedsPerSide, This->selectedIndices);

		// Geometry constraint
		nbSeeds = 0;
		for (int i=0; i<Params->nbSeedsPerSide; i++){
			x = ShutterPosHere[1]; 
			y = This->selectedIndices[i] + ShutterPosHere[2] +margin;

            This->seedsX[nbSeeds] = x; This->seedsY[nbSeeds] = y; nbSeeds ++;
		}

		// Tracking
		for (int i=0; i<nbSeeds; i++){
			This->lt->AddSeed((float)This->seedsX[i],(float)This->seedsY[i]);//, _PI);
			This->lt->Track();

			This->lt->GetBestTrackAndScores(This->bestTrack[nbTracks], &This->scoreTracks[nbTracks], &This->length[nbTracks]);

			ComputeTrackIndicators(This->bestTrack[nbTracks], Params->trackerRingRadius, Params->NbStepForAngleF
								, Params->DThetaIrregArg0, Params->DThetaIrregArg1, This->ImPotential, This->Iw, This->Ih
								, &This->orientation[nbTracks], &This->contrast[nbTracks], &This->homogeneity[nbTracks]);

			This->lt->Reset();
			nbTracks++;
		}
	}

	// *** Copy extracted tracks
	This->nbExtractedTracks = nbTracks;
	CBDResults->Advanced.nbTracks = nbTracks;
	for (int i=0; i<nbTracks; i++){
		if (This->extractedTracks[i] != NULL){
			free(This->extractedTracks[i]->X); free(This->extractedTracks[i]->Y);
			free(This->extractedTracks[i]);
		}

		This->extractedTracks[i] = (CBDTrack*)malloc(sizeof(CBDTrack));
		if (This->extractedTracks[i] == NULL){
			sprintf_s(Reason, 128,"Allocation of extractedTracks %d (/%d) impossible", i, nbTracks);
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CatheterTracking", CBD_ERR_MEM_ALLOC, Reason);
			return CBD_ERR_MEM_ALLOC;
		}
		This->extractedTracks[i]->nbElts = This->bestTrack[i]->Size();
		This->extractedTracks[i]->X = (float*)malloc(This->bestTrack[i]->Size() * sizeof(float));
		if (This->extractedTracks[i]->X == NULL){
			sprintf_s(Reason, 128,"Allocation of extractedTracks[%d]->X (/%d) impossible", i, nbTracks);
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CatheterTracking", CBD_ERR_MEM_ALLOC, Reason);
			return CBD_ERR_MEM_ALLOC;
		}
		This->extractedTracks[i]->Y = (float*)malloc(This->bestTrack[i]->Size() * sizeof(float));
		if (This->extractedTracks[i]->Y == NULL){
			sprintf_s(Reason, 128,"Allocation of extractedTracks[%d]->Y (/%d) impossible", i, nbTracks);
			ErrStartErrorReportFromLeaf(This->ErrHdl, "CatheterTracking", CBD_ERR_MEM_ALLOC, Reason);
			return CBD_ERR_MEM_ALLOC;
		}
		for (int j=0; j<This->bestTrack[i]->Size(); j++){
			This->extractedTracks[i]->X[j] = This->bestTrack[i]->m_Pts[j].X;
			This->extractedTracks[i]->Y[j] = This->bestTrack[i]->m_Pts[j].Y;
		}
		CBDResults->Advanced.extractedTracks[i] = This->extractedTracks[i];

		CBDResults->Advanced.indicTrack[i].scoreTracks = This->scoreTracks[i];
		CBDResults->Advanced.indicTrack[i].orientation = This->orientation[i];
		CBDResults->Advanced.indicTrack[i].length = This->length[i];
		CBDResults->Advanced.indicTrack[i].contrast = This->contrast[i];
		CBDResults->Advanced.indicTrack[i].homogeneity = This->homogeneity[i];
	}

	// *****************************************************
	// ***************** Best track selection **************
	// *****************************************************
	bool catheterFound = false;
	int it = 0;

	CBDResults->detectedCath = 0;
	CBDResults->detectedTrack = NULL;

	while ((!catheterFound) && (it < nbTracks)){
            
		// Find maximum score, the long ones first
		float bestScore = -FLT_MAX; 
        int indiceBestScore = -1;
		for (int i=0; i<nbTracks; i++)
            if ( (CBD_ABS_F(This->orientation[i]) < CBD_ORIENTATION_LIMIT) &&
                (bestScore < This->scoreTracks[i]) && (This->length[i] >= This->Params.limitShortLong) ){
				bestScore = This->scoreTracks[i]; indiceBestScore = i;
			}
        if (bestScore == -FLT_MAX){
		    for (int i=0; i<nbTracks; i++)
			    if ( (CBD_ABS_F(This->orientation[i]) < CBD_ORIENTATION_LIMIT) &&  (bestScore < This->scoreTracks[i]) ){
				    bestScore = This->scoreTracks[i]; indiceBestScore = i;
			    }
        }
        if (indiceBestScore == -1)
            break;

		This->scoreTracks[indiceBestScore] = -FLT_MAX;

		// Discussion
		if ( (This->length[indiceBestScore] <= This->Params.limitShortLong) &&	// Short (normalized over a 512x512 image)
			(This->contrast[indiceBestScore] > This->Params.contrastCriteriumShort) &&
			(This->homogeneity[indiceBestScore] / This->contrast[indiceBestScore] < This->Params.homogenityCriteriumShort) ){	// Strongly contrasted (0.18)
			catheterFound = true;
		} else {								// Long
			if ( (This->contrast[indiceBestScore] > This->Params.contrastCriteriumLong) &&	// Contrasted (0.13 - 0.4)
				(This->homogeneity[indiceBestScore] / This->contrast[indiceBestScore] < This->Params.homogenityCriteriumLong) )	// Homogeneous (0.11 - 0.16)
				catheterFound = true;
		}

		if (catheterFound){
			if (This->detectedTrack != NULL){
				free(This->detectedTrack->X); free(This->detectedTrack->Y);
				free(This->detectedTrack);
			}

			This->detectedTrack = (CBDTrack*)malloc(sizeof(CBDTrack));
			if (This->detectedTrack == NULL){
				ErrStartErrorReportFromLeaf(This->ErrHdl, "CatheterTracking", CBD_ERR_MEM_ALLOC
											, "Allocation of detectedTrack impossible");
				return CBD_ERR_MEM_ALLOC;
			}

			This->detectedTrack->nbElts = This->bestTrack[indiceBestScore]->Size();
			This->detectedTrack->X = (float*)malloc(This->detectedTrack->nbElts * sizeof(float));
			if (This->detectedTrack->X == NULL){
				ErrStartErrorReportFromLeaf(This->ErrHdl, "CatheterTracking", CBD_ERR_MEM_ALLOC
											, "Allocation of detectedTrack->X impossible");
				return CBD_ERR_MEM_ALLOC;
			}
			This->detectedTrack->Y = (float*)malloc(This->detectedTrack->nbElts * sizeof(float));
			if (This->detectedTrack->Y == NULL){
				ErrStartErrorReportFromLeaf(This->ErrHdl, "CatheterTracking", CBD_ERR_MEM_ALLOC
											, "Allocation of detectedTrack->Y impossible");
				return CBD_ERR_MEM_ALLOC;
			}

			for (int j=0; j<This->bestTrack[indiceBestScore]->Size(); j++){
				This->detectedTrack->X[j] = This->bestTrack[indiceBestScore]->m_Pts[j].X;
				This->detectedTrack->Y[j] = This->bestTrack[indiceBestScore]->m_Pts[j].Y;
			}

			CBDResults->detectedCath = 1;
			CBDResults->detectedTrack = This->detectedTrack;
		}
		it++;
	}

	CBDResults->Advanced.ImPotential = This->ImPotential; 

	for (int j=0; j<4*Params->nbSeedsPerSide; j++)
		delete This->bestTrack[j];

	return 0;
}

// ************************************************************************************
// ********************************** Utility function  *******************************
// ************************************************************************************

// The tracks are not copied to the result structure. When the handle is deleted
// they are erased as well. So the user needs to be able to copy them, and to delete them afterwards

int CBDCopyResults(CCBDResults *PtrCBDResIn, CCBDResults *PtrCBDResOut){

	PtrCBDResOut->detectedCath = PtrCBDResIn->detectedCath;

	if (PtrCBDResOut->detectedCath == 0) {
		PtrCBDResOut->detectedTrack = NULL;
	} else {
		PtrCBDResOut->detectedTrack = (CBDTrack*) malloc(sizeof(CBDTrack));
        if (PtrCBDResOut->detectedTrack == NULL){
            printf("CBDCopyResults: could not allocate detectedTrack\n");
            return -1;
        }
        
		PtrCBDResOut->detectedTrack->nbElts = PtrCBDResIn->detectedTrack->nbElts;

		PtrCBDResOut->detectedTrack->X = (float*) malloc(PtrCBDResOut->detectedTrack->nbElts * sizeof(float));
		PtrCBDResOut->detectedTrack->Y = (float*) malloc(PtrCBDResOut->detectedTrack->nbElts * sizeof(float));
        if (PtrCBDResOut->detectedTrack->X == NULL ||PtrCBDResOut->detectedTrack->Y == NULL){
            printf("CBDCopyResults: could not allocate detectedTrack->X or ->Y\n");
            return -1;
        }

		for (int i=0; i<PtrCBDResOut->detectedTrack->nbElts; i++){
			PtrCBDResOut->detectedTrack->X[i] = PtrCBDResIn->detectedTrack->X[i];
			PtrCBDResOut->detectedTrack->Y[i] = PtrCBDResIn->detectedTrack->Y[i];
		}
	}
	return 0;
}

int CBDDeleteCopiedResults(CCBDResults * PtrCBDResOut){

	if (PtrCBDResOut->detectedCath == 1){
		free(PtrCBDResOut->detectedTrack->X); free(PtrCBDResOut->detectedTrack->Y);
	}
	free(PtrCBDResOut->detectedTrack);
	PtrCBDResOut->detectedTrack = NULL;

	return 0;
}

// *** Copy parameter structure
int CBDCopyParams(CCBDParams ParamsIn, CCBDParams *ParamsOut){

	ParamsOut->KernelRdg = ParamsIn.KernelRdg;
	ParamsOut->percentileL = ParamsIn.percentileL;
	ParamsOut->percentileH = ParamsIn.percentileH;
	ParamsOut->nbSeedsPerSide = ParamsIn.nbSeedsPerSide;
	ParamsOut->trackerMaxNbTracks = ParamsIn.trackerMaxNbTracks;
	ParamsOut->trackerSectorOut = ParamsIn.trackerSectorOut;
	ParamsOut->trackerRingRadius = ParamsIn.trackerRingRadius;
	ParamsOut->DThetaIrregArg0 = ParamsIn.DThetaIrregArg0;
	ParamsOut->DThetaIrregArg1 = ParamsIn.DThetaIrregArg1;
	ParamsOut->NbStepForAngleF = ParamsIn.NbStepForAngleF;
	ParamsOut->AngleLimitForVirtual = ParamsIn.AngleLimitForVirtual;
	ParamsOut->maxNbVirtual = ParamsIn.maxNbVirtual;

	ParamsOut->limitShortLong			= ParamsIn.limitShortLong;
	ParamsOut->contrastCriteriumShort = ParamsIn.contrastCriteriumShort;
	ParamsOut->contrastCriteriumLong  = ParamsIn.contrastCriteriumLong;
	ParamsOut->homogenityCriteriumLong = ParamsIn.homogenityCriteriumLong;

	return 0;
}