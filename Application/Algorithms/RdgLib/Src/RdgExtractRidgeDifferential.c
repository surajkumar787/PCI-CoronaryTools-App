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
#include <assert.h>
#include <windows.h> 
#include <RdgDev.h>
#include <RdgGaussian.h>

#define EPS         1e-07


static int Bo1 = 2;  // image border

// HessKernel parameters
static float Lambda = (float)0.33;
static float Ro = (float)2;
static float Beta = (float)1.5;


/*************************************************************************************/
// calculate gaussian filtered image (with possibly pre-normalization by LF)
/*************************************************************************************/
void RdgNormAndCalcGaussIm(short **Ori, float **Gau, int Iw, int Ih, int DynOri, int NormOriFlag, float LFSig, 
                           double GaussSig, float OriSig)
{
   int    i, j;
   float  **Big;
   
   int    SizeForDim[3];
   //int    BorderMinForDim[] = {0, 0, 0};
   //int    BorderMaxForDim[] = {0, 0, 0};
   //double PixelSizeForDim[] = {1.0, 1.0, 1.0};

#if RDG_PRINT_FOR_DEBUG
   printf("RdgNormAndCalcGaussIm:  OriSig=%f\n", OriSig);
#endif

   SizeForDim[0]=Iw; SizeForDim[1]=Ih; SizeForDim[2]=1;

   // normalization of the original image (division by LF)
   if(NormOriFlag)
   {
      float dark_region_penalty_factor = (float) 1.0/(1<<(DynOri-1)); // in fact, 1.0/((2^DynOri)/2)

#if RDG_USE_SSE_GAUSSIAN
      RdgImAlignedAlloc(0, &Big, sizeof(float), Iw, Ih, 16);
#else
      RdgImAlloc       (0, &Big, sizeof(float), Iw, Ih);
#endif
      for(i=0; i<Ih; i++) for(j=0; j<Iw; j++) Big[i][j] = Ori[i][j];

      if (LFSig!=0)
      {
#if RDG_USE_SSE_GAUSSIAN
          if (Iw%4==0)
              RdgIsoGaussFilterFloat2D_SSE(Big[0], Big[0], LFSig, Iw, Ih);
          else
              RdgIsoGaussFilterFloat2D(Big[0], Big[0], LFSig, Iw, Ih);
#else
          RdgIsoGaussFilterFloat2D(Big[0], Big[0], LFSig, Iw, Ih);
#endif
      }

      for(i=0; i<Ih; i++) 
         for(j=0; j<Iw; j++) 
         {
            //				Gau[i][j] = RDG_MAX((float)Ori[i][j], OriSig) / RDG_MAX(Big[i][j], 1);                    PL 2004-01-03
            //				Gau[i][j] = RDG_MIN(RDG_MAX((float)Ori[i][j], 2*OriSig) / RDG_MAX(Big[i][j], 1), 1);
            Gau[i][j] = RDG_MIN(RDG_MAX((float)Ori[i][j], 2*OriSig) / RDG_MAX(Big[i][j], 32), 1);
            //				Gau[i][j] = RDG_MIN(RDG_MAX((float)Ori[i][j], 10*OriSig) / RDG_MAX(1000, 1), 1);

            // Added by SV 14/03/05
            // if the current region is dark, limit the effect of normalization and leave it dark
            if (Big[i][j]<(1<<(DynOri-1)))
               Gau[i][j] *= (Big[i][j]*dark_region_penalty_factor);
         }

#if RDG_USE_SSE_GAUSSIAN
         RdgImAlignedFree(0, Big);
#else
         RdgImFree(0, Big);
#endif
   }
   // copy of Ori in Gau
   else
   {
      for(i=0; i<Ih; i++) for(j=0; j<Iw; j++) Gau[i][j] = Ori[i][j];
   }

   // Gaussian calculation
   if (GaussSig!=0)
   {
#if RDG_USE_SSE_GAUSSIAN
       if (Iw%4==0)
           RdgIsoGaussFilterFloat2D_SSE(Gau[0], Gau[0], (float) GaussSig, Iw, Ih);
       else
           RdgIsoGaussFilterFloat2D(Gau[0], Gau[0], (float) GaussSig, Iw, Ih);
#else
       RdgIsoGaussFilterFloat2D(Gau[0], Gau[0], (float) GaussSig, Iw, Ih);
#endif
   }
}


/*************************************************************************************/
// Calculate Gaussian derivates only
/*************************************************************************************/
void RdgCalcGaussDerivOnly(float **Gau, int i, int j, float *Lx, float *Ly)
{

   *Lx  = (Gau[i  ][j+1] - Gau[i  ][j-1]) * (float).5;
   *Ly  = (Gau[i+1][j  ] - Gau[i-1][j  ]) * (float).5;
}


/*************************************************************************************/
// Calculate Gaussian derivates
/*************************************************************************************/
void RdgCalcGaussDeriv(float **Gau, int i, int j, float *Lx, float *Ly, float *Lxx, float *Lyy)
{
	int rdg_step_for_differentials = RDG_STEP_FOR_DIFFERENTIALS; //just to make compiler happy
   *Lx  = (Gau[i  ][j+1] - Gau[i  ][j-1]) * (float).5;
   *Ly  = (Gau[i+1][j  ] - Gau[i-1][j  ]) * (float).5;

   if(rdg_step_for_differentials == RDG_CST_ONE_STEP)
   {
      *Lxx = (Gau[i  ][j+2] + Gau[i  ][j-2] - 2*Gau[i][j]) * (float).25;
      *Lyy = (Gau[i+2][j  ] + Gau[i-2][j  ] - 2*Gau[i][j]) * (float).25;
   }
   else
   {
      *Lxx = Gau[i  ][j+1] + Gau[i  ][j-1] - 2*Gau[i][j];
      *Lyy = Gau[i+1][j  ] + Gau[i-1][j  ] - 2*Gau[i][j];
   }
}


/*************************************************************************************/
// Calculate Gaussian derivates and Lxy
/*************************************************************************************/
void RdgCalcGaussDeriv2(float **Gau, int i, int j, float *Lx, float *Ly, float *Lxx, float *Lyy, float *Lxy)
{
	int rdg_step_for_differentials = RDG_STEP_FOR_DIFFERENTIALS; //just to make compiler happy

   *Lx  = (Gau[i  ][j+1] - Gau[i  ][j-1]) * (float).5;
   *Ly  = (Gau[i+1][j  ] - Gau[i-1][j  ]) * (float).5;

   if(rdg_step_for_differentials == RDG_CST_ONE_STEP)
   {
      *Lxx = (Gau[i  ][j+2] + Gau[i  ][j-2] - 2*Gau[i][j]) * (float).25;
      *Lyy = (Gau[i+2][j  ] + Gau[i-2][j  ] - 2*Gau[i][j]) * (float).25;
   }
   else
   {
      *Lxx = Gau[i  ][j+1] + Gau[i  ][j-1] - 2*Gau[i][j];
      *Lyy = Gau[i+1][j  ] + Gau[i-1][j  ] - 2*Gau[i][j];
   }

   *Lxy = (Gau[i+1][j+1] + Gau[i-1][j-1] - Gau[i-1][j+1] - Gau[i+1][j-1]) * (float).25;
}




/*************************************************************************************/
// Calculate Gaussian Hessian: Lxx and Lyy
/*************************************************************************************/
void RdgCalcGaussHessian(float **Gau, int i, int j, float *Lxx, float *Lyy)
{
	int rdg_step_for_differentials = RDG_STEP_FOR_DIFFERENTIALS; //just to make compiler happy
   if(rdg_step_for_differentials == RDG_CST_ONE_STEP)
   {
      *Lxx = (Gau[i  ][j+2] + Gau[i  ][j-2] - 2*Gau[i][j]) * (float).25;
      *Lyy = (Gau[i+2][j  ] + Gau[i-2][j  ] - 2*Gau[i][j]) * (float).25;
   }
   else
   {
      *Lxx = Gau[i  ][j+1] + Gau[i  ][j-1] - 2*Gau[i][j];
      *Lyy = Gau[i+1][j  ] + Gau[i-1][j  ] - 2*Gau[i][j];
   }
}


/*************************************************************************************/
// Calculate Gaussian Hessian: Lxx, Lyy and Lxy
/*************************************************************************************/
void RdgCalcGaussHessian2(float **Gau, int i, int j, float *Lxx, float *Lyy, float *Lxy)
{
	int rdg_step_for_differentials = RDG_STEP_FOR_DIFFERENTIALS; //just to make compiler happy
   if(rdg_step_for_differentials == RDG_CST_ONE_STEP)
   {
      *Lxx = (Gau[i  ][j+2] + Gau[i  ][j-2] - 2*Gau[i][j]) * (float).25;
      *Lyy = (Gau[i+2][j  ] + Gau[i-2][j  ] - 2*Gau[i][j]) * (float).25;
   }
   else
   {
      *Lxx = Gau[i  ][j+1] + Gau[i  ][j-1] - 2*Gau[i][j];
      *Lyy = Gau[i+1][j  ] + Gau[i-1][j  ] - 2*Gau[i][j];
   }

   *Lxy = (Gau[i+1][j+1] + Gau[i-1][j-1] - Gau[i-1][j+1] - Gau[i+1][j-1]) * (float).25;
}


/*************************************************************************************/
// Calculate Gaussian Hessian trace
/*************************************************************************************/
void RdgCalcGaussHessianTrace(float **Gau, int i, int j, float *Trace)
{
 	int rdg_step_for_differentials = RDG_STEP_FOR_DIFFERENTIALS; //just to make compiler happy
  if(rdg_step_for_differentials == RDG_CST_ONE_STEP)
   {
      *Trace = (Gau[i+2][j  ] + Gau[i  ][j-2] - 4*Gau[i][j] + Gau[i  ][j+2] + Gau[i-2][j  ]) * (float).25;
   }
   else
   {
      *Trace = Gau[i-1][j  ] + Gau[i  ][j-1] - 4*Gau[i][j] + Gau[i  ][j+1] + Gau[i+1][j  ];
   }
}


/*************************************************************************************/
// Calculate Gaussian derivates and Hessian eigenvalues
/*************************************************************************************/
void RdgCalcGaussDerivAndHessEigenValues(float **Gau, int i, int j, 
                                         float *Lx, float *Ly, float *Lxx, float *Lyy, float *Lxy, float *L1, float *L2)
{
   float lxx, lyy, lxy, Det;

   RdgCalcGaussDeriv2(Gau, i, j, Lx, Ly, &lxx, &lyy, &lxy);
   *Lxx = lxx;   *Lyy = lyy;

   *Lxy = lxy;

   Det = (float)sqrt( (double)( RDG_SQ(lxx-lyy) + 4*RDG_SQ(lxy) ) );
   *L2  = (lxx + lyy - Det)*(float)0.5;
   *L1  = (lxx + lyy + Det)*(float)0.5;

   // added by SV
   // sort the eigenvalues by decreasing magnitude
   if (RDG_ABS(*L1)<RDG_ABS(*L2))
   {
      float tmp = *L1;
      *L1 = *L2;
      *L2 = tmp;
   }
}


/*************************************************************************************/
// Calculate Gaussian Hessian eigenvalues
/*************************************************************************************/
void RdgCalcGaussHessEigenValues(float **Gau, int i, int j, 
                                 float *Lxx, float *Lyy, float *Lxy, float *L1, float *L2)
{
   float lxx, lyy, lxy, Det;

   RdgCalcGaussHessian2(Gau, i, j, &lxx, &lyy, &lxy);
   *Lxx = lxx;   *Lyy = lyy;

   *Lxy = lxy;

   Det = (float)sqrt( (double)( RDG_SQ(lxx-lyy) + 4*RDG_SQ(lxy) ) );
   *L2  = (lxx + lyy - Det)*(float)0.5;
   *L1  = (lxx + lyy + Det)*(float)0.5;

   // added by SV
   // sort the eigenvalues by decreasing magnitude
   if (RDG_ABS(*L1)<RDG_ABS(*L2))
   {
      float tmp = *L1;
      *L1 = *L2;
      *L2 = tmp;
   }
}

/*************************************************************************************/
// Calculate Gaussian Hessian eigenvalues only
/*************************************************************************************/
void RdgCalcGaussHessEigenValuesOnly(float **Gau, int i, int j, 
                                     float *L1, float *L2)
{
   float lxx, lyy, lxy, Det;

   RdgCalcGaussHessian2(Gau, i, j, &lxx, &lyy, &lxy);

   Det = (float)sqrt( (double)( RDG_SQ(lxx-lyy) + 4*RDG_SQ(lxy) ) );
   *L2  = (lxx + lyy - Det)*(float)0.5;
   *L1  = (lxx + lyy + Det)*(float)0.5;

   // added by SV
   // sort the eigenvalues by decreasing magnitude
   if (RDG_ABS(*L1)<RDG_ABS(*L2))
   {
      float tmp = *L1;
      *L1 = *L2;
      *L2 = tmp;
   }
}


/*************************************************************************************/
// Calculate NoEdge value based on Lxx and Lyy (used to diminish the influence of edges vs ridges)
/*************************************************************************************/
void RdgCalcNoEdgeValWithLxxLyy(float Lx, float Ly, float Lxx, float Lyy, float InvGaussSig, float NoEdgeFac, float *NoEdgeVal)
{
   float  Buf, Epsi=(float)0.01;
   //	float  Buf, Epsi=(float)10;

   Buf = InvGaussSig * (float)sqrt((double)(RDG_SQ(Lx) + RDG_SQ(Ly))) / RDG_MAX(Lxx + Lyy, Epsi);  // (Lxx + Lyy) est un invariant (trace de la matrice du Hessien)
   *NoEdgeVal = (float)exp( (double)-NoEdgeFac*Buf );
}


/*************************************************************************************/
// Calculate NoEdge image (used to diminish the influence of edges vs ridges)
/*************************************************************************************/
void RdgCalcNoEdgeIm(short **Ori, float **NoEdgeIm, int Iw, int Ih, float KernelSig, 
                     float NoEdgeFac)
{
   int    i, j;
   float  **Gau;
   float  Lx, Ly, Lxx, Lyy, NoEdgeVal;

   int		 SizeForDim[3];
   //int		 BorderMinForDim[] = {0, 0, 0};
   //int		 BorderMaxForDim[] = {0, 0, 0};
   //double PixelSizeForDim[] = {1.0, 1.0, 1.0};
   double GaussSig          = (double)KernelSig;
   float  InvGaussSig       = 1 / KernelSig;

   SizeForDim[0]=Iw; SizeForDim[1]=Ih; SizeForDim[2]=1;

#if RDG_USE_SSE_GAUSSIAN
   RdgImAlignedAlloc(0, &Gau, sizeof(float), Iw, Ih, 16);
#else
   RdgImAlloc       (0, &Gau, sizeof(float), Iw, Ih);
#endif

   for(i=0; i<Ih; i++) for(j=0; j<Iw; j++) Gau[i][j] = Ori[i][j];

   if (GaussSig!=0)
   {
#if RDG_USE_SSE_GAUSSIAN
       if (Iw%4==0)
           RdgIsoGaussFilterFloat2D_SSE(Gau[0], Gau[0], (float) GaussSig, Iw, Ih);
       else
           RdgIsoGaussFilterFloat2D(Gau[0], Gau[0], (float) GaussSig, Iw, Ih);
#else
       RdgIsoGaussFilterFloat2D(Gau[0], Gau[0], (float) GaussSig, Iw, Ih);
#endif
   }

   for(i=Bo1;i<Ih-Bo1;i++) 
      for(j=Bo1;j<Iw-Bo1;j++)
      {
         RdgCalcGaussDeriv(Gau, i, j, &Lx, &Ly, &Lxx, &Lyy);

         RdgCalcNoEdgeValWithLxxLyy(Lx, Ly, Lxx, Lyy, InvGaussSig, NoEdgeFac, &NoEdgeVal);
         NoEdgeIm[i][j] = NoEdgeVal;
      }

#if RDG_USE_SSE_GAUSSIAN
      RdgImAlignedFree(0, Gau);
#else
      RdgImFree(0, Gau);
#endif
}



/*************************************************************************************/
// Differential ridge (based on determination of kernels from Hessian eigenvalues)
/*************************************************************************************/
void RdgRdgDifferential(short **Ori, 
                        float **Rwo, float **NoE, 
                        float **Gau, float **Rdg, float **Dir, int Iw, int Ih, int DynOri,
                        CRdgRoiCoord *RoiCoord, int RoiFlag, int NormOriFlag, float LFSig, 
                        int RidgeType, float KernelSig, 
                        int EdgeFromOriFlag, float NoEdgeFac, 
                        int DirImgIsNeeded, // necessary to have this flag, since non-maxima suppression requires direction image
                        int ThrType, float* pSig, float *L1Bg /* can be NULL */)
{
   int    i, j, I, J, Ex, Ey, N;
   //	float  **Gau;
   float  OriSig; // Noise in original picture, before filtering
   float  **K2 = NULL;
   float  lxx, lyy, lxy, L1, L2, Bias = 0;
   float  NoEdgeVal;

   float  Vx, Vy, Fangle, C1, S1;
   float  Fx, Fy, Ax, Ay, AxAy, K2a, K2b;
   float  Epsi=(float)0.01;

   double GaussSig = (double)KernelSig;
   //float  InvGaussSig = 1 / KernelSig;
   float  RoKernelSig = Ro * KernelSig;
   float  InvK3Norm = 1 / (2*Beta+1);

   int dBo = (int)(Ro*KernelSig) + 1;
   //int Bo2 = Bo1 + dBo;

   int IMin1 = (RoiFlag) ? (RDG_MAX(RoiCoord->YMin - dBo, Bo1))      : (Bo1);
   int IMax1 = (RoiFlag) ? (RDG_MIN(RoiCoord->YMax + dBo, Ih-1-Bo1)) : (Ih-1-Bo1);
   int JMin1 = (RoiFlag) ? (RDG_MAX(RoiCoord->XMin - dBo, Bo1))      : (Bo1);
   int JMax1 = (RoiFlag) ? (RDG_MIN(RoiCoord->XMax + dBo, Iw-1-Bo1)) : (Iw-1-Bo1);

   int IMin2 = IMin1 + dBo;
   int IMax2 = IMax1 - dBo;
   int JMin2 = JMin1 + dBo;
   int JMax2 = JMax1 - dBo;

   int NoEdgeFlag = (NoEdgeFac>Epsi);

   // flag to indicate that ridge directions will be computed in double bistouriquette, 
   // if possible (i.e. depending on current ridge type)
   int skip_angle_computation = !(RidgeType == RDG_CST_RDG_DIFFAL_HESSKERNEL_PLINE || RidgeType == RDG_CST_RDG_DIFFAL_ANGLE_DIFF || DirImgIsNeeded);

   // cache image for hessian angle vector components
   CRdgVf **CachedVComponents = NULL;

   if (pSig)
      OriSig = *pSig;
   else
      OriSig = 1;

   //
   // Ridge potential computation in 3 steps
   //   * compute Gaussian derivates and potentials (Rdg[][])
   //   * then compute ridge angles (Dir[][])
   //   * finally get ridge values for types that need the angles
   //

   // Behaviour estimation on Noise only (in case of noise dependent threshold applied)
   if(ThrType == RDG_CST_NOISE_DEP_THR && *pSig>=0)
   {
      float *Gau1, **Gau2;
      short *Noise1, **Noise2;
      float *RwoTp1, **RwoTp2;
      float *NoETp1, **NoETp2;
      float *NoiTp1, **NoiTp2;
      float *Resul1, **Resul2;
      float *Direc1, **Direc2;
      int    LocK, Border=10, Side = 250;
      float  DummySig=-1; // To stop iterative calling

#if RDG_USE_SSE_GAUSSIAN
      RdgImAlignedAlloc(&Gau1,   &Gau2,   sizeof(float), Side, Side, 16);
#else
      RdgImAlloc(&Gau1,   &Gau2,   sizeof(float), Side, Side);
#endif
      RdgImAlloc(&NoiTp1, &NoiTp2, sizeof(float), Side, Side);
      RdgImAlloc(&Noise1, &Noise2, sizeof(short), Side, Side);
      RdgImAlloc(&RwoTp1, &RwoTp2, sizeof(float), Side, Side);
      RdgImAlloc(&NoETp1, &NoETp2, sizeof(float), Side, Side);
      RdgImAlloc(&Resul1, &Resul2, sizeof(float), Side, Side);
      RdgImAlloc(&Direc1, &Direc2, sizeof(float), Side, Side);
      for(LocK=0; LocK<Side*Side; LocK++) Noise1[LocK]=128;
      RdgAddNoise(Noise1, Side, Side, *pSig);

      RdgRdgDifferential(Noise2, 
         RwoTp2, NoETp2,
         Gau2, Resul2, Direc2, Side, Side, DynOri, RoiCoord, 0, NormOriFlag, LFSig, 
         RidgeType, KernelSig, 
         EdgeFromOriFlag, NoEdgeFac, 
         FALSE, 
         ThrType, &DummySig, NULL);

      RdgEstimStat(Resul2, Side, Side, Border, &Bias, pSig);

#if RDG_USE_SSE_GAUSSIAN
      RdgImAlignedFree(Gau1, Gau2);
#else
      RdgImFree(Gau1, Gau2);
#endif
      RdgImFree(NoiTp1, NoiTp2);
      RdgImFree(Noise1, Noise2);
      RdgImFree(RwoTp1, RwoTp2);
      RdgImFree(NoETp1, NoETp2);
      RdgImFree(Resul1, Resul2);
      RdgImFree(Direc1, Direc2);
   }

   if (!skip_angle_computation)
   {
       // allocate the angle vector component cache
       RdgImAlloc(0, &CachedVComponents, sizeof(CachedVComponents[0][0]), Iw, Ih);
       memset(CachedVComponents[0], 0, Iw*Ih*sizeof(CachedVComponents[0][0]));

       RdgImAlloc(0, &K2,  sizeof(float), Iw, Ih);
   }

   // init
   for(i=0; i<Ih; i++)
      for(j=0; j<Iw; j++)
      {
         NoE[i][j] = 1;
      }
   memset(Rwo[0], 0, Iw*Ih*sizeof(Rwo[0][0]));
   memset(Rdg[0], 0, Iw*Ih*sizeof(Rdg[0][0]));
   memset(Dir[0], 0, Iw*Ih*sizeof(Dir[0][0]));

   // normalization of original image and 
   // calculation of the Gaussian image used to determine the eigenvalues
   RdgNormAndCalcGaussIm(Ori, Gau, Iw, Ih, DynOri, NormOriFlag, LFSig, GaussSig, OriSig);

   // NoEdge image calculation
   if(NoEdgeFlag && NormOriFlag && EdgeFromOriFlag)
      RdgCalcNoEdgeIm(Ori, NoE, Iw, Ih, KernelSig, NoEdgeFac);

   if (L1Bg) *L1Bg = 0;

#if RDG_PRINT_FOR_DEBUG
   printf("IMin1=%d, IMax1=%d, JMin1=%d, JMax1=%d, IMin2=%d, IMax2=%d, JMin2=%d, JMax2=%d\n", IMin1, IMax1, JMin1, JMax1, IMin2, IMax2, JMin2, JMax2);
#endif


   if (skip_angle_computation)
   {
      if (RidgeType == RDG_CST_RDG_DIFFAL_ANGLE_DIFF)
      {
         assert(0 && "should not get here");
      }
      else
      if (RidgeType == RDG_CST_RDG_DIFFAL_HESSIAN_TRACE)
      {
         for(i=IMin1;i<=IMax1;i++) 
            for(j=JMin1;j<=JMax1;j++)
            {
               float trace;

               // calculate Gaussian derivates 
               RdgCalcGaussHessianTrace(Gau, i, j, &trace);

               if(trace <= 0)
               {
               }
               else
               {
                  Rdg[i][j] = trace;

                  if(NoEdgeFlag && (!NormOriFlag || !EdgeFromOriFlag))
                  {
                     float Lx, Ly;

                     RdgCalcGaussDeriv(Gau, i, j, &Lx, &Ly, &lxx, &lyy);

                     // calculate NoEdge value (if not yet determined)
                     // RdgCalcNoEdgeValWithLxxLyy(Lx, Ly, lxx, lyy, InvGaussSig, NoEdgeFac, &NoEdgeVal);
                     RdgCalcNoEdgeValWithLxxLyy(Lx, Ly, lxx, lyy, 1.0 /* SV: filters are normalized at this point */, NoEdgeFac, &NoEdgeVal);
                     NoE[i][j] = NoEdgeVal;
                     Rwo[i][j] = Rdg[i][j];
                     Rdg[i][j] = Rdg[i][j] * NoEdgeVal;      
                  }
               }
               if (L1Bg) *L1Bg += RDG_SQ(trace);
            }

            if (L1Bg) 
            {
               N = (IMax1-IMin1+1)*(IMax2-IMin2+1);

               *L1Bg = (float)sqrt(((double)(*L1Bg))/(double)N);
            }
      }
      else 
      if (RidgeType == RDG_CST_RDG_DIFFAL_HESSKERNEL_PLINE)
      {
         assert(0 && "should not get here");
      }
      else
      // RidgeType is RDG_CST_RDG_DIFFAL_K1PLINE or RDG_CST_RDG_DIFFAL_K1_LAMBDAK2_PLINE or RDG_CST_RDG_DIFFAL_FRANGI
      {
         for(i=IMin1;i<=IMax1;i++) 
            for(j=JMin1;j<=JMax1;j++)
            {
               // calculate Gaussian Hessian eigenvalues
               RdgCalcGaussHessEigenValues(Gau, i, j, &lxx, &lyy, &lxy, &L1, &L2);

               if(L1 <= 0)
               {
               }
               else
               {
                  if(RidgeType == RDG_CST_RDG_DIFFAL_K1PLINE)
                     Rdg[i][j] = L1;
                  else 
                     if(RidgeType == RDG_CST_RDG_DIFFAL_K1_LAMBDAK2_PLINE)
                        Rdg[i][j] = L1 - L2 * Lambda;
                     else
                        // RDG_CST_RDG_DIFFAL_FRANGI
                        // beta = 1;
                        // c = 10;
                        // R=Lambda2./(Lambda1+.0001);
                        // S = abs(Lambda1);
                        // Ridgeness = exp(-(R.^2)/(2*beta^2)).*(1-exp(-(S)/(2*c^2)));
                        Rdg[i][j] = (float) (exp(-(RDG_SQ(L2/(L1+.0001f)))/(2))*(1-exp(-L1/(2*100))));

                  if(NoEdgeFlag && (!NormOriFlag || !EdgeFromOriFlag))
                  {
                     float Lx, Ly;

                     RdgCalcGaussDerivOnly(Gau, i, j, &Lx, &Ly);

                     // calculate NoEdge value (if not yet determined)
                     // RdgCalcNoEdgeValWithLxxLyy(Lx, Ly, lxx, lyy, InvGaussSig, NoEdgeFac, &NoEdgeVal);
                     RdgCalcNoEdgeValWithLxxLyy(Lx, Ly, lxx, lyy, 1.0 /* SV: filters are normalized at this point */, NoEdgeFac, &NoEdgeVal);
                     NoE[i][j] = NoEdgeVal;
                     Rwo[i][j] = Rdg[i][j];
                     Rdg[i][j] = Rdg[i][j] * NoEdgeVal;      
                  }
               }
               if (L1Bg) *L1Bg += RDG_SQ(L1);
            }

            if (L1Bg) 
            {
               N = (IMax1-IMin1+1)*(IMax2-IMin2+1);

               *L1Bg = (float)sqrt(((double)(*L1Bg))/(double)N);

#if RDG_PRINT_FOR_DEBUG
               printf("L1Bg = %f\n", *L1Bg);
#endif
            }
      }
   }
   else
   {
      if (RidgeType == RDG_CST_RDG_DIFFAL_ANGLE_DIFF)
      {
         for(i=IMin1;i<=IMax1;i++) 
            for(j=JMin1;j<=JMax1;j++)
            {
               // calculate Gaussian derivates 
               RdgCalcGaussHessian2(Gau, i, j, &lxx, &lyy, &lxy);

               // Squared Hessian components
               // LXX = lxx * lxx + lxy * lxy;
               // LYY = lyy * lyy + lxy * lxy;
               // LXY = lxy * (lxx + lyy);

               // 
               //       ________\	x             (the angle goes from x to y : indirect angle)
               //      |        /
               //      |
               //      |
               //      |
               //      \/
               //      y

               // store values in angle vector component cache
               CachedVComponents[i][j].X = lxx * lxx - lyy * lyy; // = LXX - LYY = lxx * lxx + lxy * lxy - lyy * lyy - lxy * lxy;
               CachedVComponents[i][j].Y = 2*(lxy * (lxx + lyy)); // = 2*LXY = 2*(lxy * (lxx + lyy));
            }
      }
      else
      if (RidgeType == RDG_CST_RDG_DIFFAL_HESSIAN_TRACE)
      {
         for(i=IMin1;i<=IMax1;i++) 
            for(j=JMin1;j<=JMax1;j++)
            {
               float trace;

               // calculate Gaussian derivates 
               RdgCalcGaussHessian2(Gau, i, j, &lxx, &lyy, &lxy);

               // Squared Hessian components
               // LXX = lxx * lxx + lxy * lxy;
               // LYY = lyy * lyy + lxy * lxy;
               // LXY = lxy * (lxx + lyy);

               // 
               //       ________\	x             (the angle goes from x to y : indirect angle)
               //      |        /
               //      |
               //      |
               //      |
               //      \/
               //      y

               // store values in angle vector component cache
               CachedVComponents[i][j].X = lxx * lxx - lyy * lyy; // = LXX - LYY = lxx * lxx + lxy * lxy - lyy * lyy - lxy * lxy;
               CachedVComponents[i][j].Y = 2*(lxy * (lxx + lyy)); // = 2*LXY = 2*(lxy * (lxx + lyy));

               // trace of hessian
               trace = lxx+lyy;

               if(trace <= 0)
               {
               }
               else
               {
                  Rdg[i][j] = trace;

                  if(NoEdgeFlag && (!NormOriFlag || !EdgeFromOriFlag))
                  {
                     float Lx, Ly;

                     RdgCalcGaussDerivOnly(Gau, i, j, &Lx, &Ly);

                     // calculate NoEdge value (if not yet determined)
                     // RdgCalcNoEdgeValWithLxxLyy(Lx, Ly, lxx, lyy, InvGaussSig, NoEdgeFac, &NoEdgeVal);
                     RdgCalcNoEdgeValWithLxxLyy(Lx, Ly, lxx, lyy, 1.0 /* SV: filters are normalized at this point */, NoEdgeFac, &NoEdgeVal);
                     NoE[i][j] = NoEdgeVal;
                  }
               }
               if (L1Bg) *L1Bg += RDG_SQ(trace);
            }

            if (L1Bg) 
            {
               N = (IMax1-IMin1+1)*(IMax2-IMin2+1);

               *L1Bg = (float)sqrt(((double)(*L1Bg))/(double)N);
            }
      }
      else 
      // RidgeType is RDG_CST_RDG_DIFFAL_K1PLINE, RDG_CST_RDG_DIFFAL_K1_LAMBDAK2_PLINE, RDG_CST_RDG_DIFFAL_HESSKERNEL_PLINE or RDG_CST_RDG_DIFFAL_FRANGI
      {
         for(i=IMin1;i<=IMax1;i++) 
            for(j=JMin1;j<=JMax1;j++)
            {
               // calculate Gaussian Hessian eigenvalues
               RdgCalcGaussHessEigenValues(Gau, i, j, &lxx, &lyy, &lxy, &L1, &L2);

               // Squared Hessian components
               // LXX = lxx * lxx + lxy * lxy;
               // LYY = lyy * lyy + lxy * lxy;
               // LXY = lxy * (lxx + lyy);

               // 
               //       ________\	x             (the angle goes from x to y : indirect angle)
               //      |        /
               //      |
               //      |
               //      |
               //      \/
               //      y

               // store values in angle vector component cache
               CachedVComponents[i][j].X = lxx * lxx - lyy * lyy; // = LXX - LYY = lxx * lxx + lxy * lxy - lyy * lyy - lxy * lxy;
               CachedVComponents[i][j].Y = 2*(lxy * (lxx + lyy)); // = 2*LXY = 2*(lxy * (lxx + lyy));

               if(L1 <= 0)
               {
               }
               else
               {
                  if(RidgeType == RDG_CST_RDG_DIFFAL_K1PLINE)
                     Rdg[i][j] = L1;
                  else
                  if(RidgeType == RDG_CST_RDG_DIFFAL_K1_LAMBDAK2_PLINE)
                     Rdg[i][j] = L1 - L2 * Lambda;
                  else
                     // RDG_CST_RDG_DIFFAL_FRANGI
                     // beta = 1;
                     // c = 10;
                     // R=Lambda2./(Lambda1+.0001);
                     // S = abs(Lambda1);
                     // Ridgeness = exp(-(R.^2)/(2*beta^2)).*(1-exp(-(S)/(2*c^2)));
                     Rdg[i][j] = (float) (exp(-(RDG_SQ(L2/(L1+.0001f)))/(2))*(1-exp(-L1/(2*100))));


                  if(NoEdgeFlag && (!NormOriFlag || !EdgeFromOriFlag))
                  {
                     float Lx, Ly;

                     RdgCalcGaussDerivOnly(Gau, i, j, &Lx, &Ly);

                     // calculate NoEdge value (if not yet determined)
                     // RdgCalcNoEdgeValWithLxxLyy(Lx, Ly, lxx, lyy, InvGaussSig, NoEdgeFac, &NoEdgeVal);
                     RdgCalcNoEdgeValWithLxxLyy(Lx, Ly, lxx, lyy, 1.0 /* SV: filters are normalized at this point */, NoEdgeFac, &NoEdgeVal);
                     NoE[i][j] = NoEdgeVal;
                     Rwo[i][j] = Rdg[i][j];
                     Rdg[i][j] = Rdg[i][j] * NoEdgeVal;      
                  }
               }
               if (L1Bg) *L1Bg += RDG_SQ(L1);
            }

            if (L1Bg) 
            {
               N = (IMax1-IMin1+1)*(IMax2-IMin2+1);

               *L1Bg = (float)sqrt(((double)(*L1Bg))/(double)N);

#if RDG_PRINT_FOR_DEBUG
               printf("L1Bg = %f\n", *L1Bg);
#endif
            }
      }
   }

      if (!skip_angle_computation)
      {
         if(RidgeType == RDG_CST_RDG_DIFFAL_HESSKERNEL_PLINE)
            // K2 will be needed in this case
            memcpy(K2[0], Rdg[0], Iw*Ih*sizeof(Rdg[0][0]));

         // Computation of angle
         for(i=IMin2;i<=IMax2;i++) 
            for(j=JMin2;j<=JMax2;j++)
            {
               // Double-angle averaging in window of size 3
               // (from CxDirectionHessAng2)
               Vx = Vy =(float)0.0;
               for(I=-1; I<=1; I++)
               {
                  for(J=-1; J<=1; J++)
                  {
                      // sum up the angle vector components
                      Vx -= (float) CachedVComponents[i+I][j+J].X; // (LXX - LYY);
                      Vy -= (float) CachedVComponents[i+I][j+J].Y; // (2*LXY); 
                  }
               }

               Dir[i][j] = Fangle = (float)(0.5*atan2((double) Vy, (double) Vx));

               // ridge computation
               if(RidgeType == RDG_CST_RDG_DIFFAL_HESSKERNEL_PLINE)
               {
                  C1 = (float)(cos((double) Fangle));  S1 = (float)(sin((double) Fangle));

                  // calculation of K2 on each side in the longitudinal direction 
                  // (K2a and K2b) with bilinear interpolation
                  Fx = j + RoKernelSig * C1;  Fy = i - RoKernelSig * S1;
                  Ex = (int)Fx;               Ey = (int)Fy;
                  Ax = Fx - Ex;               Ay = Fy - Ey;
                  AxAy = Ax * Ay;

                  //				K2a = Beta * ( (1-Ay)*(1-Ax)*K2[Ey][Ex] + Ay*(1-Ax)*K2[Ey+1][Ex] 
                  //					+ (1-Ay)*Ax*K2[Ey][Ex+1] + Ay*Ax*K2[Ey+1][Ex+1] );
                  K2a = Beta * ( (1-Ax-Ay+AxAy)*K2[Ey][Ex] + (Ay-AxAy)*K2[Ey+1][Ex] 
                  + (Ax-AxAy)*K2[Ey][Ex+1] + AxAy*K2[Ey+1][Ex+1] );

                  Fx = j - RoKernelSig * C1;  Fy = i + RoKernelSig * S1;
                  Ex = (int)Fx;               Ey = (int)Fy;
                  Ax = Fx - Ex;               Ay = Fy - Ey;
                  AxAy = Ax * Ay;

                  //				K2b = Beta * ( (1-Ay)*(1-Ax)*K2[Ey][Ex] + Ay*(1-Ax)*K2[Ey+1][Ex] 
                  //					+ (1-Ay)*Ax*K2[Ey][Ex+1] + Ay*Ax*K2[Ey+1][Ex+1] );
                  K2b = Beta * ( (1-Ax-Ay+AxAy)*K2[Ey][Ex] + (Ay-AxAy)*K2[Ey+1][Ex] 
                  + (Ax-AxAy)*K2[Ey][Ex+1] + AxAy*K2[Ey+1][Ex+1] );

                  // determination of K3 from the 3 values of K2 (1 central and 2 lateral)
                  Rdg[i][j] = (K2[i][j] + K2a + K2b) * InvK3Norm;

                  Rwo[i][j] = Rdg[i][j];
                  Rdg[i][j] = Rdg[i][j] * NoE[i][j];      
               }
            }

            // added by SV
            // use non-linear ridge estimator
            if(RidgeType == RDG_CST_RDG_DIFFAL_ANGLE_DIFF)
            {
               float step = (float) (3.0/2*GaussSig);
               int   n_values = 0;
               float L1max = 0;

               if (L1Bg) *L1Bg = 0;

               #if MX_FLAG

                  printf("MX_FLAG is defined\n");
                  MxOpen() ;
                  MxSetVisible(1) ;
                  MxCommand("cd D:/MatLab/Rdg");

                  MxPutMatrixFloat(Gau[0], Iw, Ih, "Gau");
                  MxPutMatrixFloat(Dir[0], Iw, Ih, "Dir");
#endif

               for(i=IMin2;i<=IMax2;i++) 
                  for(j=JMin2;j<=JMax2;j++)
                  {
                     float Profile[3];
                     float di = (float) ( step*cos(Dir[i][j])); // sin(dir+pi/2)
                     float dj = (float) (-step*sin(Dir[i][j])); // cos(dir+pi/2)
                     int absdi = (int) (ceil(RDG_ABS(di)));
                     int absdj = (int) (ceil(RDG_ABS(dj)));

                     if (i-absdi>=0 && i+absdi+1<Ih &&
                        j-absdj>=0 && j+absdj+1<Iw)
                     {
                        int k; 

                        for (k=-1; k<=1; k+=2)
                        {
                           float interpolated_i = i+k*di;
                           float interpolated_j = j+k*dj;
                           int   ul_i = (int) interpolated_i; // upper-left corner
                           int   ul_j = (int) interpolated_j; // upper-left corner
                           float coef_i = interpolated_i-ul_i;
                           float coef_j = interpolated_j-ul_j;

                           assert(coef_i>=0);
                           assert(coef_i<1);
                           assert(coef_j>=0);
                           assert(coef_j<1);

                           Profile[k+1] = 
                              Gau[ul_i  ][ul_j  ]*(1-coef_i)*(1-coef_j)+
                              Gau[ul_i  ][ul_j+1]*(1-coef_i)*(  coef_j)+
                              Gau[ul_i+1][ul_j+1]*(  coef_i)*(  coef_j)+
                              Gau[ul_i+1][ul_j  ]*(  coef_i)*(1-coef_j);
                        }

                        // Rdg[i][j] = RDG_MAX(RDG_MIN(Profile[2],Profile[0])-Profile[1],0);
                        Rdg[i][j] = RDG_MAX(RDG_MIN(Profile[2],Profile[0])-Gau[i][j],0);

                        if (L1Bg && Rdg[i][j]>EPS)
                        {
                           *L1Bg += RDG_SQ(Rdg[i][j]);
                           // *L1Bg += ABS(Rdg[i][j]);
                           n_values++;

                           if (Rdg[i][j]>L1max) L1max = Rdg[i][j];
                        }
                     }

                     Rwo[i][j] = NoE[i][j] = Rdg[i][j];
                  }

                  if (L1Bg) 
                  {
                     // N = (IMax2-IMin2+1)*(JMax2-JMin2+1);

                     *L1Bg = (float)sqrt(((double)(*L1Bg))/(double)n_values);
                     // *L1Bg /= n_values;

#if RDG_PRINT_FOR_DEBUG
                     printf("L1Bg = %f\n", *L1Bg);
                     printf("L1max = %f\n", L1max);
#endif

                     // Use max value of enhanced ridge
                     printf("USING MAX VALUE OF ENHANCED RIDGE\n");
                     *L1Bg = L1max;
                  }
            }
      }

      #if MX_FLAG

          printf("MX_FLAG is defined\n");
          MxOpen() ;
          MxSetVisible(1) ;
          MxCommand("cd D:/MatLab/Rdg");

          MxPutMatrixFloat(Rdg[0], Iw, Ih, "Rdg");
          MxCommand(" figure(1) ; imagesc(Rdg) ; axis off ; axis image ; colormap gray(256) ; ") ; 
#endif

      if (!skip_angle_computation)
      {
          RdgImFree(0, CachedVComponents);
          RdgImFree(0, K2);
      }
}
