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

#include <CBDDev.h>

// HessKernel parameters
static const int Bo1 = 2;
static const float Lambda = (float)0.33;
static const float Ro = (float)2;

void CathBDtRdgCalcGaussHessian2(float *Gau, int i, int j, int Iw, int Ih, float *Lxx, float *Lyy, float *Lxy)
{

	*Lxx = Gau[i*Iw+j+1] + Gau[i*Iw+j-1] - 2*Gau[i*Iw+j];
	*Lyy = Gau[(i+1)*Iw+j] + Gau[(i-1)*Iw+j] - 2*Gau[i*Iw+j];

	*Lxy = (Gau[(i+1)*Iw+j+1] + Gau[(i-1)*Iw+j-1] - Gau[(i-1)*Iw+j+1] - Gau[(i+1)*Iw+j-1]) * (float).25;
}


void CathBDtRdgCalcGaussHessEigenValues(float *Gau, int i, int j, int Iw, int Ih, 
								 float *Lxx, float *Lyy, float *Lxy, float *L1, float *L2)
{
	float lxx, lyy, lxy, Det;
	float l1,l2;

	CathBDtRdgCalcGaussHessian2(Gau, i, j, Iw, Ih,&lxx, &lyy, &lxy);
	*Lxx = lxx;   
	*Lyy = lyy;  
	*Lxy = lxy;

	Det = (float)sqrt( (double)( CBD_SQ(lxx-lyy) + 4*CBD_SQ(lxy) ) );
	l1  = 0.5f * (lxx + lyy - Det);
	l2  = 0.5f * (lxx + lyy + Det);

	// added by SV
	// sort the eigenvalues by decreasing magnitude
	if (CBD_ABS_F(l1) < CBD_ABS_F(l2))
	{
		float tmp = l1;
		l1 = l2;
		l2 = tmp;
	}

	*L1 = l1;
	*L2 = l2;
}

int CathBDtRdgMonoscaleComputation(short *Ori, int Iw, int Ih, int ShutterPos[4]
									, float KernelSig, float *Rdg, CCBD * This)
{
	int    i, j, p, Err;
	float lxx, lyy, lxy, L1, L2;
	float * Gau;

    int IMin1 = CBD_MAX(ShutterPos[2], Bo1)+1;
    int IMax1 = CBD_MIN(ShutterPos[3], Ih-1-Bo1)-1;
    int JMin1 = CBD_MAX(ShutterPos[0], Bo1)+1;
    int JMax1 = CBD_MIN(ShutterPos[1], Iw-1-Bo1)-1;

	memset(Rdg, 0, Iw*Ih*sizeof(float));

	// calculation of the Gaussian image used to determine the eigenvalues
	Gau = (float*)calloc(Iw*Ih, sizeof(float));
	if (Gau == NULL){
		ErrStartErrorReportFromLeaf(This->ErrHdl, "CathBDtRdgMonoscaleComputation", CBD_ERR_MEM_ALLOC
									, "Allocation of Gau impossible");
		return CBD_ERR_MEM_ALLOC;
	}
	for (p=0; p<Iw*Ih; p++)
		Gau[p] = (float)Ori[p];

    Err = BxIsoGaussFilterFloat2DROI(Gau, Gau, KernelSig, Iw, Ih, JMin1-1, JMax1+1, IMin1-1, IMax1+1);
    if (Err == -1){
        free(Gau);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "CathBDtRdgMonoscaleComputation", CBD_ERR_MEM_ALLOC
									, "Allocation error in BxIsoGaussFilterFloat2DROI");
		return CBD_ERR_MEM_ALLOC;
    }

	for(i=IMin1;i<=IMax1;i++) {
		for(j=JMin1;j<=JMax1;j++)
		{
			// calculate Gaussian Hessian eigenvalues
			CathBDtRdgCalcGaussHessEigenValues(Gau, i, j, Iw, Ih, &lxx, &lyy, &lxy, &L1, &L2);
			//float Vx =- (lxx - lyy); 
			//float Vy =- 2*lxy; 

			if(L1 <= 0)
				continue;
			
			// Ridgeness
			Rdg[i*Iw+j] = L1 - L2 * Lambda;
		}
	}	
	free(Gau);

	return 0;
}

// *** Mono-thread version

int CathBDtRdgEnhanceMultiScaleRidge(short *Ori, float *Rdg, int Iw, int Ih, int ShutterPos[4]
								, float *KernelSigs, int nKernelSigs, float gamma, CCBD * This)
{
	int IMin1 = CBD_MAX(ShutterPos[2], 0);
	int IMax1 = CBD_MIN(ShutterPos[3], Ih-1);
	int JMin1 = CBD_MAX(ShutterPos[0], 0);
	int JMax1 = CBD_MIN(ShutterPos[1], Iw-1);
	int i,j,k;
	float * RdgScale = (float*)calloc(Iw*Ih, sizeof(float));

    if (RdgScale == NULL){
		ErrStartErrorReportFromLeaf(This->ErrHdl, "CathBDtRdgEnhanceMultiScaleRidge", CBD_ERR_MEM_ALLOC, "Allocation of RdgScale impossible");
		return CBD_ERR_MEM_ALLOC;
	}

	// initialize the input pictures
	memset(Rdg, 0, Iw*Ih*sizeof(float));

	// get all potentials for all required scales 
	for (k=0;k <nKernelSigs; k++) 
	{
		// compute ridges and directions 
		int Err = CathBDtRdgMonoscaleComputation(Ori, Iw, Ih, ShutterPos, KernelSigs[k], RdgScale, This);
		if(Err)
		{
			ErrAppendErrorReportFromCaller(This->ErrHdl, "CathBDtRdgEnhanceMultiScaleRidge", "CathBDtRdgMonoscaleComputation", 0, Err);
            free(RdgScale);
			return Err;
		}

		float normalizationFactor = (float) exp(gamma*log(KernelSigs[k]));
		for(i=IMin1;i<=IMax1;i++) 
			for(j=JMin1;j<=JMax1;j++)
			{ 
				float ridgeness = RdgScale[i*Iw+j]*normalizationFactor;
				if (Rdg[i*Iw+j]<ridgeness) 
					Rdg  [i*Iw+j] = ridgeness;
			}
	}

	free(RdgScale);

	return 0;
}

float CathBDtPercentileComputation(float *Rdg, int Iw, int Ih, int ShutterPos[4], int NbPixKept)
{
	int   i, j, k, HistoLen = 1000, *Histo, *Cumul, Level;
	int   iMin, iMax, jMin, jMax;
	float MaxRdg = 0, NormFac;

	iMin = ShutterPos[2];
	iMax = ShutterPos[3];
	jMin = ShutterPos[0];
	jMax = ShutterPos[1];

	// Get Max
	for(i=iMin; i<=iMax; i++)
		for(j=jMin; j<=jMax; j++)
			MaxRdg = CBD_MAX(MaxRdg, Rdg[i*Iw+j]);

	// Compute histo
	Histo = (int *)malloc((HistoLen+1) * sizeof(int));
	Cumul = (int *)malloc((HistoLen+1) * sizeof(int));
    if (Histo == NULL || Cumul == NULL){
        if (Histo !=NULL){free(Histo); }
        if (Cumul !=NULL){free(Cumul); }
		printf("CathBDtPercentileComputation: could not allocate Histo or Cumul\n");
        return -1;
	}
	for(k = 0; k <= HistoLen; k++) { Histo[k] = 0; Cumul[k] = 0; }
	NormFac = (float)HistoLen / MaxRdg;

	for(i=iMin; i<=iMax; i++) {
		for(j=jMin; j<=jMax; j++)
		{
			Level = (int)(NormFac * Rdg[i*Iw+j]);
			Histo[Level]++;
		}
	}

	Cumul[0]=Histo[0];
	for(k = 1; k <= HistoLen; k++) Cumul[k] = Cumul[k-1] + Histo[k];

	// Get threshold corresponding to number of pixels to be kept
	k = 0;
	while(Cumul[HistoLen] - Cumul[k] > NbPixKept) k++;
	k = CBD_CLIP(k, 0, HistoLen);

	// Ending
	free(Histo);
	free(Cumul);

	return (k / NormFac);
}

int CathBDtImAdjust(CCBD * This, float * Rdg, int Iw, int Ih, int ShutterPos[4], float percentileL
					, float percentileH, float * RdgRescaled){

	int p, nbPixels, Err;
	float thresL, thresH;
	float minRdg = FLT_MAX, maxRdg = -FLT_MAX;
	float * RdgTemp = (float*)malloc(Iw*Ih*sizeof(float));
	if (RdgTemp == NULL){
		ErrStartErrorReportFromLeaf(This->ErrHdl, "CathBDtImAdjust", CBD_ERR_MEM_ALLOC
									, "Allocation of RdgTemp impossible");
		return CBD_ERR_MEM_ALLOC;
	}

	nbPixels = (int)CBD_RND(percentileL *(ShutterPos[3]-ShutterPos[2])*(ShutterPos[1]-ShutterPos[0]));
	thresL = CathBDtPercentileComputation(Rdg, Iw, Ih, ShutterPos, nbPixels);
	nbPixels = (int)CBD_RND(percentileH *(ShutterPos[3]-ShutterPos[2])*(ShutterPos[1]-ShutterPos[0]));
	thresH = CathBDtPercentileComputation(Rdg, Iw, Ih, ShutterPos, nbPixels);

	memcpy(RdgTemp, Rdg, Iw*Ih*sizeof(float));
	for (p=0; p<Iw*Ih; p++){
		if (RdgTemp[p] < thresL)
			RdgTemp[p] = thresL;
		if (RdgTemp[p] > thresH)
			RdgTemp[p] = thresH;
	}

	//float factor = 1/(thresH-thresL);
	//for (p=0; p<Iw*Ih; p++)
	//	RdgTemp[p] = factor * (RdgTemp[p]-thresL);

	Err = BxIsoGaussFilterFloat2D(RdgTemp, RdgRescaled, CBD_SIGMASMOOTH, Iw, Ih);
    if (Err == -1){
        free(RdgTemp);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "CathBDtImAdjust", CBD_ERR_MEM_ALLOC
									, "Allocation error in BxIsoGaussFilterFloat2D");
		return CBD_ERR_MEM_ALLOC;
    }

	for (p=0; p<Iw*Ih; p++){
		if (minRdg > RdgRescaled[p])
			minRdg = RdgRescaled[p];
		if (maxRdg < RdgRescaled[p])
			maxRdg = RdgRescaled[p];
	}

	float factor = 1.f/(maxRdg-minRdg);
	for (p=0; p<Iw*Ih; p++)
		RdgRescaled[p] = factor * (RdgRescaled[p]-minRdg);

	free(RdgTemp);

	return 0;
}