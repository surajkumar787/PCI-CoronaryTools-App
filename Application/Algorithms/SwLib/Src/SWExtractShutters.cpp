//Copyright (c)2014 Koninklijke Philips Electronics N.V.,
//All Rights Reserved.
//
//This source code and any compilation or derivative thereof is the
//proprietary information of Koninklijke Philips Electronics N.V.
//and is confidential in nature.
//Under no circumstances is this software to be combined with any Open Source
//Software in any way or placed under an Open Source License of any type 
//without the express written permission of Koninklijke Philips 
//Electronics N.V.

#include <SWBxGaussian.h>
#include <SWLibDev.h>

#define SW_LIB_SHUTTERS_NON_EXTREMA_SCALE 10
#define SW_LIB_SHUTTERS_EDGENESS_SCALE 4
#define SW_LIB_SHUTTER_NON_SHUTTER_MIN_LEVEL_RATIO 1.3f
#define SW_LIB_PICTURE_BORDER_NORMALIZED_VARIANCE .001f
#define SW_LIB_REFERENCE_SCALE_IW 1024
#define SW_LIB_SHUTTER_SYMMETRICITYLIMIT 40


enum {
	SW_LIB_SHUTEXT_OK = 0,					// Processing Ok
	SW_LIB_SHUTEXT_WARNING_LIM_ABOVE_HALF, // (At least) one of the components was estimated on the wrong half of the image
	SW_LIB_SHUTEXT_ERR_LIM					// (At least) one of the shutter position had max and min confused
};

// Provided as a convenience function
// Detection of Shutters/Borders
int SWExtractShutters(short* InIma, int Iw, int Ih, CSWRoiCoord *RoiCoord)
{
   int               res;

   // algorithm parameters
   int   safety_margin; // number of pixels as safety margin
   int   non_extrema_scale;
   int   edgeness_scale;
   float shutter_non_shutter_min_level_ratio; // minimum level ratio between shuttered areas and non shuttered areas
   float picture_border_normalized_variance; // maximum allowed variance for picture border

   int left_pos, right_pos, top_pos, bottom_pos;
   int i, j;

   int x, y, xMin, yMin, xMax, yMax, offset, lineConstant, columnConstant, symmetricityLimit;
   short val;

   short *GauIma;
   int   *col_sum ;
   int   *row_sum;
   float *col_sq_sum;
   float *row_sq_sum;
   int   *col_gradient_sum;
   int   *row_gradient_sum;
   int   *tmp_array;

   GauIma = (short*)malloc(Iw*Ih*sizeof(short));

   safety_margin = 0;//SW_LIB_SHUTTERS_SAFETY_MARGIN * Iw / SW_LIB_REFERENCE_SCALE_IW; 
   non_extrema_scale = SW_LIB_SHUTTERS_NON_EXTREMA_SCALE * Iw / SW_LIB_REFERENCE_SCALE_IW;
   edgeness_scale = SW_LIB_SHUTTERS_EDGENESS_SCALE * Iw / SW_LIB_REFERENCE_SCALE_IW;
   shutter_non_shutter_min_level_ratio = SW_LIB_SHUTTER_NON_SHUTTER_MIN_LEVEL_RATIO;
   picture_border_normalized_variance = SW_LIB_PICTURE_BORDER_NORMALIZED_VARIANCE;
   symmetricityLimit = SW_LIB_SHUTTER_SYMMETRICITYLIMIT *  Iw / SW_LIB_REFERENCE_SCALE_IW;

   // Look for digital shutter - VA
   val = InIma[0];
   yMin = 0; lineConstant = 1;
   while ((yMin < Ih-1) && (lineConstant == 1)){
	   offset = yMin * Iw;
	   for (x=0; x<Iw; x++)
		   if (InIma[offset+x] != val){
			   lineConstant = 0;
			   break;
		   }
		if (lineConstant)
			yMin++;
   }
   yMax = Ih-1; lineConstant = 1;
   while ((yMax > yMin) && (lineConstant == 1)){
	   offset = yMax * Iw;
	   for (x=0; x<Iw; x++)
		   if (InIma[offset+x] != val){
			   lineConstant = 0;
			   break;
		   }
		if (lineConstant)
			yMax--;
   }
   xMin = 0; columnConstant = 1;
   while ((xMin < Iw-1) && (columnConstant == 1)){
	   for (y=0; y<Ih; y++)
		   if (InIma[y*Iw+xMin] != val){
			   columnConstant = 0;
			   break;
		   }
		if (columnConstant)
			xMin++;
   }
   xMax = Iw-1; columnConstant = 1;
   while ((xMax > xMin) && (columnConstant == 1)){
	   for (y=0; y<Ih; y++)
		   if (InIma[y*Iw+xMax] != val){
			   columnConstant = 0;
			   break;
		   }
		if (columnConstant)
			xMax--;
   }

   // run a gaussian filter over the picture


   // alloc memory
   SW_LIB_CALLOC(col_sum, Iw);
   SW_LIB_CALLOC(row_sum, Ih);
   SW_LIB_CALLOC(col_sq_sum, Iw);
   SW_LIB_CALLOC(row_sq_sum, Ih);
   SW_LIB_CALLOC(col_gradient_sum, Iw);
   SW_LIB_CALLOC(row_gradient_sum, Ih);
   SW_LIB_MALLOC(tmp_array, SW_LIB_MAX(Iw, Ih));

#if SW_LIB_USE_SSE_GAUSSIAN
   if (Iw%4==0)
   {
      // necessary to copy picture in GauIma, since memory alignment cannot be guaranteed for InIma
      SW_LIB_MEMCPY(GauIma, InIma, Iw*Ih);
    
      SWBxIsoGaussFilterFastShort2D(GauIma, GauIma, (float) edgeness_scale, Iw, Ih);
   }
   else
      SWBxIsoGaussFilterShort2D(InIma, GauIma, (float) edgeness_scale, Iw, Ih);
#else
   SWBxIsoGaussFilterShort2D(InIma, GauIma, (float) edgeness_scale, Iw, Ih);
#endif

   // compute sums of pixels on columns and lines
   for (i=yMin; i<=yMax; i++)
   {
      for (j=xMin; j<=xMax; j++)
      {
         col_sum[j] += GauIma[i*Iw+j];
         row_sum[i] += GauIma[i*Iw+j];
      }
   }
   // compute sums of gradients on columns and lines
   for (i=yMin; i<=yMax-1; i++)
   {
      row_gradient_sum[i] = row_sum[i+1]-row_sum[i];
   }
   row_gradient_sum[Ih-1] = 0;
   for (j=xMin; j<=xMax-1; j++)
   {
      col_gradient_sum[j] = col_sum[j+1]-col_sum[j];
   }
   col_gradient_sum[Iw-1] = 0;
   
   // filter once more the gradients before doing the non-extrema suppression
   SW_LIB_MEMCPY(tmp_array, row_gradient_sum, Ih);
   for (i=yMin+1; i<=yMax-1; i++)
   {
      int k;
      int sum = 0;

      for (k=-1; k<=1; k++)
         sum += tmp_array[i+k];

      row_gradient_sum[i] = sum;
   }
   row_gradient_sum[0] = tmp_array[0]+tmp_array[1];
   SW_LIB_MEMCPY(tmp_array, col_gradient_sum, Iw);
   for (j=xMin+1; j<=xMax-1; j++)
   {
      int k;
      int sum = 0;

      for (k=-1; k<=1; k++)
         sum += tmp_array[j+k];

      col_gradient_sum[j] = sum;
   }
   col_gradient_sum[0] = tmp_array[0]+tmp_array[1];

   // recompute col/row sum and compute col/row variances on non filtered pixels
   SW_LIB_MEMZERO(col_sum, Iw);
   SW_LIB_MEMZERO(row_sum, Ih);
   for (i=yMin; i<=yMax; i++)
   {
      for (j=xMin; j<=xMax; j++)
      {
         col_sum[j] += InIma[i*Iw+j];
         row_sum[i] += InIma[i*Iw+j];

         col_sq_sum[j] += ((float) InIma[i*Iw+j])*InIma[i*Iw+j];
         row_sq_sum[i] += ((float) InIma[i*Iw+j])*InIma[i*Iw+j];
      }
   }

   // transform col_sq_sum and row_sq_sum in second order moments 
   // (= sum of (pixel-mean(pixel))^2)
   for (i=yMin; i<=yMax; i++)
   {
      row_sq_sum[i] = (row_sq_sum[i]-SW_LIB_SQ((float) row_sum[i])/Iw);
   }
	for (j=xMin; j<=xMax; j++)
   {
      col_sq_sum[j] = (col_sq_sum[j]-SW_LIB_SQ((float) col_sum[j])/Ih);
   }

   // non-extrema suppression
   SW_LIB_MEMCPY(tmp_array, row_gradient_sum, Ih);
   for (i=yMin; i<=yMax; i++)
   {
      //if (i==0 || i==Ih-1 || (tmp_array[i-1]<tmp_array[i] && tmp_array[i]<tmp_array[i+1]) || (tmp_array[i-1]>tmp_array[i] && tmp_array[i]>tmp_array[i+1]))
      //   row_gradient_sum[i] = 0;
      int min = INT_MAX;
      int max = INT_MIN;
      int k;

      for (k=-non_extrema_scale; k<=non_extrema_scale; k++)
      {
         if (i+k>=yMin && i+k<=yMax)
         {
            if (min>tmp_array[i+k])
               min = tmp_array[i+k];
            if (max<tmp_array[i+k])
               max = tmp_array[i+k];
         }
      }

      // non-extrema condition
      if (i==yMin || i==yMax || (min!=tmp_array[i] && max!=tmp_array[i]))
         row_gradient_sum[i] = 0;
   }
   SW_LIB_MEMCPY(tmp_array, col_gradient_sum, Iw);
   for (j=xMin; j<=xMax; j++)
   {
      //if (j==0 || j==Iw-1 || (tmp_array[j-1]<tmp_array[j] && tmp_array[j]<tmp_array[j+1]) || (tmp_array[j-1]>tmp_array[j] && tmp_array[j]>tmp_array[j+1]))
      //   col_gradient_sum[j] = 0;
      int min = INT_MAX;
      int max = INT_MIN;
      int l;

      for (l=-non_extrema_scale; l<=non_extrema_scale; l++)
      {
         if (j+l>=xMin && j+l<=xMax)
         {
            if (min>tmp_array[j+l])
               min = tmp_array[j+l];
            if (max<tmp_array[j+l])
               max = tmp_array[j+l];
         }
      }

      // non-extrema condition
      if (j==xMin || j==xMax || (min!=tmp_array[j] && max!=tmp_array[j]))
         col_gradient_sum[j] = 0;
   }

   // initialize shutter position
   left_pos = xMin;
   right_pos = xMax;
   top_pos = yMin;
   bottom_pos = yMax;

   // 
   // search left shutter position
   //
   for (j=xMin; j<=xMax; j++)
   {
      if (col_gradient_sum[j]>0 && j>xMin+2*edgeness_scale)
      {
         int k, n_points;
         float left_mean, right_mean, right_var;

         // test this position

         // left mean pixel luminance
         left_mean = 0;
         n_points = 0;
         for (k=j-3*edgeness_scale; k<=j-edgeness_scale; k++)
         {
            if (k>=xMin && k<=xMax)
            {
               left_mean += col_sum[k];
               n_points++;
            }
         }
         if (n_points)
            left_mean /= n_points*(yMax-yMin+1);

         // right mean pixel luminance
         right_mean = 0;
         n_points = 0;
         for (k=j+edgeness_scale; k<=j+3*edgeness_scale; k++)
         {
            if (k>=xMin && k<xMax)
            {
               right_mean += col_sum[k];
               n_points++;
            }
         }
         if (n_points)
            right_mean /= n_points*(yMax-yMin+1);

         // right mean column-variance
         right_var = 0;
         n_points = 0;
         for (k=j+edgeness_scale; k<=j+3*edgeness_scale; k++)
         {
            if (k>=xMin && k<=xMax)
            {
               right_var += col_sq_sum[k];
               n_points++;
            }
         }
         if (n_points)
            right_var /= n_points*(yMax-yMin+1);

         // left shutter condition
         if (left_mean*shutter_non_shutter_min_level_ratio<right_mean && right_var>picture_border_normalized_variance*SW_LIB_SQ(right_mean))
         {
            left_pos = j;
            break;
         }
      }
   }

   //
   // if no left shutter was found, look for potential picture solid border
   //
   if (left_pos==xMin)
   {
	  for (j=xMin; j<=xMax; j++)
      {
         if (col_gradient_sum[j]!=0 && j>2*edgeness_scale)
         {
            int k, n_points;
            float left_mean, right_mean, left_var, right_var;

            // test this position

            // left mean pixel luminance
            left_mean = 0;
            n_points = 0;
            for (k=j-3*edgeness_scale; k<=j-edgeness_scale; k++)
            {
               if (k>=xMin && k<=xMax)
               {
                  left_mean += col_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               left_mean /= n_points*Ih;

            // right mean pixel luminance
            right_mean = 0;
            n_points = 0;
            for (k=j+edgeness_scale; k<=j+3*edgeness_scale; k++)
            {
               if (k>=xMin && k<=xMax)
               {
                  right_mean += col_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               right_mean /= n_points*(yMax-yMin+1);

            // left mean column-variance
            left_var = 0;
            n_points = 0;
            for (k=j-3*edgeness_scale; k<=j-edgeness_scale; k++)
            {
               if (k>=xMin && k<=xMax)
               {
                  left_var += col_sq_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               left_var /= n_points*(yMax-yMin+1);

            // right mean column-variance
            right_var = 0;
            n_points = 0;
            for (k=j+edgeness_scale; k<=j+3*edgeness_scale; k++)
            {
               if (k>=xMin && k<=xMax)
               {
                  right_var += col_sq_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               right_var /= n_points*(yMax-yMin+1);

            // left border condition
            if (left_var<=picture_border_normalized_variance*SW_LIB_SQ(left_mean) && right_var>picture_border_normalized_variance*SW_LIB_SQ(right_mean))
            {
               left_pos = j;
               break;
            }
         }
      }
   }

   // 
   // search right shutter position
   //
   for (j=xMax; j>=xMin; j--)
   {
      if (j<Iw-1-2*edgeness_scale && col_gradient_sum[j]<0)
      {
         int k, n_points;
         float left_mean, right_mean, left_var;

         // test this position

         // left mean pixel luminance
         left_mean = 0;
         n_points = 0;
         for (k=j-3*edgeness_scale; k<=j-edgeness_scale; k++)
         {
           if (k>=xMin && k<=xMax)
            {
               left_mean += col_sum[k];
               n_points++;
            }
         }
         if (n_points)
            left_mean /= n_points*(yMax-yMin+1);

         // right mean pixel luminance
         right_mean = 0;
         n_points = 0;
         for (k=j+edgeness_scale; k<=j+3*edgeness_scale; k++)
         {
            if (k>=xMin && k<=xMax)
            {
               right_mean += col_sum[k];
               n_points++;
            }
         }
         if (n_points)
            right_mean /= n_points*(yMax-yMin+1);

         // left mean column-variance
         left_var = 0;
         n_points = 0;
         for (k=j-3*edgeness_scale; k<=j-edgeness_scale; k++)
         {
            if (k>=xMin && k<=xMax)
            {
               left_var += col_sq_sum[k];
               n_points++;
            }
         }
         if (n_points)
            left_var /= n_points*(yMax-yMin+1);

         // right shutter condition
         if (left_mean>right_mean*shutter_non_shutter_min_level_ratio && left_var>picture_border_normalized_variance*SW_LIB_SQ(left_mean))
         {
            right_pos = j;
            break;
         }
      }
   }

   // 
   // if none found, look for potential picture solid border
   //


   if (right_pos==xMax)
   {
      for (j=xMax; j>=xMin; j--)
      {
         if (j<Iw-1-2*edgeness_scale && col_gradient_sum[j]<0)
         {
            int k, n_points;
            float left_mean, right_mean, left_var, right_var;

            // test this position

            // left mean pixel luminance
            left_mean = 0;
            n_points = 0;
            for (k=j-3*edgeness_scale; k<=j-edgeness_scale; k++)
            {
               if (k>=xMin && k<xMax)
               {
                  left_mean += col_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               left_mean /= n_points*(yMax-yMin+1);

            // right mean pixel luminance
            right_mean = 0;
            n_points = 0;
            for (k=j+edgeness_scale; k<=j+3*edgeness_scale; k++)
            {
               if (k>=xMin && k<xMax)
               {
                  right_mean += col_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               right_mean /= n_points*Ih;

            // left mean column-variance
            left_var = 0;
            n_points = 0;
            for (k=j-3*edgeness_scale; k<=j-edgeness_scale; k++)
            {
               if (k>=xMin && k<xMax)
               {
                  left_var += col_sq_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               left_var /= n_points*(yMax-yMin+1);

            // right mean column-variance
            right_var = 0;
            n_points = 0;
            for (k=j+edgeness_scale; k<=j+3*edgeness_scale; k++)
            {
               if (k>=xMin && k<xMax)
               {
                  right_var += col_sq_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               right_var /= n_points*(yMax-yMin+1);

            // right border condition
            if (left_var>picture_border_normalized_variance*SW_LIB_SQ(left_mean) && right_var<picture_border_normalized_variance*SW_LIB_SQ(right_mean))
            {
               right_pos = j;
               break;
            }
         }
      }
   }

   
   // 
   // search top shutter position
   //
   for (i=yMin; i<=yMax; i++)
   {
      if (row_gradient_sum[i]>0 && i>2*edgeness_scale)
      {
         int k, n_points;
         float top_mean, bottom_mean, bottom_var;

         // test this position

         // top mean pixel luminance
         top_mean = 0;
         n_points = 0;
         for (k=i-3*edgeness_scale; k<=i-edgeness_scale; k++)
         {
            if (k>=yMin && k<=yMax)
            {
               top_mean += row_sum[k];
               n_points++;
            }
         }
         if (n_points)
            top_mean /= n_points*(xMax-xMin+1);

         // bottom mean pixel luminance
         bottom_mean = 0;
         n_points = 0;
         for (k=i+edgeness_scale; k<=i+3*edgeness_scale; k++)
         {
            if (k>=yMin && k<=yMax)
            {
               bottom_mean += row_sum[k];
               n_points++;
            }
         }
         if (n_points)
            bottom_mean /= n_points*(xMax-xMin+1);

         // bottom mean column-variance
         bottom_var = 0;
         n_points = 0;
         for (k=i+edgeness_scale; k<=i+3*edgeness_scale; k++)
         {
            if (k>=yMin && k<=yMax)
            {
               bottom_var += row_sq_sum[k];
               n_points++;
            }
         }
         if (n_points)
            bottom_var /= n_points*(xMax-xMin+1);

         // top shutter condition
         if (top_mean*shutter_non_shutter_min_level_ratio<bottom_mean && bottom_var>picture_border_normalized_variance*SW_LIB_SQ(bottom_mean))
         {
            top_pos = i;
            break;
         }
      }
   }

   //
   // if no top shutter was found, look for potential picture solid border
   //
   if (top_pos==yMin)
   {
      for (i=yMin; i<=yMax; i++)
      {
         if (row_gradient_sum[i]!=0 && i>2*edgeness_scale)
         {
            int k, n_points;
            float top_mean, bottom_mean, top_var, bottom_var;

            // test this position

            // top mean pixel luminance
            top_mean = 0;
            n_points = 0;
            for (k=i-3*edgeness_scale; k<=i-edgeness_scale; k++)
            {
               if (k>=yMin && k<=yMax)
               {
                  top_mean += row_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               top_mean /= n_points*(xMax-xMin+1);

            // bottom mean pixel luminance
            bottom_mean = 0;
            n_points = 0;
            for (k=i+edgeness_scale; k<=i+3*edgeness_scale; k++)
            {
               if (k>=yMin && k<=yMax)
               {
                  bottom_mean += row_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               bottom_mean /= n_points*(xMax-xMin+1);

            // top mean column-variance
            top_var = 0;
            n_points = 0;
            for (k=i-3*edgeness_scale; k<=i-edgeness_scale; k++)
            {
               if (k>=yMin && k<=yMax)
               {
                  top_var += row_sq_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               top_var /= n_points*(xMax-xMin+1);

            // bottom mean column-variance
            bottom_var = 0;
            n_points = 0;
            for (k=i+edgeness_scale; k<=i+3*edgeness_scale; k++)
            {
               if (k>=yMin && k<=yMax)
               {
                  bottom_var += row_sq_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               bottom_var /= n_points*(xMax-xMin+1);

            // top border condition
            if (top_var<=picture_border_normalized_variance*SW_LIB_SQ(top_mean) && bottom_var>picture_border_normalized_variance*SW_LIB_SQ(bottom_mean))
            {
               top_pos = i;
               break;
            }
         }
      }
   }

   // 
   // search bottom shutter position
   //
   for (i=yMax; i>=yMin; i--)
   {
      if (i<Ih-1-2*edgeness_scale && row_gradient_sum[i]<0)
      {
         int k, n_points;
         float top_mean, bottom_mean, top_var;

         // test this position

         // top mean pixel luminance
         top_mean = 0;
         n_points = 0;
         for (k=i-3*edgeness_scale; k<=i-edgeness_scale; k++)
         {
            if (k>=yMin && k<yMax)
            {
               top_mean += row_sum[k];
               n_points++;
            }
         }
         if (n_points)
            top_mean /= n_points*(xMax-xMin+1);

         // bottom mean pixel luminance
         bottom_mean = 0;
         n_points = 0;
         for (k=i+edgeness_scale; k<=i+3*edgeness_scale; k++)
         {
            if (k>=yMin && k<yMax)
            {
               bottom_mean += row_sum[k];
               n_points++;
            }
         }
         if (n_points)
            bottom_mean /= n_points*(xMax-xMin+1);

         // top mean column-variance
         top_var = 0;
         n_points = 0;
         for (k=i-3*edgeness_scale; k<=i-edgeness_scale; k++)
         {
            if (k>=yMin && k<yMax)
            {
               top_var += row_sq_sum[k];
               n_points++;
            }
         }
         if (n_points)
            top_var /= n_points*(xMax-xMin+1);

         // bottom shutter condition
         if (top_mean>bottom_mean*shutter_non_shutter_min_level_ratio && top_var>picture_border_normalized_variance*SW_LIB_SQ(top_mean))
         {
            bottom_pos = i;
            break;
         }
      }
   }

   // 
   // if none found, look for potential picture solid border
   //
   if (bottom_pos==yMax)
   {
      for (i=yMax; i>=yMin; i--)
      {
         if (i<Ih-1-2*edgeness_scale && row_gradient_sum[i]<0)
         {
            int k, n_points;
            float top_mean, bottom_mean, top_var, bottom_var;

            // test this position

            // top mean pixel luminance
            top_mean = 0;
            n_points = 0;
            for (k=i-3*edgeness_scale; k<=i-edgeness_scale; k++)
            {
               if (k>=yMin && k<=yMax)
               {
                  top_mean += row_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               top_mean /= n_points*(xMax-xMin+1);

            // bottom mean pixel luminance
            bottom_mean = 0;
            n_points = 0;
            for (k=i+edgeness_scale; k<=i+3*edgeness_scale; k++)
            {
               if (k>=yMin && k<=yMax)
               {
                  bottom_mean += row_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               bottom_mean /= n_points*(xMax-xMin+1);

            // top mean column-variance
            top_var = 0;
            n_points = 0;
            for (k=i-3*edgeness_scale; k<=i-edgeness_scale; k++)
            {
               if (k>=yMin && k<=yMax)
               {
                  top_var += row_sq_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               top_var /= n_points*(xMax-xMin+1);

            // bottom mean column-variance
            bottom_var = 0;
            n_points = 0;
            for (k=i+edgeness_scale; k<=i+3*edgeness_scale; k++)
            {
               if (k>=yMin && k<=yMax)
               {
                  bottom_var += row_sq_sum[k];
                  n_points++;
               }
            }
            if (n_points)
               bottom_var /= n_points*(xMax-xMin+1);

            // bottom border condition
            if (top_var>picture_border_normalized_variance*SW_LIB_SQ(top_mean) && bottom_var<picture_border_normalized_variance*SW_LIB_SQ(bottom_mean))
            {
               bottom_pos = j;
               break;
            }
         }
      }
   }

   // fill in RoiCoord return struct - tests on results values
   RoiCoord->XMin   = SW_LIB_MIN(left_pos  +safety_margin, Iw);
   RoiCoord->XMax   = SW_LIB_MAX(right_pos -safety_margin, 0);

   res = SW_LIB_SHUTEXT_OK;
   if (RoiCoord->XMin >= RoiCoord->XMax){
		res = SW_LIB_SHUTEXT_ERR_LIM;

		//Default values
		if (RoiCoord->XMin >= Iw/2){
			if (xMin < Iw/2)
				RoiCoord->XMin = SW_LIB_MIN(xMin  +safety_margin, Iw);
			else
				RoiCoord->XMin = safety_margin;
		}
		if (RoiCoord->XMax < Iw/2){
			if (xMax >= Iw/2)
				RoiCoord->XMax = SW_LIB_MAX(xMax-safety_margin, 0);
			else
				RoiCoord->XMax = Iw-safety_margin;
		}				
   } else {
		//Default values
		if (RoiCoord->XMin >= Iw/2){
			if (res == SW_LIB_SHUTEXT_OK)
				res = SW_LIB_SHUTEXT_WARNING_LIM_ABOVE_HALF;

			if (xMin < Iw/2)
				RoiCoord->XMin = SW_LIB_MIN(xMin+safety_margin, Iw);
			else
				RoiCoord->XMin = safety_margin;
		}

		if (RoiCoord->XMax < Iw/2){
			if (res == SW_LIB_SHUTEXT_OK)
				res = SW_LIB_SHUTEXT_WARNING_LIM_ABOVE_HALF;

			if (xMax >= Iw/2)
				RoiCoord->XMax = SW_LIB_MAX(xMax-safety_margin, 0);
			else
				RoiCoord->XMax = Iw-safety_margin;
		}	
   }

   RoiCoord->YMin   = SW_LIB_MIN(top_pos   +safety_margin, Ih);
   RoiCoord->YMax   = SW_LIB_MAX(bottom_pos-safety_margin, 0);

   if (RoiCoord->YMin >= RoiCoord->YMax){
		res = SW_LIB_SHUTEXT_ERR_LIM;

		//Default values
		if (RoiCoord->YMin >= Ih/2){
			if (yMin < Ih/2)
				RoiCoord->YMin = SW_LIB_MIN(yMin+safety_margin, Ih);
			else
				RoiCoord->YMin = safety_margin;
		}
		if (RoiCoord->YMax < Ih/2){
			if (yMax >= Ih/2)
				RoiCoord->YMax = SW_LIB_MAX(yMax-safety_margin, 0);
			else
				RoiCoord->YMax = Ih-safety_margin;
		}				
   } else {
		//Default values
		if (RoiCoord->YMin >= Ih/2){
			if (res == SW_LIB_SHUTEXT_OK)
				res = SW_LIB_SHUTEXT_WARNING_LIM_ABOVE_HALF;

			if (yMin < Ih/2)
				RoiCoord->YMin = SW_LIB_MIN(yMin+safety_margin, Ih);
			else
				RoiCoord->YMin = safety_margin;
		}

		if (RoiCoord->YMax < Ih/2){
			if (res == SW_LIB_SHUTEXT_OK)
				res = SW_LIB_SHUTEXT_WARNING_LIM_ABOVE_HALF;

			if (yMax >= Ih/2)
				RoiCoord->YMax = SW_LIB_MAX(yMax-safety_margin, 0);
			else
				RoiCoord->YMax = Ih-safety_margin;
		}	
   }

   // Check symmetricity
	if (Ih-RoiCoord->YMax < RoiCoord->YMin - symmetricityLimit)
		RoiCoord->YMax = Ih - RoiCoord->YMin;
	if (RoiCoord->YMin < Ih-RoiCoord->YMax - symmetricityLimit)
		RoiCoord->YMin = Ih - RoiCoord->YMax;

	if (Iw-RoiCoord->XMax < RoiCoord->XMin - symmetricityLimit)
		RoiCoord->XMax = Iw - RoiCoord->XMin;
	if (RoiCoord->XMin < Iw-RoiCoord->XMax - symmetricityLimit)
		RoiCoord->XMin = Iw - RoiCoord->XMax;


   // free allocated memory
   free(tmp_array);
   free(col_sum);
   free(row_sum);
   free(col_sq_sum);
   free(row_sq_sum);
   free(col_gradient_sum);
   free(row_gradient_sum);

   free(GauIma);

   return res;
}
