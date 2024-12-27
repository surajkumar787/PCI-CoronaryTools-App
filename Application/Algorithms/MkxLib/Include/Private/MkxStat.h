// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


//====================================================================
//Protection against multiple decalaration
#ifndef		_MKX_STAT_H
#define		_MKX_STAT_H

//====================================================================

float MkxStatMax(int n, float*x);
float MkxStatMin(int n, float*x);

void MkxStatComput(int N, float* X, float* pt_Mean, float* pt_Min, float* pt_Max, float* pt_Stdv);
void MkxHistoComput(int N, float* X, float Min, float Max, int HistoN, float *HistoX, int *HistoY, int *CumulY);
int MkxMedianComput(int N, float* X, float* Median);
void MkxHistoComput(int N, float* X, float Min, float Max, int HistoN, float *HistoX, int *HistoY, int *CumulY);
void MkxStatWeightedLinearFit(int n, float *x, float *y, float *w, float *aw, float *bw, float *meanw);

int plBubbleSort_flag(float *a, int n);
int plMedian(int n, float* x, float *pMed);
void plKLowest(int N, float *X, int K, float *KX, int Acc);
void plKHighest(int N, float *X, int K, float *KX, int Acc);

void MkxStatPrint(int n, float* x);
void MkxStatPrint(int n, int* x);
//====================================================================
// End of protection against multiple declaration
#endif   // End of include protection agains multiple declaration
