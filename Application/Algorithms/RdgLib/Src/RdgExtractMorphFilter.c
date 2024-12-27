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
#include <limits.h>
#include <RdgDev.h>


#define RDG_USE_SEPARABLE_MORPH_FILTERS 1


/*************************************************************************************/
// Morphological Grayscale Dilatation
//
// Se is an array containing zero and non-zero values, 
// giving the shape of the structuring element
/*************************************************************************************/
void RdgMorphDilate(short ** const Ori, 
                    short **Dil, const int Iw, const int Ih, 
                    CRdgRoiCoord *RoiCoord, 
                    short ** const Se /*structuring element */, const int Sew, const int Seh, 
                    const int Apw /* active point on width dimension */, const int Aph /* active point on height dimension */)
{
   const int IMin1 = RDG_MAX(RoiCoord->YMin, 0);
   const int IMax1 = RDG_MIN(RoiCoord->YMax, Ih-1);
   const int JMin1 = RDG_MAX(RoiCoord->XMin, 0);
   const int JMax1 = RDG_MIN(RoiCoord->XMax, Iw-1);

   int i, j;

   for (j=JMin1; j<=JMax1; j++)
   {
      for (i=IMin1; i<=IMax1; i++)
      {
         short maxvalue = 0; // maximum gray level value over structuring element
         int m, n;

         for (m=0; m<Seh; m++)
         {
            for (n=0; n<Sew; n++)
            {
               int ii = i+m-Aph;
               int jj = j+n-Apw;

               if (maxvalue<Ori[ii][jj] && Se[m][n]>0 && ii>=IMin1 && ii<=IMax1 && jj>=JMin1 && jj<=JMax1)
                  maxvalue = Ori[ii][jj];
            }
         }

         Dil[i][j] = maxvalue;
      }
   }
}


/*************************************************************************************/
// Morphological Grayscale Erosion
//
// Se is an array containing zero and non-zero values, 
// giving the shape of the structuring element
/*************************************************************************************/
void RdgMorphErode(short ** const Ori, 
                   short **Dil, const int Iw, const int Ih, 
                   CRdgRoiCoord *RoiCoord, 
                   short ** const Se /* structuring element */, const int Sew, const int Seh, 
                   const int Apw /* active point on width dimension */, const int Aph /* active point on height dimension */)
{
   const int IMin1 = RDG_MAX(RoiCoord->YMin, 0);
   const int IMax1 = RDG_MIN(RoiCoord->YMax, Ih-1);
   const int JMin1 = RDG_MAX(RoiCoord->XMin, 0);
   const int JMax1 = RDG_MIN(RoiCoord->XMax, Iw-1);

   int i, j;

   for (j=JMin1; j<=JMax1; j++)
   {
      for (i=IMin1; i<=IMax1; i++)
      {
         short minvalue = SHRT_MAX; // minimum gray level value over structuring element
         int m, n;

         for (m=0; m<Seh; m++)
         {
            for (n=0; n<Sew; n++)
            {
               int ii = i+m-Aph;
               int jj = j+n-Apw;

               if (minvalue>Ori[ii][jj] && Se[m][n]>0 && ii>=IMin1 && ii<=IMax1 && jj>=JMin1 && jj<=JMax1)
                  minvalue = Ori[ii][jj];
            }
         }

         Dil[i][j] = minvalue;
      }
   }
}


/*************************************************************************************/
// Morphological Grayscale Dilatation
// 
// Structuring Element is rectangular
/*************************************************************************************/
void RdgMorphDilateRectSE(short ** const Ori, 
                          short **Dil, const int Iw, const int Ih, 
                          CRdgRoiCoord *RoiCoord, 
                          const int Sew, const int Seh, // sructuring element width and height */
                          const int Apw /* active point on width dimension */, const int Aph /* active point on height dimension */)
{
   const int IMin1 = RDG_MAX(RoiCoord->YMin, 0);
   const int IMax1 = RDG_MIN(RoiCoord->YMax, Ih-1);
   const int JMin1 = RDG_MAX(RoiCoord->XMin, 0);
   const int JMax1 = RDG_MIN(RoiCoord->XMax, Iw-1);

   int i, j;

#if RDG_USE_SEPARABLE_MORPH_FILTERS

   // since structuring element is rectangular, use separable filters
   short **tmpDil;

   RdgImAlloc(NULL, &tmpDil, sizeof(tmpDil[0][0]), Iw, Ih);

   //
   // first, operate on colums
   //
   for (j=JMin1; j<=JMax1; j++)
   {
      for (i=IMin1; i<=IMax1; i++)
      {
         short maxvalue = 0; // maximum gray level value over column structuring element
         int   m;

         for (m=0; m<Seh; m++)
         {

            int ii = i+m-Aph;              

            if (maxvalue<Ori[ii][j] && ii>=IMin1 && ii<=IMax1)
               maxvalue = Ori[ii][j];
         }

         tmpDil[i][j] = maxvalue;
      }
   }

   // 
   // second, operate on lines
   //
   for (i=IMin1; i<=IMax1; i++)
   {
      for (j=JMin1; j<=JMax1; j++)
      {
         short maxvalue = 0; // maximum gray level value over line structuring element
         int   n;

         for (n=0; n<Sew; n++)
         {
            int jj = j+n-Apw;

            if (maxvalue<tmpDil[i][jj] && jj>=JMin1 && jj<=JMax1)
               maxvalue = tmpDil[i][jj];
         }

         Dil[i][j] = maxvalue;
      }
   }

   RdgImFree(NULL, tmpDil);

#else // RDG_USE_SEPARABLE_MORPH_FILTERS

   for (j=JMin1; j<=JMax1; j++)
   {
      for (i=IMin1; i<=IMax1; i++)
      {
         short maxvalue = 0; // maximum gray level value over structuring element
         int m, n;

         for (m=0; m<Seh; m++)
         {
            for (n=0; n<Sew; n++)
            {
               int ii = i+m-Aph;
               int jj = j+n-Apw;

               if (maxvalue<Ori[ii][jj] && ii>=IMin1 && ii<=IMax1 && jj>=JMin1 && jj<=JMax1)
                  maxvalue = Ori[ii][jj];
            }
         }

         Dil[i][j] = maxvalue;
      }
   }

#endif // else RDG_USE_SEPARABLE_MORPH_FILTERS

}


/*************************************************************************************/
// Morphological Grayscale Erosion
// 
// Structuring Element is rectangular
/*************************************************************************************/
void RdgMorphErodeRectSE(short ** const Ori, 
                   short **Dil, const int Iw, const int Ih, 
                   CRdgRoiCoord *RoiCoord, 
                   const int Sew, const int Seh, /* structuring element width and height */
                   const int Apw /* active point on width dimension */, const int Aph /* active point on height dimension */)
{
   const int IMin1 = RDG_MAX(RoiCoord->YMin, 0);
   const int IMax1 = RDG_MIN(RoiCoord->YMax, Ih-1);
   const int JMin1 = RDG_MAX(RoiCoord->XMin, 0);
   const int JMax1 = RDG_MIN(RoiCoord->XMax, Iw-1);

   int i, j;

#if RDG_USE_SEPARABLE_MORPH_FILTERS

   // since structuring element is rectangular, use separable filters
   short **tmpDil;

   RdgImAlloc(NULL, &tmpDil, sizeof(tmpDil[0][0]), Iw, Ih);

   //
   // first, operate on colums
   //
   for (j=JMin1; j<=JMax1; j++)
   {
      for (i=IMin1; i<=IMax1; i++)
      {
         short minvalue = SHRT_MAX; // minimum gray level value over column structuring element
         int m;

         for (m=0; m<Seh; m++)
         {

            int ii = i+m-Aph;              

            if (minvalue>Ori[ii][j] && ii>=IMin1 && ii<=IMax1)
               minvalue = Ori[ii][j];
         }

         tmpDil[i][j] = minvalue;
      }
   }

   // 
   // second, operate on lines
   //
   for (i=IMin1; i<=IMax1; i++)
   {
      for (j=JMin1; j<=JMax1; j++)
      {
         short minvalue = SHRT_MAX; // minimum gray level value over line structuring element
         int n;

         for (n=0; n<Sew; n++)
         {
            int jj = j+n-Apw;

            if (minvalue>tmpDil[i][jj] && jj>=JMin1 && jj<=JMax1)
               minvalue = tmpDil[i][jj];
         }

         Dil[i][j] = minvalue;
      }
   }

   RdgImFree(NULL, tmpDil);

#else // RDG_USE_SEPARABLE_MORPH_FILTERS

   for (j=JMin1; j<=JMax1; j++)
   {
      for (i=IMin1; i<=IMax1; i++)
      {
         short minvalue = SHRT_MAX; // minimum gray level value over structuring element
         int m, n;

         for (m=0; m<Seh; m++)
         {
            for (n=0; n<Sew; n++)
            {
               int ii = i+m-Aph;
               int jj = j+n-Apw;

               if (minvalue>Ori[ii][jj] && ii>=IMin1 && ii<=IMax1 && jj>=JMin1 && jj<=JMax1)
                  minvalue = Ori[ii][jj];
            }
         }

         Dil[i][j] = minvalue;
      }
   }

#endif // else RDG_USE_SEPARABLE_MORPH_FILTERS
}