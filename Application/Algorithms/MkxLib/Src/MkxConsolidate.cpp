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

#include <MkxDev.h>
#include <MkxConsolidate.h>

//-------------------------------------------------------------------------------------
// CslGetResults: export all results through an array 
void CslGetResults(CMkxExtractResults* inOutRes, int n, Csl* csl)
{
	for(int i = 0; i < n; i++)
	{
		inOutRes[i].Confidence = csl->CslStatus[i] == MKX_CST_EXTRACT_RES_OK ? csl->Confidence[i] : -1 ;
		inOutRes[i].Advanced.Cx = csl->Cx[i]; inOutRes[i].Advanced.Cy = csl->Cy[i];
		inOutRes[i].Advanced.Br = csl->Br[i]; inOutRes[i].Advanced.An = csl->An[i];
		inOutRes[i].X1Refined= csl->X1[i]; inOutRes[i].Y1Refined = csl->Y1[i];
		inOutRes[i].X2Refined= csl->X2[i]; inOutRes[i].Y2Refined = csl->Y2[i];
	}
}

void CslUpdateStatusFunc(int n, CMkxSkipFromConfidencePara para, float *conf, CMkxExtractionStatus *status)
{
	int   *flag         = (int*)calloc(n, sizeof(int));
	int   *best         = (int*)calloc(n, sizeof(int));//best[0] is the index of the best result (highest confidence)

  //sort according to confidence
	for(int i=0;i<n;i++)
	{
		float max=-1000;		int jMax=i; //PL_110902 
		for(int j=0;j<n;j++)
		{
			if(flag[j]==0&&conf[j]>max)
			{
				max = conf[j];				jMax = j;
			}
		}
		best[i]=jMax;		flag[jMax]=1;	
	}
	//
	int actualNb=para.MinNbImages;
	if(para.Mode == MKX_CST_SKIPFROMCONFIDENCE_THRESHOLDBASEDSKIPPING)
		for(int i=para.MinNbImages;i<n;i++)		if(conf[best[i]]>=para.Threshold)actualNb++;

	if(para.Mode == MKX_CST_SKIPFROMCONFIDENCE_THRESHOLDBASEDSKIPPING)
	{
		int cnt=0;
		for(int i=0;i<n;i++)
		{
			int j=best[i];
			if(cnt<para.MinNbImages)
			{
				if(status[j] == MKX_CST_EXTRACT_RES_OK)cnt++; //count the number of not skipped results up to minNbIma
			}
			else
			{
				if(conf[j]<para.Threshold) status[j]=MKX_CST_EXTRACT_RES_SKIP; //skip if the ranking is higher than minNbIma and conf below threshold
			}
		}
	}
	if(para.Mode == MKX_CST_SKIPFROMCONFIDENCE_NBBESTBASEDSKIPPING||para.Mode == MKX_CST_SKIPFROMCONFIDENCE_PERCENTBESTBASEDSKIPPING)
	{
		if(para.Mode == MKX_CST_SKIPFROMCONFIDENCE_NBBESTBASEDSKIPPING)
			actualNb = para.NbBest;
		else if(para.Mode == MKX_CST_SKIPFROMCONFIDENCE_PERCENTBESTBASEDSKIPPING)
		{
			int nNotSkip = 0;
			for(int i=0;i<n;i++)				if(status[i]== MKX_CST_EXTRACT_RES_OK)nNotSkip++;
			actualNb = MKX_MAX(para.MinNbImages, MKX_RND(nNotSkip*para.PercentBest/100.0f));//PL_110823
		}
		int cnt=0;
		for(int i=0;i<n;i++)
		{
			int j=best[i];
			if(cnt<actualNb)
			{
				if(status[j] == MKX_CST_EXTRACT_RES_OK)cnt++; //count the number of not skipped results up to actualNb
			}
			else
			{
				status[j]=MKX_CST_EXTRACT_RES_SKIP; //skip if the ranking is higher than actualNb
			}
		}
	}

	free(best);
	free(flag);
}

void CslUpdateStatus(CMkxExtractResults* inOutRes, int n, CMkxSkipFromConfidencePara skipFromConfPara)
{
	if(skipFromConfPara.Mode==MKX_CST_SKIPFROMCONFIDENCE_NOSKIPPING)return;//nothing more to be skipped!

	//get the number of non skipped images
	int nNotSkip=0;
	for(int i=0;i<n;i++)
	{
		if(inOutRes[i].Status==MKX_CST_EXTRACT_RES_OK) nNotSkip++;
	}
	if((skipFromConfPara.Mode==MKX_CST_SKIPFROMCONFIDENCE_PERCENTBESTBASEDSKIPPING || skipFromConfPara.Mode==MKX_CST_SKIPFROMCONFIDENCE_THRESHOLDBASEDSKIPPING) && nNotSkip<=skipFromConfPara.MinNbImages)return;//nothing more to be skipped!//PL_160308
	if(skipFromConfPara.Mode==MKX_CST_SKIPFROMCONFIDENCE_NBBESTBASEDSKIPPING && nNotSkip<=skipFromConfPara.NbBest) return;

	CMkxExtractionStatus   *statusIn  = (CMkxExtractionStatus *)malloc(n*sizeof(CMkxExtractionStatus));
	CMkxExtractionStatus   *statusOut = (CMkxExtractionStatus *)malloc(n*sizeof(CMkxExtractionStatus));
	float *conf      = (float *)malloc(n*sizeof(float));

	for(int i=0;i<n;i++)
	{
		conf[i]=inOutRes[i].Confidence;
		statusOut[i]=statusIn[i]=inOutRes[i].Status;
	}

	CslUpdateStatusFunc(n, skipFromConfPara, conf, statusOut);

	for(int i=0;i<n;i++)
	{
		inOutRes[i].Status = statusOut[i];
	}

	free(conf);
	free(statusIn);
	free(statusOut);
}

//-------------------------------------------------------------------------------------
// MkxConsolidate: inOutRes must be client allocated to nbIma instances
int MkxConsolidate(CMkxHdl MkxHdl, int nbIma,  CMkxExtractResults *inOutRes)
{
	CMkx *This = (CMkx*)(MkxHdl.Pv);

	if(nbIma<1||inOutRes==NULL)
	{
		int err = MKX_ERR_FUNC_ARG;
		char Reason[256];
		char FuncName[] = "MkxConsolidate";
		sprintf(Reason, "Wrong argument: NbIma=%d and InOutTab=%p", nbIma, inOutRes);
		ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, err, Reason);
		return MKX_ERR_FUNC_ARG;
	}
	Csl csl;
	CslCreate(&csl, nbIma, inOutRes);
	CslXyToCba(&csl); //add Cx, Cy, Br, An fields

	CslConfidence mc;
	mc.N = csl.N;
	S3dAllocMarkerConfidence(&mc);
	CslUpdateConfidence(&csl, &mc); // add features and confidence on features

#ifdef CSL_DEBUG_MX
	CslMxShowConfidence(&mc, "dummy");
#endif
	S3dFreeMarkerConfidence(&mc);

	CslPolarize(&csl);
	CslXyToCba(&csl);

	CslGetResults(inOutRes, nbIma, &csl);

	CslFree(&csl);

	return 0;
}

//-------------------------------------------------------------------------------------
// Fill the status in the NbIma instances of InOutTab
int  MkxGetSkipFromConfidence(CMkxHdl MkxHdl,  CMkxSkipFromConfidencePara skipFromConfPara, int nbIma, CMkxExtractResults *inOutRes)
{
	CMkx *This = (CMkx*)(MkxHdl.Pv);

	if(nbIma<1||inOutRes==NULL)
	{
		int err = MKX_ERR_FUNC_ARG;
		char Reason[256];
		char FuncName[] = "MkxGetSkipFromConfidence";
		sprintf(Reason, "Wrong argument: NbIma=%d and InOutRes=%p", nbIma, inOutRes);
		ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, err, Reason);
		return MKX_ERR_FUNC_ARG;
	}

	if(skipFromConfPara.Mode != MKX_CST_SKIPFROMCONFIDENCE_NOSKIPPING)
		CslUpdateStatus(inOutRes, nbIma, 		skipFromConfPara);

	return 0;
}
