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



//************************************************************************************
// from rf_noise.c
//************************************************************************************
#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

float RdgNoiseUnif(int *idum)

{
	int j;
	long k;
	static long iy=0;
	static long iv[NTAB];
	float temp;

	if (*idum <= 0 || !iy) 
	{

		if (-(*idum) < 1) *idum=1;
		else              *idum = -(*idum);

		for (j=NTAB+7;j>=0;j--) 
		{
			k=(*idum)/IQ;
			*idum=IA*(*idum-k*IQ)-IR*k;
			if (*idum < 0) *idum += IM;
			if (j < NTAB) iv[j] = *idum;
		}

		iy=iv[0];
	}

	k=(*idum)/IQ;
	*idum=IA*(*idum-k*IQ)-IR*k;
	
	if (*idum < 0) *idum += IM;
	
	j=iy/NDIV;
	iy=iv[j];
	iv[j] = *idum;

    /*
	if ((temp=(float)AM*iy) > RNMX) printf("Noise unif %f\n", (float)RNMX);
	else                            printf("Noise unif %f\n", (float)temp);
	*/

	if ((temp=(float)AM*iy) > RNMX) return (float)RNMX;
	else                            return temp;

}

#undef IA
#undef IM
#undef AM
#undef IQ
#undef IR
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX


//************************************************************************************
// from rf_noise.c
//************************************************************************************
float RdgNoiseGauss(int *idum)
{
	static int iset=0;
	static float gset;
	float fac,rsq,v1,v2;

	if(iset == 0) 
	{
		do 
		{
			v1=(float)2.0*RdgNoiseUnif(idum)-(float)1.0;
			v2=(float)2.0*RdgNoiseUnif(idum)-(float)1.0;
			rsq=v1*v1+v2*v2;
		} while (rsq >= 1.0 || rsq == 0.0);

		fac=(float)sqrt((double)(-2.0*(float)log((double)rsq)/rsq));
		gset=v1*fac;
		iset=1;
		return v2*fac;
	} 
	else 
	{
		iset=0;
		return gset;
	}

}


//************************************************************************************
void RdgAddNoise(short *Ori, int Iw, int Ih, float Sig)
{
	static int idum, AddNoiseFirst = 1;
	int k, Size, Noise;

	if(AddNoiseFirst)
	{
		idum = (int)-1;
		RdgNoiseGauss(&idum);
		AddNoiseFirst=0;
	}
	
	Size = Iw * Ih;
	for(k=0; k<Size; k++) 
	{
		Noise = (int)(Sig*RdgNoiseGauss(&idum) + (float)0.5);
		Ori[k] = (short)RDG_MAX(Ori[k] + Noise, 0);
	}

}




//************************************************************************************
// Sub-sample the original image of a factor "SSFactor"
//************************************************************************************
void RdgSubSampleImage(short **Ori, short **Sub, CRdgVi **Pel, int IwSub, int IhSub, int SSFactor)
{
	int i, j, i0, j0, k, l, k0, l0;

	if(SSFactor == 1)
	{
      memcpy(Sub[0], Ori[0], IwSub*IhSub*sizeof(Ori[0][0]));
	}
	else
	{
		for(i=0; i<IhSub; i++)
			for(j=0; j<IwSub; j++)
			{
				i0 = SSFactor * i;
				j0 = SSFactor * j;
				k0 = 0;
				l0 = 0;
				for(k=0; k<SSFactor; k++)
					for(l=0; l<SSFactor; l++)
					{
						if(Ori[i0+k][j0+l] < Ori[i0+k0][j0+l0])
						{
							k0 = k;
							l0 = l;
						}
					}
				Sub[i][j] = Ori[i0+k0][j0+l0];
				Pel[i][j].X = j0+l0;
				Pel[i][j].Y = i0+k0;
			}
	}
}


//************************************************************************************
// Piwel-wise performs Dst = Dst-Src and Dst<0 = 0
//************************************************************************************
void RdgSubtractImageAndKeepPositive(float **Dst, float **Src, int Iw, int Ih, CRdgRoiCoord *RoiCoord)
{
   int IMin1 = RDG_MAX(RoiCoord->YMin, 0);
   int IMax1 = RDG_MIN(RoiCoord->YMax, Ih-1);
   int JMin1 = RDG_MAX(RoiCoord->XMin, 0);
   int JMax1 = RDG_MIN(RoiCoord->XMax, Iw-1);

   int i, j;

   for(i=IMin1;i<=IMax1;i++) 
      for(j=JMin1;j<=JMax1;j++)
      { 
         float pixel = Dst[i][j]-Src[i][j];

         if (pixel<0) pixel = 0;

         Dst[i][j] = pixel;
      }
}
