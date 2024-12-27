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
// RdgBackgroundEstimation : background estimation routine
// t: current time
// ImIn: input image at time t
// ImCircBuffer: circular buffer of images
// Iw: width of input image
// Ih: height of input image
// RoiCoord: ROI
// ImBkg: output background image
// ImFrg: output foreground image
//**************************************************************************
void RdgBackgroundEstimation(int t, float **ImIn, float ***ImCircBuffer, int Iw, int Ih, int CircBufferSize, CRdgRoiCoord *RoiCoord, float **ImBkg, float **ImFrg)
{
   // Coordinates of ROI for background estimation 
   int IMin1 = RDG_MAX(RoiCoord->YMin, 0);
   int IMax1 = RDG_MIN(RoiCoord->YMax, Ih-1);
   int JMin1 = RDG_MAX(RoiCoord->XMin, 0);
   int JMax1 = RDG_MIN(RoiCoord->XMax, Iw-1);
   int i, j, k;

   // copy input picture to circular buffer
   memcpy(ImCircBuffer[t%CircBufferSize][0], ImIn[0], Iw*Ih*sizeof(ImIn[0][0]));

   // init background picture by setting it to input picture
   memcpy(ImBkg[0], ImIn[0], Iw*Ih*sizeof(ImBkg[0][0]));

   // get the pixelwise minimum across scale
   for (k=0; k<RDG_MIN(t, CircBufferSize); k++)
   {
      // skip picture t%CircBufferSize, 
      // since this is the input image that initialized ImBkg
      if (k==t%CircBufferSize)
         continue;

      for(i=IMin1;i<=IMax1;i++) 
         for(j=JMin1;j<=JMax1;j++)
         { 
            float pixel = ImCircBuffer[k][i][j];

            if (ImBkg[i][j]>pixel)
            {
               ImBkg[i][j] = pixel;
            }
         }
   }

   // get the foreground image
   if (t>0)
      for(i=IMin1;i<=IMax1;i++) 
         for(j=JMin1;j<=JMax1;j++)
         { 
            ImFrg[i][j] = ImIn[i][j]-ImBkg[i][j];

            assert(ImFrg[i][j]>=0);
         }
   else
      // if first picture, too early to have a foreground picture
      // copy input one
      for(i=IMin1;i<=IMax1;i++) 
         for(j=JMin1;j<=JMax1;j++)
         { 
            ImFrg[i][j] = ImIn[i][j];
         }
}
