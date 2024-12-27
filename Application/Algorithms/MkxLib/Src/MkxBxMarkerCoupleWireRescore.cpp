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
#include <stdlib.h>
#include <math.h> 
#include <float.h>
#include <assert.h>

#include <MkxDev.h> 
#include <BxWire.h>
#include <BxCurve.h>

#define MKX_MARKERS_CORRELATION_HALF_LENGTH                 3 // LENGTH IS 1+2*HALF_LENGTH

static int cnt=0;
/*****************************************************************************************/
// BxWireCorrelateWith1DWireModel: correlates profiles along the curve with a 1D model   */
/*****************************************************************************************/
static int BxWireCorrelateWith1DWireModel(BxCurve *curve, float *profiles, int profIw, int profIh, const int profiles_half_width, float * returned_correlation)
{
	// next array is { 0 0 -0.5 -1 -0.5 0 0 }, normalized (zero mean, energy 1)
	const float model[] = { 0.296500f, 0.296500f, -0.222375f, -0.741249f, -0.222375f, 0.296500f, 0.296500f };
	const int half_width = 3;
	const int offset = profiles_half_width-half_width; // offset due to differing length of pre-computed and used profiles

	int   s, k;
	float *correlations;

	// check model array size
	assert(MKX_N_ELEMS(model)==(1+2*half_width));
	assert(half_width<=profiles_half_width);

	correlations =  (float *)calloc(curve->npoints, sizeof(correlations[0]));

	// go through all curve points, and store local profiles
	for (s=0; s<curve->npoints; s++)
	{	
		float sum;
		float sqsum;
		float factor;

		// now correlate the profile with wire model
		correlations[s] = 0;
		sum = 0;
		sqsum = 0;

		for (k=0; k<1+2*half_width; k++)
		{
			float pixel = profiles[s*profIw+k + offset];

			correlations[s] += pixel*model[k];
			sum             += pixel;
			sqsum           += pixel*pixel;
		}

		// rectify correlation, as if profiles had been normalized
		factor = sqsum-(sum*sum)/(1+2*half_width);
		if (factor>0)
		{
			correlations[s] /= (float) sqrt(factor);
		}
		else
			correlations[s] = 0;
	}

	// compute stats on contrast values
	{ 
		float mean = 0, n_values = 0;// , var = 0;

		for (s=0; s<curve->npoints; s++)
		{
			// exclude points around the markers
			if ((s>=curve->slandmark[0]-5 && s<=curve->slandmark[0]+5) /* first  marker */ || 
				(s>=curve->slandmark[1]-5 && s<=curve->slandmark[1]+5) /* second marker */)
				continue;

			mean += correlations[s];
			//var  += correlations[s]*correlations[s];
			n_values++;
		}

		if (n_values)
		{
			mean /= n_values;
			//var = var/n_values - mean*mean;

			// printf("Normalized correlation mean %f, variance %f\n", mean, var);
		}
		else
		{
			mean = 0;
			//var  = 0;
		}

		// return the correlation with wire model of width 3
		*returned_correlation = mean;
	}
    free(correlations);

	return 0 ;
}

/*************************************************************************************************/
// BxWireCorrelateMarkers: computes the normalize correlation of the markers along the curve     */
// Several marker center positions are tried, to make up for approximate marker center detection */ 
/*************************************************************************************************/
static int BxWireCorrelateMarkers(BxCurve *curve, float * const profile, int profIw, int profIh, const int profile_half_width, float *return_correlation)
{
	int   k, l;
	const int   dz = 2; // (int) params->ContrastDeadZone;
	const int   bk = 3; // 2;
	const int   side_margin = 2; // 2;

	float variance[2]; // variance of each marker
	int   offset_c, offset_m; // offset curve, offset margin
	const int offset_width = profile_half_width-(bk+dz+side_margin);
	float best_correlation = -1;

	assert(dz+bk+side_margin<=profile_half_width);

	for (offset_m = -MKX_MIN(2,side_margin); offset_m<=MKX_MIN(2,side_margin); offset_m++)
		for (offset_c = -2; offset_c<=2; offset_c++)
		{
			// do the markers correlation
			{
				float sum = 0;
				float mean  [2] = { 0, 0 };
				float energy[2] = { 0, 0 };
				int   n_values = 0;

				for (k=-MKX_MARKERS_CORRELATION_HALF_LENGTH; k<=MKX_MARKERS_CORRELATION_HALF_LENGTH; k++)
				{
					int s1 = curve->slandmark[0]+k+offset_c;
					int s2 = curve->slandmark[1]+k;

					if (s1>=0 && s1<curve->npoints &&
						s2>=0 && s2<curve->npoints)
					{	
						for (l=0; l<1+2*(bk+dz); l++)
						{
							sum += profile[s1*profIw + l + side_margin + offset_m + offset_width] * profile[s2*profIw+l + side_margin + offset_width];

							mean[0] += profile[s1*profIw+l + side_margin + offset_m + offset_width];
							mean[1] += profile[s2*profIw+l + side_margin + offset_width];
							energy[0] += profile[s1*profIw + l + side_margin + offset_m + offset_width] * profile[s1*profIw+l + side_margin + offset_m + offset_width];
							energy[1] += profile[s2*profIw + l + side_margin + offset_width] * profile[s2*profIw+l + side_margin + offset_width];

							n_values++;
						}
					}
				}

				// sum contains the unnormalized correlation
				// rectify it
				variance[0] = energy[0]-mean[0]*mean[0]/n_values;
				variance[1] = energy[1]-mean[1]*mean[1]/n_values;

				{
					float factor = (energy[0]-mean[0]*mean[0]/n_values)*(energy[1]-mean[1]*mean[1]/n_values);
					if (factor>0)
						sum = (sum-mean[0]*mean[1]/n_values)/(float) (sqrt(factor));
					else
						sum = 0;
				}
				if (variance[0]>0 && variance[1]>0)
					sum *= (float) sqrt(MKX_MIN(variance[0],variance[1])/MKX_MAX(variance[0],variance[1]));
				else
					sum = 0;

				if (best_correlation<sum)
					best_correlation = sum;
			}
		}

		// return correlation value
		*return_correlation = best_correlation; 

		return 0 ;
}

/*****************************************************************************************/
// BxWireCorrelateMarkersWith2DModel: extracts Marker Images and correlates them         */
// with 2D marker models                                                                 */
/*****************************************************************************************/
static int BxWireCorrelateMarkersWith2DModel(BxCurve *curve, float **profile, const int profile_half_width, float *return_correlation)
{
	int l;
	const int half_width = MKX_MARKERS_CORRELATION_HALF_LENGTH;
	const int offset = profile_half_width-half_width;

	float marker[2][(1+2*MKX_MARKERS_CORRELATION_HALF_LENGTH)*(1+2*MKX_MARKERS_CORRELATION_HALF_LENGTH)]; // marker pixels

	assert(MKX_MARKERS_CORRELATION_HALF_LENGTH<=profile_half_width);

	*return_correlation = 1;

	for (l=0; l<2; l++)
	{
		if (curve->slandmark[l]-half_width>=0 && curve->slandmark[l]+half_width<curve->npoints)
		{
			int i, j;

			for (j=-(half_width); j<=(half_width); j++)
			{
				for (i=-(half_width); i<=(half_width); i++)
				{
					marker[l][(j+half_width)*(1+2*half_width)+i+half_width] = profile[curve->slandmark[l]+j][i+half_width+offset];
				}
			}
		}
	}

	// normalized correlation with 2D models of markers
	{
		int   width_extend, height_extend;
		float best_mean_norm_correlation = -1;
		//int   best_model_width_for_mean_correlation, best_model_height_for_mean_correlation;
		float best_min_norm_correlation = -1;
		//int   best_model_width_for_min_correlation, best_model_height_for_min_correlation;
		float best_fitted_model_error = FLT_MAX;
		float best_fitted_correlation=-1;


		// sum of 2D model points
		float sum_model;

		// next arrays will be used to respectively test:
		// 3x3 steps on 5x5 supports ( 9 step pixels among 25 support pixels)
		// 3x5 steps on 5x7 supports (15 step pixels among 35 support pixels)
		// 5x5 steps on 7x7 supports (25 step pixels among 49 support pixels)
		int step_width_extend_tab[]  = { 1, 1, 2 };
		int step_height_extend_tab[] = { 1, 2, 2 };
		int support_width_extend_tab[] = { 2, 2, 3 };
		int support_height_extend_tab[] = { 2, 3, 3 };

		int n_points; // number of points of support area

		int index;

		float   sum[2]; // sum of marker pixels
		float sqsum[2]; // sum of squared marker pixels


		for (index=0; index<3; index++)
		{
			int   support_width_extend, support_height_extend;
			float mean_norm_correlation = 0;
			float min_norm_correlation = 1;
			float fitted_model_error = 0;
			float model_depth[2];

			width_extend  = step_width_extend_tab [index];
			height_extend = step_height_extend_tab[index];
			support_width_extend  = support_width_extend_tab[index];
			support_height_extend = support_height_extend_tab[index];

			n_points = (1+2*support_width_extend)*(1+2*support_height_extend);

			sum_model = (float) (n_points-2*(1+2*width_extend)*(1+2*height_extend)); // sum of 2D model points

			// compute sum of pixels and sum of squared pixels over support
			for (l=0; l<2; l++)
			{
				float partial_sum = 0;
				float partial_sqsum = 0;
				int   i, j;

				for (j=-support_height_extend; j<=support_height_extend; j++)
				{
					for (i=-support_width_extend; i<=support_width_extend; i++)
					{
						partial_sum += marker[l][(1+2*half_width)*(j+half_width)+(i+half_width)];
						partial_sqsum += MKX_SQ(marker[l][(1+2*half_width)*(j+half_width)+(i+half_width)]);
					}
				}

				sum[l] = partial_sum;
				sqsum[l] = partial_sqsum;
			}

			for (l=0; l<2; l++)
			{
				float normalized_correlation;
				float partial_sum = 0;
				float partial_sqsum = 0;
				int   i, j;

				for (j=-height_extend; j<=height_extend; j++)
				{
					for (i=-width_extend; i<=width_extend; i++)
					{
						partial_sum += marker[l][(1+2*half_width)*(j+half_width)+(i+half_width)];
						partial_sqsum += MKX_SQ(marker[l][(1+2*half_width)*(j+half_width)+(i+half_width)]);
					}
				}

				// correlation with 2D marker rectangular model, having for example the profile
				// +1 +1 +1 -1 -1 -1 +1 +1 +1
				// the correlation of the marker pixels with the +1 points is sum[l]-partial_sum
				// the correlation of the marker pixels with the -1 points is       -partial_sum
				// the correlation of the marker pixels with the 2D model  is thus sum[l]-2*partial_sum
				normalized_correlation = sum[l]-2*partial_sum; 

				// normalize the correlation of marker/model signals
				normalized_correlation -= sum[l]*sum_model/n_points;

				{
					float factor = (sqsum[l]-sum[l]*sum[l]/n_points) * (n_points-sum_model*sum_model/n_points);
					if (factor>0)
						normalized_correlation /= (float) sqrt( factor );
					else 
						normalized_correlation = 0;
				}

				// depth is the difference between the mean signal at 1 and -1 locations
				model_depth[l] = (((sum[l]-partial_sum)/(n_points-(1+2*width_extend)*(1+2*height_extend))) - (partial_sum/((1+2*width_extend)*(1+2*height_extend))));

				// fitted model squared error
				fitted_model_error += sqsum[l]-partial_sqsum-MKX_SQ(sum[l]-partial_sum)/(n_points-(1+2*width_extend)*(1+2*height_extend)) // corresponds to +1 points
					+ partial_sqsum-MKX_SQ(partial_sum)/((1+2*width_extend)*(1+2*height_extend));

				fitted_model_error /= n_points;

				mean_norm_correlation += normalized_correlation;
				if (min_norm_correlation>normalized_correlation)
					min_norm_correlation = normalized_correlation;
			}
			mean_norm_correlation /= 2;

			if (best_mean_norm_correlation < mean_norm_correlation)
			{
				best_mean_norm_correlation = mean_norm_correlation;
				//best_model_width_for_mean_correlation = 1+2*width_extend;
				//best_model_height_for_mean_correlation = 1+2*height_extend; 
			}
			if (best_min_norm_correlation < min_norm_correlation)
			{
				best_min_norm_correlation = min_norm_correlation;
				//best_model_width_for_min_correlation = 1+2*width_extend;
				//best_model_height_for_min_correlation = 1+2*height_extend; 
			}
			if (best_fitted_model_error > fitted_model_error)
			{
				best_fitted_model_error = fitted_model_error;
				best_fitted_correlation = mean_norm_correlation;
				// weight the best_fitted_correlation by the difference in depths of fitted models
				if (model_depth[0]>0 || model_depth[1]>0)
					best_fitted_correlation *= MKX_MIN(model_depth[0],model_depth[1])/MKX_MAX(model_depth[0],model_depth[1]);
				else
					best_fitted_correlation = 0;
			}
		}

		*return_correlation = best_fitted_correlation;
	}

	return 0 ;
}

static void GetProfiles(short *Ori, int IwOri, int IhOri, BxCurve * curve, float* profiles1D, int profIw, int profIh, int half_width)
{
	// go through all curve points, and store local profiles
	for (int s=0; s<curve->npoints; s++)
	{	
		int   l;
		float nx, ny;

		nx    = -curve->dy[s] ;
		ny    =  curve->dx[s] ;

		if (curve->ds[s] > 0)	
		{
			nx = nx / curve->ds[s] ; 
			ny = ny / curve->ds[s] ; 
		}

		for (l=-half_width; l<=half_width; l++)
		{
			int x = (int) ( curve->x[s] + l * nx + 0.5f ) ;
			int y = (int)(curve->y[s] + l * ny + 0.5f);

			// if ((x>=0) && (y>=0) && (x<IwOri) && (y<IhOri))
			if ((x>=1) && (y>=1) && (x<IwOri-1) && (y<IhOri-1))
			{
				profiles1D[s*profIw+l + half_width] = Ori[x + y*IwOri];
			}
			else
			{
				profiles1D[s*profIw+l + half_width] = 0;
			}
		}
	}
}

/***************************************************************************/
// MkxMarkerCoupleWireRescore :                                            */
// Rescores the couple intensities using measures                          */
// along the best fitting snake for each candidate couple                  */
/***************************************************************************/
int MkxMarkerCoupleWireRescore(CMkxBlobCouple *RawCouples, int NbRawCouples, 
															 short *Ima, const int Iw, const int Ih, 
															 short *Ori, const int IwOri, const int IhOri,
															 void *ExtraMeasuresWireHdl, 
															 int MaxNbCouples, float PruningFactor 
															 )
{
	int     k;
	BxCurve curve;
	float   max_intensity_after_modulation = 0;

	struct { 
		float wire_correlation; 
		float marker1_model_correlation;
		float marker2_model_correlation;
		float markers_correlation;
	} best_couple_correlations = { 0.0f, 0.0f, 0.0f, 0.0f }, current_couple_correlations = { 0.0f, 0.0f, 0.0f, 0.0f };

	char funcName[] = "MkxMarkerCoupleWireRescore";

	// no need to create/initialize the BxWire instance
	// this is now done by MkxCreate()

	// skip rescoring if second best candidate couple is far from first one
	// and not tracing to file
	//or if NbRawCouples==1 //PL_080717
	if (/*NbRawCouples==1 ||*/(NbRawCouples>1 && RawCouples[1].CplMerit.CplMeritOverall < PruningFactor))
	{
		for(k = 0; k < NbRawCouples; k++)
			RawCouples[k].CplMerit.CplMeritWrtWire= 1.0f;

		return 0;
	}

	for(k = 0; k < NbRawCouples; k++)
	{
		//float cplIntensityWrtWire=RawCouples[k].CplMerit.CplMeritOverall;
		float cplMeritInput=RawCouples[k].CplMerit.CplMeritOverall;
		float cplIntensityWrtWire=0;

//			printf("k= %3d\n", k);

		if (k<MaxNbCouples // put an upper bound on the number of checked couples */
			// if couple intensity is below the maximum intensity after modulation, 
			// no need to do the modulation, because it will not improve the max intensity
			&& max_intensity_after_modulation * PruningFactor < cplMeritInput
			)
		{
			// curve fitting is always performed on subsampled image
			if(BxWireDoExtract (ExtraMeasuresWireHdl, &curve, Ima, Iw, Ih, 
				(float) RawCouples[k].Blob1.V.X, (float) RawCouples[k].Blob1.V.Y, 
				(float) RawCouples[k].Blob2.V.X, (float) RawCouples[k].Blob2.V.Y))return 1;

			// if measures are made on original image, upsample the curve
			if (IwOri!=Iw)
				BxCurveUpsampleCoordinates(&curve, (int) ((float) IwOri)/Iw); 

			{
				const int half_width = 7;
				float *profiles1D;

				profiles1D = (float*)malloc((1 + 2 * half_width)*curve.npoints*sizeof(float));
				if (profiles1D != NULL)
				{
					GetProfiles(Ori, IwOri, IhOri, &curve, profiles1D, 1 + 2 * half_width, curve.npoints, half_width);

					// get the normalized correlation between markers
					BxWireCorrelateMarkers(&curve, profiles1D, 1 + 2 * half_width, curve.npoints, half_width, &current_couple_correlations.markers_correlation);

					// get correlation with 1D wire model along curve
					BxWireCorrelateWith1DWireModel(&curve, profiles1D, 1 + 2 * half_width, curve.npoints, half_width, &current_couple_correlations.wire_correlation);

					free(profiles1D);
				}
			}

			//combine factors
			cplIntensityWrtWire = MKX_MAX(0,current_couple_correlations.markers_correlation)*MKX_MAX(0,current_couple_correlations.wire_correlation);

			// keep track of maximum intensity
			if (max_intensity_after_modulation<cplMeritInput*cplIntensityWrtWire)
					{ 
				max_intensity_after_modulation = cplMeritInput*cplIntensityWrtWire;
				best_couple_correlations =  current_couple_correlations ;
//				printf("cplMeritInput: %8.3f cplIntensityWrtWire: %8.3f max_intensity_after_modulation: %8.3f\n", cplMeritInput, cplIntensityWrtWire, max_intensity_after_modulation);

					}
			RawCouples[k].CplMerit.CplMeritWrtWireAvailable= 1;
			RawCouples[k].CplMerit.CplMeritWrtWire= cplIntensityWrtWire;
			RawCouples[k].CplMerit.CplMeritWrtWire_markers_correlation= current_couple_correlations.markers_correlation;
			RawCouples[k].CplMerit.CplMeritWrtWire_wire_correlation= current_couple_correlations.wire_correlation;
			//int n = curve.npoints;
			//for(int i=0;i<n;i++)
			//		{ 
			//	RawCouples[k].WireX[i]=curve.x[i];
			//	RawCouples[k].WireY[i]=curve.y[i];
			//}
			//RawCouples[k].WireNbPoints = n;
		}
		else
		{
//			printf("penalize current couple: %3d\n", k);
			// penalize current couple
			//if (max_intensity_after_modulation > 0) cplIntensityWrtWire = max_intensity_after_modulation / 3;
			cplIntensityWrtWire = 0;
			RawCouples[k].CplMerit.CplMeritWrtWire= cplIntensityWrtWire;
		}
	}
	return 0;
}


/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
