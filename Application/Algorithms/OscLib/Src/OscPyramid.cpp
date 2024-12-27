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


//*****************************************************************************
// OscPyramid.c
//
// Adapted from file UsrMultiResolution.c, in FITX AppliLib
//
//*****************************************************************************


#define DOWN(dim) (((dim) % 2) == 0 ? ((dim) >> 1) : ((dim) >> 1) + 1)
#define ODD(dim)  ((dim) % 2)


//*****************************************************************************
// Get pyramid dimensions (image size at each level)

int OscGetPyramidDim(COsc *This, int NbBand, int Iw, int Ih, int **PtPyW, int **PtPyH)
{
	int *PyW, *PyH;
	int Level;

    OSC_MALLOC(PyW, NbBand+1); OSC_MALLOC(PyH, NbBand+1);
	*PtPyW = PyW; *PtPyH = PyH;
	if(!PyW || !PyH)
	{
        This->ExtractProc.FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscGetPyramidDim", OSC_ERR_MEM_ALLOC, "Memory allocation Pb");
		return OSC_ERR_MEM_ALLOC;
	}

	PyW[0] = Iw; PyH[0] = Ih;
	for(Level = 1; Level <= NbBand; Level++){
		PyW[Level] = DOWN(PyW[Level-1]);
		PyH[Level] = DOWN(PyH[Level-1]);
	}	

	return 0;
}

//*****************************************************************************
// Free pyramid dimensions 
int OscFreePyramidDim(COsc *This, int *PtPyW, int *PtPyH)
{
	if (PtPyW) free(PtPyW);
	if (PtPyH) free(PtPyH);

	return 0;
}

//*****************************************************************************
// Allocate pyramid's memory (from dimension arrays)
//    Pyr1[Level][i*W+j], Pyr2[Level][i][j]
//*****************************************************************************
int OscAllocatePyramid(COsc *This, short ***PtPyr, int *PyW, int *PyH, int NbBand)
{
	short **Pyr;
	int Level;

	OSC_MALLOC(Pyr, NbBand+1);
	if(!Pyr)
	{
        This->ExtractProc.FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAllocatePyramid", OSC_ERR_MEM_ALLOC, "Memory allocation Pb");
		return OSC_ERR_MEM_ALLOC;
	}
	*PtPyr = Pyr;

    for(Level = 0; Level <= NbBand; Level++)
        Pyr[Level] = NULL;

	for(Level = 0; Level <= NbBand; Level++){
		OSC_CALLOC(Pyr[Level], PyW[Level]*PyH[Level]);

		if(Pyr[Level] == NULL){
            This->ExtractProc.FatalErrorOnPreviousImage = 1;
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAllocatePyramid", OSC_ERR_MEM_ALLOC, "Memory allocation Pb");
			return OSC_ERR_MEM_ALLOC;
		}
	}

	return 0;
}

//*****************************************************************************
// Free pyramid's allocation
int OscFreePyramid(COsc *This, short **Pyr, int NbBand)
{
	int Level;

	for(Level = 0; Level <= NbBand; Level++){
        if (Pyr[Level] != NULL){free(Pyr[Level]);}
    }
    if (Pyr!=NULL){ free(Pyr);}

	return 0;
}

//*****************************************************************************
// Image down sampling by 2 using a binomial filter
// Vertical (Src -> Buf) then horizontal down-sampling (Buf -> Dst)
// Down-sampling (binomial filter 1,2,1)
//*****************************************************************************
int OscMrDownSampling(short *pSource, short *pDest, int *PyW, int *PyH, int Level)
{
	int nCol              = PyW[Level];
	int nLine             = PyH[Level];
	int nLineLength		    = nCol;
	int nLineLengthHalf		= PyW[Level+1];
	int nLineLength2		  = nLineLength + nLineLength;
	int i = 0, j = 0;

	const short *pSourceLineCurr;
	const short *pSourceLineAbove;
	const short *pSourceLineBelow;
	short *pDestLine;

	int nOddLine = ODD(nLine);
	int nOddCol  = ODD(nCol);

	short *pBuf;

	OSC_CALLOC(pBuf, PyW[Level]*PyH[Level+1]);

	// Vertial down-sampling : 1st line : combine with below : weights (3,1)
	pSourceLineCurr	= &pSource[0];
	pSourceLineBelow	= &pSource[nLineLength];
	pDestLine			= &pBuf[0];
	for (j = 0; j < nCol; j++)
	{
		pDestLine[j] = (pSourceLineCurr[j] + pSourceLineCurr[j] + pSourceLineCurr[j] + pSourceLineBelow[j]) >> 2;
	}

	// Vertical down-sampling : current lines : weights (1,2,1)
	for (i = 1; i < (nLine >> 1); i++)
	{
		pSourceLineCurr		= &pSource[i*nLineLength2];
		pSourceLineAbove	= &pSource[i*nLineLength2 - nLineLength];
		pSourceLineBelow	= &pSource[i*nLineLength2 + nLineLength];
		pDestLine			= &pBuf[i*nLineLength];

		for (j = 0; j < nCol; j++)
		{
			pDestLine[j] = (pSourceLineAbove[j] + pSourceLineCurr[j] 
			+ pSourceLineCurr[j] + pSourceLineBelow[j]) >> 2;
		}
	}

	// Vertical down-sampling : if odd # of lines : combine with above (3,1)
	if (nOddLine)
	{
		pDestLine		= &pBuf[i*nLineLength];
		pSourceLineCurr	= &pSource[(nLine - 1) * nLineLength];
		pSourceLineAbove	= &pSource[(nLine - 2) * nLineLength];
		for (j = 0; j < nCol; j++)
		{
			pDestLine[j] = (3*pSourceLineCurr[j]+pSourceLineAbove[j])>>2;
		}
	}

	// Horizontal down-sampling
	for (i = 0; i < (nLine >> 1) + nOddLine; i++)
	{
		pDestLine		= &pDest[i*nLineLengthHalf];
		pSourceLineCurr	= &pBuf[i*nLineLength];

		// First point : combine with right-hand side : weights (3, 1)
		pDestLine[0] = (pSourceLineCurr[0] + pSourceLineCurr[0] 
		+ pSourceLineCurr[0] + pSourceLineCurr[1]) >> 2;

		// Current point : combine with neigbours : weights (1, 2, 1)
		for (j = 1; j < (nCol >> 1); j++)
		{
			pDestLine[j] = (pSourceLineCurr[2*j-1] + pSourceLineCurr[2*j] 
			+ pSourceLineCurr[2*j] + pSourceLineCurr[2*j+1]) >> 2;
		}

		//If pDest is used instead of pBuf (at vertical down-sampling) 
		//replace epilogue (the block below) by the 2 following commented lines
		//Otherwise the values for last col are over-written (read-write crossing)

		//if (nOddCol)  
		//pDestLine[j] = (3*pSourceLineCurr[2*j]+pSourceLineCurr[2*j-1])>> 2;
	}

	// Horizontal down-sampling : if odd # of cols : combine with left (3,1)
	if(nOddCol)
	{
		j = nCol >> 1; 
        int jj = nCol - 1;
		for (i = 0; i < (nLine >> 1) + nOddLine; i++)
		{
			pDestLine		 = &pDest[i*nLineLengthHalf];
			pSourceLineCurr = &pBuf[i*nLineLength];
			pDestLine[j]    = (3*pSourceLineCurr[jj]+pSourceLineCurr[jj-1])>> 2;
		}
	}

	free(pBuf);

	return 0;
}

//*****************************************************************************
// Multi-resolution Gaussian decomposition along NbBand-1 levels (Pyr[Level] : low freq)
// Obtains a Laplacian pyramid (high-pass pictures)
// Assumes first pyramid band (Pyr1[0]) has been filled
//*****************************************************************************
void OscMrGaussianDecompPyr(short **Pyr, int NbBand, int *PyW, int *PyH)
{
	int Level = 0;

	for(Level=0; Level<NbBand; Level++)
	{
		OscMrDownSampling(Pyr[Level], Pyr[Level+1], PyW, PyH, Level);
	}
}

//*****************************************************************************
// Image down sampling by 2 using a binomial filter
// Vertical (Src -> Buf) then horizontal down-sampling (Buf -> Dst)
// Down-sampling (binomial filter 1,2,1)
//*****************************************************************************
int OscMrDownSamplingShort2(short (*pSource)[2], short (*pDest)[2], int *PyW, int *PyH, int Level)
{
	int nCol              = PyW[Level];
	int nLine             = PyH[Level];
	int nLineLength		    = nCol;
	int nLineLengthHalf		= PyW[Level+1];
	int nLineLength2		  = nLineLength + nLineLength;
	int i = 0, j = 0;

	const short (*pSourceLineCurr)[2];
	const short (*pSourceLineAbove)[2];
	const short (*pSourceLineBelow)[2];
	short (*pDestLine)[2];

	int nOddLine = ODD(nLine);
	int nOddCol  = ODD(nCol);

	short (*pBuf)[2];

	OSC_MALLOC(pBuf, PyW[Level]*PyH[Level+1]);

	// Vertial down-sampling : 1st line : combine with below : weights (3,1)
	pSourceLineCurr	    = (short const (*)[2])&pSource[0];
	pSourceLineBelow	= (short const (*)[2])&pSource[nLineLength];
	pDestLine			= &pBuf[0];

	for (j = 0; j < nCol; j++)
	{
		pDestLine[j][0] = (pSourceLineCurr[j][0] + pSourceLineCurr[j][0] + pSourceLineCurr[j][0] + pSourceLineBelow[j][0]) >> 2;
		pDestLine[j][1] = (pSourceLineCurr[j][1] + pSourceLineCurr[j][1] + pSourceLineCurr[j][1] + pSourceLineBelow[j][1]) >> 2;
	}

	// Vertical down-sampling : current lines : weights (1,2,1)
	for (i = 1; i < (nLine >> 1); i++)
	{
		pSourceLineCurr		= (short const (*)[2])&pSource[i*nLineLength2];
		pSourceLineAbove	= (short const (*)[2])&pSource[i*nLineLength2 - nLineLength];
		pSourceLineBelow	= (short const (*)[2])&pSource[i*nLineLength2 + nLineLength];
		pDestLine			= &pBuf[i*nLineLength];

		for (j = 0; j < nCol; j++)
		{
			pDestLine[j][0] = (pSourceLineAbove[j][0] + pSourceLineCurr[j][0] + pSourceLineCurr[j][0] + pSourceLineBelow[j][0]) >> 2;
            pDestLine[j][1] = (pSourceLineAbove[j][1] + pSourceLineCurr[j][1] + pSourceLineCurr[j][1] + pSourceLineBelow[j][1]) >> 2;
		}
	}

	// Vertical down-sampling : if odd # of lines : combine with above (3,1)
	if (nOddLine)
	{
		pDestLine        = &pBuf[i*nLineLength];
		pSourceLineCurr  = (short const (*)[2])&pSource[(nLine - 1) * nLineLength];
		pSourceLineAbove = (short const (*)[2])&pSource[(nLine - 2) * nLineLength];
		for (j = 0; j < nCol; j++)
		{
			pDestLine[j][0] = (3*pSourceLineCurr[j][0]+pSourceLineAbove[j][0])>>2;
			pDestLine[j][1] = (3*pSourceLineCurr[j][1]+pSourceLineAbove[j][1])>>2;
		}
	}

	// Horizontal down-sampling
	for (i = 0; i < (nLine >> 1) + nOddLine; i++)
	{
		pDestLine		= &pDest[i*nLineLengthHalf];
		pSourceLineCurr	= (short const (*)[2])&pBuf[i*nLineLength];

		// First point : combine with right-hand side : weights (3, 1)
		pDestLine[0][0] = (pSourceLineCurr[0][0] + pSourceLineCurr[0][0] + pSourceLineCurr[0][0] + pSourceLineCurr[1][0]) >> 2;
		pDestLine[0][1] = (pSourceLineCurr[0][1] + pSourceLineCurr[0][1] + pSourceLineCurr[0][1] + pSourceLineCurr[1][1]) >> 2;

		// Current point : combine with neigbours : weights (1, 2, 1)
		for (j = 1; j < (nCol >> 1); j++)
		{
			pDestLine[j][0] = (pSourceLineCurr[2*j-1][0] + pSourceLineCurr[2*j][0] + pSourceLineCurr[2*j][0] + pSourceLineCurr[2*j+1][0]) >> 2;
			pDestLine[j][1] = (pSourceLineCurr[2*j-1][1] + pSourceLineCurr[2*j][1] + pSourceLineCurr[2*j][1] + pSourceLineCurr[2*j+1][1]) >> 2;
		}

		//If pDest is used instead of pBuf (at vertical down-sampling) 
		//replace epilogue (the block below) by the 2 following commented lines
		//Otherwise the values for last col are over-written (read-write crossing)

		//if (nOddCol)  
		//pDestLine[j] = (3*pSourceLineCurr[2*j]+pSourceLineCurr[2*j-1])>> 2;
	}

	// Horizontal down-sampling : if odd # of cols : combine with left (3,1)
	if(nOddCol)
	{
		j = nCol >> 1; 
        int jj = nCol - 1;
		for (i = 0; i < (nLine >> 1) + nOddLine; i++)
		{
			pDestLine		 = &pDest[i*nLineLengthHalf];
			pSourceLineCurr = (short const (*)[2])&pBuf[i*nLineLength];
			pDestLine[j][0] = (3*pSourceLineCurr[jj][0]+pSourceLineCurr[jj-1][0])>> 2;
			pDestLine[j][1] = (3*pSourceLineCurr[jj][1]+pSourceLineCurr[jj-1][1])>> 2;
		}
	}

	free(pBuf);

	return 0;
}

//*****************************************************************************
// Multi-resolution Gaussian decomposition along NbBand-1 levels (Pyr[Level] : low freq)
// Obtains a Laplacian pyramid (high-pass pictures)
// Assumes first pyramid band (Pyr1[0]) has been filled
//*****************************************************************************
void OscMrGaussianDecompPyrShort2(short (**Pyr)[2], int NbBand, int *PyW, int *PyH)
{
	int Level = 0;

	for(Level=0; Level<NbBand; Level++)
	{
		OscMrDownSamplingShort2(Pyr[Level], Pyr[Level+1], PyW, PyH, Level);
	}
}

//*****************************************************************************
// Allocate pyramid's memory (from dimension arrays)
//    Pyr1[Level][i*W+j], Pyr2[Level][i][j]
//*****************************************************************************
int OscAllocatePyramidGeneric(COsc *This, void ***PtPyr, int nb_bytes, int *PyW, int *PyH, int NbBand)
{
	void **Pyr;
	int Level;

	OSC_MALLOC(Pyr, NbBand+1);

	if(PtPyr && !Pyr)
	{
        This->ExtractProc.FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAllocatePyramid", OSC_ERR_MEM_ALLOC, "Memory allocation Pb");
		return OSC_ERR_MEM_ALLOC;
	}

	*PtPyr = Pyr;

    for(Level = 0; Level <= NbBand; Level++)
        Pyr[Level] = NULL;

	for(Level = 0; Level <= NbBand; Level++){
		Pyr[Level] = (void*)calloc(PyW[Level]*PyH[Level],nb_bytes);
		if (Pyr[Level] == NULL){
            This->ExtractProc.FatalErrorOnPreviousImage = 1;
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAllocatePyramid", OSC_ERR_MEM_ALLOC, "Memory allocation Pb");
			return OSC_ERR_MEM_ALLOC;
		}
	}

	return 0;
}


//*****************************************************************************
// Free pyramid's allocation
//*****************************************************************************
int OscFreePyramidGeneric(COsc *This, void **Pyr, int NbBand)
{
	int Level;

	for(Level = 0; Level <= NbBand; Level++){
        if (Pyr[Level] != NULL){ free(Pyr[Level]);}
    }
    if (Pyr != NULL){ free(Pyr);}

	return 0;
}
