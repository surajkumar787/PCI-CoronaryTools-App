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
#include <stdlib.h> 
#include <math.h> 
#include <assert.h>

#include <MkxDev.h>
//#include <MkxGaussian.h> 

#include <algorithm>
#include <limits>

void MkxBlobGetMinMax(short * minValuePtr, short * maxValuePtr, short const * image, int stride, CMkxRoiCoord roi)
{
  short minValue = *minValuePtr, maxValue = *maxValuePtr;

  for (int y = roi.YMin; y <= roi.YMax; y++)
  {
    for (int x = roi.XMin; x <= roi.XMax; x++)
    {
      short val = image[x + y * stride];
      if (val < minValue)
      {
        minValue = val;
      }
      if (val > maxValue)
      {
        maxValue = val;
      }
    }
  }

  *minValuePtr = minValue;
  *maxValuePtr = maxValue;
}


static void MkxRing(int i0, int j0, int w0, CMkxRoiCoord roi, int nDir, short** tabGau1,
  const CMkxVi* TabRing, short* gau1, float* enh, float * enhMin, float * enhMax, int IwEnh)
{

  int i, j, l, k, l1;
  short bkgMin;
  int iMin = roi.YMin - i0;
  int iMax = roi.YMax - i0;
  int jMin = roi.XMin - j0;
  int jMax = roi.XMax - j0;

#if PLATFORM_INTEL_SSE >= 20
  if (w0 > 0 && nDir <= 36)
  {
    using namespace Philips::Medisys::PXVec;

    typedef PXVEC_TYPE(short) VecType;
    typedef PXVEC_TYPE_FORSIZE(float, VecType::SCALAR_COUNT) VecTypeFloat;

    VecTypeFloat enhMinVec = std::numeric_limits<float>::max();
    VecTypeFloat enhMaxVec = -std::numeric_limits<float>::max();

    //short const * * ringLines = (short const * *)_alloca(sizeof(*ringLines) * nDir);
    //#define TEST_RINGCACHE
#ifdef TEST_RINGCACHE
    int halfKerSize = 3;
    short * ringLinesBuffer = (short *)_alloca((2 * halfKerSize + 1) * w0 * sizeof(*ringLinesBuffer));
    short const * * ringBufferLines = (short const * *)_alloca((2 * halfKerSize + 1) * sizeof(*ringBufferLines));
    short * ringLinesBufferEnd = ringLinesBuffer + (2 * halfKerSize + 1) * w0;

    short * currentDestLine = ringLinesBuffer + jMin - halfKerSize;
    short const * currentSourceLine = gau1 + (iMin - halfKerSize) * w0 + jMin - halfKerSize;

    int fullLineSize = jMax - jMin + 1 + halfKerSize;

    for (int dy = -halfKerSize; dy < halfKerSize; dy ++)
    {
      ringBufferLines[dy + halfKerSize] = currentDestLine;
      PX_CopyArray(currentDestLine, currentSourceLine, fullLineSize);

      currentDestLine += w0;
      currentSourceLine += w0;
    }
#endif

    short const * ringLines[36];
    float * outputLine;
    short const * centralLine;

    for (int y = iMin; y <= iMax; y++)
    {
#ifdef TEST_RINGCACHE
      //PX_CopyArray(currentDestLine, currentSourceLine, fullLineSize);
      {
        int x = 0;
        for (; x <= fullLineSize - VecType::SCALAR_COUNT; x += VecType::SCALAR_COUNT)
        {
          storeu(&currentDestLine[x], loadu<VecType>(&currentSourceLine[x]));
        }
        if (x < fullLineSize)
        {
          store<false>(&currentDestLine[x], loadu<VecType>(&currentSourceLine[x]), fullLineSize - x);
        }
      }

      currentDestLine += w0;
      currentSourceLine += w0;

      if (currentDestLine >= ringLinesBufferEnd)
      {
        currentDestLine -= (2 * halfKerSize + 1) * w0;
      }

      ringBufferLines[2 * halfKerSize] = currentDestLine;

      for (int k = 0; k < nDir; k++)
      {
        ringLines[k] = ringBufferLines[halfKerSize + TabRing[k].Y] + halfKerSize + TabRing[k].X;
      }

      for (int dy = -halfKerSize; dy < halfKerSize; dy++)
      {
        ringBufferLines[dy + halfKerSize] = ringBufferLines[dy + halfKerSize + 1];
      }
#else
      for (int k = 0; k < nDir; k++)
      {
        ringLines[k] = tabGau1[k] + y * w0;
      }
#endif
      //outputLine = enh[y + i0] + j0;
      outputLine = enh + (y + i0)*IwEnh + j0;
      centralLine = gau1 + y * w0;

      struct ProcessVector
      {
        static FORCEINLINE void Go(float * enhLine, short const * centralLine, short const * * ringLines, int nDir, int x, int count,
        VecTypeFloat & enhMinVec, VecTypeFloat & enhMaxVec)
        {
          VecType bkgMin = loadu<VecType>(ringLines[0] + x);
          VecType input = loadu<VecType>(centralLine + x);

          int k = 0;
          for (; k <= nDir - 8; k += 8)
          {
            if (k == 0)
              bkgMin = loadu<VecType>(ringLines[0] + x);
            else
              bkgMin = min(bkgMin, loadu<VecType>(ringLines[k] + x));

            //bkgMin = min(bkgMin, loadu<VecType>(ringLines[k    ]));
            bkgMin = min(bkgMin, loadu<VecType>(ringLines[k + 1] + x));
            bkgMin = min(bkgMin, loadu<VecType>(ringLines[k + 2] + x));
            bkgMin = min(bkgMin, loadu<VecType>(ringLines[k + 3] + x));
            bkgMin = min(bkgMin, loadu<VecType>(ringLines[k + 4] + x));
            bkgMin = min(bkgMin, loadu<VecType>(ringLines[k + 5] + x));
            bkgMin = min(bkgMin, loadu<VecType>(ringLines[k + 6] + x));
            bkgMin = min(bkgMin, loadu<VecType>(ringLines[k + 7] + x));
          }

          for (; k < nDir; k++)
          {
            bkgMin = min(bkgMin, loadu<VecType>(ringLines[k] + x));
          }

          VecTypeFloat enh = convert<VecTypeFloat>(max(VecType::zero(), bkgMin - input));

          if (count < VecType::SCALAR_COUNT)
          {
            VecTypeFloat mask = partial_mask<VecTypeFloat>(count);
            enhMinVec = select(mask & (enh < enhMinVec), enh, enhMinVec);
            enhMaxVec = select(mask & (enh > enhMaxVec), enh, enhMaxVec);
            store_partial(enhLine + x, enh, count);
          }
          else
          {
            enhMinVec = min(enhMinVec, enh);
            enhMaxVec = max(enhMaxVec, enh);
            storeu(enhLine + x, enh);
          }
        }
      };

      int x, jMax_8 = jMax - VecType::SCALAR_COUNT + 1;
      for (x = jMin; x <= jMax_8; x += VecType::SCALAR_COUNT)
      {
        // Process pixels 8 by 8
        ProcessVector::Go(outputLine, centralLine, ringLines, nDir, x, VecType::SCALAR_COUNT, enhMinVec, enhMaxVec);
      }

      if (x <= jMax)
      {
        // Process remaining pixels (< 8)
        ProcessVector::Go(outputLine, centralLine, ringLines, nDir, x, jMax - x + 1, enhMinVec, enhMaxVec);
      }

#if 0 // Validation of SSE code
      float * enhRefLine = (float *)calloc(65536, 1);

      l1=y*w0;
      for(j=jMin; j<=jMax;j++)
      {
        l=l1+j;
        bkgMin = tabGau1[0][l];
        for(int k = 1; k < nDir; k++)
        {
          if (tabGau1[k][l] < bkgMin) bkgMin = tabGau1[k][l];
        }
        enhRefLine[j+j0] = bkgMin - gau1[l] > 0 ? (bkgMin - gau1[l]) : 0;
      }

      for (j=jMin; j<=jMax;j++)
      {
        if (enhRefLine[j0 + j] != outputLine[j])
        {
          printf(" NOT GOOD NO GOOD !!! ");
          break;
        }
      }
#endif
    }

    *enhMin = min_inside_scalar(enhMinVec);
    *enhMax = max_inside_scalar(enhMaxVec);
  }
  else
#endif
  {
    w0 = abs(w0);

    float enhMinValue = enh[(iMin + i0)*IwEnh + jMin + j0];
    float enhMaxValue = enh[(iMin + i0)*IwEnh + jMin + j0];

    //_freea(ringLines);
    for (i = iMin; i <= iMax; i++)
    {
      l1 = i*w0;
      for (j = jMin; j <= jMax; j++)
      {
        l = l1 + j;
        bkgMin = tabGau1[0][l];
        for (k = 1; k < nDir; k++)
        {
          if (tabGau1[k][l] < bkgMin) bkgMin = tabGau1[k][l];
        }
        int q = (i + i0)*IwEnh + j + j0;
        enh[q] = bkgMin - gau1[l] > 0 ? (float)(bkgMin - gau1[l]) : 0;
        if (enh[q] > enhMaxValue)
          enhMaxValue = enh[q];
        if (enh[q] < enhMinValue)
          enhMinValue = enh[q];
      }
    }

    *enhMin = enhMinValue;
    *enhMax = enhMaxValue;
  }

}

int MkxDarkPixelsPenalize(int i0, int j0, int w0, CMkxRoiCoord roi, float* imBlobMin, float* enh, int IwEnh, int t, int dkpxTime, float dkpxFac)
{
  //update imBlobMin
  int iMin = roi.YMin - i0;
  int iMax = roi.YMax - i0;
  int jMin = roi.XMin - j0;
  int jMax = roi.XMax - j0;


  if (t == 0)return 0;

  //const float alpha=0.05f;
  for (int i = iMin; i <= iMax; i++)
  {
    for (int j = jMin; j <= jMax; j++)
    {
      int k = j + j0 + (i + i0)*IwEnh;
      imBlobMin[k] = dkpxFac*enh[k] + (1 - dkpxFac)*imBlobMin[k];
    }
  }
  // modulate enh
  if (t >= dkpxTime)
  {
    for (int i = iMin; i <= iMax; i++)
    {
      for (int j = jMin; j <= jMax; j++)
      {
        int k = j + j0 + (i + i0)*IwEnh;
        enh[k] = MKX_MAX(enh[k] - imBlobMin[k], 0);
      }
    }
  }
  return 0;
}



#include <BasicFiltersProcessing.hpp>

void MkxBlobUpdateEnhanceIntensityFactor(float * output, int outputStride, short const * input, int inputStride, short inputMinValue, short inputMaxValue, CMkxRoiCoord roi, float intensityFactor)
{
  float rcp_dyn2 = 1.0f / (inputMaxValue - inputMinValue);

  for (int y = roi.YMin; y <= roi.YMax; y++)
  {
    for (int x = roi.XMin; x <= roi.XMax; x++)
    {
      short inValue = input[x + y * inputStride] - inputMinValue;
      float factor = std::max((float)1 - intensityFactor * square((int)inValue) * rcp_dyn2, (float)0);
      output[x + y * outputStride] *= factor;
    }
  }
}

void   MkxBlobEnhanceRing(
  const short* Ori,
  int IwProc, int IhProc,
  float *Enh,
  float* imBlobMin,
  float * EnhMin1, float * EnhMax1, float * EnhMin2, float * EnhMax2,
  CMkxRoiCoord WorkRoi, CMkxRoiCoord UsefulRoiCoord[], int UseTwoRoisFlag,
  int t,
  int NbDir,
  const CMkxVi* TabRing,
  float BFGaussSig,
  float IntensityFactor,
  int dkpxFlag,
  float dkpxFac,
  int dkpxTime
  )
{
  memset(Enh, 0, IwProc*IhProc*sizeof(float));

  short   MinCen, MaxCen;
  short** tabGau1 = (short **)malloc(NbDir*sizeof(short*));

  // Gauss filter on the full size image	
  // memory alignment necessary for SSE Gaussian
  //int oriStride = Ori[1] - Ori[0]; // Small hack, we could change the signature and pass simple pointer + stride
  int oriStride = IwProc;
  int wrkStride = WorkRoi.Width; // PX_ROUNDUP(w0, PLATFORM_DEFAULT_ALIGN/sizeof(*Gau1));

  short   *Gau1 = (short *)_aligned_malloc(sizeof(*Gau1) * wrkStride * WorkRoi.Height, PLATFORM_DEFAULT_ALIGN);
  short * gauAtOrigin = Gau1 - wrkStride * WorkRoi.YMin - WorkRoi.XMin; // "Virtual" adress of the ROI buffer at the point (0,0), valid only inside the WorkingROI (from WorkROI.XMin, WorkROI.YMin)

  if (BFGaussSig > (float)0.01)
  {
    using namespace Philips::Medisys::AlgoFragments::BasicFilters;
    using namespace Philips::Medisys::AlgoFragments;

    FilterSepProcessingContext<Image2D<short> > context;
    //context.Input.BufferAtOrigin = *Ori;
    context.Input.BufferAtOrigin = (short*)Ori;
    context.Input.LineStride = oriStride;
    context.Output.BufferAtOrigin = gauAtOrigin;
    context.Output.LineStride = wrkStride;
    context.Kernel = FilterSepKernel::GaussianKernel((double)BFGaussSig);

    // Process with border effect at the border of the ROI
    FilterSepProcessing<short>::ProcessROI(Region2D_Make<Region2D>(WorkRoi.XMin, WorkRoi.YMin, WorkRoi.XMax + 1, WorkRoi.YMax + 1),
      Region2D_Make<Region2D>(WorkRoi.XMin, WorkRoi.YMin, WorkRoi.XMax + 1, WorkRoi.YMax + 1),
      context);
  }
  else
  {
    for (int y = WorkRoi.YMin; y < WorkRoi.YMax + 1; y++)
    {
      short * destLine = gauAtOrigin + WorkRoi.XMin + y * wrkStride;
      short const * sourceLine = Ori + WorkRoi.XMin + y * oriStride;

      memcpy(destLine, sourceLine, wrkStride * sizeof(*Gau1));
    }
  }

  // Circular excursion	
  for (int k = 0; k < NbDir; k++)
  {
    tabGau1[k] = &Gau1[0] + TabRing[k].Y * wrkStride + TabRing[k].X;
  }

  //std::sort(tabGau1, tabGau1 + NbDir);

  // Computing contrast
  if (UseTwoRoisFlag)
  {
    MkxRing(WorkRoi.YMin, WorkRoi.XMin, wrkStride, UsefulRoiCoord[1], NbDir, tabGau1, TabRing, Gau1, Enh, EnhMin1, EnhMax1, IwProc);
    if (dkpxFlag)MkxDarkPixelsPenalize(WorkRoi.YMin, WorkRoi.XMin, wrkStride, UsefulRoiCoord[1], imBlobMin, Enh, IwProc, t, dkpxTime, dkpxFac);//to be corrected for 2 rois

    MkxRing(WorkRoi.YMin, WorkRoi.XMin, wrkStride, UsefulRoiCoord[2], NbDir, tabGau1, TabRing, Gau1, Enh, EnhMin2, EnhMax2, IwProc);
    if (dkpxFlag)MkxDarkPixelsPenalize(WorkRoi.YMin, WorkRoi.XMin, wrkStride, UsefulRoiCoord[2], imBlobMin, Enh, IwProc, t, dkpxTime, dkpxFac);
  }
  else
  {
    MkxRing(WorkRoi.YMin, WorkRoi.XMin, wrkStride, UsefulRoiCoord[0], NbDir, tabGau1, TabRing, Gau1, Enh, EnhMin1, EnhMax1, IwProc);
    if (dkpxFlag)MkxDarkPixelsPenalize(WorkRoi.YMin, WorkRoi.XMin, wrkStride, UsefulRoiCoord[0], imBlobMin, Enh, IwProc, t, dkpxTime, dkpxFac);
  }


  // Apply weighting factor for favoring low intensity: 
  if (IntensityFactor > 0.01)
  {
    // get min and max
    MinCen = MaxCen = Gau1[0];
    if (UseTwoRoisFlag)
    {
      MkxBlobGetMinMax(&MinCen, &MaxCen, gauAtOrigin, wrkStride, UsefulRoiCoord[1]);
      MkxBlobGetMinMax(&MinCen, &MaxCen, gauAtOrigin, wrkStride, UsefulRoiCoord[2]);
    }
    else
    {
      MkxBlobGetMinMax(&MinCen, &MaxCen, gauAtOrigin, wrkStride, UsefulRoiCoord[0]);
    }
    //weighting by intensity of center
    //int enhStride = Enh[1] - Enh[0];
    int enhStride = IwProc;
    if (UseTwoRoisFlag)
    {
      MkxBlobUpdateEnhanceIntensityFactor(Enh, enhStride, gauAtOrigin, wrkStride, MinCen, MaxCen, UsefulRoiCoord[1], IntensityFactor);
      MkxBlobUpdateEnhanceIntensityFactor(Enh, enhStride, gauAtOrigin, wrkStride, MinCen, MaxCen, UsefulRoiCoord[2], IntensityFactor);
    }
    else
    {
      MkxBlobUpdateEnhanceIntensityFactor(Enh, enhStride, gauAtOrigin, wrkStride, MinCen, MaxCen, UsefulRoiCoord[0], IntensityFactor);
    }
  }

  if (t == 0 && dkpxFlag)    memcpy(imBlobMin, Enh, IwProc*IhProc*sizeof(float));//to be moved according to correction for darkpixelpenalize

  // Gau1 and Gau2 were memory aligned
  _aligned_free(Gau1);
  free(tabGau1);
}


/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/


