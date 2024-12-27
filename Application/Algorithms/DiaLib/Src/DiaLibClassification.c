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
#include "float.h"

// float cmp function needed by qsort - for subsequent median computations

static int flt_cmp2(const void *f1, const void *f2) 
{ 
	float diff = *(float *)f1 - *(float *)f2 ;

	if (diff<=0)
	{
		if (diff==0) 
			return 0;
		else
			return -1;
	}
	else 
		return +1;
}


// *************************************
// Selection of the best parabol from every parabols extracted during the initialization
//
// Adaboost on parabola features and contrast, and angulations 

int DiaClassif(CDiaHdl DiaHdl, short * ImFull, int IwFull, int SSFactor, CDiaRoiCoord RoiCoord			// Input image, dimension and ROI
				, CDiaDiaphragmParabs *coeffs		// Representant is updated with the live images if an extracted "live" parabol is close enough (seuilDistChoice) from it
				, int delta, float Rot, float Ang
				, CDiaDiaphragmParab *selectedCoeff, int *diaphragmDetected){	// Delta to compute contrast, Final representant and classif result (contrasted enough and more homogenous)

	CDia			*This = (CDia*)DiaHdl.Pv;
	CDiaProc		*DiaProc = &(This->DiaProc);
	short minIm, maxIm;
	int xMin = RoiCoord.XMin, xMax = RoiCoord.XMax, yMin = RoiCoord.YMin, yMax = RoiCoord.YMax
		, i, j, x, y, yDia, yOut;
	float bestH;

	// For adaboost
	int features[20] = {1, 6, 4, 5, 3, 6, 4, 4, 3, 1, 6, 3, 5, 4, 4, 4, 3, 5, 5, 5};	// 1 - median contrast, normalized, 2 - Rot, 3- And, 4- a parabola param, 5- b, 6- c
	float thress[20] = {0.1281f, 289.0790f*IwFull/512, 0.0005f, 0.5344f, 2.6800f, 423.2651f*IwFull/512
						, 0.0005f, -0.0002f, 19.9700f, 0.2368f, 255.0694f*IwFull/512, -0.4300f, 0.8929f
						, 0.0006f, 0.0013f, -0.0002f, 21.2000f, 0.9926f, -0.6625f, -0.1264f};	// Adaboost threadholds
	int directs[20] = {0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1};				// Adaboost signs
	float alphas[20] = {0.7395f, 0.4848f, 0.3531f, 0.3577f, 0.3514f, 0.2697f, 0.1767f, 0.1848f, 0.2324f
						, 0.1894f, 0.1695f, 0.1334f, 0.1373f, 0.1524f, 0.1424f, 0.1187f, 0.1664f, 0.1332f, 0.1364f, 0.1459f};	// Adaboost weights

	// Classification diaphragm/no diaphragm - for every extracted diaphragm
	xMin *= SSFactor; xMax *= SSFactor; yMin *= SSFactor; yMax *= SSFactor;

	minIm = ImFull[yMin*IwFull+xMin]; maxIm = ImFull[yMin*IwFull+xMin];
	for (y=yMin; y<yMax; y++)
		for (x=xMin; x<xMax; x++){
			if (ImFull[y*IwFull+x] < minIm)
				minIm = ImFull[y*IwFull+x];
			if (ImFull[y*IwFull+x] > maxIm)
				maxIm = ImFull[y*IwFull+x];
		}

	bestH = -FLT_MAX;
	for (j=0; j<coeffs->nbExtractedDiaphragm; j++){
        int nbElts, nbEltsCont;
        float contrastNorm, a, b, c, meanHeight, h;

		a = coeffs->Param[j][0] / SSFactor; 
		b = coeffs->Param[j][1]; 
		c = coeffs->Param[j][2] * SSFactor;

		// Features
		nbEltsCont = 0; nbElts = 0; meanHeight = 0;
		for (x=xMin; x<=xMax; x++){
			y = (int)(a*(x-IwFull/2)*(x-IwFull/2) + b*(x-IwFull/2) +c+.5f);

			if ( (yMin <= y) && (yMax >= y) ){
				meanHeight += y; nbElts++;

				yDia = DIA_CLIP(yMin, y+delta, yMax);
				yOut = DIA_CLIP(yMin, y-delta, yMax);
				DiaProc->cont[nbEltsCont] = (float)(ImFull[yOut*IwFull+x]-ImFull[yDia*IwFull+x]);
				nbEltsCont++;
			}
		}
		meanHeight /= nbElts;
		qsort(DiaProc->cont, nbEltsCont, sizeof(float), flt_cmp2);
		contrastNorm = DiaProc->cont[nbEltsCont/2] / (maxIm - minIm);

		// b-Adaboost decision
		h = 0;
		for (i=0; i<20; i++){
			switch (features[i]){
				case 1:
					if (directs[i] == 1){
						if (contrastNorm <= thress[i])
							h += alphas[i];
						else
							h -= alphas[i];
					} else {
						if (contrastNorm > thress[i])
							h += alphas[i];
						else
							h -= alphas[i];
					}
					break;
				case 2:
					if (directs[i] == 1){
						if (Rot <= thress[i])
							h += alphas[i];
						else
							h -= alphas[i];
					} else {
						if (Rot > thress[i])
							h += alphas[i];
						else
							h -= alphas[i];
					}
					break;
				case 3:
					if (directs[i] == 1){
						if (Ang <= thress[i])
							h += alphas[i];
						else
							h -= alphas[i];
					} else {
						if (Ang > thress[i])
							h += alphas[i];
						else
							h -= alphas[i];
					}
					break;
				case 4:
					if (directs[i] == 1){
						if (a <= thress[i])
							h += alphas[i];
						else
							h -= alphas[i];
					} else {
						if (a > thress[i])
							h += alphas[i];
						else
							h -= alphas[i];
					}
					break;
				case 5:
					if (directs[i] == 1){
						if (b <= thress[i])
							h += alphas[i];
						else
							h -= alphas[i];
					} else {
						if (b > thress[i])
							h += alphas[i];
						else
							h -= alphas[i];
					}
					break;
				case 6:
					if (directs[i] == 1){
						if (c <= thress[i])
							h += alphas[i];
						else
							h -= alphas[i];
					} else {
						if (c > thress[i])
							h += alphas[i];
						else
							h -= alphas[i];
					}
					break;
			}
		}

		if (h>=bestH){
			bestH = h;
			selectedCoeff->Param[0] = coeffs->Param[j][0];
			selectedCoeff->Param[1] = coeffs->Param[j][1];
			selectedCoeff->Param[2] = coeffs->Param[j][2];
		}
	}

	if (bestH >= 0)
		*diaphragmDetected = 1;
	else
		*diaphragmDetected = 0;

	return 0;
}
