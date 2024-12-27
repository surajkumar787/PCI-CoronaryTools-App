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


/*************************************************************************************/
// Displaying/Monitoring of Angio Overlays

/*************************************************************************************/
int OscAngioPicGetCurrentVesselImage(COscHdl OscHdl, float *VesselIm /* Has to be allocated beforehand, at least IwProc x IhProc*/){

    int p;
    COsc                *This = (COsc*)OscHdl.Pv;
    COscExtractProc     *ExtractProc		= &(This->ExtractProc);

    for (p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++)
        VesselIm[p] = ExtractProc->ImFrg[p] * ExtractProc->ImMask[p];

    return 0;
}

/*************************************************************************************/
unsigned char* OscAngioSeqGetOverlayAssociatedToAngioFrame(COscHdl OscHdl, int t){
	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);

	if ( (t<ExtractSequence->CardiacCycleStart) || (t>ExtractSequence->CardiacCycleEnd) )
		return NULL;
	else
		return ExtractSequence->ExtractPictures[t].ImOverlayMask;
}

/*************************************************************************************/
int OscAngioSeqGetRegisteredOverlay(COscHdl OscHdl, int AngioIndex, int RegisterIndex, unsigned char *OutOverlayIma /* user allocated */)
{
   COsc                *This = (COsc*)OscHdl.Pv;
   COscExtractProc     *ExtractProc		= &(This->ExtractProc);
   COscExtractSequence *ExtractSequence	= &(This->ExtractSequence);
   char                 Reason[128];
   int					indexMin, indexMax;
   int					Iw = ExtractProc->IwProcAngio, Ih = ExtractProc->IhProcAngio;

   // Check that angio pictures have been processed
   if (!ExtractProc->angioProcessed)
   {
      sprintf_s(Reason, 128, "OscAngioSeqGetRegisteredOverlay called before angio sequence has been processed"); 
      ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioProcessSequence", OSC_ERR_ANGIO_UNPROCESSED, Reason);
      return OSC_ERR_ANGIO_UNPROCESSED;
   }

   indexMin = ExtractSequence->CardiacCycleStart; indexMax = ExtractSequence->CardiacCycleEnd;

   if ((   AngioIndex>=indexMin &&    AngioIndex<=indexMax) &&
       (RegisterIndex>=indexMin && RegisterIndex<=indexMax))
   {
	   int dptX = ExtractSequence->dXAngioToAngio[AngioIndex][RegisterIndex];
	   int dptY = ExtractSequence->dYAngioToAngio[AngioIndex][RegisterIndex];
	   int xMin1 = OSC_MAX(0, dptX), yMin1 = OSC_MAX(0, dptY), xMax1 = OSC_MIN(Iw-1+dptX, Iw-1), yMax1 = OSC_MIN(Ih-1+dptY, Ih-1)
		   , xMin2 = OSC_MAX(0, -dptX), yMin2 = OSC_MAX(0, -dptY), width = xMax1-xMin1+1, height = yMax1-yMin1+1, x, y;

	   OSC_MEMZERO(OutOverlayIma, Iw*Ih);

	   for (y=0; y<height; y++)
		   for (x=0; x<width; x++)
			   OutOverlayIma[(yMin2+y)*Iw+xMin2+x] 
                    = ExtractSequence->ExtractPictures[AngioIndex].ImOverlayMask[(yMin1+y)*ExtractProc->IwProcAngio+xMin1+x];
   }
   else
   {
      sprintf_s(Reason, 128, "OscAngioSeqGetRegisteredOverlay called with incorrect angio picture indices"); 
      ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioSeqGetRegisteredOverlay", OSC_ERR_INCORRECT_TIME, Reason);
      return OSC_ERR_INCORRECT_TIME;
   }

	return 0;
}

/*************************************************************************************/
void OscAngioGetOvrSize(COscHdl OscHdl, int *IwOvr, int *IhOvr, int *SSFactor){
    COsc                *This = (COsc*)OscHdl.Pv;
    COscExtractProc     *ExtractProc		= &(This->ExtractProc);
    
    *IwOvr = ExtractProc->IwProcAngio; *IhOvr = ExtractProc->IhProcAngio; *SSFactor = ExtractProc->SSFactor;
}

/*************************************************************************************/
unsigned char * OscFluoroGetOverlay(COscHdl OscHdl, int *IwOvr, int *IhOvr, int *SSFactor){

    COsc                *This = (COsc*)OscHdl.Pv;
    COscExtractProc     *ExtractProc		= &(This->ExtractProc);
   COscExtractSequence *ExtractSequence	= &(This->ExtractSequence);
    
    *IwOvr = ExtractProc->IwProcAngio; *IhOvr = ExtractProc->IhProcAngio; *SSFactor = ExtractProc->SSFactor;
    return ExtractSequence->ExtractPictures[ExtractProc->SelectedPhase].ImOverlayMask;
}

/*************************************************************************************/
int OscFluoroGetCurrentWarpedOverlay(COscHdl OscHdl, COscFluoroPictureResults *ExtractResults, unsigned char *OvrW){
	
	COsc                *This = (COsc*)OscHdl.Pv;
	int					Iw = ExtractResults->Advanced.IwOri, Ih = ExtractResults->Advanced.IhOri
						, dX = ExtractResults->dXSelectedAngio, dY = ExtractResults->dYSelectedAngio, x, y;

	if (ExtractResults->Advanced.Volatile.Ovr == NULL){
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscGetCurrentWarpedOverlay", OSC_ERR_EMPTY_POINTER, "Overlay is NULL");
		return OSC_ERR_EMPTY_POINTER;
	}

	for (y=OSC_MAX(0,-dY); y<OSC_MIN(Ih,Ih-dY); y++)
		for (x=OSC_MAX(0,-dX); x<OSC_MIN(Iw,Iw-dX); x++)
			OvrW[y*Iw+x] =ExtractResults->Advanced.Volatile.Ovr[(y+dY)*Iw+x+dX];

	return 0;
}