// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <stdlib.h>
#include <math.h> 

#include <MkxDev.h>
#include <MkxConsolidate.h>

//=============================================================================================================
void  S3dRectifAngle(int n, int* status, float *ang);

//other functions
//=============================================================================================================
void CslXyToCbaOne(float x1, float y1, float x2, float y2, float* cx, float* cy, float* br, float* an )
{
	*cx = (x1 + x2) / (float)2;
	*cy = (y1 + y2) / (float)2;
	*br = (float) sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	*an = (float)((180 / MKX_PI) * atan2((y2 - y1), (x2 - x1)));
}
//=============================================================================================================
void S3dUpdateXyFromCbaOne(float cx, float cy, float br, float an, float* x1, float* y1, float* x2, float* y2)
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

//=============================================================================================================
void S3dMeanFilt(int n, float* in_val, int* in_coef, float* out_val, int halfLength)
{
	int i, k;

	for (i = 0; i < n; i++)
	{
		float f = 0;
		int cnt = 0;
		for (k = -halfLength; k <= halfLength; k++)
		{
			if (i + k >= 0 && i + k < n)
			{
				f += in_val[i + k] * in_coef[i + k];
				cnt += in_coef[i + k];
			}
		}
		out_val[i] = f / cnt;
	}
}
//=============================================================================================================
float S3dDist(float x1, float y1, float x2, float y2)
{
	return (float)sqrt((double)((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));
}
//=============================================================================================================
void S3dRectifAngle(int n, int* status, float *ang)
{
	int k;
	float *yIn, *yOut, diff, minVal=0, maxVal=0, meanVal=0, stdVal=0;

	if(n < 1) return;

	yIn=(float *)calloc(n, sizeof(float));
	yOut=(float *)calloc(n, sizeof(float));

	int q=0;
	for (int i = 0; i < n; i++)
	{
		if (status[i] == MKX_CST_EXTRACT_RES_OK)
		{
			yIn[q++]=ang[i];
		}
	}
	yOut[0]=yIn[0];
	for (int i = 1; i < q; i++)
	{
		int kMin = 1;
		float diffMin = MKX_ABS(yIn[i] - yOut[i - 1]);
		for ( k = -1; k < 4; k++)
		{
			diff = MKX_ABS(yIn[i] + (k - 1) * 180 - yOut[i - 1]);
			if (diff < diffMin)
			{
				diffMin = diff;
				kMin = k;
			}
		}
		yOut[i] = yIn[i] + (kMin - 1) * 180;
	}
	S3dStatComput(q, yOut, &meanVal, &minVal, &maxVal, &stdVal);
	if(minVal>90&&maxVal>180)
	{
		for (int i = 0; i < q; i++)yOut[i]-=180;
	}
	S3dStatComput(q, yOut, &meanVal, &minVal, &maxVal, &stdVal);
	if(maxVal<0)
	{
		for (int i = 0; i < q; i++)yOut[i]+=180;
	}
	for (int i = 0, q=0; i < n; i++)
	{
		if (status[i] == MKX_CST_EXTRACT_RES_OK)
		{
			ang[i]=yOut[q++];
		}
	}
	free(yIn);
	free(yOut);
}




/*************************************************************************************/
// Compute statistics on a float variable
/*************************************************************************************/
void S3dStatComput(int N, float* X, float* pt_Mean, float* pt_Min, float* pt_Max, float* pt_Stdv)
{
	float Min, Max, Mean, Stdv, Sum, Sum2;

	if(N <= 0)return;

	Min = Max = X[0];
	Sum = Sum2 = (float) 0;
	for(int i = 0; i < N; i++)
	{
		Sum += X[i];
		Sum2 += X[i]*X[i];
		Min = MKX_MIN(Min, X[i]);
		Max = MKX_MAX(Max, X[i]);
	}
	Mean = Sum /(float) N;

	if(N==1)Stdv = (float)0;
	else		Stdv = (float) sqrt(MKX_MAX(0,(double)(Sum2 / (float) N  - Mean * Mean )));

	*pt_Mean = Mean;
	*pt_Min = Min;
	*pt_Max = Max;
	*pt_Stdv = Stdv;
}

/*************************************************************************************/
// Compute median on a float variable
/*************************************************************************************/
void S3dMedianComput(int n, float* x, float* median)
{
	int *flag, ind;
	float *x2, max;

	if(n <= 0)return;
	if(n==1)
	{
		*median = x[0];
		return;
	}
	// put in order in x2
	max = x[0];
	for(int i = 0; i < n; i++) max = MKX_MAX(max, x[i]);

	x2 = (float *)malloc(n * sizeof(float));
	flag = (int *)calloc(n, sizeof(int));

	ind = 0;
	for(int j = 0; j < n; j++)
	{
		x2[j] = max;
		for(int i = 0; i < n; i++)
		{
			if(!flag[i] && x[i] <= x2[j])
			{
				x2[j] = x[i];
				ind = i;
			}
		}
		flag[ind] = 1;
	}
	// take the value at the middle
	if(n%2 == 0)*median = (x2[n/2]+x2[n/2-1])/2;
	else		    *median = x2[(n-1)/2];

	free(x2);
	free(flag);
}


//=============================================================================================================
void S3dMadComput(int n, float* x, float* mad)
{
	float *y;
	float median;

	if(n <= 0)return;
	if(n==1)
	{
		*mad = x[0];
		return;
	}
	S3dMedianComput(n, x, &median);

	y = (float *)malloc(n * sizeof(float));
	for (int i = 0; i < n; i++)
	{
		y[i] = MKX_ABS(x[i] - median);
	}
	S3dMedianComput(n, y, mad);
	free(y);
}

//=============================================================================================================
void S3dSmooth(int n, float* yIn, float* yOut, int nAvr, int nIter)
{
	float *x, *y;
	int p, kIter, k;

	if(nAvr % 2 == 0) nAvr++; //must be an odd number
	x = (float *)calloc(n ,sizeof(float));
	y = (float *)calloc(n ,sizeof(float));

	for( int i=0; i< n; i++)
		x[i] = yIn[i];

	p = (nAvr-1)/2;

	for( kIter=0; kIter<nIter; kIter++)
	{
		for( int i = 0; i < n; i++)
		{
			float yy = 0;
			k = 0;
			for( int j = 0; j < 2*p+1; j++)
			{
				if(i-p+j >= 0 && i-p+j < n) 
				{
					yy += x[i-p+j];
					k++;
				}
			}
			y[i] = yy/(float)k;
		}
		for( int i=0; i< n; i++)
		{
			x[i] = y[i];
		}
	}
	for(int  i=0; i< n; i++)
		yOut[i] = y[i];

	free(x);
	free(y);
}

//=============================================================================================================
float S3dTdiFuzzy(float x, float thresh)
{
	float y1, y2;
	float ret;
	float alpha = 0.5f / thresh;

	y1 = (float) (1 - alpha * x);
	y2 = (float) MKX_MAX(0.5 - 0.1 * alpha * (x - thresh), 0);
	ret = MKX_MAX(y1, y2);
	return ret;
}
//=============================================================================================================
float S3dFuzzy(float x, float th1, float th2)
{
	float ret;

	x = MKX_ABS(x);
	if (x <= th1) ret = 1;
	else if (x > th2) ret = 0;
	else
	{
		ret = 1 - (x - th1) / (th2 - th1);
	}
	return ret;
}


//=============================================================================================================
void S3dUpdateWireStatus(Csl* ex)
{
	int n=ex->N;

	if(n<1)return;

	for (int i = 0; i < n; i++)
	{
		if(ex->WireStatus[i] == MKX_CST_WIRE_AVAILABLE && ex->CslStatus[i] == MKX_CST_EXTRACT_RES_OK)
		{
			ex->WireStatus[i] = MKX_CST_WIRE_AVAILABLE;
		}
		else
		{
			ex->WireStatus[i] = MKX_CST_WIRE_NOT_AVAILABLE;
		}
	}
}

//=============================================================================================================
int  S3dUpdateXyFromCba(Csl* ex)
{
	int n=ex->N;
	if(n<1)return 1;

	for (int i = 0; i < n; i++)
	{
		if (ex->CslStatus[i] == MKX_CST_EXTRACT_RES_OK)
			S3dUpdateXyFromCbaOne(ex->Cx[i], ex->Cy[i], ex->Br[i], ex->An[i], &ex->X1[i], &ex->Y1[i], &ex->X2[i], &ex->Y2[i]); 
	}
	CslPolarize(ex);

	return 0;
}

//=============================================================================================================
void CslPolarize(Csl* ex)
{
	float XA, YA, XB, YB;
	int i = 0;
	int n=ex->N;

	if(n < 1) return;
	while ((i < n) && (ex->CslStatus[i] == MKX_CST_EXTRACT_RES_SKIP))
		i++;
	if (i > n - 2) //too many skip
		return;

	XA = ex->X1[i];
	YA = ex->Y1[i];
	XB = ex->X2[i];
	YB = ex->Y2[i];

	i++;

	while (i < n)
	{
		if (ex->CslStatus[i] == MKX_CST_EXTRACT_RES_OK)
		{
			float p = (XB-XA) * (ex->X2[i]-ex->X1[i]) + (YB-YA) * (ex->Y2[i]-ex->Y1[i]);

			if (p >= 0)
			{
				XA = ex->X1[i];
				YA = ex->Y1[i];
				XB = ex->X2[i];
				YB = ex->Y2[i];
			}
			else  //swap 1 and 2
			{
				XA = ex->X2[i];
				YA = ex->Y2[i];
				XB = ex->X1[i];
				YB = ex->Y1[i];
				if(ex->WireStatus[i] == MKX_CST_WIRE_AVAILABLE)
				{
					BxCurveSwap(&ex->Wire[i]);
				}
			}
			ex->X1[i] = XA;
			ex->Y1[i] = YA;
			ex->X2[i] = XB;
			ex->Y2[i] = YB;
		}
		i++;
	}
}
int CslCreate(Csl* csl, int n, CMkxExtractResults *tabRes)
{
	csl->N=n;
	csl->An = (float *)calloc(n, sizeof(float));
	csl->Br = (float *)calloc(n, sizeof(float));
	csl->Cx = (float *)calloc(n, sizeof(float));
	csl->Cy = (float *)calloc(n, sizeof(float));
	csl->CslStatus = (int *)malloc(n * sizeof(float));
	csl->X1 = (float *)malloc(n * sizeof(float));
	csl->Y1 = (float *)malloc(n * sizeof(float));
	csl->X2 = (float *)malloc(n * sizeof(float));
	csl->Y2 = (float *)malloc(n * sizeof(float));
	csl->Confidence = (float *)malloc(n * sizeof(float));
	csl->Wire = (BxCurve *)malloc(n * sizeof(BxCurve));
	csl->WireStatus = (int *)malloc(n * sizeof(int));
	csl->Advanced.A1 = (float *)malloc(n * sizeof(float));
	csl->Advanced.A2 = (float *)malloc(n * sizeof(float));
	csl->Advanced.B1 = (float *)malloc(n * sizeof(float));
	csl->Advanced.B2 = (float *)malloc(n * sizeof(float));
	csl->Advanced.Dir1 = (float *)malloc(n * sizeof(float));
	csl->Advanced.Dir2 = (float *)malloc(n * sizeof(float));
	csl->Advanced.RefiningConf1 = (float *)malloc(n * sizeof(float));
	csl->Advanced.RefiningConf2 = (float *)malloc(n * sizeof(float));
	csl->Advanced.Intens1 = (float *)malloc(n * sizeof(float));
	csl->Advanced.Intens2 = (float *)malloc(n * sizeof(float));
	csl->Advanced.LevelIn1 = (float *)malloc(n * sizeof(float));
	csl->Advanced.LevelIn2 = (float *)malloc(n * sizeof(float));
	csl->Advanced.LevelOut1 = (float *)malloc(n * sizeof(float));
	csl->Advanced.LevelOut2 = (float *)malloc(n * sizeof(float));
	csl->Advanced.Contrast1 = (float *)malloc(n * sizeof(float));
	csl->Advanced.Contrast2 = (float *)malloc(n * sizeof(float));
	csl->Advanced.Locked = (int *)malloc(n * sizeof(int));

	for(int i=0;i<csl->N;i++)
	{
		CslGetForImage(csl, i, &tabRes[i]);
	}
	return 0;
}


//=============================================================================================================
void CslFree(Csl* csl)
{
	if(csl->An) free(csl->An );
	if(csl->Br) free(csl->Br );
	if(csl->Cx) free(csl->Cx );
	if(csl->Cy) free(csl->Cy );
	if(csl->CslStatus) free(csl->CslStatus);
	if(csl->Confidence) free(csl->Confidence); 
	if(csl->X1) free(csl->X1);
	if(csl->Y1) free(csl->Y1);
	if(csl->X2) free(csl->X2);
	if(csl->Y2) free(csl->Y2);
	if(csl->WireStatus) free(csl->WireStatus);
	if(csl->Wire) free(csl->Wire);

	if(csl->Advanced.A1             ) free(             csl->Advanced.A1);
	if(csl->Advanced.A2			  ) free(   				csl->Advanced.A2);
	if(csl->Advanced.B1			  ) free(   				csl->Advanced.B1);
	if(csl->Advanced.B2	  ) free(   						csl->Advanced.B2);
	if(csl->Advanced.Dir1	  ) free(   					csl->Advanced.Dir1);
	if(csl->Advanced.Dir2	  ) free(   					csl->Advanced.Dir2);
	if(csl->Advanced.RefiningConf1	  ) free(   					csl->Advanced.RefiningConf1);
	if(csl->Advanced.RefiningConf2	  ) free(   					csl->Advanced.RefiningConf2);
	if(csl->Advanced.Intens1  ) free(   						csl->Advanced.Intens1);
	if(csl->Advanced.Intens2	  ) free(   					csl->Advanced.Intens2);
	if(csl->Advanced.LevelIn1	  ) free(   				csl->Advanced.LevelIn1);
	if(csl->Advanced.LevelIn2	  ) free(   				csl->Advanced.LevelIn2);
	if(csl->Advanced.LevelOut1  ) free(   					csl->Advanced.LevelOut1);
	if(csl->Advanced.LevelOut2  ) free(   					csl->Advanced.LevelOut2);
	if(csl->Advanced.Contrast1  ) free(   					csl->Advanced.Contrast1);
	if(csl->Advanced.Contrast2  ) free(   					csl->Advanced.Contrast2);
	if(csl->Advanced.Locked	  ) free(   					csl->Advanced.Locked);

}

void CslGetForImage(Csl* csl, int i, CMkxExtractResults* er)
{
	//main part
	csl->CslStatus[i] = er->Status;

	csl->X1[i] = er->X1Refined;
	csl->Y1[i] = er->Y1Refined;
	csl->X2[i] = er->X2Refined;
	csl->Y2[i] = er->Y2Refined;

	csl->WireStatus[i] = er->WireStatus;
	if(er->WireStatus == MKX_CST_WIRE_AVAILABLE)
	{
		BxCurveCopy(&(er->Wire), &(csl->Wire[i]));
	}

	//advanced part
		if (er->Status == MKX_CST_EXTRACT_RES_OK)
	{
		csl->Advanced.Intens1[i] = er->Advanced.Intens1;
		csl->Advanced.Intens2[i] = er->Advanced.Intens2;
		csl->Advanced.A1[i] = er->Advanced.Length1;
		csl->Advanced.B1[i] = er->Advanced.Thickness1;
		csl->Advanced.A2[i] = er->Advanced.Length2;
		csl->Advanced.B2[i] = er->Advanced.Thickness2;//PL_080414
		csl->Advanced.Dir1[i] = er->Advanced.Dir1;
		csl->Advanced.Dir2[i] = er->Advanced.Dir2;
		csl->Advanced.RefiningConf1[i] = er->Advanced.RefiningConf1;
		csl->Advanced.RefiningConf2[i] = er->Advanced.RefiningConf2;
		csl->Advanced.Locked[i] = er->Locked;
		csl->Advanced.LevelIn1[i] = er->Advanced.LevelIn1;
		csl->Advanced.LevelIn2[i] = er->Advanced.LevelIn2;
		csl->Advanced.LevelOut1[i] =er->Advanced.LevelOut1;
		csl->Advanced.LevelOut2[i] =er->Advanced.LevelOut2;
		if(er->Advanced.LevelIn1 + er->Advanced.LevelOut1 > 0)
		{
			csl->Advanced.Contrast1[i] = (float)(-100 * (er->Advanced.LevelIn1 - er->Advanced.LevelOut1) / (0.5 * (er->Advanced.LevelIn1 + er->Advanced.LevelOut1)));
		}
		else
		{
			csl->Advanced.Contrast1[i] = 0;
		}
		if(er->Advanced.LevelIn2 + er->Advanced.LevelOut2 > 0)
		{
			csl->Advanced.Contrast2[i] = (float)(-100 * (er->Advanced.LevelIn2 - er->Advanced.LevelOut2) / (0.5 * (er->Advanced.LevelIn2 + er->Advanced.LevelOut2)));
		}
		else
		{
			csl->Advanced.Contrast2[i] = 0;
		}
	}
	else
	{
		csl->Advanced.Intens1[i] = -1;
		csl->Advanced.Intens2[i] = -1;
		csl->Advanced.A1[i]      = -1;
		csl->Advanced.B1[i]      = -1;
		csl->Advanced.A2[i]      = -1;
		csl->Advanced.B2[i]      = -1;
		csl->Advanced.Dir1[i]    = -1;
		csl->Advanced.Dir2[i]    = -1;
		csl->Advanced.RefiningConf1[i]    = -1;
		csl->Advanced.RefiningConf2[i]    = -1;
		csl->Advanced.Locked[i]  = -1;

		csl->Advanced.LevelIn1[i] = -1.f;
		csl->Advanced.LevelIn2[i] = -1.f;
		csl->Advanced.LevelOut1[i] = -1.f;
		csl->Advanced.LevelOut2[i] = -1.f;
		csl->Advanced.Contrast1[i] = -1.f;
		csl->Advanced.Contrast2[i] = -1.f;
	}
	S3dRectifAngle(csl->N, csl->CslStatus, csl->Advanced.Dir1);
	S3dRectifAngle(csl->N, csl->CslStatus, csl->Advanced.Dir2);

}
//=============================================================================================================
void CslXyToCba(Csl* ex)
{
	int n=ex->N;

	if(n < 1) return;

	for (int i = 0; i < n; i++)
	{
		if (ex->CslStatus[i] == MKX_CST_EXTRACT_RES_OK)
			CslXyToCbaOne(ex->X1[i], ex->Y1[i], ex->X2[i], ex->Y2[i], &ex->Cx[i], &ex->Cy[i], &ex->Br[i], &ex->An[i]); 
	}
	S3dRectifAngle(n, ex->CslStatus, ex->An);
}


//=============================================================================================================
void S3dConfidenceForOneFeature(CslFeatConfidence *conf, float *v, int n, int *used, int nUsed, int nAvr, int nIter)
{
	float *time = (float *)malloc(nUsed * sizeof(float));
	float *meas = (float *)malloc(nUsed * sizeof(float));
	float *estim = (float *)malloc(nUsed * sizeof(float));
	float *r = (float *)malloc(nUsed * sizeof(float));
	float *confidence = (float *)malloc(nUsed * sizeof(float));
	float thresh = CSL_MATCHDIST;
	int k=0;

	for (int i = 0; i < n; i++)
	{
		if (used[i] == CSL_TRUE)
		{
			time[k] = (float)i;
			meas[k] = v[i];
			k++;
		}
	}
	S3dTdiEstim(estim, nUsed, time, meas);

	// smoothing needed
	if(nIter>0 && nAvr>2)
	{
		float rMean, rMin, rMax, rStdv, th1, th2;

		S3dSmooth(nUsed, estim, estim, nAvr, nIter); 
		for (int i = 0; i < nUsed; i++)
		{
			r[i] = MKX_ABS(estim[i] - meas[i]);
		}
		S3dStatComput(nUsed, r, &rMean, &rMin, &rMax, &rStdv);
		th1 = 2*rStdv;
		th2 = 5*rStdv;
		for (int i = 0; i < nUsed; i++)
		{
			confidence[i]=S3dFuzzy(r[i], th1, th2);
		}
	}
	else
	{	
		for (int i = 0; i < nUsed; i++)
		{
			r[i] = MKX_ABS(estim[i] - meas[i]);
			confidence[i]=S3dTdiFuzzy(r[i], thresh);
		}
	}
	for (int i = 0, k = 0; i < n; i++)
	{
		if (used[i] == CSL_TRUE)
		{
			conf->Confidence[i] = confidence[k];
			conf->Meas[i] = meas[k];
			conf->Estim[i] = estim[k];
			k++;
		}
		else
		{
			conf->Confidence[i] = -1;
			conf->Meas[i] = -1;
			conf->Estim[i] = -1;
		}
	}
	free(time);
	free(meas);
	free(estim);
	free(r);
	free(confidence);
}

//=============================================================================================================
void S3dScoreConfidence(CslConfidence *mc, int t, float *sCpl, float *sM1, float *sM2, float *sGlob)
{
	float s0, s1, s2, s3;
	float a0=CSL_CoupleConfidenceCoef;
	float a1=CSL_SingleMarkerConfidenceCoef;
	float a2=CSL_SingleMarkerConfidenceCoef;
	float th1 = CSL_ConfidenceFuzzyCoefLow;
	float th2 = CSL_ConfidenceFuzzyCoefHigh;

	//score on couple
	s0 = mc->Conf[ CSL_ConfIndex_Cx].Confidence[t]
	+ mc->Conf[CSL_ConfIndex_Cy].Confidence[t]
	+ mc->Conf[CSL_ConfIndex_Br].Confidence[t]
	+ mc->Conf[CSL_ConfIndex_An].Confidence[t];
	s0 /= 4;

	//score on marker1
	if (mc->Err1[t] == -1)//no information on marker 1
	{
		s1 = 0.5;
	}
	else
	{
		s1 = mc->Conf[CSL_ConfIndex_Intens1].Confidence[t]
		+ mc->Conf[CSL_ConfIndex_A1].Confidence[t]
		+ mc->Conf[CSL_ConfIndex_B1].Confidence[t]
		+ mc->Conf[CSL_ConfIndex_Contrast1].Confidence[t]
		+ mc->Conf[CSL_ConfIndex_Dir1].Confidence[t];
		s1 /= 5;
	}
	//score on marker2
	if (mc->Err2[t] == -1)//no information on marker 2
	{
		s2 = 0.5;
	}
	else
	{
		s2 = mc->Conf[CSL_ConfIndex_Intens2].Confidence[t]
		+ mc->Conf[CSL_ConfIndex_A2].Confidence[t]
		+ mc->Conf[CSL_ConfIndex_B2].Confidence[t]
		+ mc->Conf[CSL_ConfIndex_Contrast2].Confidence[t]
		+ mc->Conf[CSL_ConfIndex_Dir2].Confidence[t];
		s2 /= 5;
	}
	//combination of scores
	s3 = (a0 * s0 + a1 * s1 + a2 * s2) / (a0 + a1 + a2);

	//fuzzy function for enhancing confidence values
	*sGlob = S3dFuzzy(1 - s3, th1, th2);
	*sCpl = S3dFuzzy(1 - s0, th1, th2);
	*sM1 = S3dFuzzy(1 - s1, th1, th2);
	*sM2 = S3dFuzzy(1 - s2, th1, th2);

}


//=============================================================================================================
void CslUpdateConfidence(Csl *extract, CslConfidence *mc)
{
	int nUsed = 0;
	int nUsed_A1_B1 = 0;
	int nUsed_A2_B2 = 0;
	int n=extract->N;

	if(n<1)return;

	int *Used_A1_B1 = (int *)malloc(n*sizeof(int));
	int *Used_A2_B2 = (int *)malloc(n*sizeof(int));

	//CslConfidence mc;
	//S3dAllocMarkerConfidence(&mc, n);

	for (int i = 0; i < n; i++)
	{
		mc->Locked[i] = extract->Advanced.Locked[i];
		mc->Err1[i] = extract->Advanced.RefiningConf1[i];
		mc->Err2[i] = extract->Advanced.RefiningConf2[i];
	}

	for (int i = 0; i < n; i++)
	{
		if (extract->CslStatus[i] == MKX_CST_EXTRACT_RES_OK)
		{
			nUsed++;
			mc->Used[i] = CSL_TRUE;
		}
		else
		{
			mc->Used[i] = CSL_FALSE;
		}
	}
	//check availability of measures for A1 and B1
	for (int i = 0; i < n; i++)
	{
		if (extract->CslStatus[i] == MKX_CST_EXTRACT_RES_OK && extract->Advanced.A1[i] >= 0 && extract->Advanced.B1[i] >= 0 )
		{
			nUsed_A1_B1++;
			Used_A1_B1[i] = CSL_TRUE;
		}
		else
		{
			Used_A1_B1[i] = CSL_FALSE;
		}
	}
	//check availability of measures for A2 and B2
	for (int i = 0; i < n; i++)
	{
		if (extract->CslStatus[i] == MKX_CST_EXTRACT_RES_OK && extract->Advanced.A2[i] >= 0 && extract->Advanced.B2[i] >= 0 )
		{
			nUsed_A2_B2++;
			Used_A2_B2[i] = CSL_TRUE;
		}
		else
		{
			Used_A2_B2[i] = CSL_FALSE;
		}
	}

	if (nUsed > 0)
	{
		S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_Cx], extract->Cx, n, mc->Used, nUsed, 0, 0);
		S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_Cy], extract->Cy, n, mc->Used, nUsed, 0, 0);
		S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_Br], extract->Br, n, mc->Used, nUsed, 0, 0);
		S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_An], extract->An, n, mc->Used, nUsed, 0, 0);
		S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_Intens1], extract->Advanced.Intens1, n, mc->Used, nUsed, CSL_nAvrForSmoothingForMediumStableFeature, CSL_nIterForSmoothing);
		if (nUsed_A1_B1 > 0)
		{
			S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_A1], extract->Advanced.A1, n, Used_A1_B1, nUsed_A1_B1, CSL_nAvrForSmoothingForStableFeature, CSL_nIterForSmoothing);
			S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_B1], extract->Advanced.B1, n, Used_A1_B1, nUsed_A1_B1, CSL_nAvrForSmoothingForStableFeature, CSL_nIterForSmoothing);
		}
		S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_Dir1], extract->Advanced.Dir1, n, mc->Used, nUsed, CSL_nAvrForSmoothingForMediumStableFeature, CSL_nIterForSmoothing);
		S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_Contrast1], extract->Advanced.Contrast1, n, mc->Used, nUsed, CSL_nAvrForSmoothingForMediumStableFeature, CSL_nIterForSmoothing);
		S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_Intens2], extract->Advanced.Intens2, n, mc->Used, nUsed, CSL_nAvrForSmoothingForMediumStableFeature, CSL_nIterForSmoothing);
		if (nUsed_A2_B2 > 0)
		{
			S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_A2], extract->Advanced.A2, n, Used_A2_B2, nUsed_A2_B2, CSL_nAvrForSmoothingForStableFeature, CSL_nIterForSmoothing);
			S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_B2], extract->Advanced.B2, n, Used_A2_B2, nUsed_A2_B2, CSL_nAvrForSmoothingForStableFeature, CSL_nIterForSmoothing);
		}
		S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_Dir2], extract->Advanced.Dir2, n, mc->Used, nUsed, CSL_nAvrForSmoothingForMediumStableFeature, CSL_nIterForSmoothing);
		S3dConfidenceForOneFeature(&mc->Conf[CSL_ConfIndex_Contrast2], extract->Advanced.Contrast2, n, mc->Used, nUsed, CSL_nAvrForSmoothingForMediumStableFeature, CSL_nIterForSmoothing);
	}

	for (int i = 0; i < n; i++)
	{
		S3dScoreConfidence(mc, i, &(mc->ScoreCouple[i]), &(mc->ScoreM1[i]), &(mc->ScoreM2[i]), &(mc->ScoreGlobal[i]));
	}

	//fill Confidence field
	for (int i = 0; i < n; i++)
		extract->Confidence[i] = mc->ScoreGlobal[i];

	free(Used_A1_B1);
	free(Used_A2_B2);
}

//=============================================================================================================
void S3dFreeMarkerConfidence(CslConfidence  *mc)
{
	free(mc->Err1);
	free(mc->Err2);
	free(mc->Used);
	free(mc->Locked);
	free(mc->ScoreCouple);
	if(mc->ScoreGlobal) free(mc->ScoreGlobal);
	free(mc->ScoreM1);
	free(mc->ScoreM2);

	for(int i=0; i<CSL_ConfIndex_N_FEATURES;i++)
	{
		if(mc->Conf[i].Confidence != 0)
		{
			free(mc->Conf[i].Confidence);
			free(mc->Conf[i].Meas);
			free(mc->Conf[i].Estim);
		}
	}
	free(mc->Conf);
}
//=============================================================================================================
//=============================================================================================================
void S3dAllocMarkerConfidence(CslConfidence  *mc)
{
	int n = mc->N;
	mc->Used = (int *)malloc(n*sizeof(int));

	mc->Conf = (CslFeatConfidence *)malloc(CSL_ConfIndex_N_FEATURES * sizeof(CslFeatConfidence));
	for(int i = 0; i < CSL_ConfIndex_N_FEATURES; i++)
	{
		mc->Conf[i].Confidence = (float *)malloc(n * sizeof(float));
		mc->Conf[i].Meas = (float *)malloc(n * sizeof(float));
		mc->Conf[i].Estim = (float *)malloc(n * sizeof(float));
		for(int j=0;j<n;j++)
		{
			mc->Conf[i].Confidence[j] = -1;
			mc->Conf[i].Meas[j] = -1;
			mc->Conf[i].Estim[j] = -1;
		}
	}
	mc->Locked = (int *)malloc(n*sizeof(int));
	mc->Err1 = (float *)malloc(n*sizeof(float));
	mc->Err2 = (float *)malloc(n*sizeof(float));
	mc->ScoreCouple = (float *)malloc(n*sizeof(float));
	mc->ScoreM1 = (float *)malloc(n*sizeof(float));
	mc->ScoreM2 = (float *)malloc(n*sizeof(float));
	mc->ScoreGlobal = (float *)malloc(n*sizeof(float));
}
//=============================================================================================================

