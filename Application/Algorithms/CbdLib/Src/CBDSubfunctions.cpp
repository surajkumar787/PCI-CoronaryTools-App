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

#include <windows.h> 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LocalMaxima(float * VectIn, int sizeVect, int nbSeeds, int * indicesSeeds){

	float * VectTemp = (float*)calloc(sizeVect, sizeof(float));

	// Local maxima selection
	for (int i = 1; i<sizeVect-1; i++)
		if ( (VectIn[i] >= VectIn[i+1]) && (VectIn[i] >= VectIn[i-1]) )
			VectTemp[i] = VectIn[i];

	// Best nbSeeds selected
	for (int it =0; it<nbSeeds; it++){

		float maxFound = -1; 
        int maxIndice = -1;
		for (int i=0; i<sizeVect; i++)
			if (VectTemp[i] > maxFound){
				maxFound = VectTemp[i]; maxIndice = i;
			}

		indicesSeeds[it] = maxIndice;
		VectTemp[maxIndice] = 0;
	}

	free(VectTemp);
}

float median(float * stack1, float * stack2, int x0, int pos, int xEnd)
{
	int xMinus, xPlus, i;
	float pivot;

    if (x0<0 || pos < 0 || xEnd < 0 || xEnd < x0 || xEnd < pos || pos < x0){
        return 0.f;
    }

	xMinus = x0;
	xPlus = xEnd;
	pivot = stack1[x0];

	for (i=x0+1; i<=xEnd;i++)
		if (stack1[i]<pivot){
			stack2[xMinus] = stack1[i];
			xMinus ++;
		} else if (stack1[i]>pivot) {
			stack2[xPlus] = stack1[i];
			xPlus --;
		}
	
	xMinus--; xPlus++;

	if ( (xMinus < pos) && (xPlus > pos) )
		return pivot;
	else if (xMinus >= pos)
		return median(stack2,stack1,x0,pos,xMinus);
	else 
		return median(stack2,stack1,xPlus,pos,xEnd);
}

void ComputeTrackIndicators(Lt::LtTrack * Track, float trackerSpeed, float NbStepForAngleF
							, float DThetaIrregArg0, float DThetaIrregArg1, float * Saliency
							, int Iw, int Ih, float *orientation, float *contrast, float *homogeneity){

	int lengthTrack = Track->Size();

	if (lengthTrack > 1)
		*orientation = (Track->m_Pts[lengthTrack-1].X - Track->m_Pts[0].X)
						/ (Track->m_Pts[lengthTrack-1].Y - Track->m_Pts[0].Y);
	else
		*orientation = 0;

	*contrast = 0; *homogeneity = 0;
	for (int i=0; i<lengthTrack; i++){
		*contrast += Saliency[CBD_RND(Track->m_Pts[i].Y) * Iw + CBD_RND(Track->m_Pts[i].X)];
		*homogeneity += Saliency[CBD_RND(Track->m_Pts[i].Y) * Iw + CBD_RND(Track->m_Pts[i].X)] 
							* Saliency[CBD_RND(Track->m_Pts[i].Y) * Iw + CBD_RND(Track->m_Pts[i].X)];
	}
	*contrast /= lengthTrack;		// Mean ridge
	*homogeneity = sqrt(*homogeneity / lengthTrack - *contrast * (*contrast));	// Std ridge
}

//=============================================================================================================
void CBDSubSampleImage(short *Ori, short *Sub, int IwOri, int IhOri, int IwSub, int IhSub, int SSFactor)
{

	if(SSFactor == 1)
	{
		memcpy(Sub, Ori, IwSub*IhSub*sizeof(short));
	}
	else
	{
	    int i, j, i0, j0, k, l, k0, l0;
		for(i=0; i<IhSub; i++) {
			for(j=0; j<IwSub; j++)
			{
				i0 = SSFactor * i;
				j0 = SSFactor * j;
				k0 = 0;
				l0 = 0;
				for(k=0; k<SSFactor; k++) {
					for(l=0; l<SSFactor; l++)
					{
						if(Ori[(i0+k)*IwOri+j0+l] < Ori[(i0+k0)*IwOri+j0+l0])
						{
							k0 = k;
							l0 = l;
						}
					}
				}
				Sub[i*IwSub+j] = Ori[(i0+k0)*IwOri+j0+l0];
			}
		}
	}
}


//=============================================================================================================
int CBDSysTimeMs(char *Name)
{
	static LARGE_INTEGER temps1, temps2, freq;
	static int first=1;
	static double MyFreq;
	int time_ms;

	if(first) 
	{
		QueryPerformanceFrequency(&freq);
		MyFreq = (double)(freq.QuadPart/(double)1000000);
		first=0;
	}

  if(Name==NULL)
	{
		QueryPerformanceCounter(&temps1);
		time_ms = 0;
	}
	else 
	{
	    double Current;
	    QueryPerformanceCounter(&temps2);
	 	Current = (double)((temps2.QuadPart - temps1.QuadPart)/MyFreq);
		time_ms = (int)(Current/1000.0 + 0.5);
	}

	return time_ms;
}


//=============================================================================================================
// System resolution by Gauss-Newton pivot
int CBDGaussNewton(float *A, float *B, int size)
{
	int i, j, k;
	float temp, pivot;
	int illConditioned;

	k=0; illConditioned = 0;

	while ((k< size) && (!illConditioned)){
		// Possible pivot?
		i = k;

		while ((i<size) && (A[i*size+k] == 0))
			i++;
		if (i==size)
			illConditioned = 1;

		if (!illConditioned) { // * Pivot

			// Line switch
			if (i>k) { 
				for (j=k; j<size; j++){
					temp = A[k*size+j];
					A[k*size+j] = A[i*size+j];
					A[i*size+j] = temp;
				}
				temp = B[k]; B[k] = B[i]; B[i] = temp;
			}

			// Line normalization
			pivot = A[k*size+k];
			for (j=k; j<size; j++)
				A[k*size+j] /= pivot;
			B[k] /= pivot;

			// Pivot itself
			for (i=k+1; i<size; i++){
				pivot = A[i*size+k];
				for (j=k; j<size; j++)
					A[i*size+j] -= A[k*size+j] * pivot;
				B[i] -= B[k] * pivot;
			}
			k++;
		}
	}

	if (illConditioned)
		return -1;
	else { // Pivot
		for (k=size-1; k>=0; k--)
			for (i=k-1; i>=0; i--)
				B[i] -= A[i*size+k] *B[k];

		return 0;
	}
}

//=============================================================================================================
// C_n^k = C_{n-1}^{k-1} + C_{n-1}^k
int CBD_Combination(int j, int i){
    if (j>i)
        return 0;
    else {
        if (i==j)
            return 1;
        else {
            if (j<=0)
                return 1;
            else {
                return (CBD_Combination(j-1, i-1) + CBD_Combination(j, i-1) );
            }
        }
    }
}

//=============================================================================================================
// Compute x = f(y)
float CBDComputeFromPolynom(float yIn, float *coeffsPolynom, int degreePolynom, float yNorm){

    float xOut = 0.f;
    for (int k=0; k<degreePolynom+1; k++){
        xOut += coeffsPolynom[k] * static_cast<float>(pow(yIn / yNorm, k)); 
    }
    return xOut;
}

//=============================================================================================================
// Bilinear interpolations
float CBDInterpBilinear(short *Im, int Iw, int Ih, float x, float y){

    int xI = (int)x, yI = (int)y;
    float dx = x-xI, dy = y-yI;

    xI = CBD_MIN(Iw-1, CBD_MAX(0,xI)); yI = CBD_MIN(Ih-1, CBD_MAX(0,yI)); 
    int xI1 = CBD_MIN(Iw-1, CBD_MAX(0,xI+1)), yI1 = CBD_MIN(Ih-1, CBD_MAX(0,yI+1));

    return ( (1-dy) * ( (1-dx) * Im[yI*Iw+xI] + dx * Im[yI*Iw+xI1]) +
             dy  * ( (1-dx) * Im[yI1*Iw+xI] + dx * Im[yI1*Iw+xI1]) );
}

float CBDInterpBilinearF(float *Im, int Iw, int Ih, float x, float y){

    int xI = (int)x, yI = (int)y;
    float dx = x-xI, dy = y-yI;
    
    xI = CBD_MIN(Iw-1, CBD_MAX(0,xI)); yI = CBD_MIN(Ih-1, CBD_MAX(0,yI)); 
    int xI1 = CBD_MIN(Iw-1, CBD_MAX(0,xI+1)), yI1 = CBD_MIN(Ih-1, CBD_MAX(0,yI+1));

    return ( (1-dy) * ( (1-dx) * Im[yI*Iw+xI] + dx * Im[yI*Iw+xI1]) +
             dy  * ( (1-dx) * Im[yI1*Iw+xI] + dx * Im[yI1*Iw+xI1]) );
}

//=============================================================================================================
// Derivative of the robust Tukey function
float CBDDerivativeTukey(float r, float C)
{
	if ( fabs((double)r) < C ){
        float rC = (r/C);
		return (1-rC*rC)*(1-rC*rC);
    } else
		return 0;
}

/*************************************************************************************/
int CBDMedian(CCBD * This, float * stack1, float * stack2, int x0, int pos, int xEnd, float *medOut)
{
	int xMinus, xPlus, i;
	float pivot;

    if (x0<0 || pos < 0 || xEnd < 0 || xEnd < x0 || xEnd < pos || pos < x0){
        ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDMedian", CBD_ERR_INTERNALERROR, "Incoherent indices in CBDMedian");
        return CBD_ERR_INTERNALERROR;
    }

	xMinus = x0;
	xPlus = xEnd;
	pivot = stack1[x0];

	for (i=x0+1; i<=xEnd;i++)
		if (stack1[i]<pivot){
			stack2[xMinus] = stack1[i];
			xMinus ++;
		} else if (stack1[i]>pivot) {
			stack2[xPlus] = stack1[i];
			xPlus --;
		}
	
	xMinus--; xPlus++;

	if ( (xMinus < pos) && (xPlus > pos) ){
		*medOut = pivot;
        return 0;
    } else if (xMinus >= pos)
		return CBDMedian(This, stack2,stack1,x0,pos,xMinus, medOut);
	else 
		return CBDMedian(This, stack2,stack1,xPlus,pos,xEnd, medOut);
}