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
#include <windows.h> 
#include <memory.h>

#include <RdgDev.h>




/*************************************************************************************/
// Get the default parameters values for the given profile
// Profile can be either RDG_CST_MKX_PROFILE, RDG_CST_TIPX_PROFILE or RDG_CST_OSC_PROFILE
/*************************************************************************************/
int RdgGetDefaultParam(CRdgExtractParam *ExtractParam, int DefaultParametersProfile)
{	
   const CRdgExtractParam defaultParam[] = 
   {
      // comes from STB / MKX
      // RDG_CST_MKX_PROFILE
      {
            
         0,                          // int   ExtraMeasuresFlag;
         1,                          // int   SpatialParamsFor512Flag;
         1,                          // int   FastProfileFlag;

         0,                          // int   NormalizationOriFlag;
         8.0f,                       // float NormalizationLFSigma;
         RDG_CST_RDG_DIFFAL_HESSIAN_TRACE, // int   RidgeType;
         { 1.5f },                   // float KernelSigmas;
         0,                          // int   EdgeFromOriFlag;
         1.5f,                       // float NoEdgeFactor;
         RDG_CST_K_CHIABS_BARY_THR,  // int   ThresholdType;
         0,                          // int   ThinningFlag;
         15,                         // int   NoiseThreshold;
         2.0f,                       // float AbsoluteThreshold;
         0,                          // int   L1ThresholdFlag;
         200,                        // int   NbPixelsKept;
         2,                          // int   NbRangePixels;
         0,                          // int   DirLabelFlag;
         20,                         // int   MaxDirDiff;
         1,                          // int   DoubleBistouFlag;
         30,                         // int   NbLabelsKept;
         1,                          // int   GlobalMotionFlag; 
         0.6f,                       // float GlobalMotionThreshold;

         150,                        // int   SubTipLength;


         0.0f,                       // float RidgeGamma;

         0,                          // int   BackgroundEstimationFlag;
         0,                          // int   FramesPerSecond;

         0,                          // int   FreezeThresholdsAt;

         0.0f,                       // float ThresholdPercent1;
         0.0f,                       // float WireTipThresholdPercent1;
         0,                          // int   NbRangePixels1;
         0.0f,                       // float MaxDirDiff1;
         0,                          // int   MinNbPelPerLab1;
         0,                          // int   MinNbPelPerWireTipLab1;
         0.0f,                       // float ThresholdPercent2;
         0.0f,                       // float WireTipThresholdPercent2;
         0,                          // int   NbRangePixels2;
         0,                          // float MaxDirDiff2;
         0                           // int   MaxWireTipThickness;
      },

      // comes from SNB / TIPX
      // RDG_CST_TIPX_PROFILE
      {   
         1,                          // int   ExtraMeasuresFlag;
         0,                          // int   SpatialParamsFor512Flag;
         0,                          // int   FastProfileFlag;

         1,                          // int   NormalizationOriFlag;
         4.0f,                       // float NormalizationLFSigma;
         RDG_CST_RDG_DIFFAL_K1_LAMBDAK2_PLINE, // int   RidgeType;
         { 0.6f },                   // float KernelSigmas;
         0,                          // int   EdgeFromOriFlag;
         0.5f,                       // float NoEdgeFactor;
         RDG_CST_K_CHIABS_BARY_THR,  // int   ThresholdType;
         1,                          // int   ThinningFlag;
         20,                         // int   NoiseThreshold;
         4.0f,                       // float AbsoluteThreshold;
         0,                          // int   L1ThresholdFlag;
         2000,                       // int   NbPixelsKept;
         2,                          // int   NbRangePixels;
         1,                          // int   DirLabelFlag;
         40,                         // int   MaxDirDiff;
         1,                          // int   DoubleBistouFlag;
         10,                         // int   NbLabelsKept;
         1,                          // int   GlobalMotionFlag; 
         0.5f,                       // float GlobalMotionThreshold;

         150,                        // int   SubTipLength;


         0.0f,                       // float RidgeGamma;

         0,                          // int   BackgroundEstimationFlag;
         0,                          // int   FramesPerSecond;

         0,                          // int   FreezeThresholdsAt;

         0.0f,                       // float ThresholdPercent1;
         0.0f,                       // float WireTipThresholdPercent1;
         0,                          // int   NbRangePixels1;
         0.0f,                       // float MaxDirDiff1;
         0,                          // int   MinNbPelPerLab1;
         0,                          // int   MinNbPelPerWireTipLab1;
         0.0f,                       // float ThresholdPercent2;
         0.0f,                       // float WireTipThresholdPercent2;
         0,                          // int   NbRangePixels2;
         0,                          // float MaxDirDiff2;
         0                           // int   MaxWireTipThickness;
      },

      // comes from OSCAR
      // RDG_CST_OSC_PROFILE
      {
         0,                          // int   ExtraMeasuresFlag;
         0,                          // int   SpatialParamsFor512Flag;
         0,                          // int   FastProfileFlag;

         0,                          // int   NormalizationOriFlag;
         4.0f,                       // float NormalizationLFSigma;
         RDG_CST_RDG_DIFFAL_K1_LAMBDAK2_PLINE, // int   RidgeType;
         { 1.0f, 2.0f, 3.0f },       // float KernelSigmas;
         0,                          // int   EdgeFromOriFlag;
         0.5f,                       // float NoEdgeFactor;
         RDG_CST_K_CHIABS_BARY_THR,  // int   ThresholdType;
         0,                          // int   ThinningFlag;
         20,                         // int   NoiseThreshold;
         4.0f,                       // float AbsoluteThreshold;
         0,                          // int   L1ThresholdFlag;
         2000,                       // int   NbPixelsKept;
         2,                          // int   NbRangePixels;
         1,                          // int   DirLabelFlag;
         40,                         // int   MaxDirDiff;
         0,                          // int   DoubleBistouFlag;
         10,                         // int   NbLabelsKept;
         1,                          // int   GlobalMotionFlag; 
         0.5f,                       // float GlobalMotionThreshold;

         150,                        // int   SubTipLength;
         

         1.0f,                       // float RidgeGamma;

         1,                          // int   BackgroundEstimationFlag;
         15,                         // int   FramesPerSecond;

         5,                          // int   FreezeThresholdsAt;

         1.0f,                       // float ThresholdPercent1;
         0.5f,                       // float WireTipThresholdPercent1;
         3,                          // int   NbRangePixels1;
         10.0f,                      // float MaxDirDiff1;
         50,                         // int   MinNbPelPerLab1;
         15,                         // int   MinNbPelPerWireTipLab1;
         1.5f,                       // float ThresholdPercent2;
         1.0f,                       // float WireTipThresholdPercent2;
         2,                          // int   NbRangePixels2;
         180,                        // float MaxDirDiff2;
         4                           // int   MaxWireTipThickness;

      }
   };

   if (DefaultParametersProfile!=RDG_CST_MKX_PROFILE  &&
       DefaultParametersProfile!=RDG_CST_TIPX_PROFILE && 
       DefaultParametersProfile!=RDG_CST_OSC_PROFILE)
       return RDG_ERR_WRONG_PROFILE;

   *ExtractParam = defaultParam[DefaultParametersProfile];

   return 0;
}

/*************************************************************************************/
// Get the current parameters values from handle
/*************************************************************************************/
int RdgGetParam(CRdgHdl RdgHdl, CRdgExtractParam *ExtractParam)
{
   CRdg *This = (CRdg *) RdgHdl;

   *ExtractParam = This->ExtractParam;

   return 0;
}

/*************************************************************************************/
// Set the current parameters values to handle
// After calling this function, RdgExtract() can be invoked only with time t==0
/*************************************************************************************/
int RdgSetParam(CRdgHdl RdgHdl, CRdgExtractParam *ExtractParam)
{
    CRdg *This = (CRdg *) RdgHdl;
    int   Err  = 0;

    // check if parameters changed
    if (memcmp(&This->ExtractProc.CachedParam, ExtractParam, sizeof(*ExtractParam)))
    {
        int ImaWidth = This->ExtractProc.IwOri;
        int SSFactor = This->ExtractProc.SSFactor;

        // set the "new parameters" flag, and then set the parameters
        This->ExtractProc.NewParameters = TRUE;
        This->ExtractProc.CachedParam = *ExtractParam; // save external parameters for future memcmp() comparison
        This->ExtractParam = *ExtractParam;            // copy new parameters

        // adapt SubTipLength
        This->ExtractProc.SubTipLength = This->ExtractParam.TipLength / This->ExtractProc.SSFactor;

        // adapt some spatial parameters to the image scale
        if (ExtractParam->SpatialParamsFor512Flag && ImaWidth/SSFactor!=512)
        {
           int i = 0;
           while (This->ExtractParam.KernelSigmas[i])
           {
              This->ExtractParam.KernelSigmas[i] = (This->ExtractParam.KernelSigmas[i]*ImaWidth)/(512*SSFactor);
              i++;
           }
           This->ExtractParam.NbPixelsKept         = (This->ExtractParam.NbPixelsKept        *ImaWidth)/(512*SSFactor);
           This->ExtractParam.NbRangePixels        = (This->ExtractParam.NbRangePixels       *ImaWidth)/(512*SSFactor);
           if (This->ExtractParam.NbRangePixels<1) This->ExtractParam.NbRangePixels = 1;
           This->ExtractParam.NormalizationLFSigma = (This->ExtractParam.NormalizationLFSigma*ImaWidth)/(512*SSFactor);
           // if image size gets smaller, let's assume it will contain less noise for the ridge enhancer
           // lower the threshold
           This->ExtractParam.AbsoluteThreshold = (This->ExtractParam.AbsoluteThreshold*ImaWidth)/(512*SSFactor);
           if (This->ExtractParam.AbsoluteThreshold<1) This->ExtractParam.AbsoluteThreshold = 1;

           //
           // params added for RdgExtractAngio() and RdgExtractFluoro()
           //
           This->ExtractParam.NbRangePixels1         = (This->ExtractParam.NbRangePixels1        *ImaWidth)/(512*SSFactor);
           if (This->ExtractParam.NbRangePixels2<1) This->ExtractParam.NbRangePixels2 = 1;
           This->ExtractParam.MinNbPelPerLab1        = (This->ExtractParam.MinNbPelPerLab1       *ImaWidth)/(512*SSFactor);
           if (This->ExtractParam.MinNbPelPerLab1<1) This->ExtractParam.MinNbPelPerLab1 = 1;
           This->ExtractParam.MinNbPelPerWireTipLab1 = (This->ExtractParam.MinNbPelPerWireTipLab1*ImaWidth)/(512*SSFactor);
           if (This->ExtractParam.MinNbPelPerWireTipLab1<1) This->ExtractParam.MinNbPelPerWireTipLab1 = 1;
           if (This->ExtractParam.NbRangePixels1<1) This->ExtractParam.NbRangePixels1 = 1;
           This->ExtractParam.NbRangePixels2         = (This->ExtractParam.NbRangePixels2        *ImaWidth)/(512*SSFactor);
           if (This->ExtractParam.NbRangePixels2<1) This->ExtractParam.NbRangePixels2 = 1;
           This->ExtractParam.MaxWireTipThickness    = (This->ExtractParam.MaxWireTipThickness   *ImaWidth)/(512*SSFactor);
           if (This->ExtractParam.MaxWireTipThickness<1) This->ExtractParam.MaxWireTipThickness = 1;
        }

        // the number of labels may have changed, realloc the CentroidTm1 array
        if (This->ExtractProc.CentroidTm1) 
        {
            if (!Err) Err = RdgImFree(This->ExtractProc.CentroidTm1, 0);
            This->ExtractProc.CentroidTm1 = NULL;
        }
        if (This->ExtractParam.NbLabelsKept)
            if (!Err) Err = RdgImAlloc(&This->ExtractProc.CentroidTm1, 0, sizeof(CRdgVf), This->ExtractParam.NbLabelsKept, 1);


        // Estimate the norm of the ridge differential filter
        // this used to be done in RdgExtract.c for (t==0), 
        // but could be done here only once for new parameters
        {
#define RDG_DIRAC_SIZE  512

            int   k;
            short **tmpOri=NULL; //just to make compiler happy
            float **tmpRwo=NULL, **tmpNoE=NULL;
            float **tmpGau=NULL, **tmpRdg=NULL, **tmpDir=NULL, **tmpNoi=NULL, tmpNoiseSig = 0;

            CRdgRoiCoord tmpRoiCoord = { 0, 0, RDG_DIRAC_SIZE-1, RDG_DIRAC_SIZE-1 };

            if (!Err) Err = RdgImAlloc(NULL, &tmpOri, sizeof(tmpOri[0][0]), RDG_DIRAC_SIZE, RDG_DIRAC_SIZE);
            if (!Err) Err = RdgImAlloc(NULL, &tmpRwo, sizeof(tmpRwo[0][0]), RDG_DIRAC_SIZE, RDG_DIRAC_SIZE);
            if (!Err) Err = RdgImAlloc(NULL, &tmpNoE, sizeof(tmpNoE[0][0]), RDG_DIRAC_SIZE, RDG_DIRAC_SIZE);
#if RDG_USE_SSE_GAUSSIAN
            if (!Err) Err = RdgImAlignedAlloc(NULL, &tmpGau, sizeof(tmpGau[0][0]), RDG_DIRAC_SIZE, RDG_DIRAC_SIZE, 16);
#else
            if (!Err) Err = RdgImAlloc(NULL, &tmpGau, sizeof(tmpGau[0][0]), RDG_DIRAC_SIZE, RDG_DIRAC_SIZE);
#endif
            if (!Err) Err = RdgImAlloc(NULL, &tmpRdg, sizeof(tmpRdg[0][0]), RDG_DIRAC_SIZE, RDG_DIRAC_SIZE);
            if (!Err) Err = RdgImAlloc(NULL, &tmpDir, sizeof(tmpDir[0][0]), RDG_DIRAC_SIZE, RDG_DIRAC_SIZE);
            if (!Err) Err = RdgImAlloc(NULL, &tmpNoi, sizeof(tmpNoi[0][0]), RDG_DIRAC_SIZE, RDG_DIRAC_SIZE);

            // create some kind of Dirac signal : a ridge in the middle of the dummy picture tmpOri
            memset(tmpOri[0], 0, RDG_DIRAC_SIZE*RDG_DIRAC_SIZE*sizeof(tmpOri[0][0]));
            for (k=0; k<RDG_DIRAC_SIZE; k++)
                tmpOri[RDG_DIRAC_SIZE/2][k] = -1;

            // filter the dirac signal with the smallest scale
            RdgEnhanceRidge(tmpOri, 
                tmpRwo, tmpNoE, 
                tmpGau, tmpRdg, 
                tmpDir, tmpNoi, RDG_DIRAC_SIZE, RDG_DIRAC_SIZE, This->ExtractProc.DynOri, 
                &tmpRoiCoord, 
                This->ExtractParam.NormalizationOriFlag, This->ExtractParam.NormalizationLFSigma, 
                This->ExtractParam.RidgeType, This->ExtractParam.KernelSigmas[0], 
                This->ExtractParam.EdgeFromOriFlag, This->ExtractParam.NoEdgeFactor, 
                This->ExtractParam.ThresholdType, 
                FALSE, 
                &tmpNoiseSig, NULL);

            // get the filter response on the dirac signal
            This->ExtractProc.RdgFilterNorm = tmpRdg[RDG_DIRAC_SIZE/2][RDG_DIRAC_SIZE/2];

            if (!Err) Err = RdgImFree(NULL, tmpOri);
            if (!Err) Err = RdgImFree(NULL, tmpRwo);
            if (!Err) Err = RdgImFree(NULL, tmpNoE);
#if RDG_USE_SSE_GAUSSIAN
            if (!Err) Err = RdgImAlignedFree(NULL, tmpGau);
#else
            if (!Err) Err = RdgImFree(NULL, tmpGau);
#endif
            if (!Err) Err = RdgImFree(NULL, tmpRdg);
            if (!Err) Err = RdgImFree(NULL, tmpDir);
            if (!Err) Err = RdgImFree(NULL, tmpNoi);
        }
    }

   return Err;
}