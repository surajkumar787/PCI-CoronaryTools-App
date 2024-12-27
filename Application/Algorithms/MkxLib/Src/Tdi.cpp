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

#include <MkxConsolidate.h>

//=============================================================================================================
// CONSTANTS

#define MIN(a,b)    (((a) <= (b)) ? (a):   (b) )
#define MAX(a,b)    (((a) >= (b)) ? (a):   (b) )
#define CLIP(a,b,c) MIN(MAX((a), (b)), (c))
#define SIGN(a)     (((a) >= 0)   ? (1): (-(1)))
#define ABS(a)      (((a) >= 0)   ? (a): (-(a)))
#define RND(a)      (((a) >= 0.0) ? (int)((a)+0.5) : (int)((a)-0.5))
#define inf (1.e+09)

//=============================================================================================================
// STRUCTURES

typedef struct
{
	double DecisionThresh     ;          // Decision threshold on dist between data and estimate 
	double VirtualCreationThresh ;		 // Threshold on DY/DT (as sig fac) for virtual track creation 
	int    MaxVirtualAge         ;		 // Maximum length of a virtual track
	double MinVirtualCost        ;		 // Minimum elementary virtual cost (as sig fac)
	double MaxVirtualCost        ;		 // Maximum elementary virutal cost (as sig fac)
	double ArgMinVirtualCost     ;		 // Maximum age producing minimal virtual cost
	double ArgMaxVirtualCost     ;		 // Minimum age producing maximal virtual cost
	double LevelThresh           ;		 // Threshold (as sig fac) from which DY/DT is penalize (high slope)
	double PenalizingSlope       ;		 // Slope used for penalizing strong DY/DT
}
CTdiPara;

typedef struct
{
	double  T;             // Track time
	double* Ys;            // Y-coordinates for all tracks at a given instant
	double* Ts;            // Time of the non-virtual "father" of virtuals 
	int     N;             // Number of tracks at a given instant
	int*    Vage;          // Age of virtual tracks at a given instant
	int*    PrevIdx;       // Father index for every track at a given instant
	double* SumC;          // Cumulated cost along a track
	int     Best;
	double  Yi;
	double  OptC;
	int AllocSize;
}
CTdiTrack;
typedef struct
{
	double* T;      // Time (different from index since skip taken into account)
	double* Ym;     // Y after merging
	double* Y1;     // Interpolated Y in Tr1 track
	double* Y2;     // Same in Tr2
	double* RawY1;  // Raw Y (before interpol) in Tr1
	double* RawY2;  // Raw Y (before interpol) in Tr1
	double* IntC1;  // Cost in Tr1 over current discrepancy interval (0 when no diff)
	double* IntC2;  // Same over Tr2
	double* Conf;  // Same over Tr2
}
CTdiRes;

//=============================================================================================================
// DECLARATIONS

int Tdi1dTrack(CTdiTrack* tr, CTdiPara para, int ns, double* ts, double* ys, double sig);
int Tdi1dTrackReverse(CTdiTrack* tr, CTdiPara para, int ns, double* ts, double* ys, double sig);
int TdiPredict(double *Pred, double T, double Y1, double T1, double Y2, double T2);
double AssessLevelDisc(double DY, double Thresh, double Slope);
void TdiMedianComput(int n, float* x, float* median);
int TdiMergeTrack(CTdiRes* res, int ns, CTdiTrack* tr1, CTdiTrack* tr2);
int	TdiEvalRes(double* p_d1h0, double* p_d0h1, int ns, double* ts, double* ys, int nRef, double* tRef, double* yRef, double tolDist, double* conf);

//=============================================================================================================
// METHODS

void TdiTrackPrint(CTdiTrack tr)
{
	printf("T         : %5.2f\n", tr.T);
	printf("Ys[0]     : %5.2f\n", tr.Ys[0]);
	printf("Ts[0]     : %5.2f\n", tr.Ts[0]);
	printf("N         : %3d\n", tr.N);
	printf("Vage[0]   : %3d   Vage[1]   : %3d   Vage[2]   : %3d\n", tr.Vage[0] , tr.Vage[1] , tr.Vage[2]);
	printf("PrevIdx[0]: %3d   PrevIdx[1]: %3d   PrevIdx[2]: %3d\n", tr.PrevIdx[0], tr.PrevIdx[1], tr.PrevIdx[2]);
	printf("SumC[0]   : %5.2f SumC[1]   : %5.2f SumC[2]   : %5.2f\n", tr.SumC[0] , tr.SumC[1] , tr.SumC[2]);
	printf("Best      : %3d\n", tr.Best);
	printf("Yi        : %5.2f\n", tr.Yi);
	printf("OptC      : %5.2f\n", tr.OptC);
	printf("AllocSize : %3d\n", tr.AllocSize);
}			
//=============================================================================================================
CTdiPara TdiGetDefPara()
{
	//Dynamic Interpolation version x: getting default para (prior TdiProc)
	CTdiPara P;

	P.DecisionThresh        = 0.5;  
	P.VirtualCreationThresh = 1.0;  
	P.MaxVirtualAge         = 40;   
	P.MinVirtualCost        = 1;    
	P.MaxVirtualCost        = 4;    
	P.ArgMinVirtualCost     = 3;    
	P.ArgMaxVirtualCost     = 20;   
	P.LevelThresh           = 1.8;  //2  PL_080403
	P.PenalizingSlope       = 50;   //10  PL_080403

	return P;
}
//=============================================================================================================
//Evaluation of a sigma-value for the slopes of (Ts, Ys) (column vectors)
int TdiEvalSig(double *sig, int ns, double* ts, double* ys)
{
	int err=0;
	double medToSigFac = 2.5; 
	float* grad = (float*)calloc(ns-1, sizeof(float));
	double* dT =(double*)calloc(ns-1, sizeof(double));
	for(int i=0;i<ns-1;i++)
	{
		dT[i]=ts[i+1]-ts[i];
	}
	for(int i=0;i<ns-1;i++)
	{
		grad[i]=(float)(ABS((ys[i+1]-ys[i]))/dT[i]);
	}
	float fmedian;

	TdiMedianComput(ns-1, grad, &fmedian);
	*sig = medToSigFac*fmedian; 

	free(grad);
	free(dT);

	return err;
}
//=============================================================================================================
void TdiMedianComput(int n, float* x, float* median)
{
	int *flag, ind;
	float *x2, max;

	if(n <= 0)return;
	if(n==1){
		*median = x[0];
		return;
	}
	// put in order in x2
	max = x[0];
	for(int i = 0; i < n; i++) max = MAX(max, x[i]);
	x2 = (float *)malloc(n * sizeof(float));
	flag = (int *)malloc(n * sizeof(int));
	for(int j = 0; j < n; j++) flag[j] = 0;
	ind = 0;
	for(int j = 0; j < n; j++){
		x2[j] = max;
		for(int i = 0; i < n; i++){
			if(!flag[i] && x[i] <= x2[j]){
				x2[j] = x[i];
				ind = i;
			}
		}
		flag[ind] = 1;
	}
	// take the value at the middle
	if(n%2 == 0)
		*median = (x2[n/2]+x2[n/2-1])/2;
	else
		*median = x2[(n-1)/2];

	free(x2);
	free(flag);
}

//=============================================================================================================
int TdiComputeConfidence(double* conf, double* ym, int ns, double* ys, double decisionThresh)
{
	int err=0;

	for(int i = 0; i<ns;i++)
	{
		double d = ABS(ys[i]-ym[i]);
		double alpha = 0.5/decisionThresh;
		double conf1 = 1 - alpha*d;
		double conf2 = MAX(0.5 - 0.1*alpha*(d-decisionThresh), 0);
		conf[i]  = MAX(conf1, conf2);
	}

	return err;
}

//=============================================================================================================
CTdiTrack TdiTrackAlloc(int allocSize)
{
	CTdiTrack tr;
	tr.AllocSize = allocSize;
	tr.Ys      = (double *)calloc(allocSize, sizeof(double));
	tr.Ts      = (double *)calloc(allocSize, sizeof(double));
	tr.Vage    = (int *)calloc(allocSize, sizeof(int));
	tr.PrevIdx = (int *)calloc(allocSize, sizeof(int));
	tr.SumC    = (double *)calloc(allocSize, sizeof(double));
	for(int i = 0; i< allocSize;i++)
		tr.PrevIdx[i] = -1;
	return tr;
}

//=============================================================================================================
int TdiTrackFree(CTdiTrack *tr)
{
	int err=0;

	if(tr->Ys) free(tr->Ys);
	if(tr->Ts) free(tr->Ts);
	if(tr->Vage) free(tr->Vage);
	if(tr->PrevIdx) free(tr->PrevIdx);
	if(tr->SumC) free(tr->SumC);
	return err;
}


//=============================================================================================================
int TdiTrackCopy(CTdiTrack* trDst, CTdiTrack* trSrc)
{
	int err=0;

	if(trDst->AllocSize != trSrc->AllocSize)
		return 1;
	int n = trSrc->AllocSize;

	trDst->T = trSrc->T;
	trDst->Best = trSrc->Best;
	trDst->N = trSrc->N;
	trDst->OptC = trSrc->OptC;
	trDst->Yi = trSrc->Yi;
	memcpy(trDst->Ys, trSrc->Ys, n*sizeof(double));
	memcpy(trDst->Ts, trSrc->Ts, n*sizeof(double));
	memcpy(trDst->Vage, trSrc->Vage, n*sizeof(int));
	memcpy(trDst->PrevIdx, trSrc->PrevIdx, n*sizeof(int));
	memcpy(trDst->SumC, trSrc->SumC, n*sizeof(double));

	return err;
}


//=============================================================================================================
CTdiRes TdiResAlloc(int n)
{
	CTdiRes res;
	res.T = (double*)calloc(n, sizeof(double));      
	res.Ym = (double*)calloc(n, sizeof(double));     
	res.Y1 = (double*)calloc(n, sizeof(double));     
	res.Y2 = (double*)calloc(n, sizeof(double));     
	res.RawY1 = (double*)calloc(n, sizeof(double));  
	res.RawY2 = (double*)calloc(n, sizeof(double));  
	res.IntC1 = (double*)calloc(n, sizeof(double));  
	res.IntC2 = (double*)calloc(n, sizeof(double));  
	res.Conf = (double*)calloc(n, sizeof(double));  
	return res;
}

//=============================================================================================================
void TdiResFree(CTdiRes* res)
{
	free(res->T);      
	free(res->Ym);     
	free(res->Y1);     
	free(res->Y2);     
	free(res->RawY1);  
	free(res->RawY2);  
	free(res->IntC1);  
	free(res->IntC2);  
	free(res->Conf);  
}

//=============================================================================================================
int TdiProc(CTdiRes* res1, int ns, double* ts, double* ys, CTdiPara para)
{
	int err=0;
	//function Res1 =  TdiProc(Ts, Ys, P, Fig)
	//% Res1 =  TdiProc(Ts, Ys, P, Fig)
	//% Dynamic Interpolation version x: actual interpolation call
	//% Ts:     Column vector containing time values
	//% Ys:     Column vector containing signal values (same dim as Ts)
	//% P:      Parameter structure (see DiaGetDefPara)
	//% Fig:    Figure index for plotting (no plot if <=0)

	//% Setting signal variation sigma value
	//Sig = TdiEvalSig(Ts, Ys);
	double sig;
	err = TdiEvalSig(&sig, ns, ts, ys);
	CTdiTrack* tr1 = (CTdiTrack *)malloc(ns * sizeof(CTdiTrack));
	for(int i=0;i<ns; i++) tr1[i] = TdiTrackAlloc(para.MaxVirtualAge+1);

	CTdiTrack* tr2 = (CTdiTrack *)malloc(ns * sizeof(CTdiTrack));
	for(int i=0;i<ns; i++) tr2[i] = TdiTrackAlloc(para.MaxVirtualAge+1);

	err |= Tdi1dTrack(tr1, para, ns, ts, ys, sig);//PL_160308
	err |= Tdi1dTrackReverse(tr2, para, ns, ts, ys, sig);
	err |= TdiMergeTrack(res1, ns, tr1, tr2);
	err |= TdiComputeConfidence(res1->Conf, res1->Ym, ns, ys, para.DecisionThresh);

	for(int i=0;i<ns; i++) TdiTrackFree(&tr1[i]);
	free(tr1);
	for(int i=0;i<ns; i++) TdiTrackFree(&tr2[i]);
	free(tr2);

	return err;
}

//=============================================================================================================
void S3dTdiEstim(float* ysOut, int ns, float* ts, float* ys)
{
	CTdiPara para=TdiGetDefPara();
	para.PenalizingSlope=50;//PL_080403
	para.LevelThresh = 1.8;//PL_080403

	CTdiRes res1 = TdiResAlloc(ns);

	double* dbl_ysOut = (double*)malloc(ns*sizeof(double));
	double* dbl_ts = (double*)malloc(ns*sizeof(double));
	double* dbl_ys = (double*)malloc(ns*sizeof(double));
	for(int i=0;i<ns;i++)
	{
		dbl_ts[i]=(double)ts[i];
		dbl_ys[i]=(double)ys[i];
	}
	TdiProc(&res1, ns, dbl_ts, dbl_ys, para);
	memcpy(dbl_ysOut, res1.Ym, ns*sizeof(double));

	TdiResFree(&res1);

	for(int i=0;i<ns;i++)
	{
		ysOut[i]=(float)dbl_ysOut[i];
	}
	free(dbl_ysOut);
	free(dbl_ts);
	free(dbl_ys);
}

//=============================================================================================================
int GetVirtualCostFromAge(double* p_kSig, int age, double minVal, double maxVal, double argMin, double argMax)
{
	int err=0;
	double kSig = MAX(age-argMin, 0)*((maxVal-minVal)/(argMax-argMin)) + minVal;
	kSig = MIN(kSig, maxVal);
	*p_kSig=kSig;
	return err;
}

//=============================================================================================================
int Tdi1dTrackReverse(CTdiTrack* tr, CTdiPara para, int ns, double* ts, double* ys, double sig)
{
	int err=0;
	CTdiTrack* trTmp = (CTdiTrack *)malloc(ns * sizeof(CTdiTrack));
	for(int i=0;i<ns; i++) trTmp[i] = TdiTrackAlloc(para.MaxVirtualAge+1);

	double* tsRev = (double *)calloc(ns, sizeof(double));
	double* ysRev = (double *)calloc(ns, sizeof(double));
	for(int i=0; i < ns; i++)
	{
		tsRev[i] = ts[ns-i-1];
		ysRev[i] = ys[ns-i-1];
	}
	err = Tdi1dTrack(trTmp, para, ns, tsRev, ysRev, sig);
	for(int i=0; i < ns; i++)
	{
		TdiTrackCopy(&tr[i], &trTmp[ns-i-1]);
	}
	double maxCost = tr[0].OptC;
	for(int i=0; i < ns; i++) tr[i].OptC = maxCost - tr[i].OptC;
	for(int i=0; i < ns; i++) TdiTrackFree(&trTmp[i]);
	free(trTmp);
	free(tsRev);
	free(ysRev);
	return err;
}

//=============================================================================================================
int Tdi1dTrack(CTdiTrack* tr, CTdiPara para, int ns, double* ts, double* ys, double sig)
{
	int err=0;
	double virtualCost1;
	err=GetVirtualCostFromAge(&virtualCost1, 1, para.MinVirtualCost, para.MaxVirtualCost, para.ArgMinVirtualCost, para.ArgMaxVirtualCost);
	int maxTrackNb = 0;

	tr[0].T          = ts[0];
	tr[0].Ys[0]      = ys[0];
	tr[0].Ts[0]      = ts[0];
	tr[0].N          = 1;    
	tr[0].Vage[0]    = 0;    
	tr[0].PrevIdx[0] = -1;   
	tr[0].SumC[0]    = 0;    

	for(int k=1; k<ns;k++)
	{
		// Propagating non-virtual track
		tr[k].Ys[0]   = ys[k];
		tr[k].Ts[0]   = ts[k];
		tr[k].T       = ts[k];
		tr[k].N       = 1; 
		tr[k].Vage[0] = 0;

		double sumCMin = inf;
		double ly1 = -1;
		int prevIdx = -1;
		for(int j=0; j<tr[k-1].N;j++)
		{
			double dT = tr[k].T - tr[k-1].T;
			double dY = tr[k].Ys[0] - tr[k-1].Ys[j];
			double ly = ABS(dY / dT);
			if (j==0) 
			{
				if (k>=2)
				{
					double predV;
					TdiPredict(&predV, tr[k].T, tr[k-1].Ys[0], tr[k-1].T, tr[k-2].Ys[0], tr[k-2].T);
					double ly2 = ABS(tr[k].Ys[0] - predV);
					ly  = MIN(ly, ly2);
				}
				ly1 = ly; 
			}
			double val = tr[k-1].SumC[j] + AssessLevelDisc(ly, para.LevelThresh*sig, para.PenalizingSlope);
			if (val < sumCMin) 
			{
				sumCMin = val;
				prevIdx = j;
			}
		}
		tr[k].SumC[0] = sumCMin;
		tr[k].PrevIdx[0] = prevIdx;

		// Propagating Prev virtuals
		for (int j=1;j<tr[k-1].N;j++)
		{
			if( tr[k-1].Vage[j] < para.MaxVirtualAge)
			{
				int end=tr[k].N;
				tr[k].N = tr[k].N + 1;
				tr[k].Ys[end] = tr[k-1].Ys[j];
				tr[k].Ts[end] = tr[k-1].Ts[j];
				tr[k].Vage[end] = tr[k-1].Vage[j] + 1;
				int age = tr[k].Vage[end]; //different from matlab
				double virtualCost;
				GetVirtualCostFromAge(&virtualCost, age, para.MinVirtualCost, para.MaxVirtualCost, para.ArgMinVirtualCost, para.ArgMaxVirtualCost);
				tr[k].SumC[end] = tr[k-1].SumC[j] + sig*virtualCost;
				tr[k].PrevIdx[end] = j;
			}
		}

		// Creating new virtual if necessary
		if( ly1 > para.VirtualCreationThresh*sig)
		{
			int end=tr[k].N;
			tr[k].N = tr[k].N + 1;
			tr[k].Ys[end] = tr[k-1].Ys[0];
			tr[k].Ts[end] = tr[k-1].Ts[0];
			tr[k].Vage[end] = 1;
			tr[k].SumC[end] = tr[k-1].SumC[0] + sig*virtualCost1;
			tr[k].PrevIdx[end] = 0;
		}
		maxTrackNb = MAX(maxTrackNb, tr[k].N);
	}
	//-------------------------------------------------------------
	// Back-tracking and interpolation "around" virtual
	int kk = ns-1;

	int iMin = 0;
	double valMin = tr[kk].SumC[0];
	for(int i = 0; i < tr[kk].N; i++)
	{
		if(tr[kk].SumC[i] < valMin)
		{
			iMin = i;
			valMin = tr[kk].SumC[i];
		}
	}
	//double val = valMin;
	tr[kk].Best = iMin;
	tr[kk].Yi = tr[kk].Ys[tr[kk].Best];
	tr[kk].OptC = tr[kk].SumC[tr[kk].Best];
	double y2 = tr[kk].Ys[tr[kk].Best];
	double t2 = tr[kk].T;

	for(int k = ns-1; k > 0; k--)
	{
		int jk = tr[k].Best;
		tr[k-1].Best = tr[k].PrevIdx[jk];
		tr[k-1].OptC = tr[k-1].SumC[tr[k-1].Best];
		int jkm1 = tr[k-1].Best;
		if( tr[k].Vage[jk]==0 && tr[k-1].Vage[jkm1]!=0)
		{
			y2 = tr[k].Ys[jk];
			t2 = tr[k].T;
			double y1 = tr[k-1].Ys[jkm1];
			double t1 = tr[k-1].Ts[jkm1];
			tr[k-1].Yi =  ((y2-y1)/(t2-t1))*(tr[k-1].T - t1) + y1;
		}
		else if( tr[k].Vage[tr[k].Best]!=0 && tr[k-1].Vage[tr[k-1].Best]!=0)
		{
			double y1 = tr[k-1].Ys[jkm1]; 
			double t1 = tr[k-1].Ts[jkm1];
			tr[k-1].Yi =  ((y2-y1)/(t2-t1))*(tr[k-1].T - t1) + y1;
		}
		else
		{
			tr[k-1].Yi = tr[k-1].Ys[jkm1];
		}
	}
	return err;
}

//=============================================================================================================
int TdiPredict(double *Pred, double T, double Y1, double T1, double Y2, double T2)
{
	int err=0;
	if(T1==T2)
		return 1;
	*Pred = Y1 + (T-T1)*((Y1-Y2)/(T1-T2));
	return err;
}

//=============================================================================================================
double AssessLevelDisc(double DY, double Thresh, double Slope)
{
	double DY2;
	if( DY<=Thresh)
		DY2 = DY;
	else
		DY2 = Thresh + Slope*(DY-Thresh);
	return DY2;

}

//=============================================================================================================
int TdiMergeTrack(CTdiRes* res, int ns, CTdiTrack* tr1, CTdiTrack* tr2)
{
	double epsi = 0.001;

	for(int k=0; k< ns; k++)
	{
		int Jopt1    = tr1[k].Best;      // Index of best causally-found track at index k
		int Jopt2    = tr2[k].Best;      // Sam for anti-causally-found track
		res->T[k]     = tr1[k].T;         // Time (different from index since skip taken into account)
		res->Ym[k]    = tr1[k].Yi;        // Y after merging
		res->Y1[k]    = tr1[k].Yi;        // Interpolated Y in tr1 track
		res->Y2[k]    = tr2[k].Yi;        // Same in tr2
		res->RawY1[k] = tr1[k].Ys[Jopt1]; // Raw Y (before interpol) in tr1
		res->RawY2[k] = tr2[k].Ys[Jopt2]; // Raw Y (before interpol) in tr1
		res->IntC1[k] = 0;                // Cost in tr1 over current discrepancy interval (0 when no diff)
		res->IntC2[k] = 0;                // Same over tr2
	}
	int StartFlag = 0;  // Start and Stop are used to trap intervals where tr1.Yi != tr2.Yi  
	int* Start = (int *)malloc(ns* sizeof(int));
	int* Stop = (int *)malloc(ns* sizeof(int));
	for(int i = 0; i<ns;i++)
	{
		Start[i]=Stop[i]=-1;
	}
	int StartIdx = 0;
	if (ABS(tr1[0].Yi - tr2[0].Yi)>epsi)
	{
		StartFlag = 1;
		Start[StartIdx] = 0;
		StartIdx = StartIdx+1;
	}
	for(int k=0; k< ns; k++)
	{  
		if (StartFlag)// % Look for discrepancy end
		{
			if (ABS(tr1[k].Yi - tr2[k].Yi)<epsi)
			{
				Stop[StartIdx-1] = k;
				StartFlag = 0;
			}
		}
		else //% Look for discrepancy start
		{
			if (ABS(tr1[k].Yi - tr2[k].Yi)>epsi)
			{
				Start[StartIdx] = k;
				StartFlag = 1;
				StartIdx = StartIdx + 1;
			}
		}
	}
	if( StartFlag) //% When last point ends is a discrpancy point
	{
		Stop[StartIdx-1]=ns-1;
	}
	int isEmpty = 1;
	for(int i=0; i<ns;i++)
	{
		if(Stop[i] != -1)
			isEmpty = 0;
	}
	if(isEmpty) //% When no differnce between tr1 and tr2
	{
		for(int i = 0; i<ns;i++)
		{
			Start[i]=-1;
		}
	}
	int cnt=0;
	for(int i = 0; i<ns;i++)
	{
		if(Start[i]>-0.5)cnt++;
	}
	double* StartT = (double *)malloc(cnt* sizeof(double));
	double* StopT = (double *)malloc(cnt* sizeof(double));
	double* Sum1 = (double *)malloc(cnt* sizeof(double));
	double* Sum2 = (double *)malloc(cnt* sizeof(double));
	for(int i = 0; i<cnt;i++)
	{
		StartT[i]=StopT[i]=Sum1[i]=Sum2[i]=-1;
	}
	for(int j=0; j<cnt;j++)
	{
		StartT[j] = tr1[Start[j]].T;
		StopT[j]  = tr1[Stop[j]].T;
		if( Start[j]==0 )//% Discrepancy at track start
		{
			Sum1[j] = tr1[Stop[j]].OptC;
			Sum2[j] = tr2[Stop[j]].OptC;
		}
		else
		{
			Sum1[j] = tr1[Stop[j]].OptC - tr1[Start[j]-1].OptC;
			Sum2[j] = tr2[Stop[j]].OptC - tr2[Start[j]-1].OptC;
		}
		double Border = ns/20;
		double Alpha = (ABS(Sum1[j]+Sum2[j])>epsi)? (Sum1[j]/(Sum1[j]+Sum2[j])):0.5f; //PL_081118
		//% Uncomment to choose the method: Best of two passes (as opposed to Barycenter of both)
		//%   if Sum1[j]>Sum2[j]
		//%     Alpha = 1;
		//%   else
		//%     Alpha = 0;
		//%   end
		for(int k=Start[j]; k<=Stop[j];k++)
		{
			res->IntC1[k] = Sum1[j];
			res->IntC2[k] = Sum2[j];
			double AlphaK = Alpha;
			if( Start[j]<Border)
				AlphaK = 1;
			else if( Stop[j]>ns-Border)
				AlphaK = 0;
			res->Ym[k] = (1-AlphaK)*res->Y1[k] + AlphaK*res->Y2[k];
		}
	}
	free(StartT);
	free(StopT); 
	free(Sum1);
	free(Sum2);
	free(Start);
	free(Stop);


	return 0;
}
//=============================================================================================================
int	TdiEvalRes(double* p_d1h0, double* p_d0h1, int ns, double* ts, double* ys, int nRef, double* tRef, double* yRef, double tolDist, double* conf)
{
	int err=0;
	double d1h0 = 0, d0h1 = 0;
	for(int k=0;k<ns; k++)
	{
		for(int j=0; j<nRef;j++)
		{
			if(ABS(ts[k]-tRef[j])<0.5)
			{
				int h=ABS(ys[k]-yRef[j])<=tolDist;
				int d=conf[k]>=0.5;//By definition of the confidence measurement (in [0,1])
				d1h0 += (h==0 && d==1);
				d0h1 += (h==1 && d==0);
				break;
			}
		}
	}
	*p_d1h0 = 100*d1h0/(double)ns;
	*p_d0h1 = 100*d0h1/(double)ns;
	return err;
}
