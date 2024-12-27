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
#include <string.h> 
#include <math.h> 

#include <MkxDev.h> 

//************************************************************************************
// Sub-sample the original image of a factor "SSFactor", through minimum
//************************************************************************************
void MkxSubSampleImage(short *Ori, int IwOri, short *Sub, CMkxVs *Pel, int IwSub, int IhSub, int SSFactor, CMkxRoiCoord roiCoord, int NoSSE)
{
	int i, j, i0, j0, k, l, k0, l0;

	if (SSFactor <= 1)
	{
      memcpy(Sub, Ori, IwSub*IhSub*sizeof(Ori[0]));
	}
#if PLATFORM_INTEL_SSE >= 20
    else if (SSFactor == 2 && NoSSE == 0)
    {
        // SSE2 (or SSE4.1) optimization that leads to > x10 speed
        int w0 = roiCoord.XMax - roiCoord.XMin + 1;

        if (w0 > 0)
        {
            for(i = roiCoord.YMin; i <= roiCoord.YMax; i++)
            {
                using namespace Philips::Medisys::PXVec;

                typedef PXVEC_TYPE_FORSIZE(short,  8) VecType;
                typedef PXVEC_TYPE_FORSIZE(short, 16) VecType2;
                typedef PXVEC_TYPE_FORELEMSIZE(2 * sizeof(VecType::SCALAR_TYPE), VecType::SCALAR_COUNT) VecType2x32;

                // Private functors
                struct ProcessVertical
                {
                    static FORCEINLINE void Go(VecType2 & out, VecType2 & maskOut, short const * inputLine0, short const * inputLine1, int x)
                    {
                        VecType2 current0 = loadu<VecType2>(inputLine0 + 2 * x);
                        VecType2 current1 = loadu<VecType2>(inputLine1 + 2 * x);

                        maskOut = current0 > current1;
                        out = select(maskOut, current1, current0);
                    }
                };

                // Private functors
                struct ProcessHorizontal
                {
                    static FORCEINLINE void Go(short * subLine, CMkxVs * pelLine, short const * inputLine0, short const * inputLine1, VecType & xPlusOne, VecType const & yPlusOne, int x, int count)
                    {
                        VecType2 vertMin, vertMask;
                        ProcessVertical::Go(vertMin, vertMask, inputLine0, inputLine1, x);

                        // Separation of vertMin(/Mask) (16 shorts) in evenVert(Mask) (8 shorts) and oddVert(Mask) (8 shorts)
                        VecType evenVert = pack(VecType2x32(vertMin));
                        VecType  oddVert = pack(VecType2x32(shiftreg_right<1>(vertMin)));

                        VecType evenVertMask = pack(VecType2x32(vertMask));
                        VecType  oddVertMask = pack(VecType2x32(shiftreg_right<1>(vertMask)));

                        VecType horzMask = evenVert > oddVert;
                        VecType horzMin = select(horzMask, oddVert, evenVert);

                        VecType vertHorzMask  = select(horzMask, oddVertMask, evenVertMask);

                        VecType2 pel = unpack(xPlusOne - horzMask, yPlusOne - vertHorzMask);

                        store<false>((short *)(pelLine + x), pel    , 2 * count);
                        store<false>(subLine + x           , horzMin,     count);

                        xPlusOne += VecType2::SCALAR_COUNT;
                    }
                };

                VecType yPlusOne = 2 * i;
                VecType xPlusOne = (VecType(roiCoord.XMin) + VecType::zero_to_n()) << 1;

                short const * inputLine0 = Ori + (2 * i)*IwOri + 2 * roiCoord.XMin;
                short const * inputLine1 = Ori + (2 * i + 1)*IwOri + 2 * roiCoord.XMin;
                short       * subLine = Sub + i*IwSub + roiCoord.XMin;
                CMkxVs      * pelLine = Pel + i*IwSub + roiCoord.XMin;

                int x, w0_8 = w0 - VecType::SCALAR_COUNT;
                for (x = 0; x <= w0_8; x += VecType::SCALAR_COUNT)
                {
                    ProcessHorizontal::Go(subLine, pelLine, inputLine0, inputLine1, xPlusOne, yPlusOne, x, VecType::SCALAR_COUNT);
                }

                if (x < w0)
                {
                    ProcessHorizontal::Go(subLine, pelLine, inputLine0, inputLine1, xPlusOne, yPlusOne, x, w0 - x);
                }
            }
        }
    }
#endif // PLATFORM_INTEL_SSE >= 20 */
    else
	{
		//modif PL_071109
		for(i=roiCoord.YMin; i<=roiCoord.YMax; i++)
		{
			for(j=roiCoord.XMin; j<=roiCoord.XMax; j++)
			{
				i0 = SSFactor * i;
				j0 = SSFactor * j;
				k0 = 0;
				l0 = 0;
				for(k=0; k<SSFactor; k++)
				{
					for(l=0; l<SSFactor; l++)
					{
						if (Ori[(i0+k)*IwOri+j0+l] < Ori[(i0+k0)*IwOri+j0+l0])
						{
							k0 = k;
							l0 = l;
						}
					}
				}
        Sub[i*IwSub+j] = Ori[(i0+k0)*IwOri+j0+l0];
				Pel[i*IwSub+j].X = (short)(j0+l0);
				Pel[i*IwSub+j].Y = (short)(i0+k0);
			}
		}
	}
}


//************************************************************************************
// Up-sample the coordinates, given the pixel mapping CRdgVi
//************************************************************************************
void MkxUpSampleCoordinates(const CMkxVs *ImPel, int Iw, int *X, int *Y)
{
   // copy *X and *Y, since there values are going to change
   const int x = *X;
   const int y = *Y;

   *X = ImPel[x+y*Iw].X;
   *Y = ImPel[x+y*Iw].Y;
}
