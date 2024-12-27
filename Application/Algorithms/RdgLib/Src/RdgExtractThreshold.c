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
#include <assert.h>
#include <RdgDev.h> 


#define RECURSIVE_FORGET_FACTOR  0.8f
#define OFF_PEAK_FACTOR_FOR_BARY 0.7f


#if 0
#undef  RDG_PRINT_FOR_DEBUG
#define RDG_PRINT_FOR_DEBUG 1
#endif


//************************************************************************************
// Recursively update the threshold, given the weight (number of points) of the current histogram, 
// and past threshold and weight
//************************************************************************************
static
void RdgRecursivelyUpdateThreshold(float *Threshold,          // current threshold, in and out variable
int    ThresholdWeight,    // current threshold weight
float *ThresholdTm1,       // threshold at time t-1, in and out variable
int   *ThresholdWeightTm1) // weight at time t-1, in and out variable
{
#if RDG_PRINT_FOR_DEBUG
    printf("Threshold %f, ThresholdTm1 %f, Weight %d, WeightTm1 %d\n", *Threshold, *ThresholdTm1, ThresholdWeight, *ThresholdWeightTm1);
#endif

    if (ThresholdWeight)
    {
        *Threshold = (*Threshold * ThresholdWeight + RECURSIVE_FORGET_FACTOR * *ThresholdTm1 * *ThresholdWeightTm1) /
            (ThresholdWeight + RECURSIVE_FORGET_FACTOR * *ThresholdWeightTm1);
        *ThresholdTm1 = *Threshold;
        *ThresholdWeightTm1 = (int)(ThresholdWeight + RECURSIVE_FORGET_FACTOR * *ThresholdWeightTm1);
    }

#if RDG_PRINT_FOR_DEBUG
    printf("New Update Threshold %f, New weight %d\n", *ThresholdTm1, *ThresholdWeightTm1);
#endif
}


//************************************************************************************
int RdgSelectPix(float **Rdg, unsigned char **Thr, int Iw, int Ih, CRdgRoiCoord *RoiCoord, int NbPixKept, int *NbThrPel, float *RdgThresh)
{
    int   i, j, k, HistoLen = 1000, *Histo, *Cumul, Level, NbPel = 0;
    int   iMin = 0, iMax = Ih - 1, jMin = 0, jMax = Iw - 1;
    float /*Epsi = (float)0.01,*/ MaxRdg = 0, NormFac;


    iMin = RoiCoord->YMin;
    iMax = RoiCoord->YMax;
    jMin = RoiCoord->XMin;
    jMax = RoiCoord->XMax;


    // Get Max
    for (i = iMin; i <= iMax; i++)
    for (j = jMin; j <= jMax; j++)
        MaxRdg = RDG_MAX(MaxRdg, Rdg[i][j]);

    // Compute histo
    Histo = (int *)malloc((HistoLen + 1) * sizeof(int));
    Cumul = (int *)malloc((HistoLen + 1) * sizeof(int));
    for (k = 0; k <= HistoLen; k++) { Histo[k] = 0; Cumul[k] = 0; }
    NormFac = (float)HistoLen / MaxRdg;

    for (i = iMin; i <= iMax; i++)
    for (j = jMin; j <= jMax; j++)
    {
        Level = (int)(NormFac * Rdg[i][j]);
        assert(Level >= 0 && Level<HistoLen + 1);
        Histo[Level]++;
    }

    Cumul[0] = Histo[0];
    for (k = 1; k <= HistoLen; k++) Cumul[k] = Cumul[k - 1] + Histo[k];

    // Get threshold corresponding to number of pixels to be kept
    k = 0;
    while (Cumul[HistoLen] - Cumul[k] > NbPixKept) k++;
    k = RDG_CLIP(k, 0, HistoLen);

    // Build output binary image
    for (i = iMin; i <= iMax; i++)
    for (j = jMin; j <= jMax; j++)
    {
        Level = (int)(NormFac * Rdg[i][j]);
        if (Level > k)
        {
            Thr[i][j] = 1;
            NbPel++;
        }
    }

    *RdgThresh = k / NormFac;
    *NbThrPel = NbPel;

    // Ending
    free(Histo);
    free(Cumul);

    return 0;
}


//************************************************************************************
void RdgThreshold(float **Rdg, unsigned char **Thr, int Iw, int Ih, CRdgRoiCoord *RoiCoord,
    float NoiseSig, int ThrType, int NoiThresh, float AbsThresh, int L1ThreshFlag, float L1Bg, int NbPixKept, int *NbThrPel,
    float *RdgThresh,
    float *TresholdTm1,
    int *ThresholdWeightTM1)
{
    int i, j, iMin = 0, iMax = Ih - 1, jMin = 0, jMax = Iw - 1, NbPel = 0;
    float RealThresh/*, MaxRdg = 0*/;


    iMin = RoiCoord->YMin;
    iMax = RoiCoord->YMax;
    jMin = RoiCoord->XMin;
    jMax = RoiCoord->XMax;

    memset(Thr[0], 0, Iw*Ih*sizeof(Thr[0][0]));

#if RDG_PRINT_FOR_DEBUG
    {
        char *thr_strings[] = { 
            "RDG_CST_NOISE_DEP_THR",
            "RDG_CST_ABSOLUTE_THR",
            "RDG_CST_K_MEAN_THR",  
            "RDG_CST_K_PEAK_THR",
            "RDG_CST_K_CHI_PEAK_THR",
            "RDG_CST_K_CHI_BARY_THR",
            "RDG_CST_K_CHIABS_PEAK_THR",
            "RDG_CST_K_CHIABS_BARY_THR", 
            "RDG_CST_NB_PIX_THR"
        };

        printf("RdgThreshold(): using %s, AbsTrhesh %f\n", thr_strings[ThrType], AbsThresh);
    }
#endif

    if (ThrType != RDG_CST_NB_PIX_THR)
    {
        // ridge mean dependent threshold
        if (ThrType == RDG_CST_K_MEAN_THR)
        {
            float sum = 0;
            int   n_values = 0;

            for (i = iMin; i <= iMax; i++)
            for (j = jMin; j <= jMax; j++)
            {
                if (Rdg[i][j] > 0)
                {
                    sum += Rdg[i][j];
                    n_values++;
                }
            }

            if (n_values)
                RealThresh = (float)AbsThresh*sum / n_values;
            else
                RealThresh = 0;

            RdgRecursivelyUpdateThreshold(&RealThresh, n_values, TresholdTm1, ThresholdWeightTM1);

#if MX_FLAG
            if (0)
            {
                MxOpen() ;
                MxSetVisible(1) ;

                MxPutMatrixFloat(Rdg[0], Iw, Ih, "Rdg");
                MxCommand(" figure(10) ; imagesc(Rdg) ; axis off ; axis image ; colormap gray(256) ; ") ; 
                MxCommand(" figure(11) ; hist(Rdg(Rdg>0),500) ; ") ;
                // getchar() ; 
            }
#endif
        }
        // ridge peak dependent threshold
        else if (ThrType == RDG_CST_K_PEAK_THR)
        {
            float max = 0;
            float inv_max;
            int   n_values, peak_value, peak_index;
            int  *Histo, n_histo_bins, k;

            // find max value in Rdg image
            n_values = 0;
            for (i = iMin; i <= iMax; i++)
            for (j = jMin; j <= jMax; j++)
            {
                if (Rdg[i][j] > 0)
                    n_values++;
                if (Rdg[i][j] > max)
                    max = Rdg[i][j];
            }

            // target around 100 values per bin in histo, for a uniform variable
            n_histo_bins = RDG_MAX(n_values / 100, 100);
            Histo = _alloca(n_histo_bins*sizeof(Histo[0]));
            memset(Histo, 0, n_histo_bins*sizeof(Histo[0]));

            // fill in histogram
            inv_max = (n_histo_bins - 1 /* take some margin with -1 */) / max;
            for (i = iMin; i <= iMax; i++)
            for (j = jMin; j <= jMax; j++)
            {
                if (Rdg[i][j] > 0)
                    Histo[(int)(Rdg[i][j] * inv_max)]++;
            }

            // find peak value
            peak_value = 0;
            peak_index = 0;
            for (k = 1; k<n_histo_bins; k++)
            if (Histo[k]>peak_value)
            {
                peak_value = Histo[k];
                peak_index = k;
            }

            if (peak_index)
            {
                RealThresh = (float)AbsThresh*peak_index / inv_max;
            }
            else
                RealThresh = 0;

            RdgRecursivelyUpdateThreshold(&RealThresh, n_values, TresholdTm1, ThresholdWeightTM1);

#if MX_FLAG
            printf("SV RealThresh %f, peak_value %f\n", RealThresh, peak_index/inv_max);

            if (0)
            {
                MxOpen() ;
                MxSetVisible(1) ;

                MxPutFloat(peak_index/inv_max, "peak");
                MxPutMatrixFloat(Rdg[0], Iw, Ih, "Rdg");
                // MxCommand(" figure(10) ; imagesc(Rdg) ; axis off ; axis image ; colormap gray(256) ; ") ; 
                // MxCommand(" figure(11) ; hist(Rdg(Rdg>0),n_histo_bins) ; axis on ; title(peak) ; ") ;
                MxCommand(" [peak,threshold] = ShowHisto(Rdg) ; ");
                MxGetFloat(&RealThresh, "threshold");
                printf("SV RealThresh read from Matlab %f\n", RealThresh);
                // getchar() ; 
            }
#endif
        }
        // Chi peak dependent threshold
        else if (ThrType == RDG_CST_K_CHI_PEAK_THR)
        {
            float  max;
            float *chi_array, inv_max;
            int    n_values, peak_value, peak_index;
            int   *Histo, n_histo_bins, k;


            // find max value in Rdg image, 
            // and count number of positive values
            n_values = 0;
            for (i = iMin; i <= iMax; i++)
            for (j = jMin; j <= jMax; j++)
            {
                if (Rdg[i][j] > 0)
                {
                    n_values++;
                }
            }

            // target around 100 values per bin in histo, for a uniform variable
            n_histo_bins = RDG_MAX(n_values / 100, 100);
            Histo = _alloca(n_histo_bins*sizeof(Histo[0]));
            memset(Histo, 0, n_histo_bins*sizeof(Histo[0]));

            // allocate memory for array to store positive values
            chi_array = (float *)malloc(n_values*sizeof(chi_array[0]));

            // fill in the array
            k = 0;
            for (i = iMin; i <= iMax; i++)
            for (j = jMin; j <= jMax; j++)
            {
                if (Rdg[i][j] > 0)
                {
                    chi_array[k++] = RDG_SQ(Rdg[i][j]);
                }
            }

            // take half of the array, and add it to the other half, 
            // to simulate the addition of independent random variables
            max = 0;
            for (k = 0; k < n_values / 2; k++)
            {
                chi_array[k] = (float)sqrt(chi_array[k] + chi_array[k + n_values / 2]);
                if (max < chi_array[k])
                    max = chi_array[k];
            }

            // fill in histogram of chi values 
            inv_max = (n_histo_bins - 1 /* take some margin with -1 */) / max;
            for (k = 0; k < n_values / 2; k++)
            {
                Histo[(int)(chi_array[k] * inv_max)]++;
            }

            // find peak value
            peak_value = 0;
            peak_index = 0;
            for (k = 1; k<n_histo_bins; k++)
            if (Histo[k]>peak_value)
            {
                peak_value = Histo[k];
                peak_index = k;
            }

            if (peak_index)
            {
                RealThresh = (float)AbsThresh*peak_index / inv_max;
            }
            else
                RealThresh = 0;

            RdgRecursivelyUpdateThreshold(&RealThresh, n_values, TresholdTm1, ThresholdWeightTM1);

            free(chi_array);

#if MX_FLAG
            printf("SV RealThresh %f, peak_value %f\n", RealThresh, peak_index/inv_max);

            if (0)
            {
                MxOpen() ;
                MxSetVisible(1) ;

                MxPutFloat(peak_index/inv_max, "peak");
                MxPutMatrixFloat(Rdg[0], Iw, Ih, "Rdg");
                MxPutVectorInt(Histo, n_histo_bins, "Histo");
                // MxCommand(" figure(10) ; imagesc(Rdg) ; axis off ; axis image ; colormap gray(256) ; ") ; 
                // MxCommand(" figure(11) ; hist(Rdg(Rdg>0),n_histo_bins) ; axis on ; title(peak) ; ") ;
                // MxCommand(" figure(11) ; hist(Rdg(Rdg>0),n_histo_bins) ; axis on ; title(peak) ; ") ;
                // MxCommand(" figure(12) ; plot(Histo) ; axis on ; title(peak) ; ") ;
                MxCommand(" [peak,threshold] = ShowHisto(Rdg) ; ");
                MxGetFloat(&RealThresh, "threshold");
                printf("SV RealThresh read from Matlab %f\n", RealThresh);
                // getchar() ; 
            }
#endif
        }
        // barycenter around Chi peak dependent threshold
        else if (ThrType == RDG_CST_K_CHI_BARY_THR)
        {
            float  max;
            float *chi_array, inv_max;
            int    peak_value, n_values;
            int   *Histo, n_histo_bins, k;
            float  barycenter;
            int    weight;


            // find max value in Rdg image, 
            // and count number of positive values
            n_values = 0;
            for (i = iMin; i <= iMax; i++)
            for (j = jMin; j <= jMax; j++)
            {
                if (Rdg[i][j] > 0)
                {
                    n_values++;
                }
            }

            // target around 100 values per bin in histo, for a uniform variable
            n_histo_bins = RDG_MAX(n_values / 100, 100);
            Histo = _alloca(n_histo_bins*sizeof(Histo[0]));
            memset(Histo, 0, n_histo_bins*sizeof(Histo[0]));

            // allocate memory for array to store positive values
            chi_array = (float *)malloc(n_values*sizeof(chi_array[0]));

            // fill in the array
            k = 0;
            for (i = iMin; i <= iMax; i++)
            for (j = jMin; j <= jMax; j++)
            {
                if (Rdg[i][j] > 0)
                {
                    chi_array[k++] = RDG_SQ(Rdg[i][j]);
                }
            }

            // take half of the array, and add it to the other half, 
            // to simulate the addition of independent random variables
            max = 0;
            for (k = 0; k < n_values / 2; k++)
            {
                chi_array[k] = (float)sqrt(chi_array[k] + chi_array[k + n_values / 2]);
                if (max < chi_array[k])
                    max = chi_array[k];
            }

            // fill in histogram of chi values 
            inv_max = (n_histo_bins - 1 /* take some margin with -1 */) / max;
            for (k = 0; k < n_values / 2; k++)
            {
                Histo[(int)(chi_array[k] * inv_max)]++;
            }

            // find peak value
            peak_value = 0;
            for (k = 1; k<n_histo_bins; k++)
            if (Histo[k]>peak_value)
            {
                peak_value = Histo[k];
            }

            // get barycenter of values around peak
            barycenter = 0;
            weight = 0;
            for (k = 0; k < n_histo_bins; k++)
            if (Histo[k] < peak_value*OFF_PEAK_FACTOR_FOR_BARY)
            {
                barycenter += Histo[k] * k;
                weight += Histo[k];
            }
            if (weight)
                barycenter /= weight;

            if (barycenter)
            {
                RealThresh = (float)AbsThresh*barycenter / inv_max;
            }
            else
                RealThresh = 0;

            RdgRecursivelyUpdateThreshold(&RealThresh, (int)weight, TresholdTm1, ThresholdWeightTM1);

            free(chi_array);

#if MX_FLAG
            printf("n_values %d\n", n_values);

            printf("SV RealThresh %f, barycenter_value %f, %f of peak\n", RealThresh, barycenter/inv_max, (float) OFF_PEAK_FACTOR_FOR_BARY);

            if (1)
            {
                MxOpen() ;
                MxSetVisible(1) ;

                MxPutFloat(barycenter/inv_max, "peak");
                MxPutMatrixFloat(Rdg[0], Iw, Ih, "Rdg");
                MxPutVectorInt(Histo, n_histo_bins, "Histo");
                MxPutFloat(RealThresh, "RealThresh");
                MxCommand(" figure(10) ; imagesc(Rdg) ; axis off ; axis image ; colormap gray(256) ; ") ; 
                MxCommand(" figure(11) ; Show(Rdg>RealThresh) ; ") ;
                // MxCommand(" figure(11) ; hist(Rdg(Rdg>0),n_histo_bins) ; axis on ; title(peak) ; ") ;
                MxCommand(" figure(12) ; plot(Histo) ; axis on ; title(peak) ; ") ;
                // MxCommand(" [peak,threshold] = ShowHisto(Rdg) ; ");
                // getchar() ; 
            }
#endif
        }
        // Chi-like sum of absolute values peak dependent threshold
        else if (ThrType == RDG_CST_K_CHIABS_PEAK_THR)
        {
            float  max;
            float *chi_array, inv_max;
            int    n_values, peak_value, peak_index;
            int   *Histo, n_histo_bins, k;


            // find max value in Rdg image, 
            // and count number of positive values
            n_values = 0;
            for (i = iMin; i <= iMax; i++)
            for (j = jMin; j <= jMax; j++)
            {
                if (Rdg[i][j] > 0)
                {
                    n_values++;
                }
            }

            // target around 100 values per bin in histo, for a uniform variable
            n_histo_bins = RDG_MAX(n_values / 100, 100);
            Histo = _alloca(n_histo_bins*sizeof(Histo[0]));
            memset(Histo, 0, n_histo_bins*sizeof(Histo[0]));

            // allocate memory for array to store positive values
            chi_array = (float *)malloc(n_values*sizeof(chi_array[0]));

            // fill in the array
            k = 0;
            for (i = iMin; i <= iMax; i++)
            for (j = jMin; j <= jMax; j++)
            {
                if (Rdg[i][j] > 0)
                {
                    chi_array[k++] = RDG_ABS(Rdg[i][j]);
                }
            }

            // take half of the array, and add it to the other half, 
            // to simulate the addition of independent random variables
            max = 0;
            for (k = 0; k < n_values / 2; k++)
            {
                chi_array[k] = chi_array[k] + chi_array[k + n_values / 2];
                if (max < chi_array[k])
                    max = chi_array[k];
            }

            // fill in histogram of chi values 
            inv_max = (n_histo_bins - 1 /* take some margin with -1 */) / max;
            for (k = 0; k < n_values / 2; k++)
            {
                Histo[(int)(chi_array[k] * inv_max)]++;
            }

            // find peak value
            peak_value = 0;
            peak_index = 0;
            for (k = 1; k<n_histo_bins; k++)
            if (Histo[k]>peak_value)
            {
                peak_value = Histo[k];
                peak_index = k;
            }

            if (peak_index)
            {
                RealThresh = (float)AbsThresh*peak_index / (inv_max*1.24f /* 1.24 is the bias of the absolute value, compared to squared values of a real Chi */);
            }
            else
                RealThresh = 0;

            RdgRecursivelyUpdateThreshold(&RealThresh, n_values, TresholdTm1, ThresholdWeightTM1);

            free(chi_array);

#if MX_FLAG
            printf("SV RealThresh %f, peak_value %f\n", RealThresh, peak_index/(inv_max*1.24f));

            if (0)
            {
                MxOpen() ;
                MxSetVisible(1) ;

                MxPutFloat(peak_index/inv_max, "peak");
                MxPutMatrixFloat(Rdg[0], Iw, Ih, "Rdg");
                MxPutVectorInt(Histo, n_histo_bins, "Histo");
                // MxCommand(" figure(10) ; imagesc(Rdg) ; axis off ; axis image ; colormap gray(256) ; ") ; 
                // MxCommand(" figure(11) ; hist(Rdg(Rdg>0),n_histo_bins) ; axis on ; title(peak) ; ") ;
                // MxCommand(" figure(11) ; hist(Rdg(Rdg>0),n_histo_bins) ; axis on ; title(peak) ; ") ;
                // MxCommand(" figure(12) ; plot(Histo) ; axis on ; title(peak) ; ") ;
                MxCommand(" [peak,threshold] = ShowHisto(Rdg) ; ");
                MxGetFloat(&RealThresh, "threshold");
                printf("SV RealThresh read from Matlab %f\n", RealThresh);
                // getchar() ; 
            }
#endif
        }
        // barycenter around Chi-like sum of absolute values peak dependent threshold
        else if (ThrType == RDG_CST_K_CHIABS_BARY_THR)
        {
            float  max;
            float *chi_array, inv_max;
            int    peak_value, n_values;
            int   *Histo, n_histo_bins, k;
            float  barycenter;
            int    weight;


            // find max value in Rdg image, 
            // and count number of positive values
            n_values = 0;
            for (i = iMin; i <= iMax; i++)
            for (j = jMin; j <= jMax; j++)
            {
                if (Rdg[i][j] > 0)
                {
                    n_values++;
                }
            }

            // target around 100 values per bin in histo, for a uniform variable
            n_histo_bins = RDG_MAX(n_values / 100, 100);
            Histo = _alloca(n_histo_bins*sizeof(Histo[0]));
            memset(Histo, 0, n_histo_bins*sizeof(Histo[0]));

            // allocate memory for array to store positive values
            chi_array = (float *)malloc(n_values*sizeof(chi_array[0]));

            // fill in the array
            k = 0;
            for (i = iMin; i <= iMax; i++)
            for (j = jMin; j <= jMax; j++)
            {
                if (Rdg[i][j] > 0)
                {
                    chi_array[k++] = RDG_ABS(Rdg[i][j]);
                }
            }

            // take half of the array, and add it to the other half, 
            // to simulate the addition of independent random variables
            max = 0;
            for (k = 0; k < n_values / 2; k++)
            {
                chi_array[k] = chi_array[k] + chi_array[k + n_values / 2];
                if (max < chi_array[k])
                    max = chi_array[k];
            }

            // fill in histogram of chi values 
            inv_max = (n_histo_bins - 1 /* take some margin with -1 */) / max;
            for (k = 0; k < n_values / 2; k++)
            {
                Histo[(int)(chi_array[k] * inv_max)]++;
            }

            // find peak value
            peak_value = 0;
            for (k = 1; k<n_histo_bins; k++)
            if (Histo[k]>peak_value)
            {
                peak_value = Histo[k];
            }

            // get barycenter of values around peak
            barycenter = 0;
            weight = 0;
            for (k = 0; k < n_histo_bins; k++)
            if (Histo[k] < peak_value*OFF_PEAK_FACTOR_FOR_BARY)
            {
                barycenter += Histo[k] * k;
                weight += Histo[k];
            }
            if (weight)
                barycenter /= weight;

            if (barycenter)
            {
                RealThresh = (float)AbsThresh*barycenter / (inv_max*1.24f /* 1.24 is the bias of the absolute value, compared to squared values of a real Chi */);
            }
            else
                RealThresh = 0;

            RdgRecursivelyUpdateThreshold(&RealThresh, (int)weight, TresholdTm1, ThresholdWeightTM1);

            free(chi_array);

#if MX_FLAG
            printf("n_values %d\n", n_values);

            printf("SV RealThresh %f, barycenter_value %f, %f of peak\n", RealThresh, barycenter/(inv_max*1.24f), (float) OFF_PEAK_FACTOR_FOR_BARY);

            if (1)
            {
                MxOpen() ;
                MxSetVisible(1) ;

                MxPutFloat(barycenter/inv_max, "peak");
                MxPutMatrixFloat(Rdg[0], Iw, Ih, "Rdg");
                MxPutVectorInt(Histo, n_histo_bins, "Histo");
                MxPutFloat(RealThresh, "RealThresh");
                MxCommand(" figure(10) ; imagesc(Rdg) ; axis off ; axis image ; colormap gray(256) ; ") ; 
                MxCommand(" figure(11) ; Show(Rdg>RealThresh) ; ") ;
                // MxCommand(" figure(11) ; hist(Rdg(Rdg>0),n_histo_bins) ; axis on ; title(peak) ; ") ;
                MxCommand(" figure(12) ; plot(Histo) ; axis on ; title(peak) ; ") ;
                // MxCommand(" [peak,threshold] = ShowHisto(Rdg) ; ");
                // getchar() ; 
            }
#endif
        }
        // noise dependent threshold
        else if (ThrType == RDG_CST_NOISE_DEP_THR)
        {
            RealThresh = (float)NoiThresh * NoiseSig;
        }
        // absolute threshold
        else if (L1ThreshFlag)
            // RealThresh = (float)AbsThresh * L1Bg;
            // modified by SV
        {
            RealThresh = (float)(AbsThresh * L1Bg);
        }
        else
            RealThresh = AbsThresh;

#if RDG_PRINT_FOR_DEBUG
        printf("RealThresh=%8.2f NoiThresh=%d NoiseSig=%8.2f AbsThresh=%8.2f L1Bg=%8.2f\n\n", RealThresh, NoiThresh, NoiseSig, AbsThresh, L1Bg);
#endif


        for (i = iMin; i <= iMax; i++)
        for (j = jMin; j <= jMax; j++)
        {
            if (Rdg[i][j] > RealThresh)
            {
                Thr[i][j] = 1;
                NbPel++;
            }
        }

        *RdgThresh = RealThresh;
    }
    // threshold based on a number of pixels kept
    else
        RdgSelectPix(Rdg, Thr, Iw, Ih, RoiCoord, NbPixKept, &NbPel, RdgThresh);

#if RDG_PRINT_FOR_DEBUG
    printf("SV: %d pixels kept with threshold %f\n", NbPel, *RdgThresh);
#endif

    *NbThrPel = NbPel;
}
