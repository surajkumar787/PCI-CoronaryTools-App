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

#include <OscDev.h>
#include "float.h"
#include <tmmintrin.h>

// **************************************************************************
// Efficient coding of the multires global alignment via scalar product
// **************************************************************************

static __forceinline void OscRiginRegistrationComputeCorrelationVectorShort2(__m128i * scoreLine, short const (* srcLine)[2], short const (* dstLine)[2])
{
    __m128i src = _mm_lddqu_si128((__m128i const *)srcLine);
    __m128i dst = _mm_lddqu_si128((__m128i const *)dstLine);

    *scoreLine = _mm_add_epi32(*scoreLine, _mm_srai_epi32(_mm_abs_epi32(_mm_madd_epi16(src, dst)), 10));
}

static __forceinline float OscRiginRegistrationComputeCorrelationLineShort2(short const (* srcLine)[2], short const (* dstLine)[2], int width)
{
    __m128i scoreLine = _mm_setzero_si128();
    short const (* srcLineMileStone2)[2] = srcLine + width;
    short const (* srcLineMileStone1)[2] = srcLineMileStone2 - 4;
  
    for (; srcLine <= srcLineMileStone1; srcLine += 4, dstLine += 4)
    {
        OscRiginRegistrationComputeCorrelationVectorShort2(&scoreLine, srcLine, dstLine);
    }

    if (srcLine < srcLineMileStone2)
    {
        int remain = (int)(srcLineMileStone2 - srcLine);
        _MM_ALIGN16 short endBuffer[2*4][2] = { 0 };

#define UNROLL_COPY(X,Y,I) if (remain > I) endBuffer[X+I][0] = Y[I][0], endBuffer[X+I][1] = Y[I][1];
#define UNROLL_COPY_ALL(X,Y) \
    UNROLL_COPY(X,Y,0) UNROLL_COPY(X,Y,1) UNROLL_COPY(X,Y,2) UNROLL_COPY(X,Y,3)

        UNROLL_COPY_ALL(0, srcLine)
        UNROLL_COPY_ALL(4, dstLine)

#undef UNROLL_COPY_ALL
#undef UNROLL_COPY

        OscRiginRegistrationComputeCorrelationVectorShort2(&scoreLine, (short const (*) [2])endBuffer, (short const (*) [2])(endBuffer+4));
    }

    {
        float result;

        scoreLine = _mm_hadd_epi32(scoreLine, scoreLine);
        scoreLine = _mm_add_epi32(scoreLine, _mm_srli_si128(scoreLine, 4));

        _mm_store_ss(&result, _mm_cvtepi32_ps(scoreLine));

        return result;
    }
}

// **************************************************************************

float OscMultiResGlobalTranslationEstimationScalarProduct(short (** PyrDir)[2],	short (** PyrDirRef)[2], COscRoiCoord *RoiCoordRef
														, int *PyrW, int *PyrH, int *PyrWRef, int *PyrHRef
                                                        , int dXIn, int dYIn, int maxDeltaX, int maxDeltaY, int maxRes, int *dXOut, int *dYOut){

    int dXIni, dYIni;
	float bestCrit = -1.f;

	// * Loop over the resolutions
	for (int res = maxRes; res >=0; res--){
        int maxDeltaXRes, maxDeltaYRes, dX, dY, xMin, xMax, yMin, yMax
        , xMinRef, xMaxRef, yMinRef, yMaxRef, IwRes, IwResRef, bestDx = 0, bestDy = 0, OffsetRefX, OffsetRefY;
	    
		bestCrit = -1.f;

		// Initializing motion value and motion amplitude
		if (res == maxRes){
			dXIni			= dXIn >> maxRes;
			dYIni			= dYIn >> maxRes;
			maxDeltaXRes	= maxDeltaX;
			maxDeltaYRes	= maxDeltaY;
		} else {
			maxDeltaXRes	= 1;	// Only to affine
			maxDeltaYRes	= 1;
		}

		// ROI at sub resolution
		xMinRef	    = RoiCoordRef->XMin >> res; xMaxRef = RoiCoordRef->XMax >> res;
		yMinRef	    = RoiCoordRef->YMin >> res; yMaxRef = RoiCoordRef->YMax >> res;
		IwRes	    = PyrW[res]; IwResRef	= PyrWRef[res];

        OffsetRefX = (PyrW[res]-PyrWRef[res])/2;
        OffsetRefY = (PyrH[res]-PyrHRef[res])/2;

        xMin = OffsetRefX; xMax = PyrW[res]-1-OffsetRefX;
        yMin = OffsetRefY; yMax = PyrH[res]-1-OffsetRefY;

		// Find best shift
		for (dY = dYIni-maxDeltaYRes; dY <= dYIni+maxDeltaYRes; dY ++)
			for (dX = dXIni-maxDeltaXRes; dX <= dXIni+maxDeltaXRes; dX ++){
				int yy;

				float crit = 0;
				
				int xMin1 = OSC_MAX(xMinRef, xMin-dX);		// ROI attached to the reference image
				int xMax1 = OSC_MIN(xMaxRef, xMax-dX);
				int yMin1 = OSC_MAX(yMinRef, yMin-dY);
				int yMax1 = OSC_MIN(yMaxRef, yMax-dY);
				
				int xMin2 = OSC_MAX(xMinRef+dX, xMin)-OffsetRefX;		// ROI attached to the other image
				int yMin2 = OSC_MAX(yMinRef+dY, yMin)-OffsetRefY;

				int width	= xMax1-xMin1;
				int height	= yMax1-yMin1;

                short (*PyrPtr)[2]		= &PyrDir	[res][yMin1*IwRes+xMin1];
                short (*PyrRefPtr)[2]	= &PyrDirRef[res][yMin2*IwResRef+xMin2];

				for (yy = 0; yy < height; yy++){
					crit += OscRiginRegistrationComputeCorrelationLineShort2((const short (*)[2])PyrRefPtr, (const short (*)[2])PyrPtr, width);
					PyrRefPtr += IwResRef; PyrPtr += IwRes;
				}
				
				if (crit > bestCrit){
					bestCrit = crit; bestDx = dX; bestDy = dY;
				}
			}

			if (res > 0){
				dXIni = 2*bestDx; dYIni = 2*bestDy;
			} else {
				*dXOut = bestDx; *dYOut = bestDy;
			}
	}
	return bestCrit;
}




float OscMultiResGlobalTranslationEstimationProduct(short ** Pyr, short ** PyrRef, int Iw, int Ih, COscRoiCoord *RoiCoord
													, int dXIn, int dYIn, int maxDeltaX, int maxDeltaY, int maxRes, int *dXOut, int *dYOut){

    int dXIni, dYIni;
	float bestCrit = -1.f;

	// * Loop over the resolutions
	for (int res = maxRes; res >=0; res--){
        int maxDeltaXRes, maxDeltaYRes, dX, dY, xMin, xMax, yMin, yMax, IwRes, bestDx = 0, bestDy = 0;
		bestCrit = -1.f;

		// Initializing motion value and motion amplitude
		if (res == maxRes){
			dXIni			= dXIn >> maxRes;
			dYIni			= dYIn >> maxRes;
			maxDeltaXRes	= maxDeltaX;
			maxDeltaYRes	= maxDeltaY;
		} else {
			maxDeltaXRes	= 1;	// Only to affine
			maxDeltaYRes	= 1;
		}

		// ROI at sub resolution
		xMin	= RoiCoord->XMin >> res; xMax = RoiCoord->XMax >> res;
		yMin	= RoiCoord->YMin >> res; yMax = RoiCoord->YMax >> res;
		IwRes	= Iw >> res;

		// Find best shift
		for (dY = dYIni-maxDeltaYRes; dY <= dYIni+maxDeltaYRes; dY ++)
			for (dX = dXIni-maxDeltaXRes; dX <= dXIni+maxDeltaXRes; dX ++){
				int xx, yy;

				float crit = 0;
				
				int xMin1 = OSC_MAX(xMin, xMin-dX);		// ROI attached to the reference image
				int xMax1 = OSC_MIN(xMax, xMax-dX);
				int yMin1 = OSC_MAX(yMin, yMin-dY);
				int yMax1 = OSC_MIN(yMax, yMax-dY);
				
				int xMin2 = OSC_MAX(xMin+dX, xMin);		// ROI attached to the other image
				//int xMax2 = OSC_MIN(xMax+dX, xMax);
				int yMin2 = OSC_MAX(yMin+dY, yMin);
				//int yMax2 = OSC_MIN(yMax+dY, yMax);

				int width	= xMax1-xMin1+1;
				int height	= yMax1-yMin1+1;

				for (yy = 0; yy < height; yy++)
					for (xx = 0; xx < width; xx++)
						crit += Pyr[res][(yMin1+yy)*IwRes+xMin1+xx]*PyrRef[res][(yMin2+yy)*IwRes+xMin2+xx];
				

				if (crit > bestCrit){
					bestCrit = crit; bestDx = dX; bestDy = dY;
				}
			}

//#ifdef DISPLAY_INT
//			{
//				MxOpen(); MxSetVisible(1);
//				MxPutMatrixShort(Pyr[res], IwRes, Ih>>res, "ImRes");
//				MxPutMatrixShort(PyrRef[res], IwRes, Ih>>res, "ImRefRes");
//				MxCommand("figure(2); subplot('Position',[0,0.5,0.5,0.5]); imagesc(uint8(ImRes)); axis image; axis off; colormap gray;");
//				MxCommand("figure(2); subplot('Position',[0.5,0.5,0.5,0.5]); imagesc(uint8(ImRefRes)); axis image; axis off; colormap gray;");
//				MxCommand("figure(2); subplot('Position',[0,0,0.5,0.5]); imagesc(ImRes-ImRefRes); axis image; axis off; colormap gray;");
//
//				MxPutInt(bestDx, "dX"); MxPutInt(bestDy, "dY");
//				MxCommand("[Iw, Ih] = size(ImRes); ImDiffRes = ImRes(max(1,1-dX):min(Iw,Iw-dX),max(1,1-dY):min(Ih,Ih-dY)) - ImRefRes(max(1,1+dX):min(Iw,Iw+dX),max(1,1+dY):min(Ih,Ih+dY));");
//				MxCommand("figure(2); subplot('Position',[0.5,0,0.5,0.5]); imagesc(ImDiffRes); axis image; axis off; colormap gray;");
//			}
//#endif

			if (res > 0){
				dXIni = 2*bestDx; dYIni = 2*bestDy;
			} else {
				*dXOut = bestDx; *dYOut = bestDy;
			}
	}
	return bestCrit;
}

void OscBlockMatchingCompensation(float * ImRef, float * ImEst, int Iw, int Ih, COscRoiCoord *ROICoord
								, int nbBlocsX, int nbBlocsY, int maxDelta, int TRXGlobal, int TRYGlobal, float * WImEst){

	int BlocX, BlocY, gridSizeX, gridSizeY, deltaXBest = 0, deltaYBest = 0;
	int deltaX, deltaY;
	int xMinA, xMaxA, yMinA, yMaxA, xMinB, xMaxB, yMinB, yMaxB, width, height, delta;
	int dx, dy;
	float temp, fQuality, fBest;

	OSC_MEMCPY(WImEst,ImEst,Iw*Ih);

	// Initialization
	gridSizeX = (ROICoord->XMax-ROICoord->XMin+1) / nbBlocsX; 
	gridSizeY = (ROICoord->YMax-ROICoord->YMin+1) / nbBlocsY;

	// * BM2D on each block
	for (BlocY = 0; BlocY < nbBlocsY; BlocY ++)
		for (BlocX = 0; BlocX < nbBlocsX; BlocX ++){

			// Limits of the block on the reference image
			xMinA = ROICoord->XMin+BlocX * gridSizeX;
			xMaxA = xMinA + gridSizeX;
			yMinA = ROICoord->XMin+BlocY * gridSizeY;
			yMaxA = yMinA + gridSizeY;

			fBest = FLT_MAX;

			for (deltaY = TRYGlobal-maxDelta; deltaY <= TRYGlobal+maxDelta; deltaY++)
				for (deltaX = TRXGlobal-maxDelta; deltaX <= TRXGlobal+maxDelta; deltaX++){

					// Limits of the block on the second image
					xMinB = xMinA - deltaX;
					xMaxB = xMaxA - deltaX;
					yMinB = yMinA - deltaY;
					yMaxB = yMaxA - deltaY;

					width = gridSizeX; height = gridSizeY;

					if (OSC_MIN(xMinA, xMinB) < ROICoord->XMin){
						delta = ROICoord->XMin - OSC_MIN(xMinA, xMinB);
						xMinA += delta; xMinB += delta;
						width -= delta;
					}
					if (OSC_MIN(yMinA, yMinB) < ROICoord->YMin){
						delta = ROICoord->YMin - OSC_MIN(yMinA, yMinB);
						yMinA += delta; yMinB += delta;
						height -= delta;
					}
					if (OSC_MAX(xMaxA, xMaxB) > ROICoord->XMax){
						delta = OSC_MAX(xMaxA, xMaxB) - ROICoord->XMax;
						width -= delta;
					}
					if (OSC_MAX(yMaxA, yMaxB) > ROICoord->YMax){
						delta = OSC_MAX(yMaxA, yMaxB) - ROICoord->YMax;
						height -= delta;
					}

					fQuality = 0;
					for (dy = 0; dy < height; dy++)
						for (dx = 0; dx < width; dx++){
							temp = ImRef[ (yMinA + dy) * Iw + (xMinA + dx)]
									- ImEst[ (yMinB + dy) * Iw + (xMinB + dx)];
							fQuality += temp * temp;
						}
					fQuality /= (height*width);

					// Conclusion
					if ((deltaX == TRXGlobal-maxDelta && deltaY == TRYGlobal-maxDelta) 
						||(fBest > fQuality)){
						fBest = fQuality;
						deltaXBest = deltaX;
						deltaYBest = deltaY;
					}
				}

			// Warp over that block
			xMinB = xMinA - deltaXBest;
			xMaxB = xMaxA - deltaXBest;
			yMinB = yMinA - deltaYBest;
			yMaxB = yMaxA - deltaYBest;

			width = gridSizeX; height = gridSizeY;

			if (OSC_MIN(xMinA, xMinB) < ROICoord->XMin){
				delta = ROICoord->XMin - OSC_MIN(xMinA, xMinB);
				xMinA += delta; xMinB += delta;
				width -= delta;
			}
			if (OSC_MIN(yMinA, yMinB) < ROICoord->YMin){
				delta = ROICoord->YMin - OSC_MIN(yMinA, yMinB);
				yMinA += delta; yMinB += delta;
				height -= delta;
			}
			if (OSC_MAX(xMaxA, xMaxB) > ROICoord->XMax){
				delta = OSC_MAX(xMaxA, xMaxB) - ROICoord->XMax;
				width -= delta;
			}
			if (OSC_MAX(yMaxA, yMaxB) > ROICoord->YMax){
				delta = OSC_MAX(yMaxA, yMaxB) - ROICoord->YMax;
				height -= delta;
			}
			for (dy = 0; dy < height; dy++)
				for (dx = 0; dx < width; dx++)
					WImEst[ (yMinA + dy) * Iw + (xMinA + dx)]
							= ImEst[ (yMinB + dy) * Iw + (xMinB + dx)];

		}
}