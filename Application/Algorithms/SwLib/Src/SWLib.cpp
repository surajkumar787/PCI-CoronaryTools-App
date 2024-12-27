// ***************************************************************************
// * Copyright (c) 2002-2016 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************
//SWLib for Width Based Sternal Wire Library

#include "SWLibDev.h"

/*************************************************************************************/
// SWLib: Create (with specified parameters)
/*************************************************************************************/
int SWCreate(CSWHdl *Hdl, int IwMax, int IhMax, int nbCoresMax)
{
  CSW* hdl = (CSW*)malloc(sizeof(CSW));

  hdl->fOri = (float*)calloc(IwMax*IhMax, sizeof(float));
  hdl->Ctr = (float*)calloc(IwMax*IhMax, sizeof(float));
  hdl->Wid = (float*)calloc(IwMax*IhMax, sizeof(float));
  hdl->EnergyTight = (float*)calloc(IwMax*IhMax, sizeof(float));
  hdl->EnergyLoose = (float*)calloc(IwMax*IhMax, sizeof(float));

  hdl->VertiWeights = (float*)calloc(IwMax, sizeof(float));
  
  hdl->MaskLiveTight = (unsigned char*)calloc(IwMax*IhMax, sizeof(unsigned char));
  hdl->MaskRegTight = (unsigned char*)calloc(IwMax*IhMax, sizeof(unsigned char));
  hdl->MaskSeqTight = (unsigned char*)calloc(IwMax*IhMax, sizeof(unsigned char));
  hdl->MaskLiveLoose = (unsigned char*)calloc(IwMax*IhMax, sizeof(unsigned char));
  hdl->MaskRegLoose = (unsigned char*)calloc(IwMax*IhMax, sizeof(unsigned char));
  hdl->MaskSeqLoose = (unsigned char*)calloc(IwMax*IhMax, sizeof(unsigned char));
  hdl->MaskTightFinal = (unsigned char*)calloc(IwMax*IhMax, sizeof(unsigned char));
  hdl->MaskLooseFinal = (unsigned char*)calloc(IwMax*IhMax, sizeof(unsigned char));
  
  hdl->nbCoresMax = nbCoresMax;
  hdl->WidCores = (float**)calloc(hdl->nbCoresMax, sizeof(float*));
  hdl->CtrCores = (float**)calloc(hdl->nbCoresMax, sizeof(float*));
  hdl->WkBuf1Cores = (float**)calloc(hdl->nbCoresMax, sizeof(float*));
  hdl->WkBuf2Cores = (float**)calloc(hdl->nbCoresMax, sizeof(float*));
  for (int k=0; k<hdl->nbCoresMax; k++){
      hdl->WidCores[k] = (float*)calloc(IwMax*IhMax, sizeof(float));
      hdl->CtrCores[k] = (float*)calloc(IwMax*IhMax, sizeof(float));
      hdl->WkBuf1Cores[k] = (float*)calloc(IwMax*IhMax, sizeof(float));
      hdl->WkBuf2Cores[k] = (float*)calloc(IwMax*IhMax, sizeof(float));
  }

  hdl->WkBuf1S = (short*)calloc(IwMax*IhMax, sizeof(short));
  hdl->WkBuf2S = (short*)calloc(IwMax*IhMax, sizeof(short));
  hdl->WkBuf3S = (short*)calloc(IwMax*IhMax, sizeof(short));
  hdl->WkBuf4S = (short*)calloc(IwMax*IhMax, sizeof(short));

  hdl->WkBufI1 = (int*)calloc(IwMax*IhMax, sizeof(int));
  hdl->WkBufI2 = (int*)calloc(IwMax*IhMax, sizeof(int));
  hdl->WkBufI3 = (int*)calloc(IwMax*IhMax, sizeof(int));
  hdl->WkBufI4 = (int*)calloc(IwMax*IhMax, sizeof(int));

  hdl->WkBufUC1 = (unsigned char*)calloc(IwMax*IhMax, sizeof(unsigned char));
  hdl->WkBufUC2 = (unsigned char*)calloc(IwMax*IhMax, sizeof(unsigned char));

  hdl->estTx = 0; hdl->estTy = 0;
  hdl->IwAlloc = IwMax; hdl->IhAlloc = IhMax;

  SWSetVersion(hdl);

  Hdl->Pv = (void *)hdl;

  return 0;
}

int SWSetThreadpool(CSWHdl *Hdl, IThreadPool *pool)
{
    CSW* hdl = (CSW*)Hdl->Pv;

    // Check that no more thread than allocated for
    int nbProc;
    pool->execute([&nbProc](int threadIndex, int threadCount){ nbProc = threadCount;}); 
    
    if (nbProc > hdl->nbCoresMax)
        return -1;
    else {
        hdl->pool = pool;
        return 0;
    }
}

/*************************************************************************************/
// SWLib: Delete
/*************************************************************************************/
void SWDelete(CSWHdl *Hdl)
{
    CSW* hdl = (CSW*)Hdl->Pv;

    SW_LIB_FREE(hdl->fOri);
    SW_LIB_FREE(hdl->Ctr);
    SW_LIB_FREE(hdl->Wid);
    SW_LIB_FREE(hdl->EnergyTight);
    SW_LIB_FREE(hdl->EnergyLoose);
    SW_LIB_FREE(hdl->VertiWeights);
    SW_LIB_FREE(hdl->MaskLiveTight);
    SW_LIB_FREE(hdl->MaskRegTight);
    SW_LIB_FREE(hdl->MaskSeqTight);
    SW_LIB_FREE(hdl->MaskLiveLoose);
    SW_LIB_FREE(hdl->MaskRegLoose);
    SW_LIB_FREE(hdl->MaskSeqLoose);
    SW_LIB_FREE(hdl->MaskTightFinal);
    SW_LIB_FREE(hdl->MaskLooseFinal);
  
    for (int k=0; k<hdl->nbCoresMax; k++){
        SW_LIB_FREE(hdl->WidCores[k]);
        SW_LIB_FREE(hdl->CtrCores[k]);
        SW_LIB_FREE(hdl->WkBuf1Cores[k]);
        SW_LIB_FREE(hdl->WkBuf2Cores[k]);
    }
    SW_LIB_FREE(hdl->WidCores);
    SW_LIB_FREE(hdl->CtrCores);
    SW_LIB_FREE(hdl->WkBuf1Cores);
    SW_LIB_FREE(hdl->WkBuf2Cores);

    SW_LIB_FREE(hdl->WkBuf1S);
    SW_LIB_FREE(hdl->WkBuf2S);
    SW_LIB_FREE(hdl->WkBuf3S);
    SW_LIB_FREE(hdl->WkBuf4S);

    SW_LIB_FREE(hdl->WkBufI1);
    SW_LIB_FREE(hdl->WkBufI2);
    SW_LIB_FREE(hdl->WkBufI3);
    SW_LIB_FREE(hdl->WkBufI4);

    SW_LIB_FREE(hdl->WkBufUC1);
    SW_LIB_FREE(hdl->WkBufUC2);

    SW_LIB_FREE(hdl->VersionInfo);
    SW_LIB_FREE(hdl->VersionInfoCopy);



    free(hdl);
}

// ****************************************
int SWTranslateMaskToDifferentImageSize(CSWHdl *Hdl, int Iw, int Ih){
    
    CSW* hdl = (CSW*)Hdl->Pv;

    if ( (Iw == hdl->Iw) && (Ih == hdl->Ih) )
        return 0;

    if ( (Iw>hdl->IwAlloc) || (Ih>hdl->IhAlloc) )
        return -1;

    if (hdl->nbPixMask == 0){
        memset(hdl->MaskLooseFinal, 0, Iw*Ih*sizeof(unsigned char));
    } else {
        unsigned char * MaskResized = hdl->WkBufUC1;
        memset(MaskResized, 0, Iw*Ih*sizeof(unsigned char));

        int offsetOldX = SW_LIB_MAX(0,(hdl->Iw-Iw)/2), offsetOldY = SW_LIB_MAX(0,(hdl->Ih-Ih)/2);
        int offsetNewX = SW_LIB_MAX(0,(Iw-hdl->Iw)/2), offsetNewY = SW_LIB_MAX(0,(Ih-hdl->Ih)/2);

        for (int y=0; y<SW_LIB_MIN(Ih, hdl->Ih); y++)
            for (int x=0; x<SW_LIB_MIN(Iw, hdl->Iw); x++)
                MaskResized[(offsetNewY+y)*Iw+offsetNewX+x] = hdl->MaskLooseFinal[(offsetOldY+y)*hdl->Iw+offsetOldX+x];

        memcpy(hdl->MaskLooseFinal, MaskResized, Iw*Ih*sizeof(unsigned char));
    }

    hdl->Iw = Iw; hdl->Ih = Ih;

    return 0;
}
// ****************************************
int SWRestoreStatusForTracking(CSWHdl *Hdl, int Iw, int Ih, unsigned char *NewMaskIn){
    CSW* hdl = (CSW*)Hdl->Pv;
    
    if ( (Iw>hdl->IwAlloc) || (Ih>hdl->IhAlloc) )
        return -1;

    hdl->Iw = Iw; hdl->Ih = Ih;
    memcpy(hdl->MaskLooseFinal, NewMaskIn, Iw*Ih*sizeof(unsigned char));
    hdl->nbPixMask = 0;
    for (int p=0; p<Iw*Ih; p++)
        if (hdl->MaskLooseFinal[p] > 0)
            hdl->nbPixMask++;

    return 0;
}

/*************************************************************************************/
int SWSetVersion(CSW* hdl)
{
	int  NbChar;
	char *Format;

	NbChar = (int)(strlen(SW_VERSION_INFO) + 16);
	SW_LIB_MALLOC(hdl->VersionInfo    , NbChar);
	SW_LIB_MALLOC(hdl->VersionInfoCopy, NbChar);
	SW_LIB_MALLOC(Format               , NbChar);

	sprintf_s(Format, NbChar, "SWLib %s", SW_VERSION_INFO);
	sprintf_s(hdl->VersionInfo, NbChar, Format, SW_VERSION_MAJOR, SW_VERSION_MINOR, SW_VERSION_PATCH);

	free(Format);
	return 0;
}

/*************************************************************************************/
int SWGetVersionInfo(CSWHdl Hdl, char** VersionInfo){
	CSW* hdl = (CSW*)Hdl.Pv;
    int NbChar = (int)(strlen(SW_VERSION_INFO) + 16);

	strcpy_s(hdl->VersionInfoCopy,NbChar, hdl->VersionInfo);
	*VersionInfo = hdl->VersionInfoCopy;
	return 0;
}

// ****************************************
void SWUpdateParameters(CSWHdl *Hdl, int Iw, int Ih, CSWRoiCoord* RoiCoord, int t, float pixSize)
{
    CSW* hdl = (CSW*)Hdl->Pv;
    // Parameters
    hdl->Iw = Iw; hdl->Ih = Ih; hdl->t = t;
    hdl->Roi.XMin = RoiCoord->XMin; hdl->Roi.XMax = RoiCoord->XMax;
    hdl->Roi.YMin = RoiCoord->YMin; hdl->Roi.YMax = RoiCoord->YMax;
    hdl->pixSize = pixSize;

    hdl->SearchRange = (int)SW_LIB_RND( SW_SEARCH_RANGE_MM / pixSize);
    hdl->WidLowTight = SW_WID_LOW_TIGHT_MM / pixSize;
    hdl->WidHighTight = SW_WID_HIGH_TIGHT_MM / pixSize;
    hdl->WidLowLoose = SW_WID_LOW_LOOSE_MM / pixSize;
    hdl->WidHighLoose = SW_WID_HIGH_LOOSE_MM / pixSize;
    hdl->VertiSigmaPix = SW_VERTI_WEIGHT_SIGMA_MM / pixSize;
    hdl->DistToTouchBorder = (int)SW_LIB_RND(SW_DIST_TO_TOUCH_BORDER_PIX_MM / pixSize);
    
    float tabIn[16] = SW_SIGMAS;
    for (int i=0; i<16; i++)
        hdl->TabSig[i] = tabIn[i] / pixSize;

    
    hdl->RangeSameRegion = SW_LIB_RND(SW_RANGE_SAME_REGION_MM /  pixSize);
    hdl->MinDistSWX = SW_LIB_RND(SW_LIB_MIN_SIZE_X_MM /  pixSize);
    hdl->MinDistSWY = SW_LIB_RND(SW_LIB_MIN_SIZE_Y_MM /  pixSize);
    hdl->MinSurfaceTouchesBorder_1 = SW_LIB_MIN_SURFACE_BORDER_1_MM2 /  (pixSize*pixSize);
    hdl->MinSurfaceTouchesBorder_2 = SW_LIB_MIN_SURFACE_BORDER_2_MM2 /  (pixSize*pixSize);
    hdl->MinSurface_1 = SW_LIB_MIN_SURFACE_1_MM2 /  (pixSize*pixSize);
    hdl->MinSurface_2 = SW_LIB_MIN_SURFACE_2_MM2 /  (pixSize*pixSize);
    hdl->MarginShutters = SW_LIB_RND( SW_MARGIN_SHUTTERS_LOOSE * Iw / 1024);
    hdl->dilSizeMaxDistToLoose = SW_LIB_RND(SW_FINAL_MASK_MAX_DIST_TO_LOOSE_MM / pixSize);
    hdl->sigmaTracking = SW_SIGMA_TRACKING / pixSize;
}