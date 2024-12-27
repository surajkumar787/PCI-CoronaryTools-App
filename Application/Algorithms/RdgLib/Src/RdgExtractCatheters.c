// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>

#include <RdgDev.h> 


#define USE_ALLOCA_INSTEAD_OF_MALLOC   1
#define STRAIGHTNESS_MAX_L2            100 // maximum lambda2 for a catheter body




//**************************************************************************
// Based on the assumption that the catheter end label will touch exactly 
// one picture/shutter border (not 0, not 2, but 1)
//**************************************************************************
void RdgGetCathetersBoundingBoxes(int **ImLab, int Iw, int Ih, 
                                  CRdgRoiCoord *ShuttersRoiCoord, int NbLabels, int *NbPelPerLab, 
                                  int BorderMargin, 
                                  int *CatheterTipFoundFlag,  int *CatheterTipLargestSize,  CRdgRoiCoord *CatheterTipRoiCoord, 
                                  int *CatheterBodyFoundFlag, int *CatheterBodyLargestSize, CRdgRoiCoord *CatheterBodyRoiCoord, float *CatheterBodyDir)
{
   *CatheterTipFoundFlag  = FALSE;
   *CatheterBodyFoundFlag = FALSE;

   if (NbLabels)
   {
      // Coordinates of ROI 
      int IMin = RDG_MAX(ShuttersRoiCoord->YMin, 0);
      int IMax = RDG_MIN(ShuttersRoiCoord->YMax, Ih-1);
      int JMin = RDG_MAX(ShuttersRoiCoord->XMin, 0);
      int JMax = RDG_MIN(ShuttersRoiCoord->XMax, Iw-1);

      int i, j, k;

#if USE_ALLOCA_INSTEAD_OF_MALLOC
      int *top_label_pos    = (int *) _alloca(NbLabels*sizeof(top_label_pos[0]));
      int *bottom_label_pos = (int *) _alloca(NbLabels*sizeof(bottom_label_pos[0]));
      int *left_label_pos   = (int *) _alloca(NbLabels*sizeof(left_label_pos[0]));
      int *right_label_pos  = (int *) _alloca(NbLabels*sizeof(right_label_pos[0]));
      int *is_passing_through_corner = (int *) _alloca(NbLabels*sizeof(is_passing_through_corner[0]));
#else
      int *top_label_pos    = (int *) malloc(NbLabels*sizeof(top_label_pos[0]));
      int *bottom_label_pos = (int *) malloc(NbLabels*sizeof(bottom_label_pos[0]));
      int *left_label_pos   = (int *) malloc(NbLabels*sizeof(left_label_pos[0]));
      int *right_label_pos  = (int *) malloc(NbLabels*sizeof(right_label_pos[0]));
      int *is_passing_through_corner = (int *) malloc(NbLabels*sizeof(is_passing_through_corner[0]));
#endif

      int top_shutter_pos    = ShuttersRoiCoord->YMin;
      int bottom_shutter_pos = ShuttersRoiCoord->YMax;
      int left_shutter_pos   = ShuttersRoiCoord->XMin;
      int right_shutter_pos  = ShuttersRoiCoord->XMax;

      // initialize the label pos arrays
      for (k=0; k<NbLabels; k++)
      {
         top_label_pos            [k] = IMax;
         bottom_label_pos         [k] = IMin;
         left_label_pos           [k] = JMax;
         right_label_pos          [k] = JMin;
         is_passing_through_corner[k] = 0;
      }

      // go through ImLab, and find the indices of
      // topmost/bottommost/leftmost/rightmost pixels for each label
      // i.e. find the bounding box of each label   
      for(i=IMin;i<=IMax;i++) 
      {
         for(j=JMin;j<=JMax;j++)
         { 
            if (ImLab[i][j]>=2) // first label number is 2, not 1 (which means thresholded but unlabelled) */)
            {
               int label_number = ImLab[i][j]-2;

               assert(label_number<=NbLabels);

               if (top_label_pos[label_number]>i)
                  top_label_pos[label_number] = i;

               if (bottom_label_pos[label_number]<i)
                  bottom_label_pos[label_number] = i;

               if (left_label_pos[label_number]>j)
                  left_label_pos[label_number] = j;

               if (right_label_pos[label_number]<j)
                  right_label_pos[label_number] = j;


               if ((i-top_shutter_pos<=BorderMargin) + (bottom_shutter_pos-i<=BorderMargin) + (right_shutter_pos-j<=BorderMargin) + (j-left_shutter_pos<=BorderMargin) >=2)
                  is_passing_through_corner[label_number] = 1;
            }
         }
      }

      for (k=0; k<NbLabels; k++)
      {
         //
         // go through the bounding boxes, and measure the number of bounding box
         // sides that coincide with shutters
         //
         int n_coincide = (top_label_pos[k]-top_shutter_pos<=BorderMargin) + (bottom_shutter_pos-bottom_label_pos[k]<=BorderMargin) + (left_label_pos[k]-left_shutter_pos<=BorderMargin) + (right_shutter_pos-right_label_pos[k]<=BorderMargin);

         // if passing through a corner, and 2 or more shutter borders are
         // touched by label, remove 1 (because a corner is a valid position)
         if (n_coincide>=2 && is_passing_through_corner[k])
            n_coincide--;

         // if n_coincide==1, this is a catheter end
         // keep track of the largest catheter end label
         if (n_coincide==1)
         { 
            if (*CatheterTipLargestSize<NbPelPerLab[k])
            {
               *CatheterTipLargestSize = NbPelPerLab[k];

               *CatheterTipFoundFlag = TRUE;

               CatheterTipRoiCoord->XMin = left_label_pos  [k];
               CatheterTipRoiCoord->XMax = right_label_pos [k];
               CatheterTipRoiCoord->YMin = top_label_pos   [k];
               CatheterTipRoiCoord->YMax = bottom_label_pos[k];
            }
         }

         // if n_coincide>1, this is a catheter body
         // keep track of the largest catheter body label
         if (n_coincide>1)
         {
            // check whether this object is straight enough 
            // to be considered as an extra catheter body in the picture
            // use moment analysis
            float *I = (float *) malloc(NbPelPerLab[k]*sizeof(I[0]));
            float *J = (float *) malloc(NbPelPerLab[k]*sizeof(J[0]));
            float Isum = 0;
            float Jsum = 0;
            float II = 0, IJ = 0, JJ = 0; // second order moments, inertia matrix
            float L1, L2;
            int   l = 0;
            
            for(i=IMin;i<=IMax;i++) 
            {
               for(j=JMin;j<=JMax;j++)
               { 
                  if (ImLab[i][j] == k+2)
                  {
                     I[l] = (float) i;
                     J[l] = (float) j;
                     Isum += i;
                     Jsum += j;
                     l++;
                  }
               }
            }
            Isum /= l;
            Jsum /= l;
            assert(l==NbPelPerLab[k]);

            // get second order moments
            for (l=0; l<NbPelPerLab[k]; l++)
            {
               II += (I[l]-Isum)*(I[l]-Isum);
               IJ += (I[l]-Isum)*(J[l]-Jsum);
               JJ += (J[l]-Jsum)*(J[l]-Jsum);
            }
            II /= NbPelPerLab[k];
            IJ /= NbPelPerLab[k];
            JJ /= NbPelPerLab[k];
            
            // eigen value analysis of matrix
            // [ II IJ ]
            // [ IJ JJ ]
            // i.e. lambdas that are solutions of (II-lambda)*(JJ-lambda)-IJ.^2=0
            // i.e. lambda.^2-(II+JJ)*lambda+II*JJ-IJ.^2=0
            { 
               float Det = (float)sqrt( (double)( RDG_SQ(II-JJ) + 4*RDG_SQ(IJ) ) );
               L2  = (II + JJ - Det)*(float)0.5;
               L1  = (II + JJ + Det)*(float)0.5;
            }

            // printf("L1 %f, L2 %f\n", L1, L2);

            free(I);
            free(J);

            if (*CatheterBodyLargestSize<NbPelPerLab[k] && L2<STRAIGHTNESS_MAX_L2)
            {
               *CatheterBodyLargestSize = NbPelPerLab[k];

               *CatheterBodyFoundFlag = TRUE;

               CatheterBodyRoiCoord->XMin = left_label_pos  [k];
               CatheterBodyRoiCoord->XMax = right_label_pos [k];
               CatheterBodyRoiCoord->YMin = top_label_pos   [k];
               CatheterBodyRoiCoord->YMax = bottom_label_pos[k];

               // Orientation given by double angle through tensor 
               // [ II IJ ]
               // [ IJ JJ ]
               // i.e. 
               // [  II^2-JJ^2   ]
               // [ 2*IJ*(II+JJ) ]
               if (CatheterBodyDir)
                  *CatheterBodyDir = (float) -(atan2(-2*IJ*(II+JJ), -(II*II-JJ*JJ)))/2;
            }
         }
      }

#if USE_ALLOCA_INSTEAD_OF_MALLOC
      // nothing to do, arrays were _allocated, not mallocated
#else
      // free mallocated blocks
      free(top_label_pos );
      free(bottom_label_pos );
      free(left_label_pos );
      free(right_label_pos );
      free(is_passing_through_corner );
#endif
   }
}


//**************************************************************************
// float cmp function needed by qsort
//**************************************************************************
static int flt_cmp(const void *f1, const void *f2) 
{ 
   float diff = *(float *)f1 - *(float *)f2 ;

   if (diff<=0)
   {
      if (diff==0) 
         return 0;
      else
         return -1;
   }
   else 
      return +1;
}


//**************************************************************************
// Get the median direction of thresholded pixels in the given ROI
//**************************************************************************
void RdgGetMedianDirection(float **ImDir, int **ImLab, int Iw, int Ih, 
                           CRdgRoiCoord *RoiCoord, float *MedianDirection)
{
   // Coordinates of ROI 
   int IMin = RDG_MAX(RoiCoord->YMin, 0);
   int IMax = RDG_MIN(RoiCoord->YMax, Ih-1);
   int JMin = RDG_MAX(RoiCoord->XMin, 0);
   int JMax = RDG_MIN(RoiCoord->XMax, Iw-1);

   int i, j;
   int n_pels = 0;

   // first, get number of thresholded pixels in ROI
   for(i=IMin;i<=IMax;i++) 
   {
      for(j=JMin;j<=JMax;j++)
      { 
         if (ImLab[i][j]>=2)
            n_pels++;
      }
   }

   // then, get direction values and median-filter them
   if (n_pels)
   {
      float *dir_array = (float *) _alloca(n_pels*sizeof(dir_array[0]));
      int    k = 0;

      for(i=IMin;i<=IMax;i++)
      {
         for(j=JMin;j<=JMax;j++)
         { 
            if (ImLab[i][j]>=2)
            {
               dir_array[k++] = ImDir[i][j];
            }
         }
      }
      assert(k==n_pels);

      qsort(dir_array, n_pels, sizeof(dir_array[0]), flt_cmp);

      // returned median-filter result
      *MedianDirection = dir_array[n_pels/2];
   }
}



//**************************************************************************
// Zeroes out the catheter body in ridgeness image, based on its mean direction
// (all ridgeness pixels having the same direction are set to zero)
//**************************************************************************
void RdgRemoveCatheterBody(float **ImRdg, float **ImDir, int Iw, int Ih, CRdgRoiCoord *RoiCoord, float MedianDirection, float AngleRange)
{
   // Coordinates of ROI 
   int IMin = RDG_MAX(RoiCoord->YMin, 0);
   int IMax = RDG_MIN(RoiCoord->YMax, Ih-1);
   int JMin = RDG_MAX(RoiCoord->XMin, 0);
   int JMax = RDG_MIN(RoiCoord->XMax, Iw-1);

   const float c     = (float) cos(MedianDirection);
   const float s     = (float) sin(MedianDirection);
   const float range = (float) cos(AngleRange/2);

   int i, j;

   for(i=IMin;i<=IMax;i++) 
   {
      for(j=JMin;j<=JMax;j++)
      { 
         float scalar_product = (float) (c*cos(ImDir[i][j])+s*sin(ImDir[i][j]));

         // check if ridge and catheter directions match
         if (scalar_product<-range || scalar_product>range)
            ImRdg[i][j] = 0;
      }
   }
}
