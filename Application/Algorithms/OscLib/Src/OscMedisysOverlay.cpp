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

#define OSC_MEDISYS_OVERLAY_SHIFT_BITS			3				// 14-8-3. Ori dyn 14, final 8

/*************************************************************************************/
// Convenience function to set the shutters ROI
/*************************************************************************************/
int OscComputeMedisysOverlay(COscHdl OscHdl, const short* InIma, short *ImOut)
{	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	int					x, y;
	int					tViterbi = ExtractProc->t % OSC_VITERBI_FLUORO_DEPTH;
	int					dX = -ExtractProc->SSFactor*ExtractProc->FluoroToAngioDX[tViterbi][ExtractProc->SelectedPhase] + (ExtractProc->IwOriAngio-ExtractProc->IwOriFluoro)/2;
	int					dY = -ExtractProc->SSFactor*ExtractProc->FluoroToAngioDY[tViterbi][ExtractProc->SelectedPhase] + (ExtractProc->IhOriAngio-ExtractProc->IhOriFluoro)/2;
	unsigned char		*Ovr = ExtractSequence->ExtractPictures[ExtractProc->SelectedPhase].ImOverlayMask;
    int SSFactor = ExtractProc->SSFactor, xI, yI, xI1, yI1;
    float xF, yF, dx, dy;
    unsigned char		*OvrFullRes = (unsigned char *)malloc(ExtractProc->IwOriAngio*ExtractProc->IhOriAngio*sizeof(unsigned char));

	if (Ovr == NULL){
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscComputeMedisysOverlay", OSC_ERR_EMPTY_POINTER, "Overlay is NULL");
        free(OvrFullRes);
		return OSC_ERR_EMPTY_POINTER;
	}

    for (y=0; y<ExtractProc->IhOriAngio; y++)
        for (x=0; x<ExtractProc->IwOriAngio; x++){
            xF = x/(float)SSFactor; yF = y/(float)SSFactor;
            xI = OSC_MIN((int)xF, ExtractProc->IwProcAngio-1); yI = OSC_MIN((int)yF, ExtractProc->IhProcAngio-1); 
            xI1 = OSC_MIN(xI+1, ExtractProc->IwProcAngio-1); yI1 = OSC_MIN(yI+1, ExtractProc->IhProcAngio-1);
            dx = xF-xI; dy = yF-yI;
            OvrFullRes[y*ExtractProc->IwOriAngio+x] = (unsigned char) OSC_RND( (1-dx)*(1-dy)*Ovr[yI*ExtractProc->IwProcAngio+xI]
                                                                                + dx*(1-dy)*Ovr[yI*ExtractProc->IwProcAngio+xI1]
                                                                                + (1-dx)*dy*Ovr[yI1*ExtractProc->IwProcAngio+xI1]
                                                                                + dx*dy*Ovr[yI1*ExtractProc->IwProcAngio+xI1] );
        }

	OSC_MEMCPY(ImOut, InIma, ExtractProc->IwOriRawFluoro*ExtractProc->IhOriRawFluoro);
	for (y=OSC_MAX(0,-dY); y<OSC_MIN(ExtractProc->IhOriRawFluoro,ExtractProc->IhOriAngio-dY); y++)
		for (x=OSC_MAX(0,-dX); x<OSC_MIN(ExtractProc->IwOriRawFluoro,ExtractProc->IwOriAngio-dX); x++)
			ImOut[y*ExtractProc->IwOriRawFluoro+x] = OSC_MAX(0, InIma[y*ExtractProc->IwOriRawFluoro+x] 
														    - ((short)((OvrFullRes[(y+dY)*ExtractProc->IwOriAngio+x+dX]) << OSC_MEDISYS_OVERLAY_SHIFT_BITS)) );
    free(OvrFullRes);

	return 0;
}