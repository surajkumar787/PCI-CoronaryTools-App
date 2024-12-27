// ***************************************************************************
// * Copyright (c) 2002-2014 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


/*************************************************************************************/
// MkxMarkerDirection.c  PL  2005-11-04
/*************************************************************************************/
#include <stdio.h> 
#include <stdlib.h> 
#include <math.h> 

#include <MkxDev.h> 

static void MkxEigenFromInertiaMatrix(float J[][3], float V[][3], float D[], float *Dir1, float *S1, float *Dir2, float *S2)
{
	float a, b, c, d, det;

	a = J[0][0];
	b = J[1][1];
	c = J[2][2];
	d = J[1][0];

	det = (float)sqrt((double)((a-b)*(a-b)+4*d*d));

  V[0][0] = a-b-det; // 1st eigenvector associated to D[0]
	V[1][0] = 2*d;
	V[2][0] = 0;
	V[0][1] = a-b+det; // 2nd eigenvector associated to D[1]
	V[1][1] = 2*d;
	V[2][1] = 0;
	V[0][2] = 0;       // 3rd eigenvector associated to D[2]
	V[1][2] = 0;
	V[2][2] = 1;
	D[0] = (float)0.5 * (a+b-det);
	D[1] = (float)0.5 * (a+b+det);
	D[2] = c;
	*Dir1 = (float)atan2((double)V[1][0],(double)V[0][0]); //PL_110919
	*Dir2 = (float)atan2((double)V[1][1],(double)V[0][1]);
	*S1 = D[0];
	*S2 = D[1];
}

static void MkxInertiaMatrix(float *Roi, int sy, int sx, float J[][3])
{
	float Sx, Sy, sum, Cx, Cy, Sxx, Syy, Sxy, Jxx, Jyy, Jzz, Jxy;
	int i, j;

	//compute barycentre
	Sx = Sy = sum = 0;
	for(i=0;i<sy;i++)
	{
		for(j=0;j<sx;j++)
		{
			Sx += Roi[i*sx + j] * j;
			Sy += Roi[i*sx + j] * i;
			sum += Roi[i*sx + j];
		}
	}
	Cx = Sx/sum;
	Cy = Sy/sum;

	//compute elements
	Sxx = Syy = Sxy = 0;
	for(i=0;i<sy;i++)
	{
		for(j=0;j<sx;j++)
		{
			Sxx += Roi[i*sx + j] * (j - Cx)*(j - Cx);
			Syy += Roi[i*sx + j] * (i - Cy)*(i - Cy);
			Sxy += Roi[i*sx + j] * (j - Cx)*(i - Cy);
		}
	}
	Jxx = Syy;
	Jyy = Sxx;
	Jzz = Sxx + Syy;
	Jxy = - Sxy;

	J[0][0] = Jxx;
	J[0][1] = Jxy;
	J[0][2] = 0;
	J[1][0] = Jxy;
	J[1][1] = Jyy;
	J[1][2] = 0;
	J[2][0] = 0;
	J[2][1] = 0;
	J[2][2] = Jzz;
}

static void MkxClosestAngle(float Ang0, float Ang1, float *Ang)
{
	float Angle, MinVal, Val;
	int k, kMin;

	MinVal=MKX_ABS(Ang1-Ang0);
	kMin=2;
	for(k=0;k<5;k++)
	{
		Val = MKX_ABS(Ang1+(k-2)*180-Ang0);
		if(Val < MinVal){MinVal=Val; kMin=k;}
	}
	Angle = Ang1+(kMin-2)*180;

	MinVal=MKX_ABS(Angle);
	kMin=1;
	for(k=0;k<3;k++)
	{
		Val = MKX_ABS(Angle+(k-1)*360);
		if(Val < MinVal){MinVal=Val; kMin=k;}		
	}
	*Ang = Angle + (kMin-1)*360;
}

static void MkxWeightedLinearFit(float *Y, float *W, int N, float *a, float *b)
{
	float Denom, Sx, Sy, Sxy, Sxx, Sw;
	int i;
	if(N==1)
	{
		*a = 0;
		*b = Y[0];
	}
	Sx = Sy = Sxx = Sxy = Sw = 0;
	for(i=0;i<N;i++)
	{
		Sx += i*W[i];
		Sy += Y[i]*W[i];;
		Sxx += i*i*W[i];;
		Sxy += i*Y[i]*W[i];
		Sw += W[i];
	}
	Denom = Sx*Sx - Sw*Sxx;
	*a = (Sy*Sx-Sw*Sxy)/Denom;
	*b = (Sxy*Sx-Sy*Sxx)/Denom;
}

// returned Ang is between -180 and 180
static void MkxGetOneMarkerDir(const short *Ori, int IwOri, int IhOri, int HalfSize, int X, int Y, float *DirDeg, float *DirConfidence)
{
	int i, j, i0, j0, i1, j1, SizeX, SizeY, k, N, sx, sy, sxmin, symin;
	short *Roi1;
	short *RoiInv1, ValMax, ValMin;
	float J[3][3], *Ang, a, b, V[3][3], D[3], Dir1, S1, Dir2, S2, *Index;

	j0    = MKX_MAX(0, X-HalfSize);
	j1    = MKX_MIN(IwOri-1, X+HalfSize);
	i0    = MKX_MAX(0, Y-HalfSize);
	i1    = MKX_MIN(IhOri-1, Y+HalfSize);
	SizeX = j1-j0+1;
	SizeY = i1-i0+1;

  Roi1=(short *)malloc(SizeX* SizeY*sizeof(short));
  RoiInv1=(short *)malloc(SizeX* SizeY*sizeof(short));

	for(i=0;i<SizeY;i++)
		for(j=0;j<SizeX;j++)
			Roi1[j+i*SizeX]=Ori[j+j0+(i+i0)*IwOri];

	ValMax = ValMin = Roi1[0];
	for(i=0;i<SizeY;i++)
		for(j=0;j<SizeX;j++)
		{
			ValMin = MKX_MIN(ValMin, Roi1[j+i*SizeX]);
			ValMax = MKX_MAX(ValMax, Roi1[j+i*SizeX]);
		}
	for(i=0;i<SizeY;i++)
		for(j=0;j<SizeX;j++)
			RoiInv1[j+i*SizeX]=ValMax-Roi1[j+i*SizeX];

	k=0;
	sx = SizeX;
	sy = SizeY;
	sxmin = MKX_MAX(4, (SizeX/4));
	symin = MKX_MAX(4, (SizeY/4));
	while(sx>sxmin&&sy>symin){
		sx -= 2;
		sy -= 2;
		k ++;
	}
	N = k;
	Ang = (float *)malloc(N * sizeof(float));
	Index = (float *)malloc(N * sizeof(float));

	for(k = 0; k < N; k++)
	{
		i0 = j0 = k;
		sx = SizeX - 2*k;
		sy = SizeY - 2*k;

		float *JJ1 = (float*)malloc(sx*sy*sizeof(float));
		for(i=0;i<sy;i++)
			for(j=0;j<sx;j++)
				JJ1[i*sx+j]=(float)RoiInv1[j+j0+(i+i0)*SizeX]; 

		MkxInertiaMatrix(JJ1, sy, sx, J);
		MkxEigenFromInertiaMatrix(J, V, D, &Dir1, &S1, &Dir2, &S2);

		float ang = (float) (180/MKX_PI)*Dir1;
		if(k==0){
			if     ( MKX_ABS(ang-180) < MKX_ABS(ang) ) ang -= 180;
			else if( MKX_ABS(ang+180) < MKX_ABS(ang) ) ang += 180;
		}
		else{
			if     ( MKX_ABS(ang-180-Ang[k-1]) < MKX_ABS(ang-Ang[k-1]) ) ang -= 180;
			else if( MKX_ABS(ang+180-Ang[k-1]) < MKX_ABS(ang-Ang[k-1]) ) ang += 180;
		}
		Ang[k] = ang;
		Index[k] = 1-S1/S2;

		free(JJ1);
	}
	if(N==1)
	{
		*DirDeg = Ang[0]; 
		*DirConfidence = 0;
	}
	else
	{
	MkxWeightedLinearFit(Ang, Index, N, &a, &b);
		*DirDeg = (float)0.6*a*N+b; //take the measurement in the middle
		*DirConfidence = MKX_ABS(a);
	}
  if(Roi1){free(Roi1);Roi1=0;}
  if(RoiInv1){free(RoiInv1);RoiInv1=0;}

	free(Ang);
	free(Index);
}

void MkxMarkerDirection(
  const short *Ori, int IwOri, int IhOri,
  int X1, int Y1, int X2, int Y2,
  float* dir1, float* dir2, float* dir1Confidence, float* dir2Confidence,
  float BlobEnhRingRadiusInPix, int SSFactor
  )
{
  int HalfSize;
  float Dir1, Dir2, Ang0, Ang1, Ang;
  double Dir1To2;

  HalfSize = (int)(BlobEnhRingRadiusInPix)+2; //empirical value (typ. HalfSize = 6)
  if (SSFactor > 1)HalfSize *= SSFactor;
  HalfSize = MKX_MAX(1, HalfSize); //PL_070427

  MkxGetOneMarkerDir(Ori, IwOri, IhOri, HalfSize, X1, Y1, &Dir1, dir1Confidence);
  MkxGetOneMarkerDir(Ori, IwOri, IhOri, HalfSize, X2, Y2, &Dir2, dir2Confidence);

  if (X2 > X1)    Dir1To2 = atan((double)(Y2 - Y1) / (double)(X2 - X1));              //PL_060509
  else if (X2<X1) Dir1To2 = atan((double)(Y2 - Y1) / (double)(X2 - X1)) + MKX_PI;
  else            Dir1To2 = (Y2>Y1) ? (MKX_PI / (float)2) : (-(MKX_PI / (float)2));

  Dir1To2 *= 180 / MKX_PI;

  Ang0 = (float)(Dir1To2 + 180);
  Ang1 = Dir1;
  MkxClosestAngle(Ang0, Ang1, &Ang);
  *dir1 = Ang;

  Ang0 = (float)Dir1To2;
  Ang1 = Dir2;
  MkxClosestAngle(Ang0, Ang1, &Ang);
  *dir2 = Ang;
}


/************************************************************************************/
// End of file
/*************************************************************************************/
