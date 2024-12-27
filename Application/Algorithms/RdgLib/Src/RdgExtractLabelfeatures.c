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
#include <string.h> 
#include <math.h> 
#include <windows.h> 
#include <RdgDev.h> 




//**************************************************************************
// RdgIsCenterLine :
// Determine if the pixel corresponds to a center-line
//**************************************************************************
int RdgIsCenterLine(float **Rdg, int Y, int X, float Angle)
{
	int   ii, Flag = 0, nPts;
	int   x1, y1, x2, y2, x3, y3, x4, y4;
	float Profile[3];
	float X0, Y0, DX, DY;
	float I1, I2, I3, I4, dx, dy, x, y;

	int   nStep = 1;
	float Step = (float)0.5;

	nPts = 2*nStep +1;

	X0 = (float)X;
	Y0 = (float)Y;
	DX = Step*(float)cos(Angle);
	DY = Step*(float)sin(Angle);

	for(ii=0;ii<nPts;ii++)
	{
		// calculation of the float pixels along the profile 
		x = X0 + (float)(ii - nStep) * DX;
		y = Y0 + (float)(ii - nStep) * DY;

		// bilinear interpolation to obtain the ridge value
		x1 = (int)x; y1 = (int)y;
		x2 = x1+1;   y2 = y1;
		x3 = x1;     y3 = y1+1;
		x4 = x1+1;   y4 = y1+1;
		dx = x-x1;   dy = y-y1;
		I1 = Rdg[y1][x1];
		I2 = Rdg[y2][x2];
		I3 = Rdg[y3][x3];
		I4 = Rdg[y4][x4];
		Profile[ii] = I1*(1-dx)*(1-dy) + I2*dx*(1-dy) + I3*(1-dx)*dy + I4*dx*dy;
	}
	if(Rdg[Y][X] >= Profile[0] && Rdg[Y][X] >= Profile[2])
		Flag = 1;

	return Flag;
}

//**************************************************************************
// RdgComputeProfile :
// Compute a float profile from an image and an angle
//**************************************************************************
int RdgComputeProfile(int nStep, float Step, float *Profile, short **Ori, int i, int j, int Iw, int Ih, float Angle)
{
	float X0, Y0, DX, DY;
	int x1, y1, x2, y2, x3, y3, x4, y4;
	float I1, I2, I3, I4, dx, dy, x, y;
	int ii;

	X0 = (float)j;
	Y0 = (float)i;
	DX = Step*(float)cos((double)Angle);
	DY = Step*(float)sin((double)Angle);

	for(ii=0; ii<2*nStep+1; ii++)
	{
		x = X0 - (float)nStep * DX + (float)ii * DX;
		y = Y0 - (float)nStep * DY + (float)ii * DY;
		if(x>0 && x<Iw-1 && y>0 && y<Ih-1)
		{
			x1 = (int)x; y1 = (int)y;
			x2 = x1+1;   y2 = y1;
			x3 = x1;     y3 = y1+1;
			x4 = x1+1;   y4 = y1+1;
			dx = x-x1;   dy = y-y1;
			I1 = Ori[y1][x1];
			I2 = Ori[y2][x2];
			I3 = Ori[y3][x3];
			I4 = Ori[y4][x4];
			Profile[ii] = I1*(1-dx)*(1-dy) + I2*dx*(1-dy) + I3*(1-dx)*dy + I4*dx*dy;
		}
		else return -1;
	}
	return 0;
}


//PL 2004-01-03
//**************************************************************************
// RdgComputeProfileModeling :
// Compute the model profile corresponding to the input profile and
// output caracteristics of this model profile
//**************************************************************************
int RdgComputeProfileModeling(int nStep, float *F, int *p_a, float *p_g1, float *p_g2, float *p_e)
{
	float sum, sum1, sum2;
	int i, j, i1, i2, jMin;
	float *Val, *g1Tab, *g2Tab;
	int a; 
	float MinVal, g1, g2;
	float e; // normalised residual error

	Val = (float *)malloc(nStep * sizeof(float));
	g1Tab = (float *)malloc(nStep * sizeof(float));
	g2Tab = (float *)malloc(nStep * sizeof(float));

	for(j=0;j<nStep;j++)
	{
		i1 = nStep - j;
		i2 = nStep + j;
		sum = (float)0;
		for(i=nStep-j; i<= nStep+j; i++) sum += F[i];
		g1 = sum/(float)(2*j+1);

		sum = (float)0;
		for(i=0; i<= nStep-j; i++) sum += F[i];
		for(i=nStep+j; i<= nStep+nStep; i++) sum += F[i];
		g2 = sum /(float)(2*(nStep-j+1));

		sum1 = (float)0;
		for(i=nStep-j; i<= nStep+j; i++) sum1 += (F[i]-g1)*(F[i]-g1);
		sum1 /= (float)(2*j+1);

		sum2 = (float)0;
		for(i=0; i<= nStep-j; i++) sum2 += (F[i]-g2)*(F[i]-g2);
		for(i=nStep+j; i<= 2*nStep; i++) sum2 += (F[i]-g2)*(F[i]-g2);
		sum2 /= (float)(2*(nStep-j+1));

		Val[j] = sum1+sum2;
		g1Tab[j] = g1;
		g2Tab[j] = g2;
	}

	MinVal = Val[0];
	jMin=0;
	for(j=1;j<nStep;j++)
	{
		if(Val[j]<MinVal)
		{
			MinVal = Val[j];
			jMin = j;
		}
	}

	a = jMin;
	g1 = g1Tab[jMin];
	g2 = g2Tab[jMin];

	free(Val);
	free(g1Tab);
	free(g2Tab);

	*p_a  = a;
	*p_g1 = g1;
	*p_g2 = g2;

	if(g2-g1 == (float)0)
	{
		*p_e = (float)0;
		return 1;
	}
	else
		e = MinVal/((g2-g1)*(g2-g1));

	*p_e = e;

	return 0;
}




//**************************************************************************
// RdgComputeThicknessFromOri :
// Compute the thickness, sharpness and contrast around a given pixel
// belonging to	a ridge in the original image by fitting with a model profile
//**************************************************************************
int RdgComputeThicknessFromOri(short **Ori, int Y, int X, int Iw, int Ih, float Step, int nStep, float Angle, float *Thickness, float *Sharpness, float *Contrast)
{
	int   nPts, a;
	float g1, g2, e;
	float *Profile;

	nPts = 2*nStep +1;

	Profile = (float *)malloc(nPts * sizeof(float));

	// calculate profile with a resolution of Step on a length of 1+2*nStep
	if(RdgComputeProfile(nStep, Step, Profile, Ori, Y, X, Iw, Ih, Angle) == -1)
	{
		free(Profile);
		return -1;
	}

	// calculate model half size a
	RdgComputeProfileModeling(nStep, Profile, &a, &g1, &g2, &e);

	*Thickness = (float)(2*a) * Step;
	*Contrast = RDG_MAX(g2 - g1, 0);
	*Sharpness = e;

	free(Profile);
	return 0;
}




//**************************************************************************
// RdgCalcCenterLinePelList :
// Calculate the center-line pixel list from the object pixel list.
// The center-line pixels are determined, with their thickness, 
// sharpness and contrast (if ExtraMeasuresFlag==true).
//**************************************************************************
void RdgCalcCenterLinePelList(CRdgObjPelList *OPelList, CRdgCenterLinePelList *CLPelList, CRdgMeasures *Measures, 
	short **Ori, CRdgVi **Pel, float **Rdg, float **Dir, int IwOri, int IhOri, int SSFactor, int ExtraMeasuresFlag)
{
	int   k, X, Y, XSub, YSub, Idx = 0;
	float Angle, OrthAngle, pi_2 = (float)RDG_PI*(float).5;
	float SumT = 0, SumS = 0, SumC = 0;

	int   Size = RDG_MAX(RDG_MAX(IwOri, IhOri) / 50, 10); // overall size of support for profile calculation depending of the original image size (in pixels)
	float Step = (float)0.1;
	int   nStep = (int)((float)Size/((float)2 * Step));

	for(k=0; k<OPelList->NbPel; k++)
	{
		YSub = OPelList->ObjPels[k].ObjPel.Y;
		XSub = OPelList->ObjPels[k].ObjPel.X;
		if(SSFactor > 1)
		{
			Y = Pel[YSub][XSub].Y;
			X = Pel[YSub][XSub].X;
		}
		else
		{
			Y = YSub;
			X = XSub;
		}
		Angle = Dir[YSub][XSub];
		if(Angle > 0) OrthAngle = Angle - pi_2;
		else          OrthAngle = Angle + pi_2;

		// determine if pixel belongs to center-line
      if(RdgIsCenterLine(Rdg, YSub, XSub, OrthAngle))
      {
         if (ExtraMeasuresFlag)
         {
            float Thickness, Sharpness, Contrast;

            // compute thickness, sharpness and contrast of center-line pixel
            if(RdgComputeThicknessFromOri(Ori, Y, X, IwOri, IhOri, Step, nStep, OrthAngle, &Thickness, &Sharpness, &Contrast) == 0)
            {
               SumT += Thickness;
               SumS += Sharpness;
               SumC += Contrast;

               // filling center-line pixel structure
               CLPelList->CenterLinePels[Idx].CenterLinePel.X = X;
               CLPelList->CenterLinePels[Idx].CenterLinePel.Y = Y;
               CLPelList->CenterLinePels[Idx].Thickness = Thickness;
               CLPelList->CenterLinePels[Idx].Sharpness = Sharpness;
               CLPelList->CenterLinePels[Idx].Contrast  = Contrast;
               CLPelList->CenterLinePels[Idx].ObjPelIdx = k;

               // filling object pixel structure
               OPelList->ObjPels[k].CenterLinePelIdx = Idx;

               Idx++;
            }
            else
               OPelList->ObjPels[k].CenterLinePelIdx = -1;
         }
         else
         {
            // filling center-line pixel structure
            CLPelList->CenterLinePels[Idx].CenterLinePel.X = X;
            CLPelList->CenterLinePels[Idx].CenterLinePel.Y = Y;
            CLPelList->CenterLinePels[Idx].ObjPelIdx = k;

            // filling object pixel structure
            OPelList->ObjPels[k].CenterLinePelIdx = Idx;

            Idx++;
         }
      }
      else
      {
         // filling object pixel structure
         OPelList->ObjPels[k].CenterLinePelIdx = -1;
      }
	}

	CLPelList->NbPel = Idx;

   if (ExtraMeasuresFlag)
   {
      if(Idx == 0)
      {
         free(CLPelList->CenterLinePels);
         Measures->Thickness = (float)-1;
         Measures->Sharpness = (float)-1;
         Measures->Contrast  = (float)-1;
      }
      else
      {
         Measures->Thickness = SumT / (float)Idx;
         Measures->Sharpness = SumS / (float)Idx;
         Measures->Contrast  = SumC / (float)Idx;
      }
   }
   else
   {
      if(Idx == 0)
      {
         free(CLPelList->CenterLinePels);
      }
   }
}
