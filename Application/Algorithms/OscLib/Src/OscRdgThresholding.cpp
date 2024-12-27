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

int OscRdgAngioHysteresisLabeling(COscHdl OscHdl){
	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	int					Err, Iw = ExtractProc->IwProcAngio, Ih = ExtractProc->IhProcAngio, ErrOut = 0;
	float				*ImFrg = ExtractProc->ImFrg;
	COscRoiCoord		*RoiCoord = &ExtractProc->ShuttersRoiSafe;
	int					x,y, nbLab;
	short				*stackX = ExtractProc->BufsS[1], *stackY = ExtractProc->BufsS[2], *stackX0 = ExtractProc->BufsS[3], *stackY0 = ExtractProc->BufsS[4]
						, *nbIterOnMaskIn = ExtractProc->BufsS[5];
	int					*NbPelPerLabOut = ExtractProc->BufsI[0];
	float				*CumSumRdgPerLab = ExtractProc->BufsF[4];
	int					minNbPelPerLabH = OSC_ANGIO_NBMINPIX_H / (ExtractProc->SSFactor*ExtractProc->SSFactor);

	// *** High threshold

	// Threshold computation
	if (ExtractProc->t <= OSC_FREEZE_THRESHOLDS_AT){
		Err = OscPercentileComputation(This, ImFrg, Iw, Ih, RoiCoord, OSC_ANGIO_THRPERCENT_H, &ExtractProc->thresFrgH);
        OSC_ERR_REPORT_FROM_CALLER(Err, ErrOut, "OscPercentileComputation", "OscRdgAngioHysteresisLabeling");
	}

	// Thresholding
	OSC_MEMZERO(ExtractProc->ImSee, Iw*Ih);
	for (y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
		for (x=RoiCoord->XMin; x<=RoiCoord->XMax; x++)
			if (ImFrg[y*Iw+x] > ExtractProc->thresFrgH)
				ExtractProc->ImSee[y*Iw+x] = 1;

	// Keep only connected components that are large enough
	Err = OscRdgDirectionalLabelingFromSeed(OscHdl, (const unsigned char*)ExtractProc->ImSee, (const unsigned char*)ExtractProc->ImSee
        //, OSC_ANGIO_NBRANGEPIX_H, OSC_ANGIO_MAXDIRDIFF_H, ExtractProc->ImLab, &nbLab, NbPelPerLabOut, CumSumRdgPerLab);
        , (int)(OSC_ANGIO_NBRANGEPIX_H * 2 / ExtractProc->SSFactor), OSC_ANGIO_MAXDIRDIFF_H, ExtractProc->ImLab, &nbLab, NbPelPerLabOut, CumSumRdgPerLab);
	OSC_ERR_REPORT_FROM_CALLER(Err, ErrOut, "OscRdgDirectionalLabelingFromSeed", "OscRdgAngioHysteresisLabeling"); 
    
	OscRdgThresholdLabelsBySize(ExtractProc->ImLab, ExtractProc->ImSee, Iw, Ih, RoiCoord, nbLab, NbPelPerLabOut, minNbPelPerLabH, 1);
    
	// *** Low threshold

	// Threshold computation
	if (ExtractProc->t <= OSC_FREEZE_THRESHOLDS_AT){
		Err = OscPercentileComputation(This, ImFrg, Iw, Ih, RoiCoord, OSC_ANGIO_THRPERCENT_L, &ExtractProc->thresFrgL);
		OSC_ERR_REPORT_FROM_CALLER(Err, ErrOut, "OscPercentileComputation", "OscRdgAngioHysteresisLabeling");
	}

	// Thresholding
	OSC_MEMZERO(ExtractProc->ImThr, Iw*Ih);
	for (y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
		for (x=RoiCoord->XMin; x<=RoiCoord->XMax; x++)
			if (ImFrg[y*Iw+x] > ExtractProc->thresFrgL)
				ExtractProc->ImThr[y*Iw+x] = 1;


	// Hysteresis thresholding (Non directional labeling)
	Err = OscRdgLabelingFromSeed(OscHdl, (const unsigned char *) ExtractProc->ImSee, (const unsigned char *)ExtractProc->ImThr
								//, OSC_ANGIO_NBRANGEPIX_L, ExtractProc->ImLab, &nbLab, NbPelPerLabOut, CumSumRdgPerLab);
								, (int)(OSC_ANGIO_NBRANGEPIX_L * 2 / ExtractProc->SSFactor), stackX, stackY, stackX0, stackY0, nbIterOnMaskIn
                                , ExtractProc->ImLab, &nbLab, NbPelPerLabOut, CumSumRdgPerLab);
    OSC_ERR_REPORT_FROM_CALLER(Err, ErrOut, "OscRdgLabelingFromSeed", "OscRdgAngioHysteresisLabeling"); 

    //if (ExtractProc->t >= 75){
    //    MxOpen(); MxSetVisible(1);
    //    short *LabShort = (short*)malloc(Iw*Ih*sizeof(short));
    //    for (int p=0; p<Iw*Ih; p++)
    //        LabShort[p] = (short)ExtractProc->ImLab[p];
    //    MxPutMatrixShort(LabShort, Iw, Ih, "LabBefore");
    //    free(LabShort);
    //}

	//OscRdgThresholdLabelsByCumRdg(ExtractProc->ImLab, ExtractProc->ImThr, Iw, Ih, RoiCoord, nbLab, NbPelPerLabOut, CumSumRdgPerLab, OSC_CUMRDG_THRES_RATIO);
	
    //if (ExtractProc->t >= 75){
    //    short *LabShort = (short*)malloc(Iw*Ih*sizeof(short));
    //    for (int p=0; p<Iw*Ih; p++)
    //        LabShort[p] = (short)ExtractProc->ImLab[p];
    //    MxPutMatrixShort(LabShort, Iw, Ih, "LabAfter");
    //    MxCommand("figure(1); subplot(2,2,1); Show(LabBefore'); subplot(2,2,2); Show(LabAfter');");
    //    free(LabShort);
    //}

    //// *** Relabel (continuously)
    //int *nbEltsHere = (int*)malloc(nbLab*sizeof(int));
    //memset(nbEltsHere, 0, nbLab*sizeof(int));
    //for (int p=0; p<Iw*Ih; p++)
    //    if (ExtractProc->ImLab[p] >= 2){
    //        nbEltsHere[ExtractProc->ImLab[p]-2] ++;
    //    }
    //    
    //int *LabCorrespondance = (int*)malloc(nbLab*sizeof(int));
    //int currentLab = 0;
    //for (int lab = 0; lab<nbLab; lab++)
    //    if (nbEltsHere[lab] > 0){
    //        LabCorrespondance[lab] = currentLab; 
    //        currentLab++;
    //    } else {
    //        LabCorrespondance[lab] = 0;
    //    }

    //for (int p=0; p<Iw*Ih; p++)
    //    if (ExtractProc->ImLab[p] >= 2)
    //        ExtractProc->ImLab[p] = LabCorrespondance[ExtractProc->ImLab[p]-2]+2;

    //free(nbEltsHere); free(LabCorrespondance);

	// *** Final Mask
	OSC_MEMZERO(ExtractProc->ImMask, Iw*Ih);
	for (y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
		for (x=RoiCoord->XMin; x<=RoiCoord->XMax; x++)
			if (ExtractProc->ImLab[y*Iw+x] >= 2)
				ExtractProc->ImMask[y*Iw+x] = 1;

	return ErrOut;
}

//**************************************************************************
// RdgThresholdLabelsByCumRdg : label thresholding by cumulated ridge value
//**************************************************************************
int OscRdgThresholdLabelsByCumRdg(int *Lab, unsigned char *Thr, int Iw, int Ih, COscRoiCoord *RoiCoord
								, int NbLabels, int *NbPelPerLab, float *CumRdgPerLab, float RidgenessThresholdRatio){

	int x, y, i, k, maxLabNr;
	char *selectedFlags;
	float maxCumRdg;

	// Maximum number of labels to be kept
	maxLabNr = OSC_MIN(OSC_MAXLABELSNB, NbLabels);
	maxCumRdg = 0.f;
	for (k=1; k<NbLabels; k++)
		maxCumRdg = OSC_MAX(maxCumRdg,CumRdgPerLab[k]);

	// Iteratively take the max
	selectedFlags = (char*)malloc(NbLabels*sizeof(char));
    if (selectedFlags == NULL){
        printf("OscRdgThresholdLabelsByCumRdg: could not allocate selectedFlags\n");
        return -1;
    }
	OSC_MEMZERO(selectedFlags, NbLabels);
	for (i=0; i<maxLabNr; i++){
		int lBest = 0;
		float curMax = -1.f;
		for (k=0; k<NbLabels; k++)
			if (CumRdgPerLab[k] >= curMax){
				curMax = CumRdgPerLab[k]; lBest = k;
			}

		if (curMax <= RidgenessThresholdRatio*maxCumRdg)
			break;
		else {
			selectedFlags[lBest] = 1; CumRdgPerLab[lBest] = 0;
		}
    }

	// Remove the labels to delete
    if (NbLabels > 0){
	    for (y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
		    for (x=RoiCoord->XMin; x<=RoiCoord->XMax; x++)
			    if ( (Lab[y*Iw+x] >= 2) && (selectedFlags[Lab[y*Iw+x]-2] ==0) ){
				    Lab[y*Iw+x] = 0; //Thr[y*Iw+x] = 0;
			    }
    }

	free(selectedFlags);

    return 0;
}


//**************************************************************************
// RdgThresholdLabelsBySize : label thresholding by size
//**************************************************************************
void OscRdgThresholdLabelsBySize(int *Lab, unsigned char *Thr, int IwProc, int IhProc, COscRoiCoord *RoiCoord
								 , int NbLabels, int *NbPelPerLab, int MinNbPelPerLab, int KeepLargestLabel)
{
	int        x, y, k; 

	// determination of the labels to be removed
	char *toBeRemoved = (char *) malloc((NbLabels+1)*sizeof(char));
	for (k=0; k<NbLabels; k++)
		toBeRemoved[k] = (NbPelPerLab[k]<MinNbPelPerLab);

	// optinally, the largest label is kept
	if (KeepLargestLabel){
		int NbPelLargestLab = 0, indMax = 0;

		for (k=0; k<NbLabels; k++)
			if (NbPelLargestLab<NbPelPerLab[k]){
				NbPelLargestLab = NbPelPerLab[k];
				indMax = k;
			}
		toBeRemoved[indMax] = 0;
	}

	// now remove the labels to be removed
	for(y = RoiCoord->YMin; y <= RoiCoord->YMax; y++)
		for(x = RoiCoord->XMin; x <= RoiCoord->XMax; x++)
			if (Thr[y*IwProc+x] && (Lab[y*IwProc+x]==1 || toBeRemoved[Lab[y*IwProc+x]-2])){
				Thr[y*IwProc+x] = 0; Lab[y*IwProc+x] = 0;
			}
	free(toBeRemoved);
}

//**************************************************************************
// OscRdgDirectionalLabelingFromSeed : labelling from a SeedIn binary image
// This version manages its own labelling recursion (quicker than passing many parameters as recursive arguments)
//**************************************************************************
int OscRdgDirectionalLabelingFromSeed(COscHdl OscHdl, const unsigned char *SeedIn, const unsigned char *ThrIn
									, int NbRangePix, float MaxDirInDiff
									, int *LabOut, int *NbLabelsOut, int *NbPelPerLabOut, float *CumRdgOut)
{
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
    
	short *stack_x			= ExtractProc->BufsS[0];
	short *stack_y			= ExtractProc->BufsS[1];
	short *stack_x0			= ExtractProc->BufsS[2];
	short *stack_y0			= ExtractProc->BufsS[3];
	short *nbIterOnMaskIn	= ExtractProc->BufsS[4];

	return OscRdgDirectionalLabelingFromSeedDetailed(OscHdl, SeedIn, ThrIn, ExtractProc->ImFrg, ExtractProc->ImDir
                                    , ExtractProc->IwProcAngio, ExtractProc->IhProcAngio
									, NbRangePix, MaxDirInDiff, stack_x, stack_y, stack_x0, stack_y0, nbIterOnMaskIn
                                    , LabOut, NbLabelsOut, NbPelPerLabOut, CumRdgOut);
}

int OscRdgDirectionalLabelingFromSeedDetailed(COscHdl OscHdl, const unsigned char *SeedIn, const unsigned char *ThrIn
											, float *RdgIn, float *DirIn, int Iw, int Ih, int NbRangePix, float MaxDirInDiff
                                            , short *stack_x, short *stack_y, short *stack_x0, short *stack_y0, short *nbIterOnMaskIn
											, int *LabOut, int *NbLabelsOut, int *NbPelPerLabOut, float *CumRdgOut)
{
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	const float cos_angle = (float) cos(OSC_PI * MaxDirInDiff / 180);
	int x, y, x0, y0, xMin, xMax, yMin, yMax, p, etiquette = 2, k;
	int nbMaxIterOnMaskIn = 1;
	int recursion_level, max_recursion_level;

    unsigned char*MaskIn = ExtractProc->SWRes.SWMask;

	COscRoiCoord *RoiCoord = &ExtractProc->ShuttersRoiSafe;
    
	if (MaxDirInDiff >= 90)
		return OscRdgLabelingFromSeed(OscHdl, SeedIn, ThrIn, NbRangePix, stack_x, stack_y, stack_x0, stack_y0, nbIterOnMaskIn, LabOut, NbLabelsOut, NbPelPerLabOut, CumRdgOut);


	xMin = OSC_MAX(NbRangePix, RoiCoord->XMin);
	xMax = OSC_MIN(Iw-NbRangePix-1, RoiCoord->XMax);
	yMin = OSC_MAX(NbRangePix, RoiCoord->YMin);
	yMax = OSC_MIN(Ih-NbRangePix-1, RoiCoord->YMax);

	max_recursion_level = (xMax-xMin+1)*(yMax-yMin+1);

	// initialization of LabOut to ThrIn
	for(y=0; y<Ih; y++)
		for(x=0; x<Iw; x++)
			LabOut[y*Iw+x] = (int)ThrIn[y*Iw+x];

	// *** Loop on the image
	// When new unlabeled pixel met, initialize new label and perform its labeling completely.
	// It is done by storing the next points to discuss into the different stacks

	for(y = yMin; y <= yMax; y++)
		for(x = xMin; x <= xMax; x++){

			// * Start new label
			if(SeedIn[y*Iw+x]==1 && LabOut[y*Iw+x]==1){

				// Initializations
				int   nbPixLabel = 0, nbCurrentIterOnMaskIn = 0;
				float sumRdg = 0;
				recursion_level = 0; x0 = x; y0 = y;

				// * Labeling the complete image with that label

				// Pixel of interest
				// 1- being thresholded and unlabeled 2- being on the mask 
				// or 3- if there are points to be processed stored in the stacks
				while (LabOut[y*Iw+x]==1 || ((LabOut[y*Iw+x]==0) && (MaskIn[y*Iw+x] == 1))
                        || recursion_level>0){

					// Case 1- thresholded pixel
					if (LabOut[y*Iw+x]==1){

						// Directional constraint
						float scalar_product  = (float) (cos(DirIn[y*Iw+x])*cos(DirIn[y0*Iw+x0])+sin(DirIn[y*Iw+x])*sin(DirIn[y0*Iw+x0]));

						if ((scalar_product>0 && scalar_product>=cos_angle) 
							|| (scalar_product<0 && scalar_product<=-cos_angle) ){

							// Labelling pixel
							nbPixLabel++;
							sumRdg += RdgIn[y*Iw+x];
							LabOut[y*Iw+x] = etiquette;

							// Store the current pixel as a point to be studied later
							// And iterate x and y
							if (recursion_level<max_recursion_level){

								// Store the current pixel as a point to be studied later
								stack_x[recursion_level] = x; stack_y[recursion_level] = y;
								stack_x0[recursion_level] = x0; stack_y0[recursion_level] = y0;
								nbIterOnMaskIn[recursion_level] = nbCurrentIterOnMaskIn;

								// And iterate x and y
								x0 = x; y0 = y;
								x = OSC_MAX(x0-NbRangePix, 1); y = OSC_MAX(y0-NbRangePix, 1);
								nbCurrentIterOnMaskIn = 0;

								recursion_level++;
							}
							else
							{
                                ExtractProc->FatalErrorOnPreviousImage = 1;
								ErrStartErrorReportFromLeaf(This->ErrHdl, "OscRdgDirectionalLabelingFromSeedDetailed", OSC_ERR_LABELING, "Label too long: aborted");
								return OSC_ERR_LABELING;
							}
							continue;
						}
					}

					// Case 2 - on the mask
					if ((LabOut[y*Iw+x]==0) && (MaskIn[y*Iw+x] == 1))
					{
						// check that another iteration on the MaskIn is possible
						if (nbCurrentIterOnMaskIn < nbMaxIterOnMaskIn){
							LabOut[y*Iw+x] = -1;

							// Store the current pixel as a point to be studied later
							// And iterate x and y
							if (recursion_level<max_recursion_level){
								// Store the current pixel as a point to be studied later
								stack_x[recursion_level] = x; stack_y[recursion_level] = y;
								stack_x0[recursion_level] = x0; stack_y0[recursion_level] = y0;
								nbIterOnMaskIn[recursion_level] = nbCurrentIterOnMaskIn;

								// And iterate x and y
								x0 = x; y0 = y;
								nbCurrentIterOnMaskIn ++;
								x = OSC_MAX(x0-NbRangePix, 1); y = OSC_MAX(y0-NbRangePix, 1);

								recursion_level++;
							}
							else
							{
                                ExtractProc->FatalErrorOnPreviousImage = 1;
								ErrStartErrorReportFromLeaf(This->ErrHdl, "OscRdgDirectionalLabelingFromSeedDetailed", OSC_ERR_LABELING, "Label too long: aborted");
								return OSC_ERR_LABELING;
							}

							continue;
						}
					}

					// Increment x and y in the search range
					x++;
					if (x>OSC_MIN(x0+NbRangePix,Iw-1))
					{
						y++;
						if (y>OSC_MIN(y0+NbRangePix,Ih-1))
						{       
							// We reach the end of the search region (from x0,y0)
							// We change the reference to the stacked x,y (and x0, y0)                      

							//end of scanning in this recursion level
							recursion_level--;

							// restore i, j, i0 and j0
							x  = stack_x [recursion_level];
							y  = stack_y [recursion_level];
							x0 = stack_x0[recursion_level];
							y0 = stack_y0[recursion_level];

							nbCurrentIterOnMaskIn = nbIterOnMaskIn[recursion_level];

							continue;
						}
						else
						{
							x= OSC_MAX(x0-NbRangePix, 0);
						}                              
					}
				}

				// End of labelling of current label
				NbPelPerLabOut[etiquette-2] = nbPixLabel;
				CumRdgOut[etiquette-2] = sumRdg;
				etiquette++;
			}
		}

	for(p=0; p<Ih*Iw; p++)
		if (LabOut[p] == -1)
			LabOut[p] = 0;

	*NbLabelsOut = etiquette-2;
	
	// Cum ridge as a mean - with a penalization for large regions
	for(k=0; k<*NbLabelsOut; k++)
		CumRdgOut[k] *= (OSC_CUMRDG_PEN_SURF / (ExtractProc->SSFactor*ExtractProc->SSFactor)) 
						/ (float)OSC_MAX((OSC_CUMRDG_PEN_SURF / (ExtractProc->SSFactor*ExtractProc->SSFactor)) , NbPelPerLabOut[k]);

	return 0;
}


// *******************************************************************
// Non-directional version

int OscRdgLabelingFromSeed(COscHdl OscHdl, const unsigned char *SeedIn, const unsigned char *ThrIn
							, int NbRangePix, short *stack_x, short *stack_y, short *stack_x0, short *stack_y0, short *nbIterOnMaskIn, int *LabOut
                            , int *NbLabelsOut, int *NbPelPerLabOut, float *CumRdgOut)
{
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	int x, y, x0, y0, xMin, xMax, yMin, yMax, p, etiquette = 2, k;
	int recursion_level, max_recursion_level;

	float *RdgIn			= ExtractProc->ImFrg;
	int Iw = ExtractProc->IwProcAngio, Ih = ExtractProc->IhProcAngio;
	COscRoiCoord *RoiCoord = &ExtractProc->ShuttersRoiSafe;
	
	xMin = OSC_MAX(NbRangePix, RoiCoord->XMin);
	xMax = OSC_MIN(Iw-NbRangePix-1, RoiCoord->XMax);
	yMin = OSC_MAX(NbRangePix, RoiCoord->YMin);
	yMax = OSC_MIN(Ih-NbRangePix-1, RoiCoord->YMax);

	max_recursion_level = (xMax-xMin+1)*(yMax-yMin+1);

	// initialization of LabOut to ThrIn
	for(y=0; y<Ih; y++)
		for(x=0; x<Iw; x++)
			LabOut[y*Iw+x] = (int)ThrIn[y*Iw+x];

	// *** Loop on the image
	// When new unlabeled pixel met, initialize new label and perform its labeling completely.
	// It is done by storing the next points to discuss into the different stacks

	for(y = yMin; y <= yMax; y++)
		for(x = xMin; x <= xMax; x++){

			// * Start new label
			if(SeedIn[y*Iw+x]==1 && LabOut[y*Iw+x]==1){

				// Initializations
				int   nbPixLabel = 0, nbCurrentIterOnMaskIn = 0;
				float sumRdg = 0.f;
				recursion_level = 0; x0 = x; y0 = y;

				// * Labeling the complete image with that label

				// Pixel of interest
				// 1- being thresholded and unlabeled 2- being on the mask 
				// or 3- if there are points to be processed stored in the stacks
				while (LabOut[y*Iw+x]==1 || recursion_level>0){

					// Case 1- thresholded pixel
					if (LabOut[y*Iw+x]==1){

						// Labelling pixel
						nbPixLabel++;
						sumRdg += RdgIn[y*Iw+x];
						LabOut[y*Iw+x] = etiquette;

						// Store the current pixel as a point to be studied later
						// And iterate x and y
						if (recursion_level<max_recursion_level){

							// Store the current pixel as a point to be studied later
							stack_x[recursion_level] = x; stack_y[recursion_level] = y;
							stack_x0[recursion_level] = x0; stack_y0[recursion_level] = y0;
							nbIterOnMaskIn[recursion_level] = nbCurrentIterOnMaskIn;

							// And iterate x and y
							x0 = x; y0 = y;
							x = OSC_MAX(x0-NbRangePix, 1); y = OSC_MAX(y0-NbRangePix, 1);
							nbCurrentIterOnMaskIn = 0;

							recursion_level++;
						}
						else
						{
                            ExtractProc->FatalErrorOnPreviousImage = 1;
							ErrStartErrorReportFromLeaf(This->ErrHdl, "OscRdgLabelFromSeed", OSC_ERR_LABELING, "Label too long: aborted");
							return OSC_ERR_LABELING;
						}
						continue;
					}

					// Increment x and y in the search range
					x++;
					if (x>OSC_MIN(x0+NbRangePix,Iw-1))
					{
						y++;
						if (y>OSC_MIN(y0+NbRangePix,Ih-1))
						{       
							// We reach the end of the search region (from x0,y0)
							// We change the reference to the stacked x,y (and x0, y0)                      

							//end of scanning in this recursion level
							recursion_level--;

							// restore i, j, i0 and j0
							x  = stack_x [recursion_level];
							y  = stack_y [recursion_level];
							x0 = stack_x0[recursion_level];
							y0 = stack_y0[recursion_level];

							nbCurrentIterOnMaskIn = nbIterOnMaskIn[recursion_level];

							continue;
						}
						else
						{
							x= OSC_MAX(x0-NbRangePix, 0);
						}                              
					}
				}

				// End of labelling of current label
				NbPelPerLabOut[etiquette-2] = nbPixLabel;
				CumRdgOut[etiquette-2] = sumRdg;
				etiquette++;
			}
		}

	for(p=0; p<Ih*Iw; p++)
		if (LabOut[p] == -1)
			LabOut[p] = 0;

	*NbLabelsOut = etiquette-2;
	
	// Cum ridge as a mean - with a penalization for large regions
	for(k=0; k<*NbLabelsOut; k++)
		CumRdgOut[k] *= (OSC_CUMRDG_PEN_SURF / (ExtractProc->SSFactor*ExtractProc->SSFactor)) 
					/ (float)OSC_MAX((OSC_CUMRDG_PEN_SURF / (ExtractProc->SSFactor*ExtractProc->SSFactor)) , NbPelPerLabOut[k]);

	return 0;
}
