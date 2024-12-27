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
// *** Gradient computation

int DiaGradientComputation(CDiaHdl DiaHdl, short * Im, float * Grad, int Iw, int Ih, CDiaRoiCoord RoiCoord, float sigma){
	
	CDia			*This = (CDia*)DiaHdl.Pv;
	CDiaProc		*DiaProc = &(This->DiaProc);
	int				x,y,p;
	float			gradX, gradY;
	float			* Gau = DiaProc->BufSS2F;
    int             Err;

	int dBo = (int)(GRAD_RO*sigma) + 1;
	int XMin = (DIA_MAX(RoiCoord.XMin + dBo, GRAD_BO1))+1;
	int XMax = (DIA_MIN(RoiCoord.XMax - dBo, Iw-1-GRAD_BO1));
	int YMin = (DIA_MAX(RoiCoord.YMin + dBo, GRAD_BO1))+1;
	int YMax = (DIA_MIN(RoiCoord.YMax - dBo, Ih-1-GRAD_BO1))-1;
	
	// Original Gaussian filtering
	for (p=0; p<Iw*Ih; p++)
		Gau[p] =  (float)Im[p];
	Err = DiaBxIsoGaussFilterFloat2D(Gau, Gau, sigma, Iw, Ih);
    if (Err == -1){
		ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaGradientComputation", DIA_ERR_MEM_ALLOC, "Allocation error in DiaBxIsoGaussFilterFloat2D");
		return DIA_ERR_MEM_ALLOC;
    }

	// Gradient itself
	memset(Grad, 0, Iw*Ih*sizeof(float));
	for (y=YMin; y<=YMax; y++)
		for (x=XMin; x<=XMax; x++){
			gradX = Gau[y*Iw+x+1] - Gau[y*Iw+x-1];
			gradY = Gau[(y+1)*Iw+x] - Gau[(y-1)*Iw+x];

			Grad[y*Iw+x] = -gradY + DIA_COEFF_GX * DIA_ABS(gradX);
		}

	return 0;
}

//************************************************************************************
// Sub-sample the original image of a factor "SSFactor"
//************************************************************************************
int DiaSubSampleImage(CDiaHdl DiaHdl, short *Ori, short *Sub, int IwOri, int IhOri, int IwSub, int IhSub, int SSFactor)
{

	if(SSFactor == 1)
	{
		memcpy(Sub, Ori, IwSub*IhSub*sizeof(short));
	}
	else
	{
		int x, y, xx, yy;
		//int i, j, i0, j0, k, l, k0, l0;

		if ( (IwSub*SSFactor > IwOri) || (IhSub*SSFactor > IhOri) ){
            char Reason[128];
			sprintf_s(Reason,128, "Incoherent arguments in DiaSubSampleImage (IwSub %d IwOri %d IhSub %d IhOri %d SSFactor %d)", IwSub, IwOri, IhSub, IhOri, SSFactor);
			ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaSubSampleImage", DIA_ERR_ARGUMENTS, Reason);
			return DIA_ERR_ARGUMENTS;
		}

		for(y=0; y<IhSub; y++) 
			for(x=0; x<IwSub; x++){

				Sub[y*IwSub+x] = Ori[y*SSFactor*IwOri+x*SSFactor];

				for(yy=y*SSFactor; yy<(y+1)*SSFactor; yy++)
					for(xx=x*SSFactor; xx<(x+1)*SSFactor; xx++)
						Sub[y*IwSub+x] = DIA_MIN(Sub[y*IwSub+x], Ori[yy*IwOri+xx]);
			}

		//for(i=0; i<IhSub; i++) {
		//	for(j=0; j<IwSub; j++)
		//	{
		//		i0 = SSFactor * i;
		//		j0 = SSFactor * j;
		//		k0 = 0;
		//		l0 = 0;
		//		for(k=0; k<SSFactor; k++) {
		//			for(l=0; l<SSFactor; l++)
		//			{
		//				if(Ori[(i0+k)*IwOri+j0+l] < Ori[(i0+k0)*IwOri+j0+l0])
		//				{
		//					k0 = k;
		//					l0 = l;
		//				}
		//			}
		//		}
		//		Sub[i*IwSub+j] = Ori[(i0+k0)*IwOri+j0+l0];
		//	}
		//}
	}

	return 0;
}

// ****************************************************************************************
// *** Histogram construction (for percentile thresholding)

int DiaHistoConstruction(CDiaHdl DiaHdl, float * Im, int Iw, int Ih, CDiaRoiCoord RoiCoord, int nbBins, long * hist, float *minHisto, float *maxHisto){
	
	int x,y;
	float step;

	memset(hist,0,nbBins*sizeof(long));

	*maxHisto = Im[RoiCoord.YMin*Iw+RoiCoord.XMin]; *minHisto = Im[RoiCoord.YMin*Iw+RoiCoord.XMin];
	for (y=RoiCoord.YMin; y<=RoiCoord.YMax; y++)
		for (x=RoiCoord.XMin; x<=RoiCoord.XMax; x++){
			if (Im[y*Iw+x] > *maxHisto)
				*maxHisto = Im[y*Iw+x];
			if (Im[y*Iw+x] < *minHisto)
				*minHisto = Im[y*Iw+x];
	}

	if (*minHisto >= *maxHisto){
        char Reason[128];
		sprintf_s(Reason,128, "DiaHistoConstruction : Attempt to compute histograms on a uniform image.");
		ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaSubSampleImage", DIA_ERR_ARGUMENTS, Reason);
		return DIA_ERR_ARGUMENTS;
	}

	step = (*maxHisto - *minHisto) / (nbBins-1);

	for (y=RoiCoord.YMin; y<=RoiCoord.YMax; y++)
		for (x=RoiCoord.XMin; x<=RoiCoord.XMax; x++)
			hist[ (int)((Im[y*Iw+x]- *minHisto) / step) ] ++;

	return 0;
}

float DiaPercentile(int Iw, int Ih, CDiaRoiCoord RoiCoord
				 , long * hist, int nbBins, float minHisto, float maxHisto, float percentile){

	int i, nb, nbLimit;

	nbLimit = (int) (percentile * (RoiCoord.XMax-RoiCoord.XMin+1)*(RoiCoord.YMax-RoiCoord.YMin+1));
	i=-1; nb = 0;
	do {
		i++;
		nb += hist[i];
	} while (nb < nbLimit);

	return (minHisto + i*(maxHisto-minHisto)/(nbBins-1));
}

/***************************************************************************/
/*         END OF FILE                                                     */
/***************************************************************************/
