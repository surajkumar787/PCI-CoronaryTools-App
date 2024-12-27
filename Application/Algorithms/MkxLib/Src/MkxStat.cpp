// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define STAT_MAX(a,b) (((a)>(b))?(a):(b))
#define STAT_MIN(a,b) (((a)<(b))?(a):(b))
#define STAT_ABS(a)   (((a)>0)?(a):(-(a)))

float MkxStatMax(int n, float*x)
{
  float theMax = x[0];
  for (int k = 0; k < n; k++)
    theMax = STAT_MAX(x[k], theMax);
  return theMax;
}
float MkxStatMin(int n, float*x)
{
  float theMin = x[0];
  for (int k = 0; k < n; k++)
    theMin = STAT_MIN(x[k], theMin);
  return theMin;
}
/*************************************************************************************/
// Compute statistics on a float variable
/*************************************************************************************/
void MkxStatComput(int N, float* X, float* pt_Mean, float* pt_Min, float* pt_Max, float* pt_Stdv)
{
	float Min, Max, Mean, Stdv, Sum, Sum2;
	int i;

	if(N <= 0)return;

	Min = Max = X[0];
	Sum = Sum2 = (float) 0;
	for(i = 0; i < N; i++)
	{
		Sum += X[i];
		Sum2 += X[i]*X[i];
		Min = STAT_MIN(Min, X[i]);
		Max = STAT_MAX(Max, X[i]);
	}
	Mean = Sum /(float) N;
	if(N==1)
		Stdv = (float)0;
	else
		Stdv = (float) sqrt(STAT_MAX(0,(double)(Sum2 / (float) N  - Mean * Mean )));

	*pt_Mean = Mean;
	*pt_Min = Min;
	*pt_Max = Max;
	*pt_Stdv = Stdv;
}

/*************************************************************************************/
// Compute median on a float variable
/*************************************************************************************/
int cmp(const void* x1, const void* x2)
{
  if(*(float*)x2>*(float*)x1)return 1;
  else if (*(float*)x2<*(float*)x1)return -1;
  else return 0;
}
//first sort data then get the middle position
int MkxMedianComput(int n, float* x, float *pMed)
{
	if(n<1)return 1;
	float* sortedX = (float*)malloc(n*sizeof(float));

	memcpy(sortedX, x, n*sizeof(float));
  qsort(sortedX,n,sizeof(float),cmp);
	//plBubbleSort_flag(sortedX, n);
	float median = n%2==1?sortedX[n/2]:(sortedX[n/2-1]+sortedX[n/2])/(float)2;

	free(sortedX);
	*pMed=median;
  return 0;
}

/*************************************************************************************/
// Print stats
/*************************************************************************************/
void MkxStatPrint(int n, float* x)
{
  float min, max, mean, stdv, median;
  MkxStatComput(n, x, &mean, &min, &max, &stdv);
  MkxMedianComput(n, x, &median);
  printf("n=%5d min=%9.3f max=%9.3f mean=%9.3f stdv=%9.3f median=%9.3f\n", n, min, max, mean, stdv, median);
}
void MkxStatPrint(int n, int* ix)
{
  float* x = (float*)malloc(n*sizeof(float));
  MkxStatPrint(n, x);
  free(x);
}

/*************************************************************************************/
// Compute histogram on a float variable
/*************************************************************************************/
void MkxHistoComput(int N, float* X, float Min, float Max, int HistoN, float *HistoX, int *HistoY, int *CumulY)
{
	int i, j;

	if(N <= 0)return;

	for(i = 0; i <= HistoN; i++)
		HistoX[i] = (float)Min + (float)i * (Max-Min) / (float) HistoN;

	for(i = 0; i <= HistoN; i++)
		HistoY[i] = 0;

	for(i = 0; i < HistoN; i++)
	{
		for(j = 0; j < N; j++)
		{
			if((X[j] >= HistoX[i]) && (X[j] < HistoX[i+1]))
				HistoY[i]++;
		}
	}
	i = HistoN;
	for(j = 0; j < N; j++)
		if(X[j] == HistoX[i])
			HistoY[i]++;

	CumulY[0] = HistoY[0];
	for(i = 1; i <= HistoN; i++)
		CumulY[i] = CumulY[i-1] + HistoY[i];
}


void MkxStatWeightedLinearFit(int n, float *x, float *y, float *w, float *aw, float *bw, float *meanw)
{
	float epsi = 0.001f;
	float sw, swx, swy, swxy, swxx;

	if (n <= 1)
	{
		*aw = 0;
		*bw = y[0];
		*meanw = y[0];
	}
	else
	{
		int i;
		float d;
		sw = swx = swy = swxy = swxx = 0;
		for (i = 0; i < n; i++)
		{
			sw += w[i];
			swx += w[i] * x[i];
			swy += w[i] * y[i];
			swxy += w[i] * x[i] * y[i];
			swxx += w[i] * x[i] * x[i];
		}
		*meanw = swy / sw;
		d = sw * swxx - swx * swx;
		if (STAT_ABS(d) > epsi)
		{
			*aw = (sw * swxy - swx * swy) / d;
			*bw = (swxx * swy - swx * swxy) / d;
		}
		else
		{
			*aw = 0;
			*bw = y[0];
		}
	}
}

