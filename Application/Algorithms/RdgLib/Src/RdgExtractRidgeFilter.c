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
#include <malloc.h>
#include <assert.h>

#include <RdgDev.h> 

#define RDG_CST_DYN 32768



//************************************************************************************
// from rf_noise.c
//************************************************************************************
int RdgNoiseEstim2(short **Ima, float **Noi, int Iw, int Ih)
{
	int i, j, k, Histo[RDG_CST_DYN];
	int i1, i2, j1, j2, m, n, Fw=2;
	int Av1, Av2, MaxVal, MaxLoc, LocSig;
	float Fnorm, FBuf, Bias;
	float fLocSig;

	for(i=0;i<Ih;i++)for(j=0;j<Iw;j++)Noi[i][j]=0;
	

	i1 = j1 = Fw;
	i2 = Ih-Fw; j2 = Iw - Fw;
	Fnorm = 1/(float)RDG_SQ(2*Fw+1);
	Bias  = (float)sqrt( (double)((2*Fw+1)/(2*Fw)) );

	for(k=0; k<RDG_CST_DYN; k++) Histo[k] = 0;

	for(i=i1; i<i2; i+=1)
	{
		for(j=j1; j<j2; j+=1)
		{
			for(Av1=0, Av2=0, m=-Fw; m<=Fw; m++)
			{
				for(n=-Fw; n<=Fw; n++)
				{
					Av1 += Ima[i+m][j+n];
					Av2 += RDG_SQ(Ima[i+m][j+n]);
				}
			}
			FBuf   = RDG_MAX( (Av2*Fnorm - RDG_SQ(Av1*Fnorm)), 0);
			fLocSig = (float)(Bias*sqrt((double)FBuf) + 0.5);
			LocSig = (int)fLocSig;
			LocSig = RDG_MIN(LocSig, RDG_CST_DYN-1);
			Histo[LocSig]++;
//			Noi[i][j]=(float)log((double)(1+fLocSig));
			Noi[i][j]=fLocSig;
		}
	}

	MaxVal = Histo[1]; MaxLoc = 1;
	for(k=2; k<RDG_CST_DYN; k++)
	{
		if(MaxVal<Histo[k])
		{
			MaxVal = Histo[k];
			MaxLoc = k;
		}
	}

	return MaxLoc;
}


//************************************************************************************
void RdgEnhanceRidge(short **Ori, 
                     float **Rwo, float **NoE, 
                     float **Gau, float **Rdg, float **Dir, float **Noi, int Iw, int Ih, int DynOri,
                     CRdgRoiCoord *RoiCoord, int NormOriFlag, float LFSig, int RidgeType, 
                     float KernelSig, 
                     int EdgeFromOriFlag, float NoEdgeFac, 
                     int ThrType, 
                     int DirImgIsNeeded, 
                     float *NoiseSig, float *L1Bg /* can be NULL */)
{
    // NoiseSig is necessary for noise dependent threshold, and picture normalization
	if (ThrType == RDG_CST_NOISE_DEP_THR || NormOriFlag)
      (*NoiseSig) = (float) RdgNoiseEstim2(Ori, Noi, Iw, Ih);

	if (RidgeType == RDG_CST_RDG_DIFFAL_K1PLINE || 
        RidgeType == RDG_CST_RDG_DIFFAL_K1_LAMBDAK2_PLINE || 
        RidgeType == RDG_CST_RDG_DIFFAL_HESSKERNEL_PLINE ||
        RidgeType == RDG_CST_RDG_DIFFAL_HESSIAN_TRACE ||
        RidgeType == RDG_CST_RDG_DIFFAL_ANGLE_DIFF || 
        RidgeType == RDG_CST_RDG_DIFFAL_FRANGI)
	{
		RdgRdgDifferential(Ori, 
         Rwo, NoE, 
         Gau, Rdg, Dir, Iw, Ih, DynOri, RoiCoord, 1, NormOriFlag, LFSig, RidgeType, 
         KernelSig, 
         EdgeFromOriFlag, NoEdgeFac, 
         DirImgIsNeeded, 
         ThrType, NoiseSig, L1Bg);
	}
	else
	{
#if RDG_PRINT_FOR_DEBUG
		printf("Ridge mode : unknown mode\n");
#endif
      assert(0 && "should not get here");

		return;
	}
}



//************************************************************************************
void RdgEnhanceMultiScaleRidge(short **Ori, float **Gau, float **Rdg, float **Dir, unsigned char **Scale, int Iw, int Ih, int DynOri, CRdgRoiCoord *RoiCoord,
															 int NormOriFlag, float LFSig, int RidgeType, float *KernelSigs, int nKernelSigs, float gamma, int EdgeFromOriFlag, float NoEdgeFac)
{
   float ***localRwo, ***localNoE;
   float ***localGau, ***localRdg, ***localDir; 

   int IMin1 = RDG_MAX(RoiCoord->YMin, 0);
   int IMax1 = RDG_MIN(RoiCoord->YMax, Ih-1);
   int JMin1 = RDG_MAX(RoiCoord->XMin, 0);
   int JMax1 = RDG_MIN(RoiCoord->XMax, Iw-1);
   int k;

   localRwo = alloca(nKernelSigs*sizeof(localRwo[0]));
   localNoE = alloca(nKernelSigs*sizeof(localNoE[0]));
   localGau = alloca(nKernelSigs*sizeof(localGau[0]));
   localRdg = alloca(nKernelSigs*sizeof(localRdg[0]));
   localDir = alloca(nKernelSigs*sizeof(localDir[0]));

   for (k=0; k<nKernelSigs; k++)
   {
      RdgImAlloc(0, &localRwo[k], sizeof(localRwo[0][0][0]), Iw, Ih);
      RdgImAlloc(0, &localNoE[k], sizeof(localNoE[0][0][0]), Iw, Ih);
#if RDG_USE_SSE_GAUSSIAN
      RdgImAlignedAlloc(0, &localGau[k], sizeof(localGau[0][0][0]), Iw, Ih, 16);
#else
      RdgImAlloc(0, &localGau[k], sizeof(localGau[0][0][0]), Iw, Ih);
#endif      
      RdgImAlloc(0, &localRdg[k], sizeof(localRdg[0][0][0]), Iw, Ih);
      RdgImAlloc(0, &localDir[k], sizeof(localDir[0][0][0]), Iw, Ih);
   }

   // initialize the input pictures
   memset(&Rdg[0][0], 0, Iw*Ih*sizeof(Rdg[0][0]));

	if (RidgeType == RDG_CST_RDG_DIFFAL_K1PLINE || 
       RidgeType == RDG_CST_RDG_DIFFAL_K1_LAMBDAK2_PLINE || 
       RidgeType == RDG_CST_RDG_DIFFAL_HESSKERNEL_PLINE ||
       RidgeType == RDG_CST_RDG_DIFFAL_HESSIAN_TRACE ||
       RidgeType == RDG_CST_RDG_DIFFAL_ANGLE_DIFF || 
       RidgeType == RDG_CST_RDG_DIFFAL_FRANGI)
   {
      // get all potentials for all required scales 
      for (k=0;k <nKernelSigs; k++)
      {
         RdgRdgDifferential(Ori, 
            localRwo[k], localNoE[k], 
            localGau[k], localRdg[k], localDir[k], Iw, Ih, DynOri, RoiCoord, 1, NormOriFlag, LFSig, RidgeType, 
            KernelSigs[k], 
            EdgeFromOriFlag, NoEdgeFac, 
            TRUE, 
            RDG_CST_ABSOLUTE_THR /* dummy value here */, NULL, NULL);
      }

      // get the pixelwise maximum across scale
      for (k=0; k<nKernelSigs; k++)
      {
         const float normalization_factor = (float) exp(gamma*log(KernelSigs[k]));
         int i, j;

         for(i=IMin1;i<=IMax1;i++) 
            for(j=JMin1;j<=JMax1;j++)
            { 
               float ridgeness = localRdg[k][i][j]*normalization_factor;

               if (Rdg[i][j]<ridgeness)
               {
                  Rdg  [i][j] = ridgeness;
                  Scale[i][j] = (unsigned char)k;
               }
            }
      }

      // fill in remaining pictures
      {
         int i, j;

         for(i=IMin1;i<=IMax1;i++) 
            for(j=JMin1;j<=JMax1;j++)
            { 
               int best_scale = Scale[i][j];

               Gau[i][j] = localGau[best_scale][i][j];
               Dir[i][j] = localDir[best_scale][i][j];
#if RDG_USE_NOEDGE_FACTOR
               Rwo[i][j] = localRwo[best_scale][i][j];
               NoE[i][j] = localNoE[best_scale][i][j];
#endif
            }
      }
   }
	else
	{
#if RDG_PRINT_FOR_DEBUG
		printf("Ridge mode : unknown mode\n");
#endif
      assert(0 && "should not get here");
	}

   for (k=0;k <nKernelSigs; k++)
   { 
      RdgImFree(0, localRwo[k]);
      RdgImFree(0, localNoE[k]);
#if RDG_USE_SSE_GAUSSIAN
      RdgImAlignedFree(0, localGau[k]);
#else
      RdgImFree(0, localGau[k]);
#endif
      RdgImFree(0, localRdg[k]);
      RdgImFree(0, localDir[k]);
   }
}



//************************************************************************************
void RdgEstimStat(float **Ima, int Iw, int Ih, int Off, float *pMoy, float *pSig)
{
	int i, j, Nb;
	float Moy, Sig;

	Moy = Sig = (float)0.0;
	
	for(Nb=0, i=Off; i<Ih-Off; i++)
		for(j=Off; j<Iw-Off; j++)
		{
			Moy += Ima[i][j];
			Sig += RDG_SQ(Ima[i][j]);
			Nb++;
		}

	Moy = Moy/(float)Nb;
	Sig = Sig/(float)Nb;
	Sig = (float)sqrt( (double)RDG_MAX( Sig - RDG_SQ(Moy) , 0 ) );

	*pMoy = Moy;
	*pSig = Sig;
}




//************************************************************************************
void RdgSuppressNonMaxima(float **Rwo, float **Rdg, float **Dir, int Iw, int Ih, CRdgRoiCoord *RoiCoord)
{
	unsigned char **Tmp;
	int   i, j, di, dj, iMin = 1, iMax = Ih-2, jMin = 1, jMax = Iw-2;
	float Angle;
	float pi1_8 = (float)(RDG_PI / 8), pi3_8 = 3 * pi1_8;


	// alloc and init non-maxima image
	RdgImAlloc(0, &Tmp, sizeof(char), Iw, Ih);
    memset(Tmp[0], 0, Iw*Ih*sizeof(Tmp[0][0]));

	// ROI borders
	iMin = RDG_MAX(RoiCoord->YMin, iMin);
	iMax = RDG_MIN(RoiCoord->YMax, iMax);
	jMin = RDG_MAX(RoiCoord->XMin, jMin);
	jMax = RDG_MIN(RoiCoord->XMax, jMax);

	// fill non-maxima image (4 directions taken into account)
	for(i=iMin; i<=iMax; i++)
		for(j=jMin; j<=jMax; j++)
		{
			Angle = Dir[i][j];
			if(Angle <= -pi3_8 || Angle >= pi3_8)   // < -3*pi/8 or > 3*pi/8
			{
				di = 0;
				dj = 1;
			}
			else if(Angle <= -pi1_8)                // -3*pi/8 < ... < -pi/8
			{
				di = 1;
				dj = 1;
			}
			else if(Angle <= pi1_8)                 // -pi/8 < ... < pi/8
			{
				di = 1;
				dj = 0;
			}
			else                                    // pi/8 < ... < 3*pi/8
			{
				di = -1;
				dj = 1;
			}

			if(Rdg[i][j] >= Rdg[i+di][j+dj] && Rdg[i][j] >= Rdg[i-di][j-dj])
				Tmp[i][j] = 1;
		}


	// copy of non-maxima image in ridge image
	for(i=0; i<Ih; i++)
		for(j=0; j<Iw; j++)
		{
            if (!Tmp[i][j])
            {
                Rwo[i][j] = 0;
                Rdg[i][j] = 0;
            }
		}

	// free temporary image
	RdgImFree(0, Tmp);
}
