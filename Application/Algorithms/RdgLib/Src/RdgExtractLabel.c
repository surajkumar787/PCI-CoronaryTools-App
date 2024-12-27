// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************
//PL_120426

#include <stdio.h> 
#include <string.h> 
#include <math.h> 
#include <windows.h> 
#include <assert.h>

#include <RdgDev.h> 

#define CNTMAX 1000

//**************************************************************************
// RdgExpand : expand the labelling procedure
// The iterative procedure begins at pixel (i0, j0). Lab[i0][j0]  equals
// "etiquette". The pixels which are sufficiently close to a pixel which
// value is "etiquette" are set to the value "etiquette".
// PixelCount is equal to the current number of pixels contained in the
// current label (only valid at the end of the expansion process
//**************************************************************************
int RdgExpand(int **Lab, float **Rdg, int Iw, int Ih, int i0, int j0, int range, 
              int etiquette, int *NbPel, float *CumRdg, int *cnt)
{
   int i,j;

   if((*cnt)++ > CNTMAX) 
      return(1);

   for(i=i0-range; i<=i0+range; i++)
      for(j=j0-range; j<=j0+range; j++)
         if(i >= range && i < Ih-range && j >= range && j < Iw-range)
            if(Lab[i][j] == 1)
            {
               (*NbPel)++;
               *CumRdg += Rdg[i][j];
               Lab[i][j] = etiquette;
               return RdgExpand(Lab, Rdg, Iw, Ih, i, j, range, etiquette, NbPel, CumRdg, cnt);
            }
            return 0;
}


//**************************************************************************
// RdgExpandDir : expand the directional labelling procedure
// The iterative procedure begins at pixel (i0, j0). Lab[i0][j0]  equals
// "etiquette". The pixels which are sufficiently close to a pixel which
// value is "etiquette" are set to the value "etiquette".
// PixelCount is equal to the current number of pixels contained in the
// current label (only valid at the end of the expansion process
//**************************************************************************
int RdgExpandDir(int **Lab, float **Rdg, float **Dir, int Iw, int Ih, int i0, int j0, int range, 
                 int MaxDirDiff, int etiquette, int *NbPel, float *CumRdg, int *cnt)
{
   int i,j, SmallAngleDiffFlag;
   float AbsAngleDiff, Angle1 = (float)RDG_PI * MaxDirDiff / 180, Angle2 = (float)RDG_PI - Angle1;

   if((*cnt)++ > CNTMAX) 
      return(1);

   for(i=i0-range; i<=i0+range; i++)
      for(j=j0-range; j<=j0+range; j++)
         if(i >= range && i < Ih-range && j >= range && j < Iw-range)
         {
            if(Lab[i][j] == 1)
            {
               AbsAngleDiff = RDG_ABS(Dir[i][j] - Dir[i0][j0]);
               SmallAngleDiffFlag = (AbsAngleDiff <= Angle1 || AbsAngleDiff >= Angle2);
               if(SmallAngleDiffFlag)
               {
                  (*NbPel)++;
                  *CumRdg += Rdg[i][j];
                  Lab[i][j] = etiquette;
                   return RdgExpandDir(Lab, Rdg, Dir, Iw, Ih, i, j, range, MaxDirDiff, etiquette, NbPel, CumRdg, cnt);
               }
            }
         }
         return(0);
}



//**************************************************************************
// RdgLabel : labelling
// InPut Image : Thr is a binary image
// Output Image : Lab contains the output labels
// (each labels corresponds to a value of Lab which is >= 2)
// *NbLabels contains the number of labels obtained
//**************************************************************************
void RdgLabel(unsigned char **Thr, int **Lab, float **Rdg, float **Dir, int Iw, int Ih, CRdgRoiCoord *RoiCoord, 
              int *NbLabels, int *NbPelPerLab, float *CumRdgPerLab, int NbRangePix, int DirLabelFlag, int MaxDirDiff, int TipLength)
{
   int i, j, i0, j0, k, etiquette = 2;
   int iMin = NbRangePix, iMax = Ih-NbRangePix-1, jMin = NbRangePix, jMax = Iw-NbRangePix-1;


   iMin = RDG_MAX(iMin, RoiCoord->YMin);
   iMax = RDG_MIN(iMax, RoiCoord->YMax);
   jMin = RDG_MAX(jMin, RoiCoord->XMin);
   jMax = RDG_MIN(jMax, RoiCoord->XMax);

   // initialization of Lab to Thr
   for(i=0; i<Ih; i++)
      for(j=0; j<Iw; j++)
         Lab[i][j] = (int)Thr[i][j];

   for(i0 = iMin; i0 <= iMax; i0++)
      for(j0 = jMin; j0 <= jMax; j0++)
         if(Lab[i0][j0] == 1)
         {
            int cnt;
            Lab[i0][j0] = etiquette;
            k = etiquette - 2;
            NbPelPerLab[k] = 1;
            CumRdgPerLab[k] = Rdg[i0][j0];
            cnt = 0;
            if(DirLabelFlag)
            {
               if(RdgExpandDir(Lab, Rdg, Dir, Iw, Ih, i0, j0, NbRangePix, MaxDirDiff, etiquette, &(NbPelPerLab[k]), &(CumRdgPerLab[k]), &cnt))
                  printf("RdgExpandDir: label too long: aborted\n");
            }
            else
            {
               if(RdgExpand(Lab, Rdg, Iw, Ih, i0, j0, NbRangePix, etiquette, &(NbPelPerLab[k]), &(CumRdgPerLab[k]), &cnt))
                  printf("RdgExpand:  label too long: aborted\n");
            }
            etiquette++;
         }

         *NbLabels = etiquette-2;
         for(k=0; k<*NbLabels; k++)
         {
            CumRdgPerLab[k] *= (float)TipLength / (float)RDG_MAX(TipLength, NbPelPerLab[k]);
         }
}



//**************************************************************************
// RdgLabelFromSeed : labelling from a seed binary image
// InPut Image : Seed is a binary image, Thr is a binary image
// Output Image : Lab contains the output labels
// (each labels corresponds to a value of Lab which is >= 2)
// *NbLabels contains the number of labels obtained
// 
// This version manages its own labelling recursion (quicker than passing many parameters as recursive arguments)
//**************************************************************************
void RdgLabelFromSeed(unsigned char **Seed, unsigned char **Thr, int **Lab, float **Rdg, float **Dir, int Iw, int Ih, CRdgRoiCoord *RoiCoord, 
                      int *NbLabels, int *NbPelPerLab, float *CumRdgPerLab, int *NbExtremitiesPerLab, const int NbRangePix, const int DirLabelFlag, const float MaxDirDiff, int TipLength)
{
   // directional labelling if DirLabelFlag AND MaxDirDiff<90 degrees
   const int localDirLabelFlag = (DirLabelFlag && MaxDirDiff<90 && MaxDirDiff>0);
   const float cos_angle = (float) cos(RDG_PI * MaxDirDiff / 180);
   int i, j, i0, j0, k, etiquette = 2;
   int iMin = NbRangePix, iMax = Ih-NbRangePix-1, jMin = NbRangePix, jMax = Iw-NbRangePix-1;
   int *stack_i, *stack_j, *stack_i0, *stack_j0, *stack_dead_recursions_flag, recursion_level, max_recursion_level;

   iMin = RDG_MAX(iMin, RoiCoord->YMin);
   iMax = RDG_MIN(iMax, RoiCoord->YMax);
   jMin = RDG_MAX(jMin, RoiCoord->XMin);
   jMax = RDG_MIN(jMax, RoiCoord->XMax);

   max_recursion_level = (iMax-iMin+1)*(jMax-jMin+1);//PL_120426

   stack_i  = (int *) malloc((max_recursion_level+1)*sizeof(stack_i [0]));//PL_120426
   stack_j  = (int *) malloc((max_recursion_level+1)*sizeof(stack_j [0]));
   stack_i0 = (int *) malloc((max_recursion_level+1)*sizeof(stack_i0[0]));
   stack_j0 = (int *) malloc((max_recursion_level+1)*sizeof(stack_j0[0]));
   stack_dead_recursions_flag = (int *) malloc((max_recursion_level+1)*sizeof(stack_dead_recursions_flag[0]));


   // initialization of Lab to Thr
   for(i=0; i<Ih; i++)
      for(j=0; j<Iw; j++)
         Lab[i][j] = (int)Thr[i][j];

   for(i = iMin; i <= iMax; i++)
   {
      for(j = jMin; j <= jMax; j++)
      {
         if(Seed[i][j]==1 && Lab[i][j]==1)
         {
            // start new label
            int   cnt               = 0;
            float cumulrdg          = 0;
            int   n_dead_recursions = 0;

            k = etiquette - 2;
            recursion_level = 0;

            i0 = i;
            j0 = j;            

            // enter labelling recursion

            // enters the while loop, if Lab[i][j]==1
            // or continues the loop, if recursion_level>0
            while (Lab[i][j]==1 || recursion_level>0)     // continues loop, if recursion_level>1                                            
            {
               // see if current pixel at [i][j] has to be labelled to
               // current component 
               if (Lab[i][j]==1)
               {
                  // check that direction constraint is enforced
                  // abs_angle_diff = abs(Dir(i,j) - Dir(i0,j0));
                  float scalar_product;

                  // if localDirLabelFlag==FALSE, next condition is always true 
                  // (no need to check direction constraint)
                  if (!localDirLabelFlag || ((scalar_product = (float) (cos(Dir[i][j])*cos(Dir[i0][j0])+sin(Dir[i][j])*sin(Dir[i0][j0])))>0 && scalar_product>=cos_angle) || (scalar_product<0 && scalar_product<=-cos_angle))
                  {
                     cnt++;
                     Lab[i][j] = etiquette;
                     cumulrdg += Rdg[i][j];

                     // set dead_recursion_level_flag to false since we
                     // found a pixel
                     stack_dead_recursions_flag[recursion_level] = 0;

                     // prepare next recursion level
                     if (recursion_level<max_recursion_level)
                     {
                        // save values for current level
                        stack_j[recursion_level] = j;
                        stack_i[recursion_level] = i;

                        stack_i0[recursion_level] = i0;
                        stack_j0[recursion_level] = j0;

                        // set local values for next level
                        i0 = i;
                        j0 = j;

                        i = RDG_MAX(i0-NbRangePix, 1);
                        j = RDG_MAX(j0-NbRangePix, 1);

                        // set dead_recursion_level_flag to true for next level
                        // it will be set to false if a pixel is found in
                        // next recursion level
                        stack_dead_recursions_flag[recursion_level+1] = 1;

                        recursion_level++;
                     }
                     else
                     {
                        printf("RdgLabel: label too long: aborted\n");
                     }

                     continue;
                  }
               }

               // increment i and j, to implement 
               //   for i=max(i0-range,1):min(i0+range,Ih)
               //       for j=max(j0-range,1):min(j0+range,Iw)
               j++;
               if (j>RDG_MIN(j0+NbRangePix,Iw-1))
               {
                  // increment the i index
                  i++;
                  if (i>RDG_MIN(i0+NbRangePix,Ih-1))
                  {                     
                     // if dead_recursion_level_flag to true, update the
                     // number of dead recursions for this label
                     if (stack_dead_recursions_flag[recursion_level])
                        n_dead_recursions++;                       

                     //end of scanning in this recursion level
                     recursion_level--;

                     // restore i, j, i0 and j0
                     j  = stack_j [recursion_level];
                     i  = stack_i [recursion_level];
                     i0 = stack_i0[recursion_level];
                     j0 = stack_j0[recursion_level];

                     continue;
                  }
                  else
                  {
                     j= RDG_MAX(j0-NbRangePix, 0);
                  }                              
               }
            }

            // end of recursion and labelling of current label, 

            // keep track of number of labelled pixels for current component
            NbPelPerLab        [k] = cnt;
            CumRdgPerLab       [k] = cumulrdg;
            NbExtremitiesPerLab[k] = n_dead_recursions;

            // increment label number
            etiquette++;
         }
      }
   }


   *NbLabels = etiquette-2;
   for(k=0; k<*NbLabels; k++)
   {
      CumRdgPerLab[k] *= (float)TipLength / (float)RDG_MAX(TipLength, NbPelPerLab[k]);
   }

   free(stack_i);
   free(stack_j);
   free(stack_i0);
   free(stack_j0);
   free(stack_dead_recursions_flag);
}




/*************************************************************************************/
// Calculate Gaussian Hessian: Lxx, Lyy and Lxy
/*************************************************************************************/
static 
void RdgCalcGaussHessian2(float **Gau, int i, int j, float *Lxx, float *Lyy, float *Lxy)
{
	int rdg_step_for_differentials=RDG_STEP_FOR_DIFFERENTIALS; //just to make compiler happy
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



//**************************************************************************
// RdgAsymetricRidgenessLabels : computes the asymetric ridgeness measure for each label
// InPut Image : Lab contains the image labels
// (each labels corresponds to a value of Lab which is >= 2)
// Output: the array CumRdgPerLab is modified
//**************************************************************************
void RdgAsymetricRidgenessLabels(int **Lab, float **Gau, float **Rdg, float **Dir, int Iw, int Ih, 
                                 float KernelSig, float RdgThresh, CRdgRoiCoord *RoiCoord, 
                                 int NbLabels, int *NbPelPerLab, float *CumRdgPerLab, int NbRangePix, 
                                 int TipLength, 
                                 int ZeroOutLowRidgenessFlag, 
                                 float RdgFilterNorm)
{
   int i, j, k;
   int iMin = NbRangePix, iMax = Ih-NbRangePix-1, jMin = NbRangePix, jMax = Iw-NbRangePix-1;
   float step = (float) (3.0/2*KernelSig);

   // Response of the asymetric ridge filter over an ideal ridge (dirac-like) after Gaussian filtering
   float AsymRdgFilterNorm = (float) ((1-exp(-RDG_SQ(step)/(2*RDG_SQ(KernelSig))))/sqrt(2*RDG_PI*RDG_SQ(KernelSig)));

   #if MX_FLAG

       printf("MX_FLAG is defined\n");
       MxOpen() ;
       MxSetVisible(1) ;
       MxCommand("cd D:/MatLab/Rdg");

       MxPutMatrixFloat(Rdg[0], Iw, Ih, "Rdg");
       MxPutMatrixInt(Lab[0], Iw, Ih, "Lab");
       MxCommand(" figure(2) ; imagesc(Rdg) ; axis off ; axis image ; colormap gray(256) ; ") ; 
       MxCommand(" figure(3) ; imagesc(Lab) ; axis off ; axis image ; colormap gray(256) ; ") ; 
#endif

   iMin = RDG_MAX(iMin, RoiCoord->YMin);
   iMax = RDG_MIN(iMax, RoiCoord->YMax);
   jMin = RDG_MAX(jMin, RoiCoord->XMin);
   jMax = RDG_MIN(jMax, RoiCoord->XMax);

   // go through the image, and replace the ridgeness value by a non-linear one, 
   // which discriminates edges
   if (NbLabels)
   {
      CRdgVf **CachedV;

      // next IMin/IMax/JMin/JMax values taken from RdgRdgDifferential(), 
      // to have Dir[][] pictures that have the same boundaries 
      // regardless of RDG_COMPUTE_RDG_DIR_IN_DBLBISTOU
      float Ro = (float)2;
      int   dBo = (int)(Ro*KernelSig) + 1;
      int   Bo1 = 2;  // image border
      //int   Bo2 = Bo1 + dBo;

      int IMin1 = RDG_MAX(RoiCoord->YMin - dBo, Bo1);
      int IMax1 = RDG_MIN(RoiCoord->YMax + dBo, Ih-1-Bo1);
      int JMin1 = RDG_MAX(RoiCoord->XMin - dBo, Bo1);
      int JMax1 = RDG_MIN(RoiCoord->XMax + dBo, Iw-1-Bo1);

      int IMin2 = IMin1 + dBo;
      int IMax2 = IMax1 - dBo;
      int JMin2 = JMin1 + dBo;
      int JMax2 = JMax1 - dBo;

      RdgImAlloc(0, &CachedV, sizeof(CachedV[0][0]), Iw, Ih);
      memset(CachedV[0], 0, Iw*Ih*sizeof(CachedV[0][0]));

      // reset cumulative ridgeness array
      for (k=0; k<NbLabels; k++)
      {
         CumRdgPerLab[k] = 0;
      }

      for(i = iMin; i <= iMax; i++)
         for(j = jMin; j <= jMax; j++)
            if(Lab[i][j]>=2)
            {
                if (IMin2<=i && i<=IMax2 && 
                    JMin2<=j && j<=JMax2 && Dir[i][j]==0)
                {
                    // compute the angle needed for the oriented non-linear ridgeness here
                    // Double-angle averaging in window of size 3
                    // (from CxDirectionHessAng2)
                    float Vx = 0.0f;
                    float Vy = 0.0f;
                    int   I, J;

                    for(I=-1; I<=1; I++)
                    {
                        for(J=-1; J<=1; J++)
                        {
                            // check if value is in cache
                            if (CachedV[i+I][j+J].X)
                            {
                                Vx -= CachedV[i+I][j+J].X;
                                Vy -= CachedV[i+I][j+J].Y;
                            }
                            else
                            {
                                // not in cache, compute the values
                                float lxx, lxy, lyy;

                                RdgCalcGaussHessian2(Gau, i+I, j+J, &lxx, &lyy, &lxy);

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

                                // store values in cache
                                CachedV[i+I][j+J].X = lxx * lxx - lyy * lyy; // LXX - LYY;
                                CachedV[i+I][j+J].Y = 2*(lxy * (lxx + lyy)); // 2*LXY;

                                Vx -= CachedV[i+I][j+J].X;
                                Vy -= CachedV[i+I][j+J].Y;
                            }
                        }
                    }

                    Dir[i][j] = (float)(0.5*atan2((double) Vy, (double) Vx));
                }
               {
                  // compute the oriented non-linear ridgeness here
                  float Profile[3];
                  float di = (float) ( step*cos(Dir[i][j])); // sin(dir+pi/2)
                  float dj = (float) (-step*sin(Dir[i][j])); // cos(dir+pi/2)
                  int absdi = (int) (ceil(RDG_ABS(di)));
                  int absdj = (int) (ceil(RDG_ABS(dj)));

                  if (i-absdi>=0 && i+absdi+1<Ih &&
                      j-absdj>=0 && j+absdj+1<Iw)
                  {

                     for (k=-1; k<=1; k+=2)
                     {
                        float interpolated_i = i+k*di;
                        float interpolated_j = j+k*dj;
                        int   ul_i = (int) interpolated_i; // upper-left corner
                        int   ul_j = (int) interpolated_j; // upper-left corner
                        float coef_i = interpolated_i-ul_i;
                        float coef_j = interpolated_j-ul_j;

                        Profile[k+1] = 
                           Gau[ul_i  ][ul_j  ]*(1-coef_i)*(1-coef_j)+
                           Gau[ul_i  ][ul_j+1]*(1-coef_i)*(  coef_j)+
                           Gau[ul_i+1][ul_j+1]*(  coef_i)*(  coef_j)+
                           Gau[ul_i+1][ul_j  ]*(  coef_i)*(1-coef_j);
                     }

                     CumRdgPerLab[Lab[i][j]-2] += RDG_MAX(RDG_MIN(Profile[2],Profile[0])-Gau[i][j],0);
                  }
                  else
                  {
                     // add default ridgeness
                     CumRdgPerLab[Lab[i][j]-2] += Rdg[i][j];
                  }
               }
            }

      RdgImFree(NULL, CachedV);
   }

   for(k=0; k<NbLabels; k++)
   {
      
      // if a label represents an edge, the asymetric ridgeness will be significantly much lower than original ridgeness
      // use the (original) ridge threshold to zero-out the CumRdgPerLab of such labels
      // TO BE IMPROVED
      // ZeroOutLowRidgenessFlag added because the following test does not work if image has been normalized

      // AsymRdgFilterNorm/RdgFilterNorm is the normalization factor necessary to translate 
      // the threshold from the ridge filter dynamics to the asymetric filter dynamics
      // /5 is the "significantly" lower response on edges
      if (ZeroOutLowRidgenessFlag && CumRdgPerLab[k]<(RdgThresh*AsymRdgFilterNorm/RdgFilterNorm*NbPelPerLab[k])/5)
      {
         CumRdgPerLab[k] = 0;
      }

      CumRdgPerLab[k] *= (float)TipLength / (float)RDG_MAX(TipLength, NbPelPerLab[k]);
   }
}




//**************************************************************************
// RdgClearObject : free RdgObject structure
//**************************************************************************
void RdgClearObject(CRdgObject *Object)
{
   if(Object->ObjPelList.NbPel)
      free(Object->ObjPelList.ObjPels);
   if(Object->CenterLinePelList.NbPel)
      free(Object->CenterLinePelList.CenterLinePels);
}



//**************************************************************************
// RdgClearObjectList : free RdgObjectList structure
//**************************************************************************
void RdgClearObjectList(CRdgObjectList *ObjectList)
{
   int k;

   if(ObjectList->NbObject)
   {
      for(k=0; k<ObjectList->NbObject; k++)
      {
         RdgClearObject(&(ObjectList->Objects[k]));
      }
      free(ObjectList->Objects);
   }
   ObjectList->NbObject = 0;
}



//**************************************************************************
// RdgThresholdLabels : label thresholding
// 
// Several forms of thresholding:
// - by keeping at most a NbLabKept number of labels
// - by keeping the labels that have a ridgeness >MaxRidgeness*RidgenessThresholdRatio (ratio can be zero, should be <1)
//
// ObjectList is optional, can be NULL
//**************************************************************************
void RdgThresholdLabels(short **Ori, short **Sub, CRdgVi **Pel, int **Lab, float **Rdg, float **Dir, unsigned char **Thr,
												int IwOri, int IhOri, int IwSub, int IhSub, int SSFactor, CRdgRoiCoord *RoiCoord, CRdgObjectList *ObjectList,
												int NbLabels, int *NbPelPerLab, float *CumRdgPerLab, int NbLabKept, float RidgenessThresholdRatio,  int ExtraMeasuresFlag)
{
   int        i, j, k, l, lMax, NbPel, Label, PelIdx, XMin, YMin, XMax, YMax, MaxNbLab;
   int        iMin = 0, iMax = IhSub-1, jMin = 0, jMax = IwSub-1;
   float      MaxRdg = (float)0;
   CRdgObject *Object;
   CRdgObjPel *ObjectPel;

   char       *label_selected_flags=NULL;

   iMin = RoiCoord->YMin;
   iMax = RoiCoord->YMax;
   jMin = RoiCoord->XMin;
   jMax = RoiCoord->XMax;

   // Maximum number of labels to be kept
   MaxNbLab = RDG_MIN(NbLabels, NbLabKept);

   if (ObjectList)
   {
      // freeing previous ObjectList
      RdgClearObjectList(ObjectList);

      // allocating new ObjectList
      ObjectList->Objects = (CRdgObject*)malloc(MaxNbLab * sizeof(CRdgObject));
   }

   // prepare the "labels selected flags" array
   if (NbLabels)
   {
      label_selected_flags = (char *) _alloca(NbLabels*sizeof(label_selected_flags[0])); 
      memset(label_selected_flags, 0, NbLabels*sizeof(label_selected_flags[0]));
   }

   // determination of the best labels (which have the highest cumulative ridgeness)
   for(k=0; k<MaxNbLab; k++)
   {
      lMax = 0;
       // Find current best label among unselected labels (see end of loop)
      for(l=0; l<NbLabels; l++)
         // condition is: if label l has not been selected yet AND (ridgness(l)>ridgeness(lMax) OR ridgeness(lMax) has no significance because lMax has already been selected (because lMax set to 0 initially))
         if(!label_selected_flags[l] && (CumRdgPerLab[l] > CumRdgPerLab[lMax] || label_selected_flags[lMax]))
            lMax = l;

      if (CumRdgPerLab[lMax]>MaxRdg*RidgenessThresholdRatio)
      {
         if (ObjectList)
         {
            NbPel = NbPelPerLab[lMax];
            Object = &(ObjectList->Objects[k]);
            Object->Measures.Ridgeness = CumRdgPerLab[lMax];
            Object->ObjPelList.NbPel = NbPel;
            Object->ObjPelList.ObjPels = (CRdgObjPel*)malloc(NbPel * sizeof(CRdgObjPel));
            Object->CenterLinePelList.NbPel = NbPel;
            Object->CenterLinePelList.CenterLinePels = (CRdgCenterLinePel*)malloc(NbPel * sizeof(CRdgCenterLinePel));

            // filling ObjPelList
            Label = lMax + 2;
            PelIdx = 0;
            XMin = IwSub; XMax = 0; YMin = IhSub; YMax = 0;
            Object->ObjPelList.Centroid.X = 0;
            Object->ObjPelList.Centroid.Y = 0;
            for(i = iMin; i <= iMax; i++)
            {
               for(j = jMin; j <= jMax; j++)
               {
                  if(Lab[i][j] == Label)
                  {
                     ObjectPel = &(Object->ObjPelList.ObjPels[PelIdx]);
                     ObjectPel->ObjPel.X = j;
                     ObjectPel->ObjPel.Y = i;
                     Object->ObjPelList.Centroid.X += j;
                     Object->ObjPelList.Centroid.Y += i;
                     XMin = RDG_MIN(XMin, j); YMin = RDG_MIN(YMin, i);
                     XMax = RDG_MAX(XMax, j); YMax = RDG_MAX(YMax, i);
                     ObjectPel->Ori = Sub[i][j];
                     ObjectPel->Rdg = Rdg[i][j];
                     ObjectPel->Dir = Dir[i][j];
                     PelIdx++;
                  }
               }
            }
            Object->ObjPelList.Centroid.X /= NbPel;
            Object->ObjPelList.Centroid.Y /= NbPel;

            // Center line + width computing
            RdgCalcCenterLinePelList(&(Object->ObjPelList), &(Object->CenterLinePelList), 
               &(Object->Measures), Ori, Pel, Rdg, Dir, IwOri, IhOri, SSFactor, ExtraMeasuresFlag);

            Object->ObjPelList.MinBox.X = XMin; Object->ObjPelList.MinBox.Y = YMin;
            Object->ObjPelList.MaxBox.X = XMax; Object->ObjPelList.MaxBox.Y = YMax;
         }

         // keep track of MaxRdg (we may be here because it is the first selected label)
         MaxRdg = RDG_MAX(CumRdgPerLab[lMax], MaxRdg);

         // mark label lMax as used
         label_selected_flags[lMax] = 1;
      }
      else
      {
         // remaining labels are will not be used, stop here
         break;
      }
   }
   if (ObjectList)
      ObjectList->NbObject = k;

   {
      int remove_remaining_labels = FALSE;

      // see if there is at least a label to be removed
      for (l=0; l<NbLabels; l++)
      {
         if (label_selected_flags[l]==0)
         {
            remove_remaining_labels = TRUE;
            break;
         }
      }

      // remove all remaining labels
      if (remove_remaining_labels)
      {
         // go through the image, and check/remove labels
         for(i = iMin; i <= iMax; i++)
         {
            for(j = jMin; j <= jMax; j++)
            {
               if (Lab[i][j]-2>=0 && // this labelled pixel is still present in labelled picture
                   label_selected_flags[Lab[i][j]-2]==0) // has been unused so far, should be removed
                  {
                     Lab[i][j] = 
                     Thr[i][j] = 0;
                     Rdg[i][j] = 0;
                  }
            }
         }

         // now mark the corresponding CumRdgPerLab[] as removed
         for (l=0; l<NbLabels; l++)
         {
            if (label_selected_flags[l]==0)
            {
               CumRdgPerLab[l] = 0;
            }
         }
      }
   }

   // Normalizing ridgeness w.r.t best object's ridgeness
   if (ObjectList)
   {
      for(k=0; k<ObjectList->NbObject; k++){
         ObjectList->Objects[k].Measures.Ridgeness = ObjectList->Objects[k].Measures.Ridgeness / MaxRdg;
      }
   }
}



//**************************************************************************
// RdgThresholdLabelsBySize : label thresholding by size
//**************************************************************************
void RdgThresholdLabelsBySize(int **Lab, unsigned char **Thr, CRdgRoiCoord *RoiCoord, int NbLabels, int *NbPelPerLab, int MinNbPelPerLab, int KeepLargestLabel)
{
   int        i, j, k; 
   int        iMin, iMax, jMin, jMax;
   //float      MaxRdg = (float)0;

   // array of flags to know which labels to remove
   char *toBeRemoved = (char *) _alloca(NbLabels+1);

   iMin = RoiCoord->YMin;
   iMax = RoiCoord->YMax;
   jMin = RoiCoord->XMin;
   jMax = RoiCoord->XMax;

   // determination of the labels to be removed
   for (k=0; k<NbLabels; k++)
   {
      toBeRemoved[k] = (char) (NbPelPerLab[k]<MinNbPelPerLab);
   }

   if (KeepLargestLabel)
   {
      // search for largest label
      int NbPelLargestLab = 0;

      for (k=0; k<NbLabels; k++)
         if (NbPelLargestLab<NbPelPerLab[k])
            NbPelLargestLab = NbPelPerLab[k];

      // keep largest label
      for (k=0; k<NbLabels; k++)
         if (NbPelLargestLab==NbPelPerLab[k])
            toBeRemoved[k] = FALSE;
   }

   // now remove the labels to be removed
   for(i = iMin; i <= iMax; i++)
   {
      for(j = jMin; j <= jMax; j++)
      {
         if (Thr[i][j] && (Lab[i][j]==1 || toBeRemoved[Lab[i][j]-2]))
         {
            Thr[i][j] = 0;
            Lab[i][j] = 0;
         }
      }
   }
}



//**************************************************************************
// Calculate global motion of the image objects
//**************************************************************************
void RdgCalcGlobalMotion(int t, int **ImLab, int **ImLabTm1, float **ImRdg, int IwSub, int IhSub, 
                         int GlobalMotionFlag, CRdgRoiCoord *RoiCoord, CRdgObjectList *ObjectList, 
                         CRdgVf *CentroidTm1, int *NbObjTm1, float RdgThresh, int ThinningFlag, 
                         int FastProfileFlag, 
                         CRdgVi *GlobalMotion)
{
   int    i, j, k, l, Inter, MaxInter = 0, MinInterInImage;
   int    iMin = 0, iMax = IhSub-1, jMin = 0, jMax = IwSub-1;
   float  CenDist;
   CRdgVi EDCen, ETm1, EDCenMax, EMove;
   CRdgVf Cen;
   CRdgObjPelList *ObjPelList;
   int    NbMaxObj;
   float  DistMax = (float)sqrt((double)(RDG_SQ(IwSub) + RDG_SQ(IhSub))) * (float).25;
   float ThreshForInter = RdgThresh;

	 EDCenMax.X=EDCenMax.Y=0;//just to make compiler happy

   if (FastProfileFlag)
      NbMaxObj = 5;  // maximum number of objects taken into account because at most  5 stitches per image.
   else
      NbMaxObj = 10; // maximum number of objects taken into account because at most 10 stitches per image.

   GlobalMotion->X = 0;
   GlobalMotion->Y = 0;

   if(!GlobalMotionFlag)
      return;

   if(ThinningFlag)  // threshold proportional to the length (or width) because thinning gives ridges with 1 pixel width
      MinInterInImage = (int)(((RoiCoord->XMax-RoiCoord->XMin) + (RoiCoord->YMax-RoiCoord->YMin)) * (float)0.3);
      // MinInterInImage = (int)(((RoiCoord->XMax-RoiCoord->XMin) + (RoiCoord->YMax-RoiCoord->YMin)) * (float)0.4);
   else              // threshold proportional to area because ridge width is proportional to the length (or width) of the image
      MinInterInImage = (int)((RoiCoord->XMax-RoiCoord->XMin) * (RoiCoord->YMax-RoiCoord->YMin) * (float).005);

   if(t > 0)
   {
      // will be used to avoid testing twice the same displacement vector
      CRdgVi *checked = (CRdgVi *) _alloca(RDG_MIN(NbMaxObj, ObjectList->NbObject)*RDG_MIN(NbMaxObj, *NbObjTm1)*sizeof(checked[0]));
      int     n_checked = 0;

      iMin = RoiCoord->YMin;
      iMax = RoiCoord->YMax;
      jMin = RoiCoord->XMin;
      jMax = RoiCoord->XMax;

      // For each object of the current ObjectList, determine which object of the previous 
      // ObjectList has centroid which is not too far (centroid distance < distance max).
      // Then globally translate the ObjectList to annulate the centroid distance
      // and calculate the intersection which is the pixel positions which are in both ObjectList
      for(k=0; k<RDG_MIN(NbMaxObj, ObjectList->NbObject); k++)
      {
         // if object ridgeness is too low, it might be an edge on the boundaries of the image sensor
         // do not take it into account
         if (ObjectList->Objects[k].Measures.Ridgeness<0.1)
            continue;

         ObjPelList = &(ObjectList->Objects[k].ObjPelList);
         Cen.X = ObjPelList->Centroid.X;
         Cen.Y = ObjPelList->Centroid.Y;
         for(l=0; l<RDG_MIN(NbMaxObj, *NbObjTm1); l++)
         {
            int   m, mv_already_checked;
            float DX, DY; 

            // motion vector candidate
            DX = Cen.X - CentroidTm1[l].X;
            DY = Cen.Y - CentroidTm1[l].Y;
            CenDist = (float)sqrt((double)(RDG_SQ(DX) + RDG_SQ(DY)));

            // floor DX and DY, so that -DX and -DY are rounded to the largest integer smaller than -DX and -DY
            // this way, when (DX,DY) are substracted from positive pixel coordinates, it will give positive 
            // coordinates rounded toward 0 (or left/upper integer pel position)            
            EDCen.X = (int) ceil(DX); 
            EDCen.Y = (int) ceil(DY); 

            // check if this EDCen vector has already been tested
            mv_already_checked = FALSE;
            for (m=0; m<n_checked; m++)
            {
               if (checked[m].X==EDCen.X && checked[m].Y==EDCen.Y)
               {
#if RDG_PRINT_FOR_DEBUG
                   printf("\t\tMV already checked: %d %d\n", (int) EDCen.X, (int) EDCen.Y);
#endif       
                   mv_already_checked = TRUE;
                   break;
               }
            }

            if (!mv_already_checked)
               // add it to the array of checked motion vectors
               checked[n_checked++] = EDCen;

            // now test the motion vector given by EDCen
            if(!mv_already_checked && CenDist < DistMax) 
            {
               Inter = 0;
               for(i=iMin; i<=iMax; i++)
                  for(j=jMin; j<=jMax; j++)
                  {
                     if(ImLab[i][j] >= 2)
                     {
                        // ETm1 behaves as if j-float(DCen.X) had been rounded toward 0
                        ETm1.X = j - EDCen.X;
                        ETm1.Y = i - EDCen.Y;

                        if(ETm1.X > 0 && ETm1.X < IwSub-1 && ETm1.Y > 0 && ETm1.Y < IhSub-1)
                        {
                           // check labels at rounded and rounded+1 pixels
                           if(ImLabTm1[ETm1.Y][ETm1.X] > 0 || ImLabTm1[ETm1.Y+1][ETm1.X] > 0 || ImLabTm1[ETm1.Y][ETm1.X+1] > 0 || ImLabTm1[ETm1.Y+1][ETm1.X+1] > 0)
                              Inter++;
                        }
                     }
                  }
#if RDG_PRINT_FOR_DEBUG
                  printf("\t\tInter = %d, current GMC = %d %d\n", Inter, (int) EDCen.X, (int) EDCen.Y);
#endif       

                  if(Inter > MaxInter)
                  {
                     MaxInter   = Inter;
                     EDCenMax.X = EDCen.X;
                     EDCenMax.Y = EDCen.Y;
#if RDG_PRINT_FOR_DEBUG
                     printf("\tMaxInter = %d, current GMC = %d %d\n", MaxInter, (int) EDCenMax.X, (int) EDCenMax.Y);
#endif       
                  }
            }
         }
      }

#if RDG_PRINT_FOR_DEBUG
      printf("MaxInter = %d, current GMC = %d %d\n", MaxInter, (int) EDCenMax.X, (int) EDCenMax.Y);
#endif

      // Precise re-calculation of the intersection taking into account the ridge value
      if (MaxInter) // MaxInter==0 can happen, resulting in an unitialized DCenMax variable runtime error
      {
         int start, stop;

         MaxInter = MinInterInImage;

         // set global motion to current motion before refinment
         // GlobalMotion->X = EDCenMax.X;
         // GlobalMotion->Y = EDCenMax.Y;

         if (FastProfileFlag)
         {
            // slight refinment, since (0,0),(0,1),(1,0) and (1,1) were tested in first search loop
            start = 0;
            stop  = 2;
         }
         else
         {
            // explicit search refinment
            start = -2;
            stop  = 3;
         }

         for(k=start; k<stop; k++)
            for(l=start; l<stop; l++)
            {
               EMove.Y = EDCenMax.Y - k; // - really meant, not +
               EMove.X = EDCenMax.X - 1; // - really meant, not +

               Inter = 0;
               for(i=iMin; i<=iMax; i++)
                  for(j=jMin; j<=jMax; j++)
                  {
                     // Ridge value has to be upon a given threshold
                     // This should also remove edge pixels
                     if(ImLab[i][j] >= 2 && ImRdg[i][j] > ThreshForInter)
                     {
                        ETm1.X = j - EMove.X;
                        ETm1.Y = i - EMove.Y;
                        if(ETm1.X >= 0 && ETm1.X < IwSub && ETm1.Y >= 0 && ETm1.Y < IhSub)
                        {
                           if(ImLabTm1[ETm1.Y][ETm1.X] > 0)
                              Inter++;
                        }
                     }
                  }
                  if(Inter > MaxInter)
                  {
                     MaxInter = Inter;
                     GlobalMotion->X = EMove.X;
                     GlobalMotion->Y = EMove.Y;
                  }
            }

#if RDG_PRINT_FOR_DEBUG
            printf("\n     GlobalMotion->X: %d, GlobalMotion->Y: %d\n\n", GlobalMotion->X, GlobalMotion->Y);
#endif 
      }
   }

   // copy current data for next frame
   memcpy(ImLabTm1[0], ImLab[0], IwSub*IhSub*sizeof(ImLab[0][0]));

   // copy current centroids for next frame
   for(k=0, l=0; k<ObjectList->NbObject; k++)
   {
      // if object ridgeness is too low, it might be an edge on the boundaries of the image sensor
      // do not take it into account
      if (ObjectList->Objects[k].Measures.Ridgeness<0.1)
         continue;

      CentroidTm1[l].X = ObjectList->Objects[k].ObjPelList.Centroid.X;
      CentroidTm1[l].Y = ObjectList->Objects[k].ObjPelList.Centroid.Y;
      l++;
   }
   *NbObjTm1 = l;
}



//**************************************************************************
// Returns for each label the number of past frames where it was undergoing 
// zero or global motion
// 
// Also fill in the global motion picture mask
//
//**************************************************************************
void RdgVerifyGlobalMotion(int t, int **ImLab, int IwSub, int IhSub,  CRdgRoiCoord *RoiCoord, CRdgObjectList *ObjectList, int ThinningFlag,
													 CRdgVi *CurrentGlobalMotion, CRdgVi *PastGlobalMotion,  unsigned char ***ImPastThr, float CardiacMotionThreshold,
													 int *significant_motion_flag, int *stitch_detected_flag, unsigned char **ImGMMask)
{
   int    i, j, k, l; 
   int    MaxInter = 0, MinInterInImage;

   CRdgVi gmc;

   int NbMaxObj = ObjectList->NbObject; // RDG_MIN(150, ObjectList->NbObject);
   int n_frames_zero_motion[150];
   int n_frames_global_motion[150];

   // do it for all objects
   // NbMaxObj = ObjectList->NbObject;

   *significant_motion_flag = FALSE;
   *stitch_detected_flag = FALSE;

   // initialize the rigid motion flags
   for (k=0; k<ObjectList->NbObject; k++)
      ObjectList->Objects[k].Measures.RigidMotion = RDG_CST_RIGID_MOTION_UNKNOWN;

   // initialize the n_frames arrays
   for (k=0; k<NbMaxObj; k++)
   {
      n_frames_zero_motion[k] = 0;
      n_frames_global_motion[k] = 0; 
   }
   // initialize gmc
   gmc = *CurrentGlobalMotion;

   if(ThinningFlag)  // threshold proportional to the length (or width) because thinning gives ridges with 1 pixel width
      MinInterInImage = (int)(((RoiCoord->XMax-RoiCoord->XMin) + (RoiCoord->YMax-RoiCoord->YMin)) * (float)0.04);
   else              // threshold proportional to area because ridge width is proportional to the length (or width) of the image
      MinInterInImage = (int)((RoiCoord->XMax-RoiCoord->XMin) * (RoiCoord->YMax-RoiCoord->YMin) * (float).0005);

   if(t > 0)
   {
      int n_frames;

#if RDG_PRINT_FOR_DEBUG
      printf("CardiacMotionThreshold %f\n", CardiacMotionThreshold);
#endif

      for (n_frames=1; n_frames<=RDG_MIN(t,RDG_CST_GM_PAST_N_FRAMES); n_frames++)
      {
         int index = (t-n_frames) % RDG_CST_GM_PAST_N_FRAMES; // index of past picture in array
         unsigned char **im_past_thr = ImPastThr[index];

         // printf("GMC = %d %d\n", gmc.X, gmc.Y);

         // For each object of the current ObjectList, determine if it undergoes global motion
         // by calculating the intersection which is the pixel positions which are in both ObjectList
         for(k=0; k<RDG_MIN(NbMaxObj, ObjectList->NbObject); k++)
         {
            CRdgObjPelList *ObjPelList = &(ObjectList->Objects[k].ObjPelList);
            CRdgObjPel     *ObjPels = ObjPelList->ObjPels;

            int InterZeroMotion = 0;
            int InterGMC = 0;

            for(l=0; l<ObjPelList->NbPel; l++)
            {
               CRdgVi zero_move, gmc_move;

               // coordinates of current pel in ImLabTM1 by global motion
               zero_move.X = ObjPels[l].ObjPel.X;
               zero_move.Y = ObjPels[l].ObjPel.Y;

               gmc_move.X = zero_move.X - gmc.X;
               gmc_move.Y = zero_move.Y - gmc.Y;

               if (im_past_thr[zero_move.Y][zero_move.X]) 
                  InterZeroMotion++;

               if(gmc_move.X > 1 && gmc_move.X < IwSub-2 && gmc_move.Y > 1 && gmc_move.Y < IhSub-2)
               {
                  if(im_past_thr[gmc_move.Y  ][gmc_move.X  ] > 0
                     // increase match area
                     || 
                     im_past_thr[gmc_move.Y-1][gmc_move.X-1] > 0 || 
                     im_past_thr[gmc_move.Y-1][gmc_move.X  ] > 0 ||
                     im_past_thr[gmc_move.Y-1][gmc_move.X+1] > 0 || 
                     im_past_thr[gmc_move.Y  ][gmc_move.X-1] > 0 || 
                     im_past_thr[gmc_move.Y  ][gmc_move.X+1] > 0 ||
                     im_past_thr[gmc_move.Y+1][gmc_move.X-1] > 0 || 
                     im_past_thr[gmc_move.Y+1][gmc_move.X  ] > 0 || 
                     im_past_thr[gmc_move.Y+1][gmc_move.X+1] > 0
                     )
                     InterGMC++;
               }
            }

            // printf("Label %d, %d pels, w.r.t. t-%d, ZMC at %d percent, GMC %d percent\n", 
            //         k, (int) ObjPelList->NbPel, n_frames, (int) (100*InterZeroMotion)/ObjPelList->NbPel, (int) (100*InterGMC)/ObjPelList->NbPel);          

            if (n_frames_zero_motion[k] == n_frames-1) // stop updating the flag if no zero motion was detected before
               if (((float) InterZeroMotion)/ObjPelList->NbPel > CardiacMotionThreshold)
                  n_frames_zero_motion[k] = n_frames;

            if (n_frames_global_motion[k] == n_frames-1) // stop updating the flag if no global motion was detected before
               if (((float) InterGMC)/ObjPelList->NbPel > CardiacMotionThreshold)
                  n_frames_global_motion[k] = n_frames;

            // keep track of maximum inter value
            // if object ridgeness is too low, it might be an edge on the boundaries of the image sensor
            // do not take it into account
            if (ObjectList->Objects[k].Measures.Ridgeness>=0.1)
            {
               if (MaxInter<InterZeroMotion) MaxInter = InterZeroMotion;
               if (MaxInter<InterGMC)        MaxInter = InterGMC;
            }
         }
         // printf("\n");

         // accumulate GMC
         gmc.X += PastGlobalMotion[index].X;
         gmc.Y += PastGlobalMotion[index].Y;
      }

#if RDG_PRINT_FOR_DEBUG
      // print the found n_frames
      for (k=0; k<RDG_MIN(NbMaxObj, ObjectList->NbObject); k++)
         printf("Label %d, %d pels, ZMC since %d frames, GMC since %d \n", 
         k, (int) ObjectList->Objects[k].ObjPelList.NbPel, n_frames_zero_motion[k], n_frames_global_motion[k]);
      printf("MaxInter = %d\n", MaxInter);
#endif

      // additional tests
      {
         int min = RDG_MIN(NbMaxObj, ObjectList->NbObject), max = 0;

         for (k=0; k<RDG_MIN(NbMaxObj, ObjectList->NbObject); k++)
         {
            // if object ridgeness is too low, it might be an edge on the boundaries of the image sensor
            // do not take it into account
            if (ObjectList->Objects[k].Measures.Ridgeness<0.1)
               continue;

            if (n_frames_zero_motion[k]<min || n_frames_global_motion[k]<min)
               min = RDG_MIN(n_frames_zero_motion[k], n_frames_global_motion[k]);
            if (n_frames_zero_motion[k]>max || n_frames_global_motion[k]>max)
               max = RDG_MAX(n_frames_zero_motion[k], n_frames_global_motion[k]);
         }

         // *significant_motion_flag = MaxInter>=MinInterInImage && max>0 && min!=max && max>RDG_MIN(RDG_CST_GM_PAST_N_FRAMES*2/3, RDG_CST_GM_PAST_N_FRAMES-2) || (max>=2 && t<=6));
         *significant_motion_flag = (MaxInter>=MinInterInImage && min!=max && max>RDG_MIN((RDG_MIN(t,RDG_CST_GM_PAST_N_FRAMES))*2/3, RDG_CST_GM_PAST_N_FRAMES-2));

#if RDG_PRINT_FOR_DEBUG
         if (*significant_motion_flag)
            printf("MOTION DETECTED AND RELEVANT\n");
         else 
            printf("MOTION NOT SIGNIFICANT\n");
#endif

         if (*significant_motion_flag)
         {
            for (k=0; k<RDG_MIN(NbMaxObj, ObjectList->NbObject); k++)
            {
#if RDG_PRINT_FOR_DEBUG
               // if (n_frames_zero_motion[k]<max && n_frames_global_motion[k]<max)
               if (n_frames_zero_motion[k]<max && n_frames_zero_motion[k]<RDG_CST_GM_PAST_N_FRAMES-1 && n_frames_global_motion[k]<max && n_frames_global_motion[k]<RDG_CST_GM_PAST_N_FRAMES-1)
                  printf("label %d, %d pels, no rigid motion detected\n", k, (int) ObjectList->Objects[k].ObjPelList.NbPel);
               else
                  printf("label %d, %d pels, rigid motion detected\n", k, (int) ObjectList->Objects[k].ObjPelList.NbPel);
#endif
               // ObjectList->Objects[k].Measures.RigidMotion = (n_frames_zero_motion[k]<max && n_frames_global_motion[k]<max)?RDG_CST_RIGID_MOTION_FALSE:RDG_CST_RIGID_MOTION_TRUE;
               ObjectList->Objects[k].Measures.RigidMotion = (n_frames_zero_motion[k]<max && n_frames_zero_motion[k]<RDG_CST_GM_PAST_N_FRAMES-1 && n_frames_global_motion[k]<max && n_frames_global_motion[k]<RDG_CST_GM_PAST_N_FRAMES-1)?RDG_CST_RIGID_MOTION_FALSE:RDG_CST_RIGID_MOTION_TRUE;
            }

            for (k=0; k<RDG_MIN(NbMaxObj, ObjectList->NbObject); k++)
            {
               if ((n_frames_zero_motion[k]>RDG_MIN(RDG_CST_GM_PAST_N_FRAMES*2/3, RDG_CST_GM_PAST_N_FRAMES-2) || n_frames_global_motion[k]>RDG_MIN(RDG_CST_GM_PAST_N_FRAMES*2/3, RDG_CST_GM_PAST_N_FRAMES-2)) && ObjectList->Objects[k].ObjPelList.NbPel>100
                  // if object ridgeness is too low, it might be an edge on the boundaries of the image sensor
                  // do not take it into account
                  && ObjectList->Objects[k].Measures.Ridgeness>=0.1
                  )
               {
#if RDG_PRINT_FOR_DEBUG
                  printf("label %d, %d pels, STITCH detected\n", k, (int) ObjectList->Objects[k].ObjPelList.NbPel);
#endif
                 *stitch_detected_flag = TRUE;
               }
            }
         }
      }
   }

   // copy current data for next frame
   {
      int index = t % RDG_CST_GM_PAST_N_FRAMES; // index of past picture in array
      unsigned char **im_past_thr = ImPastThr[index];

      for(i=0; i<IhSub; i++)
         for(j=0; j<IwSub; j++)
            im_past_thr[i][j] = (unsigned char)ImLab[i][j];

      PastGlobalMotion[index].X = CurrentGlobalMotion->X;
      PastGlobalMotion[index].Y = CurrentGlobalMotion->Y;
   }

   {
      // fill in mask image
      memset(ImGMMask[0], 0, IwSub*IhSub*sizeof(ImGMMask[0][0]));

      if (*significant_motion_flag)
      {
         for (k=0; k<RDG_MIN(NbMaxObj, ObjectList->NbObject); k++)
         {
            // printf("creating mask... label %d, ridgeness %f, RigidMotion %d\n", k, ObjectList->Objects[k].Measures.Ridgeness, ObjectList->Objects[k].Measures.RigidMotion);

            if (ObjectList->Objects[k].Measures.RigidMotion == RDG_CST_RIGID_MOTION_TRUE
                // if object ridgeness is too low, it might be an edge on the boundaries of the image sensor
                // do not take it into account
                && ObjectList->Objects[k].Measures.Ridgeness>0.1)
            {  
               CRdgObjPelList *ObjPelList = &(ObjectList->Objects[k].ObjPelList);
               CRdgObjPel     *ObjPels = ObjPelList->ObjPels;

               for(l=0; l<ObjPelList->NbPel; l++)
               {  
                  ImGMMask[ObjPels[l].ObjPel.Y][ObjPels[l].ObjPel.X] = 1;
               }
            }
         }
      }
   }
}




//**************************************************************************
// Up-sample the object pixels
//**************************************************************************
void RdgUpSampleObjects(CRdgObjectList *ObjectList, CRdgVi **Pel, int SSFactor)
{
   int   k, l, X, Y;
   float XCen, YCen;
   CRdgObjPelList *ObjPelList;

   if(SSFactor == 1) return;

   for(k=0; k<ObjectList->NbObject; k++)
   {
      ObjPelList = &(ObjectList->Objects[k].ObjPelList);
      X = ObjPelList->MinBox.X;
      Y = ObjPelList->MinBox.Y;
      ObjPelList->MinBox.X = Pel[Y][X].X;
      ObjPelList->MinBox.Y = Pel[Y][X].Y;
      X = ObjPelList->MaxBox.X;
      Y = ObjPelList->MaxBox.Y;
      ObjPelList->MaxBox.X = Pel[Y][X].X;
      ObjPelList->MaxBox.Y = Pel[Y][X].Y;
      XCen = 0; YCen = 0;
      for(l=0; l<ObjPelList->NbPel; l++)
      {
         X = ObjPelList->ObjPels[l].ObjPel.X;
         Y = ObjPelList->ObjPels[l].ObjPel.Y;
         ObjPelList->ObjPels[l].ObjPel.X = Pel[Y][X].X;
         ObjPelList->ObjPels[l].ObjPel.Y = Pel[Y][X].Y;
         XCen += (float)Pel[Y][X].X;
         YCen += (float)Pel[Y][X].Y;
      }
      ObjPelList->Centroid.X = XCen / (float)ObjPelList->NbPel;
      ObjPelList->Centroid.Y = YCen / (float)ObjPelList->NbPel;
   }
}

//**************************************************************************
// Up-sample the object pixels
//**************************************************************************
void RdgUpSampleStringPelList(CRdgStringPelList *TipPelList, CRdgVi **Pel, int SSFactor)
{
   int   k, X, Y;

   if(SSFactor == 1) return;

   for(k=0; k<TipPelList->NbPel; k++)
   {
      X = TipPelList->StringPels[k].X;
      Y = TipPelList->StringPels[k].Y;
      TipPelList->StringPels[k].X = Pel[Y][X].X;
      TipPelList->StringPels[k].Y = Pel[Y][X].Y;
   }
}
