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

#include <MkxDev.h>

#include <algorithm>

static int MkxCountCouples(int NbLblBlobs)
{
    int m, k;

    for(m = 0, k = 0; k < NbLblBlobs; k++)
        m += (NbLblBlobs - k - 1);

    return m;
}

static void MkxBuildCouples(int NbLblBlobs, CMkxBlobCouple *Couple, int *NbCouples)
{
	int m, k, q;

	for(m = 0, k = 0; k < NbLblBlobs; k++)
		for(q = k+1; q < NbLblBlobs; q++)
		{
			Couple[m].Index1 = k;
			Couple[m].Index2 = q;
			Couple[m].Flag   = 1; //PL_080717 (instead of l that was in lieu of q)
			m++;
		}
		*NbCouples = m;
}

static int MkxLabelIsInRoi(CMkxBlob label, CMkxRoiCoord roiCoord)
{
	int ret, x, y;
	x = label.V.X;
	y = label.V.Y;
	ret = 0;
	ret |= (x >= roiCoord.XMin && x <= roiCoord.XMax &&  y >= roiCoord.YMin && y <= roiCoord.YMax); 

	return ret;
}



static void MkxSelectCouplesForTwoRois(CMkxRoiCoord roiCoord1, CMkxRoiCoord roiCoord2, int nbCouples,  CMkxBlobCouple *couple, const CMkxBlob *label)
{
	int m, i1, i2;
	for(m = 0; m < nbCouples; m++)
	{
		i1 = couple[m].Index1;
		i2 = couple[m].Index2;
		if(    MkxLabelIsInRoi(label[i1], roiCoord1) && MkxLabelIsInRoi(label[i2], roiCoord2)
			|| MkxLabelIsInRoi(label[i2], roiCoord1) && MkxLabelIsInRoi(label[i1], roiCoord2))
			couple[m].Flag = 1;
		else
			couple[m].Flag = 0;
	}

}

static void MkxSelectCouplesOnBreadth(int NbCouples,  CMkxBlobCouple *Couple, const CMkxBlob *Label, float breadthMin, float breadthMax)
{
	float breadthMin2 = (float) (breadthMin*breadthMin);
	float breadthMax2 = (float) (breadthMax*breadthMax);
	int m, i1, i2, X1, Y1, X2, Y2;
	float breadth2;

	for(m = 0; m < NbCouples; m++)
	{
		i1 = Couple[m].Index1;
		i2 = Couple[m].Index2;
		X1 = Label[i1].V.X;
		Y1 = Label[i1].V.Y;
		X2 = Label[i2].V.X;
		Y2 = Label[i2].V.Y;
		breadth2 = (float)((X2-X1)*(X2-X1)+(Y2-Y1)*(Y2-Y1));
		if(breadth2 >= breadthMin2 && breadth2 <= breadthMax2)
			Couple[m].Flag = 1;
		else
			Couple[m].Flag = 0;
	}
}

static void MkxReduceCoupleTable(int *NbCouples,  CMkxBlobCouple *Couple)
{
	int n, m;
	for(n = 0, m = 0; m < *NbCouples; m++)
	{
		if(Couple[m].Flag == 1)
		{
			Couple[n] = Couple[m]; //shallow copy, n<=m
			n++;
		}
	}
	*NbCouples = n;
}


static void MkxFillInCouples(int NbCouples,  CMkxBlobCouple *Couple, const CMkxBlob *Label)
{
	int m, i1, i2;

	for(m = 0; m < NbCouples; m++)
	{
		i1 = Couple[m].Index1;
		i2 = Couple[m].Index2;
		MkxBlobCopy(&Label[i1], &Couple[m].Blob1);
		MkxBlobCopy(&Label[i2], &Couple[m].Blob2);
	}
}

/***************************************************************************/
// CoupleIntensity :                                                       */
// Calculates the intensity of a label couple.                             */
/***************************************************************************/
static float MkxCoupleIntensityComput(float Intensity1, float Intensity2)
{
  float Intensity;

  Intensity = MKX_MIN(Intensity1, Intensity2) * (Intensity1 + Intensity2);
  return Intensity;
}


/***************************************************************************/
// PointTwoMarkersIntensity :                                              */
// Calculates the intensity for each couple of label.                      */
/***************************************************************************/
static void  MkxBlobCoupleIntensity(CMkxBlobCouple *RawCouples, int NbRawCouples)
{
  int   k;
  float MaxIntensity = 0;

  for (k = 0; k < NbRawCouples; k++)
  {
    float Intensity = MkxCoupleIntensityComput(RawCouples[k].Blob1.Intensity, RawCouples[k].Blob2.Intensity);
    RawCouples[k].CplMerit.CplMeritWrtBlobs = Intensity;

    if (Intensity > MaxIntensity)
      MaxIntensity = Intensity;
  }

  for (k = 0; k < NbRawCouples; k++)
  {
    RawCouples[k].CplMerit.CplMeritWrtBlobs /= MaxIntensity;
    RawCouples[k].CplMerit.CplMeritOverall = RawCouples[k].CplMerit.CplMeritWrtBlobs; //init overall
  }
}

/***************************************************************************/
// MarkerCoupleOrderByIntensity : Intensity-based ranking and                      */
// Determination of the rank of each couple                                */
/***************************************************************************/
void  MkxBlobCoupleOrderByIntensity(CMkxBlobCouple *RawCouples, int NbRawCouples)
{
  int i, j;
  CMkxBlobCouple Work;

  for (j = 1; j < NbRawCouples; j++)
  {
    MkxBlobCoupleCopy(&RawCouples[j], &Work);
    i = j - 1;
    while (i >= 0 && RawCouples[i].CplMerit.CplMeritOverall < Work.CplMerit.CplMeritOverall)
    {
      MkxBlobCoupleCopy(&RawCouples[i], &RawCouples[i + 1]);
      i--;
    }
    MkxBlobCoupleCopy(&Work, &RawCouples[i + 1]);
  }

  for (j = 0; j < NbRawCouples; j++)
    RawCouples[j].Rank = j + 1;
}


/*************************************************************************************/
// CouplesSelect 
/*************************************************************************************/
int MkxCouplesSelect(
  int nbLabels,
  const CMkxBlob *labels,
  int *nbCouples,
  CMkxBlobCouple *couples,
  int TwoRoisFlag, CMkxRoiCoord roi1, CMkxRoiCoord roi2,
  float distMin, float distMax,
  CPrmPara *ExtractParam
  )
{
	//reinit rawcouples
  int nCpl = MkxCountCouples(nbLabels);

	for(int i=0; i < nCpl; i++)
	{
		MkxBlobCoupleInit(&couples[i]);
	}

	MkxBuildCouples(nbLabels, couples, &nCpl);
	if(TwoRoisFlag)
	{
    MkxSelectCouplesForTwoRois(roi1, roi2, nCpl, couples, labels);
    MkxReduceCoupleTable(&nCpl, couples);
  }
  MkxSelectCouplesOnBreadth(nCpl, couples, labels, distMin, distMax);
  MkxReduceCoupleTable(&nCpl, couples);
  MkxFillInCouples(nCpl, couples, labels);

  // calculation of the intensity of each couple of labels
  MkxBlobCoupleIntensity(couples, nCpl);

	// fill in MarkerCouple fields and order couples following intensity
	MkxBlobCoupleOrderByIntensity(couples, nCpl);

  *nbCouples = nCpl;

  return 0;
}

void MkxCombineIntensities(int nbCouples, CMkxBlobCouple *couples, CPrmPara *extractParam)
{
	int i, flag = 1;

	if (extractParam->Val[MKX_CPLSELECTWIREFLAG_I].Int)
	{
		for(i=0;i<nbCouples;i++)
		{
			//if(couples[i].CplMerit.CplMeritWrtWireAvailable==1)
				couples[i].CplMerit.CplMeritOverall *= couples[i].CplMerit.CplMeritWrtWire;
		}
	}

	if(extractParam->Val[MKX_HISTORYFLAG_I].Int == MKX_CST_HISTORY_APPLYONLY || extractParam->Val[MKX_HISTORYFLAG_I].Int == MKX_CST_HISTORY_APPLYANDUPDATE)
	{
		for(i=0;i<nbCouples;i++)
		{
			flag &= couples[i].CplMerit.CplMeritWrtHistoryAvailable;
		}
		if(flag)
		{
			for(i=0;i<nbCouples;i++)
			{
				couples[i].CplMerit.CplMeritOverall *= couples[i].CplMerit.CplMeritWrtHistory;
			}
		}
	}
}
