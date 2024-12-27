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

// ****************************************************************************************
// Hough Parabol extraction
// ****************************************************************************************

int DiaHoughParabolExtraction(CDiaHdl DiaHdl, float * grad, int Iw, int Ih, CDiaRoiCoord RoiCoord, float gradThres				// Gradient, size, ROI and threshold
								, int nbCellsPerDimA, int nbCellsPerDimB, int nbCellsPerDimC, float rateCellsZeroed	// Nb of cells per dimensions (for a, b and c), nb of cells that are zeroed around a peak (in percentage of cellDim)
								, int nbExtractedParamMax, float seuilMinDist, CDiaDiaphragmParabs *coeffOut){		// Max number of extracted parabols, mean distance between them two parabols (else marge), extracted parameters

	CDia			*This = (CDia*)DiaHdl.Pv;
	CDiaProc		*DiaProc = &(This->DiaProc);
	int i, Err;
	float maxAcc, a, b, c;

	// Accumulation matrix computation
	Err = DiaFillAccumulationMatrix(DiaHdl, grad, Iw, Ih, RoiCoord, gradThres, nbCellsPerDimA, nbCellsPerDimB, nbCellsPerDimC);	
	if (Err){
		ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaHoughParabolExtraction", "DiaFillAccumulationMatrix", 0, Err);
		return Err;
	}

	// Iterative peak extraction
	maxAcc = 1.f; i=0;
	while ( (maxAcc > 0) && (i<nbExtractedParamMax) ) {

		maxAcc = DiaExtractFromAccumulationMatrix(DiaProc->AccumulationMatrix
			, nbCellsPerDimA, nbCellsPerDimB, nbCellsPerDimC, rateCellsZeroed, Iw, Ih, &a, &b, &c);

		if (i==0){
			(*coeffOut).Param[i][0] = a;
			(*coeffOut).Param[i][1] = b;
			(*coeffOut).Param[i][2] = c;
			i++;
		} else if (maxAcc > 0) {
			// New parabol far enough from the existing ones?

			float minDist = -1.f, dist, y1, y2;
			int j, nbPtsInRoi, x;

			for (j=0; j<i; j++){
				dist = 0; nbPtsInRoi = 0;

				for (x=RoiCoord.XMin; x<RoiCoord.XMax; x++){
					y1 =(*coeffOut).Param[j][0] * (x-Iw/2) * (x-Iw/2) 
						+ (*coeffOut).Param[j][1] * (x-Iw/2)
						+ (*coeffOut).Param[j][2];
					y2 = a * (x-Iw/2) * (x-Iw/2) + b * (x-Iw/2) + c;

					if ( (y1 >= RoiCoord.YMin) && (y1 < RoiCoord.YMax) && (y2 >= RoiCoord.YMin) && (y2 < RoiCoord.YMax) ){
						nbPtsInRoi ++;
						dist += DIA_ABS( y1-y2 );
					}
				}

				if (nbPtsInRoi > 0){
					if ((minDist == -1)||(dist / nbPtsInRoi < minDist))
						minDist = dist / nbPtsInRoi;
				}
			}

			if (minDist > seuilMinDist){
				(*coeffOut).Param[i][0] = a;
				(*coeffOut).Param[i][1] = b;
				(*coeffOut).Param[i][2] = c;
				i++;
			}
		}
	}

	coeffOut->nbExtractedDiaphragm = i;

	return 0;
}

// ******************************
// Creating and filling accumulation matrix

int DiaFillAccumulationMatrix(CDiaHdl DiaHdl, float * Im, int Iw, int Ih, CDiaRoiCoord RoiCoord, float gradThres
								, int nbCellsPerDimA, int nbCellsPerDimB, int nbCellsPerDimC){

	CDia			*This = (CDia*)DiaHdl.Pv;
	CDiaProc		*DiaProc = &(This->DiaProc);

	float aMin, aMax, aStep, bMin, bMax, bStep, cStep, cCoeff, a, b, c, val;
	int cMin, cMax, x, y, nbCellsPerDim2, indiceA, indiceB, indiceC;

	// Initializations
	memset(DiaProc->AccumulationMatrix ,0,nbCellsPerDimA *nbCellsPerDimB *nbCellsPerDimC*sizeof(float));
	nbCellsPerDim2 = nbCellsPerDimB*nbCellsPerDimC;

	// Parameter limits
	aMin = HOUGH_A_MIN/Iw; aMax = HOUGH_A_MAX/Iw; aStep = (float)(aMax-aMin) / (nbCellsPerDimA-1);
	bMin = HOUGH_B_MIN; bMax = HOUGH_A_MAX; bStep = (float)(bMax-bMin) / (nbCellsPerDimB-1);
	cMin = (int)(HOUGH_C_MIN*Iw); cMax = (int)(HOUGH_C_MAX*Iw); cStep = (float)(cMax-cMin) / (nbCellsPerDimC-1);

	a = 1/aStep; cCoeff = 1/cStep;

	// Matrix filling
	for (y=RoiCoord.YMin; y<RoiCoord.YMax; y++)
		for (x=RoiCoord.XMin; x<RoiCoord.XMax; x++)
			if (Im[y*Iw+x] > gradThres){
				val = Im[y*Iw+x];

				for (indiceA = 0; indiceA < nbCellsPerDimA; indiceA ++){
					a = aMin + aStep * indiceA;

					for (indiceB = 0; indiceB < nbCellsPerDimB; indiceB ++){

						b = bMin + bStep * indiceB;

						c = y - a*(x-Iw/2)*(x-Iw/2) - b*(x-Iw/2);	// Hough equation

						if ((c < cMax-EPSILON) && (c >= cMin)){

							indiceC = (int)(cCoeff * (c-cMin) + 0.5f);

							DiaProc->AccumulationMatrix[indiceA * nbCellsPerDim2 + indiceB * nbCellsPerDimC + indiceC] 
								+= val;
						}
					}
				}
			}

	return 0;
}

// ******************************
// Extracting the parameters corresponding to the max, and modifying the accumulation matrix

float DiaExtractFromAccumulationMatrix(float *AccumulationMatrix
										, int nbCellsPerDimA, int nbCellsPerDimB, int nbCellsPerDimC
										, float rateCellsZeroed, int Iw, int Ih
										, float * a, float *b, float *c){

	int cMin, cMax, i, iA, iB, iC, sizeAccumulationMatrix, indiceMax, indiceC, indiceB, indiceA
		, nbCellsZeroedA2, nbCellsZeroedB2, nbCellsZeroedC2, indiceAMin, indiceAMax, indiceBMin, indiceBMax
		, indiceCMin, indiceCMax, shiftB;
	float aMin, aMax, aStep, bMin, bMax, bStep, cStep, maxAcc;

	// Limits and initializations
	aMin = HOUGH_A_MIN/Iw; aMax = HOUGH_A_MAX/Iw; aStep = (float)(aMax-aMin) / (nbCellsPerDimA-1);
	bMin = HOUGH_B_MIN; bMax = HOUGH_A_MAX; bStep = (float)(bMax-bMin) / (nbCellsPerDimB-1);
	cMin = (int)(HOUGH_C_MIN*Iw); cMax = (int)(HOUGH_C_MAX*Iw); cStep = (float)(cMax-cMin) / (nbCellsPerDimC-1);

	sizeAccumulationMatrix = nbCellsPerDimA*nbCellsPerDimB*nbCellsPerDimC;
	indiceMax = 0; maxAcc = 0;

	// Maximum extraction
	for (i=0; i<sizeAccumulationMatrix; i++)
		if (AccumulationMatrix[i] > maxAcc){
			indiceMax = i; maxAcc = AccumulationMatrix[i];
		}	
	indiceA = indiceMax / (nbCellsPerDimB*nbCellsPerDimC);
	indiceB = (indiceMax - indiceA*nbCellsPerDimB*nbCellsPerDimC) / nbCellsPerDimC;
	indiceC = indiceMax - indiceA*nbCellsPerDimB*nbCellsPerDimC - indiceB*nbCellsPerDimC;

	// Zeroing around the extracted peak
	nbCellsZeroedA2 = (int)(0.5f*rateCellsZeroed*nbCellsPerDimA);
	nbCellsZeroedB2 = (int)(0.5f*rateCellsZeroed*nbCellsPerDimB);
	nbCellsZeroedC2 = (int)(0.5f*rateCellsZeroed*nbCellsPerDimC);

	indiceAMin = DIA_MAX(0,indiceA - nbCellsZeroedA2); indiceAMax = DIA_MIN(nbCellsPerDimA,indiceA + nbCellsZeroedA2);
	indiceBMin = DIA_MAX(0,indiceB - nbCellsZeroedB2); indiceBMax = DIA_MIN(nbCellsPerDimB,indiceB + nbCellsZeroedB2);
	indiceCMin = DIA_MAX(0,indiceC - nbCellsZeroedC2); indiceCMax = DIA_MIN(nbCellsPerDimC,indiceC + nbCellsZeroedC2);
	for (iA = indiceAMin; iA < indiceAMax; iA++){
		int shiftA = iA * nbCellsPerDimB*nbCellsPerDimC;
		for (iB = indiceBMin; iB < indiceBMax; iB++){
			shiftB = shiftA + iB * nbCellsPerDimC;
			for (iC = indiceCMin; iC < indiceCMax; iC++)
				AccumulationMatrix[shiftB + iC] = 0;
		}
	}

	*a = (aMin + indiceA * aStep);
	*b = (bMin + indiceB * bStep);
	*c = (cMin + indiceC * cStep);

	return maxAcc;
}

// **********************************************************************************
//	1D Hough extraction (c only)
// **********************************************************************************
int DiaHoughParabolExtraction1D(CDiaHdl DiaHdl, float * grad, int Iw, int Ih, CDiaRoiCoord RoiCoord, float gradThres
					 		    , int nbCells, float aIn, float bIn, CDiaDiaphragmParab *coeffOut){

	CDia			*This = (CDia*)DiaHdl.Pv;
	CDiaProc		*DiaProc = &(This->DiaProc);
	int Err;
	float c;

	// Accumulation matrix computation
	Err = DiaFillAccumulationMatrix1D(DiaHdl, grad, Iw, Ih, RoiCoord, gradThres, nbCells, aIn, bIn);
	if (Err){
		ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaHoughParabolExtraction", "DiaFillAccumulationMatrix1D", 0, Err);
		return Err;
	}

	// Peak extraction
	DiaExtractFromAccumulationMatrix1D(DiaProc->AccumulationMatrix, nbCells, Iw, Ih, &c);

	(*coeffOut).Param[0] = aIn;
	(*coeffOut).Param[1] = bIn;
	(*coeffOut).Param[2] = c;

	return 0;
}

// ******************************
// Creating and filling accumulation matrix

int DiaFillAccumulationMatrix1D(CDiaHdl DiaHdl, float * Im, int Iw, int Ih, CDiaRoiCoord RoiCoord, float gradThres
								, int nbCells, float a, float b){
									
	CDia			*This = (CDia*)DiaHdl.Pv;
	CDiaProc		*DiaProc = &(This->DiaProc);
	float cStep, cCoeff, c, val;
	int cMin, cMax, x, y, indiceC;

	memset(DiaProc->AccumulationMatrix,0,nbCells *sizeof(float));

	// Parameter limits
	cMin = (int)(HOUGH_C_MIN*Iw); cMax = (int)(HOUGH_C_MAX*Iw); cStep = (float)(cMax-cMin) / (nbCells-1);

	cCoeff = 1/cStep;

	for (y=RoiCoord.YMin; y<RoiCoord.YMax; y++)
		for (x=RoiCoord.XMin; x<RoiCoord.XMax; x++)
			if (Im[y*Iw+x] > gradThres){
				val = Im[y*Iw+x];

				c = y - a*(x-Iw/2)*(x-Iw/2) - b*(x-Iw/2);

				if ((c < cMax-EPSILON) && (c >= cMin)){

					indiceC = (int)(cCoeff * (c-cMin) + 0.5f);

					DiaProc->AccumulationMatrix[indiceC] += val;
				}
			}
	return 0;
}

// ******************************
// Extracting the parameters corresponding to the max, and modifying the accumulation matrix
float DiaExtractFromAccumulationMatrix1D(float *AccumulationMatrix, int nbCells, int Iw, int Ih, float *c){

	int cMin, cMax, indiceMax, i;
	float cStep, maxAcc;

	cMin = (int)(HOUGH_C_MIN*Iw); cMax = (int)(HOUGH_C_MAX*Iw); cStep = (float)(cMax-cMin) / (nbCells-1);

	indiceMax = 0; maxAcc = 0;

	for (i=0; i<nbCells; i++)
		if (AccumulationMatrix[i] > maxAcc){
			indiceMax = i; maxAcc = AccumulationMatrix[i];
		}
	
	*c = (cMin + indiceMax * cStep);

	return maxAcc;
}
