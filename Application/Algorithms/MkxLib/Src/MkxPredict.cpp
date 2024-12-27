// ***************************************************************************
// * Copyright (c) 2002-2016 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <stdio.h> 
#include <stdlib.h> 
#include <math.h> 

#include <MkxDev.h>
#include <MkxStat.h>

void MkxResetHistory(CMkxHistory *history)
{
	int i;

	history->CurrIndexInStack = 0;
	for(i = 0; i < history->StackSize; i++)
	{
		MkxHistoryPlaneInit(&history->Plane[i]);
	}
}

static void  MkxGetPreviousUsefulPlane(int CurIndex, int *IndexOfPreviousUseful, int *TimeToPreviousUseful, CMkxHistory *history)
{
	int i=CurIndex, k=0;

	do
	{
		i--;
		i += history->StackSize;
		i %= history->StackSize;
		k++;
	}
	while(  history->Plane[i].PlaneValid 
		&& (history->Plane[i].HistoryStatus == 1 || history->Plane[i].Locked == 0 )
		&& k < history->StackSize);

	if(k == history->StackSize || history->Plane[i].PlaneValid == 0 )
	{
		*IndexOfPreviousUseful = -1;
	}
	else 
	{
		*IndexOfPreviousUseful = i;
		*TimeToPreviousUseful = k;
	}
}

void MkxComputeHistoryFuzzyModulation(float *p_modul, CMkxHistoryStat *historyStat, int historyDepth)
{
	float modul;
	int n = historyStat->NbUsefulPlanes;
	float t = historyStat->UsefulPlanesMeanTime;
	int nMax = 	historyDepth;
	modul = MKX_MAX(0, MKX_MIN(1, (float)n/(float)nMax));
	modul *= MKX_MIN(1, MKX_MAX(0, (1 - ((t-(n/(float)2))/(float)nMax))));
	*p_modul = modul;
}

void MkxUpdateHistoryStat(CMkxHistoryStat *historyStat, CMkxHistory *history, int historyDepth, float historyVanishingFactor)
{
	int i, j, k, n, nUseful, p;
	float meanTime = 0;
	int nupf = 0;                           //nupf for nUsefulPlaneForFeat
	int npf = 0;                            //npf for nPredictForFeat
	int *upi = (int   *)calloc(history->StackSize, sizeof(int));//upi for usefulPlaneIndex
	int *upif;                             //upif for usefulPlaneIndexForFeat
	float *x, *y, *w;
	float mean, min, max, stdv, aw, bw, median, meanw;

	if (historyDepth <= 0)
  {
    if(upi)free(upi);//PL_160308
		return;
  }

	//determine useful planes
	i = history->CurrIndexInStack;
	j = 0;
	n = 0;
	meanTime = 0;
	while(j < historyDepth && n < history->StackSize)
	{
		if(history->Plane[i].PlaneValid && history->Plane[i].HistoryStatus == MKX_CST_EXTRACT_RES_OK && history->Plane[i].Locked == MKX_CST_EXTRACT_RES_LOCKED)
		{
			upi[j++] = i;
			meanTime += n;
		}
		i = (i - 1 + history->StackSize) % history->StackSize;
		n++;
	}
	nUseful = j;
	if(nUseful > 0) meanTime /= nUseful;
	else meanTime = 0;

	historyStat->UsefulPlanesMeanTime = meanTime;
	historyStat->NbUsefulPlanes = nUseful;

	upif = (int *)calloc(nUseful, sizeof(int));
	if(nUseful <= 0)
	{
		MkxHistoryStatInit(historyStat);
		historyStat->NbUsefulPlanes = nUseful; //put 0 instead of -1

	}
	else
	{
		for(p=0; p<MKX_CPL_N_FEATURES; p++) //loop on feature
		{
			for (nupf = 0, j = 0; j < nUseful; j++)
			{
				if (history->Plane[upi[j]].CplFeat.Feat_available[p] == 1)
				{
					upif[nupf++] = upi[j];
				}
			}
			if (nupf <= 0)
			{
				historyStat->FeatStat[p].N = 0;
				historyStat->DevStat[p].N = 0;
			}
			else
			{
				int *u;
				float t_distal, t_proximal;
				float t_cur = (float) history->Plane[history->CurrIndexInStack].Time;

				//build statistics for feature
				x = (float *)calloc(nupf, sizeof(float)); 
				y = (float *)calloc(nupf, sizeof(float)); 
				w = (float *)calloc(nupf, sizeof(float)); 
				for (k = 0; k < nupf; k++)
				{
					x[k] = (float)history->Plane[upif[k]].Time;
					y[k] = history->Plane[upif[k]].CplFeat.Feat[p];
				}
				t_distal = x[0]; //init
				t_proximal = x[0]; //init
				for (k = 0; k < nupf; k++)
				{
					t_distal = MKX_MIN(t_distal, x[k]);
					t_proximal = MKX_MAX(t_proximal, x[k]);
				}
				//t0 is the oldest time, t the current time
				for (k = 0; k < nupf; k++)
				{
					if ((int)MKX_RND(t_cur - t_distal) == 0)
					{
						w[k] = 1;
					}
					else
					{
						w[k] = (1 + x[k] - t_distal) / (float)(1 + t_cur - t_distal); // +1 for the smallest coef to be not 0
						w[k] *= w[k];  // to the square to enhance the impact of the most recent samples
					}
				}
				MkxStatWeightedLinearFit(nupf, x, y, w, &aw, &bw, &meanw); 
				historyStat->FeatStat[p].Aw = aw;
				historyStat->FeatStat[p].Bw = bw;
				historyStat->FeatStat[p].Meanw = meanw;
				historyStat->FeatStat[p].Tdistal = t_distal;
				historyStat->FeatStat[p].Tproximal = t_proximal;
				//historyStat->FeatStat[p] = new StatClass(x, y, w);
				MkxStatComput(nupf, y, &mean, &min, &max, &stdv);
				historyStat->FeatStat[p].N    = k;
				historyStat->FeatStat[p].Mean = mean;
				historyStat->FeatStat[p].Min  = min;
				historyStat->FeatStat[p].Max  = max;
				historyStat->FeatStat[p].Stdv = stdv;
				MkxMedianComput(nupf, y, &median);
				historyStat->FeatStat[p].Median = median;

				free(x); free(y); free(w);

				//build statistics of deviations for feature
				u = (int *)calloc(nupf, sizeof(int)); //plane index with predict for feat
				for (npf = 0, j = 1; j < nupf; j++)
				{
					if (history->Plane[upif[j]].HistoryPredict.FeatPredict[p].PredictVal_available == 1)
					{
						u[npf++] = upif[j];
					}
				}
				if (npf <= 0)
				{
					historyStat->DevStat[p].N = 0;
				}
				else
				{
					x = (float *)calloc(npf+1, sizeof(float)); 
					y = (float *)calloc(npf+1, sizeof(float)); 
					w = (float *)calloc(npf+1, sizeof(float)); 

					x[0] = (float) history->Plane[upif[0]].Time;
					y[0] = history->Plane[upif[0]].CplFeat.Feat[p] - history->Plane[u[0]].HistoryPredict.FeatPredict[p].PredictVal;
					y[0] *= y[0];

					for (k = 1; k < npf; k++)
					{
						x[k] = (float) history->Plane[u[k]].Time;
						y[k] = history->Plane[u[k-1]].CplFeat.Feat[p] - history->Plane[u[k]].HistoryPredict.FeatPredict[p].PredictVal;
						y[k] *= y[k];
					}
					t_distal = x[0]; //init
					t_proximal = x[0]; //init
					for (k = 0; k < npf; k++)
					{
						t_distal = MKX_MIN(t_distal, x[k]);
						t_proximal = MKX_MAX(t_proximal, x[k]);
					}
					for (k = 0; k < npf; k++)
					{
						if ((int)MKX_RND(t_cur - t_distal) == 0)
						{
							w[k] = 1;
						}
						else
						{
							w[k] = (1 + x[k] - t_distal) / (float)(1 + t_cur - t_distal); // +1 for the smallest coef to be not 0
						}
					}
					MkxStatWeightedLinearFit(npf, x, y, w, &aw, &bw, &meanw); 
					historyStat->DevStat[p].Aw = aw;
					historyStat->DevStat[p].Bw = bw;
					historyStat->DevStat[p].Meanw = meanw;
					historyStat->DevStat[p].Tdistal = t_distal;
					historyStat->DevStat[p].Tproximal = t_proximal;
					//DevStat[p] = new StatClass(x, y, w);

					MkxStatComput(npf, y, &mean, &min, &max, &stdv);
					historyStat->DevStat[p].N    = npf;
					historyStat->DevStat[p].Mean = mean;
					historyStat->DevStat[p].Min  = min;
					historyStat->DevStat[p].Max  = max;
					historyStat->DevStat[p].Stdv = stdv;
					MkxMedianComput(npf, y, &median);
					historyStat->DevStat[p].Median = median;

					free(x); free(y); free(w);
				}
				free(u);
			}
		}
	}
	free(upi); free(upif);
	{
		float modul;
		int prevIndex = (history->CurrIndexInStack + history->StackSize -1)%history->StackSize;
		float modulTm1 = history->Plane[prevIndex].HistoryStatShallow.ModulationFactor;

		MkxComputeHistoryFuzzyModulation(&modul, historyStat, historyDepth);
		if(modul>=modulTm1)
		{
			historyStat->ModulationFactor = modul;
		}
		else
		{
			historyStat->ModulationFactor = MKX_MAX(0, modulTm1 - (historyVanishingFactor*historyVanishingFactor) / historyDepth);
		}
	}

}

void MkxGetFuzzyPara(float *low, float * high, int feat, CPrmPara *ep, CMkxProcessPara *pp)
{
	switch(feat)
	{
	case MKX_CPL_Cx:
		*low = pp->HistoryCentroidXLowInPix;
		*high = pp->HistoryCentroidXHighInPix;
		break;
	case MKX_CPL_Cy:
		*low = pp->HistoryCentroidYLowInPix;
		*high = pp->HistoryCentroidYHighInPix;
		break;
	case MKX_CPL_Br:
		*low = (float) ep->Val[MKX_HistoryBreadthLow_F].Float;
		*high = (float) ep->Val[MKX_HistoryBreadthHigh_F].Float;
		break;
	case MKX_CPL_An:
		*low = (float) ep->Val[MKX_HistoryAngleLow_F].Float;
		*high = (float) ep->Val[MKX_HistoryAngleHigh_F].Float;
		break;
	}
}

static void MkxUpdatePredict(CMkxPredict *predict, CMkxHistory *history, CPrmPara *extractParam, CMkxProcessPara *processPara)
{
	float coefFuzzy = 1 + 2 * (1-extractParam->Val[MKX_HISTORYFACTOR_F].Float); //PL_080725
	float low = 0.f, high = 0.f;
	float offsetFuzzy = 0.1f;
	int nbFeatures = MKX_CPL_N_FEATURES;
	CMkxHistoryStat *hs = &history->Plane[history->CurrIndexInStack].HistoryStatShallow;
	int nbUsefulPlanes = hs->NbUsefulPlanes;
	float modul = hs->ModulationFactor;

	if (nbUsefulPlanes <= 0)
	{
		predict->PredictValid = 0;
	}
	else
	{
		for (int i = 0; i < nbFeatures; i++)
		{
			if (hs->FeatStat[i].N <= 0 || modul < 0.001) //if modul 0 no history
			{
				predict->FeatPredict[i].PredictVal_available = 0;
				predict->FeatPredict_available[i] = 0;
			}
			else
			{
				float predictVal = hs->FeatStat[i].Aw * hs->FeatStat[i].Tproximal + hs->FeatStat[i].Bw;
				float predictStdv = 0;
				if (hs->DevStat[i].N > 0)
				{
					//stdv as mean of squares of deviations
					predictStdv = (float) sqrt((double) MKX_MAX(0, hs->DevStat[i].Meanw));
				}
				MkxGetFuzzyPara(&low, &high,  i, extractParam, processPara);

				predict->FeatPredict[i].PredictVal = predictVal;
				predict->FeatPredict[i].FuzzyLow = offsetFuzzy * low + (1 - modul) * low + modul * coefFuzzy * predictStdv;
				predict->FeatPredict[i].FuzzyHigh = offsetFuzzy * high + (1 - modul) * high + modul *  2 * coefFuzzy * predictStdv;
				predict->FeatPredict[i].PredictVal_available = 1;
				predict->FeatPredict_available[i] = 1;
			}
		}
		predict->PredictValid = 1;
	}
}

void MkxUpdateCbaFromXyOne(float x1, float y1, float x2, float y2, float* cx, float* cy, float* br, float* an )
{
	*cx = (x1 + x2) / (float)2;
	*cy = (y1 + y2) / (float)2;
	*br = (float) sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	*an = (float)((180 / MKX_PI) * atan2((y2 - y1), (x2 - x1)));
}
void MkxUpdateXyFromCbaOne(float cx, float cy, float br, float an, float* x1, float* y1, float* x2, float* y2)
{
	float angRd, dx, dy;

	angRd = (float)(MKX_PI * an / (float)180);
	dx = (float)(br * cos(angRd) / (float)2);
	dy = (float)(br * sin(angRd) / (float)2);
	*x1 = cx + dx;
	*y1 = cy + dy;
	*x2 = cx - dx;
	*y2 = cy - dy;
}

void MkxPrintHistory(CMkxHistory *history, int PrintDepth)
{
	CMkxHistoryPlane *plane;
	CMkxCplFeatures *cf;
	int i, n, N;

	if(PrintDepth == 0) N = history->StackSize;
	else                N = MKX_MIN(PrintDepth, history->StackSize);

	i = history->CurrIndexInStack; 

	printf("MkxPrintHistory\n\n");
	printf("                                                     M0                   |               M1                    |                M2\n");
	printf("  n   i Time Valid Status Lock              Cx       Cy  Breadth      Ang |      Cx       Cy   Breadth      Ang |       Cx       Cy  Breadth      Ang\n");

	n = 0;
	while(n < N)
	{
		plane = &history->Plane[i];
		cf = &plane->CplFeat;
		printf("%3d %3d  %3d  %3d  %3d  %3d           %8.2f %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f %8.2f\n",
			n, i, plane->Time, plane->PlaneValid, plane->HistoryStatus, plane->Locked, 
			plane->Cx, plane->Cy, plane->Breadth, plane->Ang,
			cf->Feat[MKX_CPL_Cx_Speed], cf->Feat[MKX_CPL_Cy_Speed], cf->Feat[MKX_CPL_Br_Speed], cf->Feat[MKX_CPL_An_Speed],
			cf->Feat[MKX_CPL_Cx_Accel], cf->Feat[MKX_CPL_Cy_Accel], cf->Feat[MKX_CPL_Br_Accel], cf->Feat[MKX_CPL_An_Accel]
		);
		n++;
		i--;
		i += history->StackSize;
		i %= history->StackSize;
	}
}


void MkxPrintPredict(CMkxPredict *Predict)
{
	printf("MkxPrintPredict------------\n");

	if(!Predict->PredictValid)
		return;

	printf("Predict  :           M0                   |                M1                   |                M2\n");
	printf("            Cx       Cy  Breadth      Ang |       Cx       Cy  Breadth      Ang |       Cx       Cy  Breadth      Ang\n");
	printf("Val:  %8.2f %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f %8.2f\n", 
		Predict->FeatPredict[MKX_CPL_Cx].PredictVal, Predict->FeatPredict[MKX_CPL_Cy].PredictVal, Predict->FeatPredict[MKX_CPL_Br].PredictVal, Predict->FeatPredict[MKX_CPL_An].PredictVal,
		Predict->FeatPredict[MKX_CPL_Cx_Speed].PredictVal, Predict->FeatPredict[MKX_CPL_Cy_Speed].PredictVal, Predict->FeatPredict[MKX_CPL_Br_Speed].PredictVal, Predict->FeatPredict[MKX_CPL_An_Speed].PredictVal,
		Predict->FeatPredict[MKX_CPL_Cx_Accel].PredictVal, Predict->FeatPredict[MKX_CPL_Cy_Accel].PredictVal, Predict->FeatPredict[MKX_CPL_Br_Accel].PredictVal, Predict->FeatPredict[MKX_CPL_An_Accel].PredictVal);
	printf("Low:  %8.2f %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f %8.2f\n",
		Predict->FeatPredict[MKX_CPL_Cx].FuzzyLow, Predict->FeatPredict[MKX_CPL_Cy].FuzzyLow, Predict->FeatPredict[MKX_CPL_Br].FuzzyLow, Predict->FeatPredict[MKX_CPL_An].FuzzyLow,
		Predict->FeatPredict[MKX_CPL_Cx_Speed].FuzzyLow, Predict->FeatPredict[MKX_CPL_Cy_Speed].FuzzyLow, Predict->FeatPredict[MKX_CPL_Br_Speed].FuzzyLow, Predict->FeatPredict[MKX_CPL_An_Speed].FuzzyLow,
		Predict->FeatPredict[MKX_CPL_Cx_Accel].FuzzyLow, Predict->FeatPredict[MKX_CPL_Cy_Accel].FuzzyLow, Predict->FeatPredict[MKX_CPL_Br_Accel].FuzzyLow, Predict->FeatPredict[MKX_CPL_An_Accel].FuzzyLow);
	printf("High: %8.2f %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f %8.2f\n",
		Predict->FeatPredict[MKX_CPL_Cx].FuzzyHigh, Predict->FeatPredict[MKX_CPL_Cy].FuzzyHigh, Predict->FeatPredict[MKX_CPL_Br].FuzzyHigh, Predict->FeatPredict[MKX_CPL_An].FuzzyHigh,
		Predict->FeatPredict[MKX_CPL_Cx_Speed].FuzzyHigh, Predict->FeatPredict[MKX_CPL_Cy_Speed].FuzzyHigh, Predict->FeatPredict[MKX_CPL_Br_Speed].FuzzyHigh, Predict->FeatPredict[MKX_CPL_An_Speed].FuzzyHigh,
		Predict->FeatPredict[MKX_CPL_Cx_Accel].FuzzyHigh, Predict->FeatPredict[MKX_CPL_Cy_Accel].FuzzyHigh, Predict->FeatPredict[MKX_CPL_Br_Accel].FuzzyHigh, Predict->FeatPredict[MKX_CPL_An_Accel].FuzzyHigh);
}

void MkxPrintHistoryStat(CMkxHistoryStat *historyStat)
{
	printf("MkxPrintHistoryStat------------\n");
	printf("NbUsefulPlanes: %d\n", historyStat->NbUsefulPlanes);
	printf("UsefulPlanesMeanTime: %8.2f\n", historyStat->UsefulPlanesMeanTime);
	printf("ModulationFactor: %8.2f\n", historyStat->ModulationFactor);
	printf("Feature   N     stdv     mean     min      max    median\n");
	printf("Cx       %3d %8.2f %8.2f %8.2f %8.2f %8.2f\n", historyStat->FeatStat[MKX_CPL_Cx].N, historyStat->FeatStat[MKX_CPL_Cx].Stdv, historyStat->FeatStat[MKX_CPL_Cx].Mean, historyStat->FeatStat[MKX_CPL_Cx].Min, historyStat->FeatStat[MKX_CPL_Cx].Max, historyStat->FeatStat[MKX_CPL_Cx].Median);
	printf("Cy       %3d %8.2f %8.2f %8.2f %8.2f %8.2f\n", historyStat->FeatStat[MKX_CPL_Cy].N, historyStat->FeatStat[MKX_CPL_Cy].Stdv, historyStat->FeatStat[MKX_CPL_Cy].Mean, historyStat->FeatStat[MKX_CPL_Cy].Min, historyStat->FeatStat[MKX_CPL_Cy].Max, historyStat->FeatStat[MKX_CPL_Cy].Median);
	printf("Br       %3d %8.2f %8.2f %8.2f %8.2f %8.2f\n", historyStat->FeatStat[MKX_CPL_Br].N, historyStat->FeatStat[MKX_CPL_Br].Stdv, historyStat->FeatStat[MKX_CPL_Br].Mean, historyStat->FeatStat[MKX_CPL_Br].Min, historyStat->FeatStat[MKX_CPL_Br].Max, historyStat->FeatStat[MKX_CPL_Br].Median);
	printf("An       %3d %8.2f %8.2f %8.2f %8.2f %8.2f\n", historyStat->FeatStat[MKX_CPL_An].N, historyStat->FeatStat[MKX_CPL_An].Stdv, historyStat->FeatStat[MKX_CPL_An].Mean, historyStat->FeatStat[MKX_CPL_An].Min, historyStat->FeatStat[MKX_CPL_An].Max, historyStat->FeatStat[MKX_CPL_An].Median);
	printf("CxSpeed  %3d %8.2f %8.2f %8.2f %8.2f %8.2f\n", historyStat->FeatStat[MKX_CPL_Cx_Speed].N, historyStat->FeatStat[MKX_CPL_Cx_Speed].Stdv, historyStat->FeatStat[MKX_CPL_Cx_Speed].Mean, historyStat->FeatStat[MKX_CPL_Cx_Speed].Min, historyStat->FeatStat[MKX_CPL_Cx_Speed].Max, historyStat->FeatStat[MKX_CPL_Cx_Speed].Median);
	printf("CySpeed  %3d %8.2f %8.2f %8.2f %8.2f %8.2f\n", historyStat->FeatStat[MKX_CPL_Cy_Speed].N, historyStat->FeatStat[MKX_CPL_Cy_Speed].Stdv, historyStat->FeatStat[MKX_CPL_Cy_Speed].Mean, historyStat->FeatStat[MKX_CPL_Cy_Speed].Min, historyStat->FeatStat[MKX_CPL_Cy_Speed].Max, historyStat->FeatStat[MKX_CPL_Cy_Speed].Median);
	printf("BrSpeed  %3d %8.2f %8.2f %8.2f %8.2f %8.2f\n", historyStat->FeatStat[MKX_CPL_Br_Speed].N, historyStat->FeatStat[MKX_CPL_Br_Speed].Stdv, historyStat->FeatStat[MKX_CPL_Br_Speed].Mean, historyStat->FeatStat[MKX_CPL_Br_Speed].Min, historyStat->FeatStat[MKX_CPL_Br_Speed].Max, historyStat->FeatStat[MKX_CPL_Br_Speed].Median);
	printf("AnSpeed  %3d %8.2f %8.2f %8.2f %8.2f %8.2f\n", historyStat->FeatStat[MKX_CPL_An_Speed].N, historyStat->FeatStat[MKX_CPL_An_Speed].Stdv, historyStat->FeatStat[MKX_CPL_An_Speed].Mean, historyStat->FeatStat[MKX_CPL_An_Speed].Min, historyStat->FeatStat[MKX_CPL_An_Speed].Max, historyStat->FeatStat[MKX_CPL_An_Speed].Median);
}

void MkxUpdateCplFeatures(CMkxHistoryPlane* plane)
{
	plane->CplFeat.Feat[MKX_CPL_Cx] = plane->Cx;
	plane->CplFeat.Feat[MKX_CPL_Cy] = plane->Cy;
	plane->CplFeat.Feat[MKX_CPL_Br] = plane->Breadth;
	plane->CplFeat.Feat[MKX_CPL_An] = plane->Ang;
	plane->CplFeat.Feat_available[MKX_CPL_Cx] = 1;
	plane->CplFeat.Feat_available[MKX_CPL_Cy] = 1;
	plane->CplFeat.Feat_available[MKX_CPL_Br] = 1;
	plane->CplFeat.Feat_available[MKX_CPL_An] = 1;
}
void MkxUpdateCplFeaturesLengthAndThicknessForDebug(CMkxHistoryPlane* plane)
{
	plane->CplFeat.Feat[MKX_CPL_A1] = plane->a1;
	plane->CplFeat.Feat[MKX_CPL_B1] = plane->b1;
	plane->CplFeat.Feat[MKX_CPL_A2] = plane->a2;
	plane->CplFeat.Feat[MKX_CPL_B2] = plane->b2;
	plane->CplFeat.Feat_available[MKX_CPL_A1] = 1;
	plane->CplFeat.Feat_available[MKX_CPL_B1] = 1;
	plane->CplFeat.Feat_available[MKX_CPL_A2] = 1;
	plane->CplFeat.Feat_available[MKX_CPL_B2] = 1;
}

void MkxComputeDifferentialFeatures(CMkxCplFeatures* cur, CMkxCplFeatures* prev, int timeToPreviousUsefulPlane)
{

	//speed
	if(prev->Feat_available[MKX_CPL_Cx] == 1)
	{
		cur->Feat[MKX_CPL_Cx_Speed]= (cur->Feat[MKX_CPL_Cx]-prev->Feat[MKX_CPL_Cx])/timeToPreviousUsefulPlane;
		cur->Feat_available[MKX_CPL_Cx_Speed] = 1;
	}
	if(prev->Feat_available[MKX_CPL_Cy] == 1)
	{
		cur->Feat[MKX_CPL_Cy_Speed]= (cur->Feat[MKX_CPL_Cy]-prev->Feat[MKX_CPL_Cy])/timeToPreviousUsefulPlane;
		cur->Feat_available[MKX_CPL_Cy_Speed] = 1;
	}
	if(prev->Feat_available[MKX_CPL_Br] == 1)
	{
		cur->Feat[MKX_CPL_Br_Speed]= (cur->Feat[MKX_CPL_Br]-prev->Feat[MKX_CPL_Br])/timeToPreviousUsefulPlane;
		cur->Feat_available[MKX_CPL_Br_Speed] = 1;
	}
	if(prev->Feat_available[MKX_CPL_An] == 1)
	{
		cur->Feat[MKX_CPL_An_Speed]= (cur->Feat[MKX_CPL_An]-prev->Feat[MKX_CPL_An])/timeToPreviousUsefulPlane;
		cur->Feat_available[MKX_CPL_An_Speed] = 1;
	}

	//acceleration
	if(prev->Feat_available[MKX_CPL_Cx_Speed] == 1)
	{
		cur->Feat[MKX_CPL_Cx_Accel]= (cur->Feat[MKX_CPL_Cx_Speed]-prev->Feat[MKX_CPL_Cx_Speed])/timeToPreviousUsefulPlane;
		cur->Feat_available[MKX_CPL_Cx_Accel] = 1;
	}
	if(prev->Feat_available[MKX_CPL_Cy_Speed] == 1)
	{
		cur->Feat[MKX_CPL_Cy_Accel]= (cur->Feat[MKX_CPL_Cy_Speed]-prev->Feat[MKX_CPL_Cy_Speed])/timeToPreviousUsefulPlane;
		cur->Feat_available[MKX_CPL_Cy_Accel] = 1;
	}
	if(prev->Feat_available[MKX_CPL_Br_Speed] == 1)
	{
		cur->Feat[MKX_CPL_Br_Accel]= (cur->Feat[MKX_CPL_Br_Speed]-prev->Feat[MKX_CPL_Br_Speed])/timeToPreviousUsefulPlane;
		cur->Feat_available[MKX_CPL_Br_Accel] = 1;
	}
	if(prev->Feat_available[MKX_CPL_An_Speed] == 1)
	{
		cur->Feat[MKX_CPL_An_Accel]= (cur->Feat[MKX_CPL_An_Speed]-prev->Feat[MKX_CPL_An_Speed])/timeToPreviousUsefulPlane;
		cur->Feat_available[MKX_CPL_An_Accel] = 1;
	}
}



void MkxUpdatePlane(CMkxHistoryPlane* plane, int t, CMkxExtractResults *exr, CMkxAdvancedResults *adv, int sSFactor)
{
	plane->HistoryStatus = exr->Status;
	plane->Locked = exr->Locked;
	plane->Time = t;

	if(exr->Status == MKX_CST_EXTRACT_RES_OK)
	{
		plane->X1 = exr->X1Refined / sSFactor;
		plane->Y1 = exr->Y1Refined / sSFactor;
		plane->X2 = exr->X2Refined / sSFactor;
		plane->Y2 = exr->Y2Refined / sSFactor;
		plane->a1 = adv->Length1;
		plane->b1 = adv->Thickness1;
		plane->a2 = adv->Length2;
		plane->b2 = adv->Thickness2;

		MkxUpdateCbaFromXyOne(plane->X1, plane->Y1, plane->X2, plane->Y2, &plane->Cx, &plane->Cy, &plane->Breadth, &plane->Ang);
	}
	else
	{
		plane->X1 = -1;
		plane->Y1 = -1;
		plane->X2 = -1;
		plane->Y2 = -1;
		plane->a1 = -1;
		plane->b1 = -1;
		plane->a2 = -1;
		plane->b2 = -1;
		plane->Cx = -1;
		plane->Cy = -1;
		plane->Breadth = -1;
		plane->Ang = -1;
	}
	plane->PlaneValid = 1;
	plane->PlaneUseful = (plane->Locked == 1 && plane->HistoryStatus == MKX_CST_EXTRACT_RES_OK) ? 1 : 0;

}

void MkxAdjustAngleWithPredict(float *ang, float predictAng)
{
	int k, kmin = 0;
	float ang0 = *ang;
	float diff = MKX_ABS(ang0 - predictAng);
	for(k = -2; k <= 2; k++)
	{
		if(MKX_ABS(ang0 + k * 180 -predictAng) < diff)
		{
			diff = MKX_ABS(ang0 + k * 180 -predictAng);
			kmin = k;
		}
	}
	*ang = ang0 + kmin * 180;
}

void MkxUpdateHistory(CMkxHistory *history, int t, CMkxExtractResults *extractResults, CMkxAdvancedResults *advancedResults, int sSFactor, CPrmPara *extractParam, CMkxProcessPara *processPara)
{
	int curIndex, prevIndex;
	int timeToPreviousUsefulPlane;
	CMkxHistoryPlane *plane, *prevPlane;

	curIndex = history->CurrIndexInStack;
	plane = &(history->Plane[curIndex]);

	MkxUpdatePlane(plane, t, extractResults, advancedResults, sSFactor);

	prevIndex = (curIndex + history->StackSize -1)%history->StackSize;

	if (history->Plane[prevIndex].HistoryPredict.PredictValid == 1)
		MkxAdjustAngleWithPredict(&plane->Ang, history->Plane[prevIndex].HistoryPredict.FeatPredict[MKX_CPL_An].PredictVal);

	MkxUpdateCplFeatures(plane);
	MkxUpdateCplFeaturesLengthAndThicknessForDebug(plane);//PL_110829 only for info in a debug client

	MkxGetPreviousUsefulPlane(curIndex, &prevIndex, &timeToPreviousUsefulPlane, history);
	if(prevIndex != -1)
	{
		prevPlane = &(history->Plane[prevIndex]);
		if(prevPlane->PlaneValid)
		{
			MkxComputeDifferentialFeatures(&plane->CplFeat, &prevPlane->CplFeat, timeToPreviousUsefulPlane);
		}
	}
	MkxUpdateHistoryStat(&plane->HistoryStatShallow, history, extractParam->Val[MKX_HISTORYDEPTH_I].Int, extractParam->Val[MKX_HISTORYVANISHINGFACTOR_F].Float);
	MkxUpdatePredict(&plane->HistoryPredict, history, extractParam,processPara);//PL_110222
}

static int MkxFuzzy(float *y, float x, float x1, float x2)
{
	if(x < 0 || x1 < 0 || x2<0 || x1 >= x2)
	{
		*y = x;
		return 1;
	}
	if(x < x1) *y = 1;
	else if(x > x2) *y = 0;
	else *y = (-x + x2)/(x2-x1);

	return 0;
}

static int MkxComputeIntensityWrtHistoryPredict(float *measCx, float *measCy, float *measBreadth, float *measAng, 
								  CMkxPredict *p, float cx, float cy, float breadth, float ang, float breadthThresh)
{
	float x, x1, x2, y;
	int i, Err=0;

	if(p->FeatPredict[MKX_CPL_Cx].PredictVal_available == 0)
	{
		*measCx = 1;
	}
	else
	{
		x  = MKX_ABS(cx-p->FeatPredict[MKX_CPL_Cx].PredictVal);
		x1 = p->FeatPredict[MKX_CPL_Cx].FuzzyLow;
		x2 = p->FeatPredict[MKX_CPL_Cx].FuzzyHigh;
		Err |= MkxFuzzy(&y, x, x1, x2);
		*measCx = y;
	}

	if(p->FeatPredict[MKX_CPL_Cy].PredictVal_available == 0)
	{
		*measCy = 1;
	}
	else
	{
		x  = MKX_ABS(cy-p->FeatPredict[MKX_CPL_Cy].PredictVal);
		x1 = p->FeatPredict[MKX_CPL_Cy].FuzzyLow;
		x2 = p->FeatPredict[MKX_CPL_Cy].FuzzyHigh;
		Err |= MkxFuzzy(&y, x, x1, x2);
		*measCy = y;
	}

	if(p->FeatPredict[MKX_CPL_Br].PredictVal_available == 0)
	{
		*measBreadth = 1;
	}
	else
	{
		x  = MKX_ABS(breadth-p->FeatPredict[MKX_CPL_Br].PredictVal);
		x1 = p->FeatPredict[MKX_CPL_Br].FuzzyLow;
		x2 = p->FeatPredict[MKX_CPL_Br].FuzzyHigh;
		Err |= MkxFuzzy(&y, x, x1, x2);
		*measBreadth = y;
	}

	if(p->FeatPredict[MKX_CPL_Br].PredictVal_available == 0 || p->FeatPredict[MKX_CPL_An].PredictVal_available == 0 )
	{
		*measAng = 1;  // do not take angle into account in this case
	}
	else
	{
		if(p->FeatPredict[MKX_CPL_Br].PredictVal < breadthThresh)
		{
			*measAng = 1;  // do not take angle into account in this case
		}
		else
		{
			x  = MKX_ABS(ang-p->FeatPredict[MKX_CPL_An].PredictVal);
			for(i = -2; i<= 2; i++)
				x = MKX_MIN(x, MKX_ABS(ang-p->FeatPredict[MKX_CPL_An].PredictVal + i* 180));
			x1 = p->FeatPredict[MKX_CPL_An].FuzzyLow;
			x2 = p->FeatPredict[MKX_CPL_An].FuzzyHigh;
			Err |= MkxFuzzy(&y, x, x1, x2);
			*measAng = y;
		}
	}
	return Err;
}

static int MkxComputeIntensityWrtHistoryStat(float *measCx, float *measCy, float *measBreadth, float *measAng, 
								  CMkxHistoryStat *historyStat, float cx, float cy, float breadth, float ang, float breadthThresh, float historyFactor)
{
	float x, x1, x2, y;
	int i, Err=0;
	float coefFuzzy = 1+2*(1-historyFactor);
	float lowCoef = 1 * coefFuzzy;
	float highCoef = 2 * coefFuzzy;

	if(historyStat->FeatStat[MKX_CPL_Cx].N < 0)
	{
		*measCx = 1;
	}
	else
	{
		float mean = historyStat->FeatStat[MKX_CPL_Cx].Mean, stdv = historyStat->FeatStat[MKX_CPL_Cx].Stdv;
		x  = MKX_ABS(cx-mean);
		x1 = lowCoef * stdv;
		x2 = highCoef * stdv;
		Err |= MkxFuzzy(&y, x, x1, x2);
		*measCx = y;
	}
	if(historyStat->FeatStat[MKX_CPL_Cy].N < 0)
	{
		*measCy = 1;
	}
	else
	{
		float mean = historyStat->FeatStat[MKX_CPL_Cy].Mean, stdv = historyStat->FeatStat[MKX_CPL_Cy].Stdv;
		x  = MKX_ABS(cy-mean);
		x1 = lowCoef * stdv;
		x2 = highCoef * stdv;
		Err |= MkxFuzzy(&y, x, x1, x2);
		*measCy = y;
	}
	if(historyStat->FeatStat[MKX_CPL_Br].N < 0)
	{
		*measBreadth = 1;
	}
	else
	{
		float mean = historyStat->FeatStat[MKX_CPL_Br].Mean, stdv = historyStat->FeatStat[MKX_CPL_Br].Stdv;
		x  = MKX_ABS(breadth-mean);
		x1 = lowCoef * stdv;
		x2 = highCoef * stdv;
		Err |= MkxFuzzy(&y, x, x1, x2);
		*measBreadth = y;
	}

	if(historyStat->FeatStat[MKX_CPL_Br].N < 0 || historyStat->FeatStat[MKX_CPL_An].N < 0 )
	{
		*measAng = 1;  // do not take angle into account in this case
	}
	else
	{
		if(historyStat->FeatStat[MKX_CPL_Br].Mean < breadthThresh)
		{
			*measAng = 1;  // do not take angle into account in this case
		}
		else
		{
			float mean = historyStat->FeatStat[MKX_CPL_An].Mean, stdv = historyStat->FeatStat[MKX_CPL_An].Stdv;

			x  = MKX_ABS(ang-mean);
			for(i = -2; i<= 2; i++)
				x = MKX_MIN(x, MKX_ABS(ang-mean + i* 180));
			x1 = lowCoef * stdv;
			x2 = highCoef * stdv;
			Err |= MkxFuzzy(&y, x, x1, x2);
			*measAng = y;
		}
	}
	return Err;
}

int MkxCouplesSetIntensityWrtHistory(
  int nbCouples,
  CMkxBlobCouple *couples,
  CMkxHistory *history,
  float historyFactor,  int historyMode
  )
{
#define BREADTHTHRESH 30 //value in pixels below which angle component is not taken into account (not reliable enough)

	float breadthThresh = (float)BREADTHTHRESH;
	int i, Err = 0;
	float cx, cy, br, an;
	float x1, y1, x2, y2;
	float measCx, measCy, measBreadth, measAng;

	int curIndex = history->CurrIndexInStack-1;
	curIndex += history->StackSize;
	curIndex %= history->StackSize;

	CMkxHistoryPlane *curPlane = &history->Plane[curIndex];

	for(i=0;i<nbCouples;i++)
		couples[i].CplMerit.CplMeritWrtHistoryAvailable = 0;

	if(curPlane->PlaneValid == 0 || curPlane->HistoryStatShallow.NbUsefulPlanes < 1 || curPlane->HistoryStatShallow.FeatStat[MKX_CPL_Cx].N < 1)
		return Err;

	if(curPlane->HistoryStatShallow.ModulationFactor > 0.01)//PL_080716 to be checked
	{
		for(i=0;i<nbCouples;i++)
		{
			x1 = (float)couples[i].Blob1.V.X;
			y1 = (float)couples[i].Blob1.V.Y;
			x2 = (float)couples[i].Blob2.V.X;
			y2 = (float)couples[i].Blob2.V.Y;

      MkxUpdateCbaFromXyOne(x1, y1, x2, y2, &cx, &cy, &br, &an);

      if (historyMode == MKX_CST_HISTORY_APPLYANDUPDATE)
      {
        Err |= MkxComputeIntensityWrtHistoryPredict(&measCx, &measCy, &measBreadth, &measAng,
          &curPlane->HistoryPredict, cx, cy, br, an, breadthThresh);
      }
      else if (historyMode == MKX_CST_HISTORY_APPLYONLY)
      {
        Err |= MkxComputeIntensityWrtHistoryStat(&measCx, &measCy, &measBreadth, &measAng,
          &curPlane->HistoryStatShallow, cx, cy, br, an, breadthThresh, historyFactor);
      }

      couples[i].CplMerit.CplMeritWrtHistoryAvailable = 1;
			couples[i].CplMerit.CplMeritWrtHistory  = measCx*measCy*measBreadth*measAng;
			couples[i].CplMerit.CplMeritWrtHistoryCx      = measCx;
			couples[i].CplMerit.CplMeritWrtHistoryCy      = measCy;
			couples[i].CplMerit.CplMeritWrtHistoryBreadth = measBreadth;
			couples[i].CplMerit.CplMeritWrtHistoryAng     = measAng;
		}
	}
	return Err;
}
