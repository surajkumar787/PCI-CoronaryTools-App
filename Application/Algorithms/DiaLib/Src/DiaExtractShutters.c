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

#include <DiaLibDev.h>
#include "string.h"

#define DIA_SHUTTERS_SAFETY_MARGIN 4
#define DIA_SHUTTERS_NON_EXTREMA_SCALE 10
#define DIA_SHUTTERS_EDGENESS_SCALE 4
#define DIA_SHUTTER_NON_SHUTTER_MIN_LEVEL_RATIO 1.3f
#define DIA_PICTURE_BORDER_NORMALIZED_VARIANCE .001f
#define DIA_REFERENCE_SCALE_IW 1024
#define DIA_SHUTTER_SYMMETRICITYLIMIT 40

// Provided as a convenience function
// Detection of Shutters/Borders
int DiaExtractShutters(CDiaHdl DiaHdl, short* InIma, int IwOri, int IhOri, int ShutterPos[4])
{
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

   char				Reason[128];

   GauIma = (short*)calloc(IwOri*IhOri, sizeof(short));
	if (GauIma == NULL){
		sprintf_s(Reason, 128, "Could not allocate GauIma in DiaExtractShutters.");
		ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaExtractShutters", DIA_ERR_MEM_ALLOC, Reason);
		return DIA_ERR_MEM_ALLOC;
	}

   safety_margin = 0;//DIA_SHUTTERS_SAFETY_MARGIN * IwOri / DIA_REFERENCE_SCALE_IW; 
   non_extrema_scale = DIA_SHUTTERS_NON_EXTREMA_SCALE * IwOri / DIA_REFERENCE_SCALE_IW;
   edgeness_scale = DIA_SHUTTERS_EDGENESS_SCALE * IwOri / DIA_REFERENCE_SCALE_IW;
   shutter_non_shutter_min_level_ratio = DIA_SHUTTER_NON_SHUTTER_MIN_LEVEL_RATIO;
   picture_border_normalized_variance = DIA_PICTURE_BORDER_NORMALIZED_VARIANCE;
   symmetricityLimit = DIA_SHUTTER_SYMMETRICITYLIMIT *  IwOri / DIA_REFERENCE_SCALE_IW;

   // Look for digital shutter - VA
   val = InIma[0];
   yMin = 0; lineConstant = 1;
   while ((yMin < IhOri-1) && (lineConstant == 1)){
	   offset = yMin * IwOri;
	   for (x=0; x<IwOri; x++)
		   if (InIma[offset+x] != val){
			   lineConstant = 0;
			   break;
		   }
		if (lineConstant)
			yMin++;
   }
   yMax = IhOri-1; lineConstant = 1;
   while ((yMax > yMin) && (lineConstant == 1)){
	   offset = yMax * IwOri;
	   for (x=0; x<IwOri; x++)
		   if (InIma[offset+x] != val){
			   lineConstant = 0;
			   break;
		   }
		if (lineConstant)
			yMax--;
   }
   xMin = 0; columnConstant = 1;
   while ((xMin < IwOri-1) && (columnConstant == 1)){
	   for (y=0; y<IhOri; y++)
		   if (InIma[y*IwOri+xMin] != val){
			   columnConstant = 0;
			   break;
		   }
		if (columnConstant)
			xMin++;
   }
   xMax = IwOri-1; columnConstant = 1;
   while ((xMax > xMin) && (columnConstant == 1)){
	   for (y=0; y<IhOri; y++)
		   if (InIma[y*IwOri+xMax] != val){
			   columnConstant = 0;
			   break;
		   }
		if (columnConstant)
			xMax--;
   }

   // run a gaussian filter over the picture


   // alloc memory
   col_sum = (int *) calloc(IwOri, sizeof(col_sum[0]));
	if (col_sum == NULL){
		sprintf_s(Reason, 128, "Could not allocate col_sum in DiaExtractShutters.");
		ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaExtractShutters", DIA_ERR_MEM_ALLOC, Reason);
        free(GauIma);
		return DIA_ERR_MEM_ALLOC;
	}
   row_sum = (int *) calloc(IhOri, sizeof(row_sum[0]));	
	if (row_sum == NULL){
		sprintf_s(Reason, 128, "Could not allocate row_sum in DiaExtractShutters.");
		ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaExtractShutters", DIA_ERR_MEM_ALLOC, Reason);
        free(GauIma); free(col_sum);
		return DIA_ERR_MEM_ALLOC;
	}
   col_sq_sum = (float *) calloc(IwOri, sizeof(col_sq_sum[0]));
	if (col_sq_sum == NULL){
		sprintf_s(Reason, 128, "Could not allocate col_sq_sum in DiaExtractShutters.");
		ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaExtractShutters", DIA_ERR_MEM_ALLOC, Reason);
        free(GauIma); free(col_sum); free(row_sum);
		return DIA_ERR_MEM_ALLOC;
	}
   row_sq_sum = (float *) calloc(IhOri, sizeof(row_sq_sum[0]));
	if (row_sq_sum == NULL){
		sprintf_s(Reason, 128, "Could not allocate row_sq_sum in DiaExtractShutters.");
		ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaExtractShutters", DIA_ERR_MEM_ALLOC, Reason);
        free(GauIma); free(col_sum); free(row_sum); free(col_sq_sum);
		return DIA_ERR_MEM_ALLOC;
	}
   col_gradient_sum = (int *) calloc(IwOri,sizeof(col_gradient_sum[0]));
	if (col_gradient_sum == NULL){
		sprintf_s(Reason, 128, "Could not allocate col_gradient_sum in DiaExtractShutters.");
		ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaExtractShutters", DIA_ERR_MEM_ALLOC, Reason);
        free(GauIma); free(col_sum); free(row_sum); free(col_sq_sum); free(row_sq_sum);
		return DIA_ERR_MEM_ALLOC;
	}
   row_gradient_sum = (int *) calloc(IhOri,sizeof(row_gradient_sum[0]));
	if (row_gradient_sum == NULL){
		sprintf_s(Reason, 128, "Could not allocate row_gradient_sum in DiaExtractShutters.");
		ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaExtractShutters", DIA_ERR_MEM_ALLOC, Reason);
        free(GauIma); free(col_sum); free(row_sum); free(col_sq_sum); free(row_sq_sum); free(col_gradient_sum);
		return DIA_ERR_MEM_ALLOC;
	}
   tmp_array = (int *) malloc(DIA_MAX(IwOri, IhOri)*sizeof(tmp_array[0]));
	if (tmp_array == NULL){
		sprintf_s(Reason, 128, "Could not allocate tmp_array in DiaExtractShutters.");
		ErrStartErrorReportFromLeaf(DiaHdl.ErrHdl, "DiaExtractShutters", DIA_ERR_MEM_ALLOC, Reason);
        free(GauIma); free(col_sum); free(row_sum); free(col_sq_sum); free(row_sq_sum); free(col_gradient_sum); free(row_gradient_sum);
		return DIA_ERR_MEM_ALLOC;
	}

   DiaBxIsoGaussFilterShort2D(InIma, GauIma, (float) edgeness_scale, IwOri, IhOri);

   // compute sums of pixels on columns and lines
   for (i=yMin; i<=yMax; i++)
   {
      for (j=xMin; j<=xMax; j++)
      {
         col_sum[j] += GauIma[i*IwOri+j];
         row_sum[i] += GauIma[i*IwOri+j];
      }
   }
   // compute sums of gradients on columns and lines
   for (i=yMin; i<=yMax-1; i++)
   {
      row_gradient_sum[i] = row_sum[i+1]-row_sum[i];
   }
   row_gradient_sum[IhOri-1] = 0;
   for (j=xMin; j<=xMax-1; j++)
   {
      col_gradient_sum[j] = col_sum[j+1]-col_sum[j];
   }
   col_gradient_sum[IwOri-1] = 0;
   
   // filter once more the gradients before doing the non-extrema suppression
   memcpy(tmp_array, row_gradient_sum, IhOri*sizeof(row_gradient_sum[0]));
   for (i=yMin+1; i<=yMax-1; i++)
   {
      int k;
      int sum = 0;

      for (k=-1; k<=1; k++)
         sum += tmp_array[i+k];

      row_gradient_sum[i] = sum;
   }
   row_gradient_sum[0] = tmp_array[0]+tmp_array[1];
   memcpy(tmp_array, col_gradient_sum, IwOri*sizeof(row_gradient_sum[0]));
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
   memset(col_sum, 0, IwOri*sizeof(col_sum[0]));
   memset(row_sum, 0, IhOri*sizeof(row_sum[0]));
   for (i=yMin; i<=yMax; i++)
   {
      for (j=xMin; j<=xMax; j++)
      {
         col_sum[j] += InIma[i*IwOri+j];
         row_sum[i] += InIma[i*IwOri+j];

         col_sq_sum[j] += ((float) InIma[i*IwOri+j])*InIma[i*IwOri+j];
         row_sq_sum[i] += ((float) InIma[i*IwOri+j])*InIma[i*IwOri+j];
      }
   }

   // transform col_sq_sum and row_sq_sum in second order moments 
   // (= sum of (pixel-mean(pixel))^2)
   for (i=yMin; i<=yMax; i++)
   {
      row_sq_sum[i] = (row_sq_sum[i]-DIA_SQ((float) row_sum[i])/IwOri);
   }
	for (j=xMin; j<=xMax; j++)
   {
      col_sq_sum[j] = (col_sq_sum[j]-DIA_SQ((float) col_sum[j])/IhOri);
   }

   // non-extrema suppression
   memcpy(tmp_array, row_gradient_sum, IhOri*sizeof(row_gradient_sum[0]));
   for (i=yMin; i<=yMax; i++)
   {
      //if (i==0 || i==IhOri-1 || (tmp_array[i-1]<tmp_array[i] && tmp_array[i]<tmp_array[i+1]) || (tmp_array[i-1]>tmp_array[i] && tmp_array[i]>tmp_array[i+1]))
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
   memcpy(tmp_array, col_gradient_sum, IwOri*sizeof(row_gradient_sum[0]));
   for (j=xMin; j<=xMax; j++)
   {
      //if (j==0 || j==IwOri-1 || (tmp_array[j-1]<tmp_array[j] && tmp_array[j]<tmp_array[j+1]) || (tmp_array[j-1]>tmp_array[j] && tmp_array[j]>tmp_array[j+1]))
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
         if (left_mean*shutter_non_shutter_min_level_ratio<right_mean && right_var>picture_border_normalized_variance*DIA_SQ(right_mean))
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
               left_mean /= n_points*IhOri;

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
            if (left_var<=picture_border_normalized_variance*DIA_SQ(left_mean) && right_var>picture_border_normalized_variance*DIA_SQ(right_mean))
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
      if (j<IwOri-1-2*edgeness_scale && col_gradient_sum[j]<0)
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
         if (left_mean>right_mean*shutter_non_shutter_min_level_ratio && left_var>picture_border_normalized_variance*DIA_SQ(left_mean))
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
         if (j<IwOri-1-2*edgeness_scale && col_gradient_sum[j]<0)
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
               right_mean /= n_points*IhOri;

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
            if (left_var>picture_border_normalized_variance*DIA_SQ(left_mean) && right_var<picture_border_normalized_variance*DIA_SQ(right_mean))
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
      if (i>2*edgeness_scale && row_gradient_sum[i]>0)
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
         if (top_mean*shutter_non_shutter_min_level_ratio<bottom_mean && bottom_var>picture_border_normalized_variance*DIA_SQ(bottom_mean))
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
         if (i>2*edgeness_scale && row_gradient_sum[i]!=0)
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
            if (top_var<=picture_border_normalized_variance*DIA_SQ(top_mean) && bottom_var>picture_border_normalized_variance*DIA_SQ(bottom_mean))
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
      if (row_gradient_sum[i]<0 && i<IhOri-1-2*edgeness_scale)
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
         if (top_mean>bottom_mean*shutter_non_shutter_min_level_ratio && top_var>picture_border_normalized_variance*DIA_SQ(top_mean))
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
         if (row_gradient_sum[i]<0 && i<IhOri-1-2*edgeness_scale)
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
            if (top_var>picture_border_normalized_variance*DIA_SQ(top_mean) && bottom_var<picture_border_normalized_variance*DIA_SQ(bottom_mean))
            {
               bottom_pos = j;
               break;
            }
         }
      }
   }

   // fill in RoiCoord return struct - tests on results values
   ShutterPos[0]   = DIA_MIN(left_pos  +safety_margin, IwOri);
   ShutterPos[1]   = DIA_MAX(right_pos -safety_margin, 0);

   if (ShutterPos[0] >= ShutterPos[1]){

		//Default values
		if (ShutterPos[0] >= IwOri/2){
			if (xMin < IwOri/2)
				ShutterPos[0] = DIA_MIN(xMin  +safety_margin, IwOri);
			else
				ShutterPos[0] = safety_margin;
		}
		if (ShutterPos[1] < IwOri/2){
			if (xMax >= IwOri/2)
				ShutterPos[1] = DIA_MAX(xMax-safety_margin, 0);
			else
				ShutterPos[1] = IwOri-safety_margin;
		}				
   } else {
		//Default values
		if (ShutterPos[0] >= IwOri/2){

			if (xMin < IwOri/2)
				ShutterPos[0] = DIA_MIN(xMin+safety_margin, IwOri);
			else
				ShutterPos[0] = safety_margin;
		}

		if (ShutterPos[1] < IwOri/2){

			if (xMax >= IwOri/2)
				ShutterPos[1] = DIA_MAX(xMax-safety_margin, 0);
			else
				ShutterPos[1] = IwOri-safety_margin;
		}	
   }

   ShutterPos[2]   = DIA_MIN(top_pos   +safety_margin, IhOri);
   ShutterPos[3]   = DIA_MAX(bottom_pos-safety_margin, 0);

   if (ShutterPos[2] >= ShutterPos[3]){

		//Default values
		if (ShutterPos[2] >= IhOri/2){
			if (yMin < IhOri/2)
				ShutterPos[2] = DIA_MIN(yMin+safety_margin, IhOri);
			else
				ShutterPos[2] = safety_margin;
		}
		if (ShutterPos[3] < IhOri/2){
			if (yMax >= IhOri/2)
				ShutterPos[3] = DIA_MAX(yMax-safety_margin, 0);
			else
				ShutterPos[3] = IhOri-safety_margin;
		}				
   } else {
		//Default values
		if (ShutterPos[2] >= IhOri/2){

			if (yMin < IhOri/2)
				ShutterPos[2] = DIA_MIN(yMin+safety_margin, IhOri);
			else
				ShutterPos[2] = safety_margin;
		}

		if (ShutterPos[3] < IhOri/2){

			if (yMax >= IhOri/2)
				ShutterPos[3] = DIA_MAX(yMax-safety_margin, 0);
			else
				ShutterPos[3] = IhOri-safety_margin;
		}	
   }

   // Check symmetricity
	if (ShutterPos[3] < ShutterPos[2] - symmetricityLimit)
		ShutterPos[3] = IhOri - ShutterPos[2];
	if (ShutterPos[2] < IhOri-ShutterPos[3] - symmetricityLimit)
		ShutterPos[2] = IhOri - ShutterPos[3];

	if (IwOri-ShutterPos[1] < ShutterPos[0] - symmetricityLimit)
		ShutterPos[1] = IwOri - ShutterPos[0];
	if (ShutterPos[0] < IwOri-ShutterPos[1] - symmetricityLimit)
		ShutterPos[0] = IwOri - ShutterPos[1];




   // free allocated memory
   free(tmp_array);
   free(col_sum);
   free(row_sum);
   free(col_sq_sum);
   free(row_sq_sum);
   free(col_gradient_sum);
   free(row_gradient_sum);
   free(GauIma);

   return 0;
}
