// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <memory.h>
#include <assert.h>

#include <RdgDev.h> 



//**************************************************************************
// RdgBinaryMaskFromLabels: creates a binary mask from labels
//**************************************************************************
void RdgBinaryMaskFromLabels(unsigned char **ImMask, int **ImLab, int Iw, int Ih, CRdgRoiCoord *RoiCoord)
{
   // Coordinates of ROI for background estimation 
   int IMin1 = RDG_MAX(RoiCoord->YMin, 0);
   int IMax1 = RDG_MIN(RoiCoord->YMax, Ih-1);
   int JMin1 = RDG_MAX(RoiCoord->XMin, 0);
   int JMax1 = RDG_MIN(RoiCoord->XMax, Iw-1);
   int i, j;

   // init mask
   memset(ImMask[0], 0, Iw*Ih*sizeof(ImMask[0][0]));

   // fill in mask at the location of labels
   for(i=IMin1;i<=IMax1;i++) 
      for(j=JMin1;j<=JMax1;j++)
      { 
         if (ImLab[i][j]>=2) // first label number is 2, not 1 (which means thresholded but unlabelled)
            ImMask[i][j] = 1;         
      }
}



//**************************************************************************
// RdgBinaryMaskFromLabelsAndMask: creates a binary mask from labels and merges with given mask
//**************************************************************************
void RdgBinaryMaskFromLabelsAndMask(unsigned char **ImMask, int **ImLab, unsigned char **ImMergeMask, int Iw, int Ih, CRdgRoiCoord *RoiCoord)
{
   // Coordinates of ROI for background estimation 
   int IMin1 = RDG_MAX(RoiCoord->YMin, 0);
   int IMax1 = RDG_MIN(RoiCoord->YMax, Ih-1);
   int JMin1 = RDG_MAX(RoiCoord->XMin, 0);
   int JMax1 = RDG_MIN(RoiCoord->XMax, Iw-1);
   int i, j;

   // init mask
   memset(ImMask[0], 0, Iw*Ih*sizeof(ImMask[0][0]));

   // fill in mask at the location of labels
   for(i=IMin1;i<=IMax1;i++) 
      for(j=JMin1;j<=JMax1;j++)
      { 
         if (ImMergeMask[i][j] || ImLab[i][j]>=2) // first label number is 2, not 1 (which means thresholded but unlabelled) */
            ImMask[i][j] = 1;         
      }
}
