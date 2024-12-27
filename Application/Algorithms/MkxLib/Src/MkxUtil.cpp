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

#include <MkxDev.h> 

/*************************************************************************************/
// MkxCheckRoiCoherency
/*************************************************************************************/
int MkxCheckRoiCoherency(CMkxRoiCoord Roi)
{
	if(Roi.XMin > Roi.XMax || Roi.YMin > Roi.YMax)
		return 1;
	else
		return 0;
}

/*************************************************************************************/
// Clipping Roi
/*************************************************************************************/
int MkxClipRoi(CMkxRoiCoord *DstRoi, CMkxRoiCoord SrcRoi, CMkxRoiCoord ClippingRoi )
{
	if(MkxCheckRoiCoherency(ClippingRoi))
		return 1;
	if(MkxCheckRoiCoherency(SrcRoi))
		return 1;

  MkxRoiCoord(
    DstRoi, MKX_MIN(MKX_MAX(SrcRoi.XMin, ClippingRoi.XMin), ClippingRoi.XMax),
    MKX_MIN(MKX_MAX(SrcRoi.YMin, ClippingRoi.YMin), ClippingRoi.YMax),
    MKX_MAX(MKX_MIN(SrcRoi.XMax, ClippingRoi.XMax), ClippingRoi.XMin),
    MKX_MAX(MKX_MIN(SrcRoi.YMax, ClippingRoi.YMax), ClippingRoi.YMin)
    );

	return 0;
}
/*************************************************************************************/
// Set Roi
/*************************************************************************************/
int MkxSetRoiCoord(CErrHdl ErrHdl, float xMinFac, float xMaxFac, float yMinFac, float yMaxFac, CMkxRoiCoordFac* RoiCoordFac)
{
	int   Err = 0;
	char  Reason[128];
	char  FuncName[] ="MkxSetRoiCoord";

	if(xMinFac>=xMaxFac || yMinFac>=yMaxFac || xMaxFac<=0 || xMinFac>=1  || yMaxFac<=0 || yMinFac>=1 )
	{
		Err = MKX_WRN_CLIPPED_ARG;
		sprintf(Reason, "Wrong Roi from (%g %g) to (%g, %g), set to default (0,0)->(1,1)", xMinFac, yMinFac ,xMaxFac, yMaxFac);
		ErrStartErrorReportFromLeaf(ErrHdl, FuncName, Err, Reason);
		RoiCoordFac->XMinFac = 0;
		RoiCoordFac->XMaxFac = 1;
		RoiCoordFac->YMinFac = 0;
		RoiCoordFac->YMaxFac = 1;		
	}
	else if(xMinFac<0 || xMaxFac>1 || yMinFac<0 || yMaxFac>1 )
	{
		Err = MKX_WRN_CLIPPED_ARG;
		RoiCoordFac->XMinFac = MKX_MAX(0,xMinFac);
		RoiCoordFac->XMaxFac = MKX_MIN(1,xMaxFac);
		RoiCoordFac->YMinFac = MKX_MAX(0,yMinFac);
		RoiCoordFac->YMaxFac = MKX_MIN(1,yMaxFac);
		sprintf(Reason, "Wrong Roi from (%g %g) to (%g, %g), set to (%g, %g)->(%g, %g)", 
			xMinFac, yMinFac ,xMaxFac, yMaxFac, RoiCoordFac->XMinFac, RoiCoordFac->YMinFac ,RoiCoordFac->XMaxFac, RoiCoordFac->YMaxFac);
		ErrStartErrorReportFromLeaf(ErrHdl, FuncName, Err, Reason);
	}
	else
	{
		RoiCoordFac->XMinFac = xMinFac;
		RoiCoordFac->XMaxFac = xMaxFac;
		RoiCoordFac->YMinFac = yMinFac;
		RoiCoordFac->YMaxFac = yMaxFac;
	}
	return Err;
}


/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
void MkxExtractResultsPrint(CMkxExtractResults *er)
{
	printf("\nMkxExtractResultsPrint\n\n");

	CMkxAdvancedResults *adv = &er->Advanced;
	printf("X1Refined      =%8.2f    Y1Refined      =%8.2f    X2Refined =%8.2f    Y2Refined =%8.2f \n",er->X1Refined, er->Y1Refined, er->X2Refined, er->Y2Refined);
	printf("Status         =%8d												                 \n",er->Status);                              
	printf("Locked         =%8d												                 \n",er->Locked);                              
	printf("RoiXMin        =%8d    RoiYMin        =%8d    RoiXMax   =%8d    RoiYMax   =%8d	 \n",er->RoiXMin, er->RoiYMin, er->RoiXMax, er->RoiYMax);  
	printf("RoiXMin1       =%8d    RoiYMin1       =%8d    RoiXMax1  =%8d    RoiYMax1  =%8d	 \n",er->RoiXMin1, er->RoiYMin1, er->RoiXMax1, er->RoiYMax1);
	printf("RoiXMin2       =%8d    RoiYMin2       =%8d    RoiXMax2  =%8d    RoiYMax2  =%8d	 \n",er->RoiXMin2, er->RoiYMin2, er->RoiXMax2, er->RoiYMax2);
	printf("TwoRoisFlag    =%8d										                         \n",er->TwoRoisFlag);                         
	printf("WireStatus     =%8d											                     \n",er->WireStatus);                          
	printf("Radius1        =%8.2f    Radius2        =%8.2f									 \n",er->Advanced.Radius1, er->Advanced.Radius2);                    
	printf("SwStatus       =%8d										                         \n",er->SwStatus);                      
	printf("Confidence     =%8.2f\n", er->Confidence);
	printf("Cx             =%8.2f    Cy             =%8.2f    Br        =%8.2f    An        =%8.2f\n", adv->Cx, adv->Cy, adv->Br, adv->An);

	//printf("X1Raw          =%8d    Y1Raw          =%8d    X2Raw     =%8d    Y2Raw     =%8d   \n", adv->X1Raw, adv->Y1Raw, adv->X2Raw, adv->Y2Raw);                  
	//printf("X1Track        =%8d    Y1Track        =%8d    X2Track   =%8d    Y2Track   =%8d   \n", adv->X1Track, adv->Y1Track, adv->X2Track, adv->Y2Track);          
	printf("Dir1           =%8.2f    Dir2           =%8.2f                                     \n", adv->Dir1, adv->Dir2);                                  
	printf("Dir1Confidence =%8.2f    Dir2Confidence =%8.2f                                     \n", adv->Dir1Confidence, adv->Dir2Confidence);              
	printf("Length1        =%8.2f    Thickness1     =%8.2f    Length2   =%8.2f    Thickness2=%8.2f \n", adv->Length1, adv->Thickness1, adv->Length2, adv->Thickness2);    
	printf("Intens1        =%8.2f    Intens2        =%8.2f                                     \n", adv->Intens1, adv->Intens2);                            
	printf("LevelIn1       =%8.2f    LevelOut1      =%8.2f    LevelIn2  =%8.2f    LevelOut2 =%8.2f    RefiningConf1=%8.2f    RefiningConf2=%8.2f\n", adv->LevelIn1, adv->LevelOut1, adv->LevelIn2, adv->LevelOut2, adv->RefiningConf1, adv->RefiningConf2);
}


int MkxRoiCoord(CMkxRoiCoord *dstRoi, int xMin, int yMin, int xMax, int yMax)
{
	if(xMin < 0 || xMax < xMin || yMin < 0 || yMax < yMin) return 1;

	CMkxRoiCoord roi;

	roi.XMin = xMin;
	roi.YMin = yMin;
	roi.XMax = xMax;
	roi.YMax = yMax;
	roi.Width = xMax-xMin+1;
	roi.Height = yMax-yMin+1;

	*dstRoi = roi;

	return 0;
}

int MkxRoiCoordInit(CMkxRoiCoord *dstRoi)
{
  MkxRoiCoord(dstRoi,0,0,0,0);
	return 0;
}


void MkxRoiCoPrint(CMkxRoiCoord roi)
{
	printf("xMin: %4d yMin: %4d xMax: %4d yMax: %4d\n", roi.XMin, roi.YMin, roi.XMax, roi.YMax);
}

void	SetExtractWorkRoi(int Margin, CMkxRoiCoord src, CMkxRoiCoord *dst)
{
  // enlarge for excursion, gauss filtering and correlation
  CMkxRoiCoord tmp;
  MkxRoiCoord(&tmp, src.XMin - Margin,src.YMin - Margin,src.XMax + Margin,src.YMax + Margin);

  // WorkRoiCoord size must be a multiple of 4, 
  // for the SSE Gaussian calculation
  if (tmp.Width % 4)
  {
    if (tmp.Width % 4 == 3){ tmp.XMin += 1; tmp.XMax -= 2; }
    else if (tmp.Width % 4 == 2){ tmp.XMin += 1; tmp.XMax -= 1; }
    else                               { tmp.XMax -= 1; }
  }
  if (tmp.Height % 4)
  {
    if (tmp.Height % 4 == 3){ tmp.YMin += 1; tmp.YMax -= 2; }
    else if (tmp.Height % 4 == 2){ tmp.YMin += 1; tmp.YMax -= 1; }
    else                                { tmp.YMax -= 1; }
  }
  MkxRoiCoord(dst, tmp.XMin, tmp.YMin, tmp.XMax, tmp.YMax);
}
