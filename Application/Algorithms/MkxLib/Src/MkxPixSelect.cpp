// ***************************************************************************
// * Copyright (c) 2002-2015 Philips Medical Research Paris, Philips France. *
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

/***************************************************************************/
// PixSelect :                                                             */
// Input Image : Enh contains the intensity of the dark spots in the image.*/
// OutPut Image : Thr is a binary image which contains the brightest       */
// pixels of Enh.                                                          */
// NbPixelsKept is the maximum number of pixels kept.                      */
/***************************************************************************/
static int  MkxPixSelectFunc(const float *Enh, int IwEnh, float MaxEnh, UC *Thr, CMkxRoiCoord UsefulRoiCoord, int NbPixelsKept)
{
  int   i, j, k, HistoLen = 1000, *Histo, *Cumul, iMin, iMax, jMin, jMax;
  float Epsi = (float)0.01, NormFac;
  int   cnt = 0;

  iMin = UsefulRoiCoord.YMin;
  iMax = UsefulRoiCoord.YMax;
  jMin = UsefulRoiCoord.XMin;
  jMax = UsefulRoiCoord.XMax;

  if (MaxEnh < Epsi) return 1;  // dark image

  // Compute histo
  Histo = (int *)calloc(2 * (HistoLen + 1), sizeof(*Histo));
  Cumul = Histo + HistoLen + 1;

#if PLATFORM_INTEL_SSE >= 20
  typedef PXVEC_TYPE(unsigned short) VecType;
  typedef PXVEC_TYPE(short) VecTypeS;
  typedef PXVEC_TYPE_FORSIZE(float, VecType::SCALAR_COUNT) VecTypeFloat;

  int indicesStride = PX_ROUNDUP(jMax - jMin, 2 * VecType::SCALAR_COUNT);
  unsigned short * Indices = (unsigned short *)_aligned_malloc((iMax - iMin) * indicesStride * sizeof(*Indices), PLATFORM_DEFAULT_ALIGN);
  using namespace Philips::Medisys::PXVec;

  vec_roundingmode<RoundingMode_Truncate> rounding;
#endif

  NormFac = (float)HistoLen / MaxEnh;
  for (i = iMin; i < iMax; i++)
  {
#if PLATFORM_INTEL_SSE >= 20

    unsigned short * indicesLine = &Indices[-jMin + (i - iMin) * indicesStride];

    for (j = jMin; j <= jMax - VecType::SCALAR_COUNT; j += VecType::SCALAR_COUNT)
    {
      VecType index = convert<VecTypeS>(VecTypeFloat(NormFac) * loadu<VecTypeFloat>(&Enh[i*IwEnh + j]));

      storea(&indicesLine[j], index);

      if EXPECT(convert_to_mask(VecTypeS(index) > VecTypeS::zero()) == 0, false)
      {
        Histo[0] += VecType::SCALAR_COUNT;
      }
      else
      {
        Histo[extract<0>(index)]++;
        Histo[extract<1>(index)]++;
        Histo[extract<2>(index)]++;
        Histo[extract<3>(index)]++;
        Histo[extract<4>(index)]++;
        Histo[extract<5>(index)]++;
        Histo[extract<6>(index)]++;
        Histo[extract<7>(index)]++;
      }
    }

    if (j < jMax)
    {
      int remain = jMax - j;
      VecType index = convert<VecType>(VecTypeFloat(NormFac) * loadu<VecTypeFloat>(&Enh[i*IwEnh + j]));

      store_partial(&indicesLine[j], index, jMax - j);

      if (convert_to_mask(VecTypeS(index) > VecTypeS::zero()) == 0)
      {
        Histo[0] += remain;
      }
      else
      {
        Histo[extract<0>(index)]++;
        if (remain > 1) {
          Histo[extract<1>(index)]++;
          if (remain > 2) {
            Histo[extract<2>(index)]++;
            if (remain > 3) {
              Histo[extract<3>(index)]++;
              if (remain > 4) {
                Histo[extract<4>(index)]++;
                if (remain > 5) {
                  Histo[extract<5>(index)]++;
                  if (remain > 6) {
                    Histo[extract<6>(index)]++;
                  }
                }
              }
            }
          }
        }
      }
    }

#else
    for(j = jMin; j < jMax; j++)
    {
      Level = (int)(NormFac * Enh[i*IwEnh+j]);
      Histo[Level]++;
    }

#endif

  }

  Cumul[0] = Histo[0];
  for (k = 1; k <= HistoLen; k++) Cumul[k] = Cumul[k - 1] + Histo[k];

  // Get threshold corresponding to number of pixels to be kept
  k = 0;
  while (Cumul[HistoLen] - Cumul[k] > NbPixelsKept)  k++;
  k = MKX_CLIP(k, 0, HistoLen);

  typedef PXVEC_TYPE_FORSIZE(short, 16) VecType2;
  typedef PXVEC_TYPE_FORSIZE(unsigned char, 16) VecTypeUC;

#if PLATFORM_INTEL_SSE >= 20
  VecType2 kVec = k;
  VecTypeS maskUC = shift_right_log<8>(VecTypeS::minus_one());
  VecTypeUC onesUC = 1;
#endif

  // Build output binary image
  for (i = iMin; i < iMax; i++)
  {
#if PLATFORM_INTEL_SSE >= 20
    for (j = jMin; j <= jMax - VecType2::SCALAR_COUNT; j += VecType2::SCALAR_COUNT)
    {
      VecType2 index = loada<VecType2>((short const *)&Indices[(i - iMin) * indicesStride + j - jMin]);
      VecType2 mask = index > kVec;

      //cnt += popcount((uint32_t)convert_to_mask(mask));

      VecTypeUC thr = convert<VecTypeUC>(mask & VecType2(maskUC));

      store_mask(&Thr[i*IwEnh + j], onesUC, thr);
    }

    if (j < jMax)
    {
      VecType2 index = loada<VecType2>((short *)&Indices[(i - iMin) * indicesStride + j - jMin]);

      VecType2 mask = (index > kVec) & partial_mask<VecType2>(jMax - j);

      //cnt += popcount((uint32_t)convert_to_mask(mask));

      VecTypeUC thr = convert<VecTypeUC>(mask & VecType2(maskUC));

      store_mask(&Thr[i*IwEnh + j], onesUC, thr);
    }

#else
    for (j = jMin; j < jMax; j++)
    {
      Level = (int)(NormFac * Enh[i*IwEnh+j]);
      //assert(Level == Indices[(i - iMin) * indicesStride + j - jMin]);
      if(Level > k)  // Modif:15-04-02 > instead of >= (NbPixelsKept = Maximum instead of Minimum)
      {
        Thr[i*IwEnh+j] = 1;
        cnt++;
      }
    }
#endif
  }

#if PLATFORM_INTEL_SSE >= 20
  _aligned_free(Indices);
#endif
  // Ending
  free(Histo);

  return 0;

}


int  MkxPixSelect(
  const float* enh,
  UC* thr,
  int IwProc, int IhProc,
  float ImEnhMax1, float ImEnhMax2,
  int TwoRoisFlag,  CMkxRoiCoord* UsefulRoiCoord,
  int AdaptiveWorkNbPixelsKept
  )
{
  int Err = 0;
  memset(thr, 0, IwProc*IhProc*sizeof(UC));

  if (TwoRoisFlag)
  {
    Err |= MkxPixSelectFunc(enh, IwProc, ImEnhMax1, thr, UsefulRoiCoord[1], (int)(AdaptiveWorkNbPixelsKept / 2));
    Err |= MkxPixSelectFunc(enh, IwProc, ImEnhMax2, thr, UsefulRoiCoord[2], (int)(AdaptiveWorkNbPixelsKept / 2));
  }
  else
  {
    Err |= MkxPixSelectFunc(enh, IwProc, ImEnhMax1, thr, UsefulRoiCoord[0], AdaptiveWorkNbPixelsKept);
  }

  return Err;
}

