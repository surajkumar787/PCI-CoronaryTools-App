// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <stdlib.h>

#include <MkxDev.h> 

#include <algorithm>



static void  MkxBlobIntensity(int NbLblBlobs, CMkxBlob *blobs, const int* Lab, const float* Enh, int Iw, int Ih, CMkxRoiCoord roiCoord)
{
  if (NbLblBlobs == 0)return;

  //memset(blobs, 0, sizeof(*blobs) * NbLblBlobs);
  for (int k=0;k<NbLblBlobs;k++)
  {
    blobs[k].V.X=0;
    blobs[k].V.Y=0;
    blobs[k].Intensity=0;
    blobs[k].NbPix=0;
  }

  // load X, Y Enh fields
  for (int i = roiCoord.YMin; i < roiCoord.YMax; i++)
  {
    for (int j = roiCoord.XMin; j < roiCoord.XMax; j++)
    {
      if (Lab[i*Iw+j] >= 2)
      {
        int k = Lab[i*Iw+j] - 2;
        float w = Enh[i*Iw+j];

        blobs[k].V.X += (int)MKX_RND(w * j);
        blobs[k].V.Y += (int)MKX_RND(w * i);
        blobs[k].Intensity += w;
        blobs[k].NbPix++;
      }
    }
  }

  for (int k = 0; k < NbLblBlobs; k++)
  {
    float invIntensity = 1.0f / blobs[k].Intensity;

    blobs[k].V.X = MKX_RND(invIntensity * blobs[k].V.X);
    blobs[k].V.Y = MKX_RND(invIntensity * blobs[k].V.Y);
  }

}

/***************************************************************************/
// KBestBlobsSelect :                                                      
// Determines a maximum of MaxNbBestBlobs of the best labels,                
// i.e. the ones that have the                                             
// highest intensity.                                                      
// Input: blobs (NbLblBlobs), MaxNbBestBlobs                                  
// Output: KBestBlobs (NbBestBlobs)                                      
/***************************************************************************/
void  MkxKBestBlobsSelect(int NbLblBlobs, CMkxBlob *blobs, int *NbBestBlobs, CMkxBlob *KBestBlobs, int MaxNbBestBlobs)
{
  int        k, l;
  CMkxBlob *MarkersBuf;

  //memset(KBestBlobs, 0, sizeof(*KBestBlobs) * MaxNbBestBlobs);
  for(int k=0;k<MaxNbBestBlobs;k++)
  {
    KBestBlobs[k].Intensity=0;
    KBestBlobs[k].NbPix=0;
    KBestBlobs[k].V.X=0;
    KBestBlobs[k].V.Y=0;
  }

  if (NbLblBlobs == 0)
  {
    *NbBestBlobs = 0;
    return;
  }

  MarkersBuf = (CMkxBlob*)malloc(NbLblBlobs * sizeof(CMkxBlob));
  for (k = 0; k < NbLblBlobs; k++)
    MkxBlobCopy(&(blobs[k]), &(MarkersBuf[k]));

  *NbBestBlobs = MKX_MIN(NbLblBlobs, MaxNbBestBlobs);

  // Select the NbBestBlobs among NbLblBlobs and sort them according to intensity
  for (k = 0; k < *NbBestBlobs; k++)
  {
    int lMax = 0;
    for (l = 0; l < NbLblBlobs; l++)
    {
      if (MarkersBuf[l].Intensity > MarkersBuf[lMax].Intensity)
        lMax = l;
    }
    MkxBlobCopy(&(MarkersBuf[lMax]), &(KBestBlobs[k]));
        //memset(&MarkersBuf[lMax], 0, sizeof(MarkersBuf[lMax]));

    MarkersBuf[lMax].Intensity=0;
    MarkersBuf[lMax].NbPix=0;
    MarkersBuf[lMax].V.X=0;
    MarkersBuf[lMax].V.Y=0;

  }

  free(MarkersBuf);
}

/*************************************************************************************/
// MarkersSelect 
/*************************************************************************************/
void MkxBlobSelect(
  int NbLblBlobs,
  const int* Lab, const float* Enh,
  int Iw, int Ih,
  CMkxRoiCoord roiCoord,
  int *NbBestLblBlobs,
  CMkxBlob *KBestLblBlobs,
  int MaxNbBlobs
  )
{
  CMkxBlob *blobs = (CMkxBlob*)malloc(std::max(1, NbLblBlobs)* sizeof(CMkxBlob)); /*(+1) for the case where NbLblBlobs = 0*/

  // measures on labels so as to estimate their intensity 
  MkxBlobIntensity(NbLblBlobs, blobs, Lab, Enh, Iw, Ih, roiCoord);

  // the best labels are kept and stored in KBestLblBlobs (max number of labels kept = ExtractParam->NbLabelsKept)
  MkxKBestBlobsSelect(NbLblBlobs, blobs, NbBestLblBlobs, KBestLblBlobs, MaxNbBlobs);

  free(blobs);
}
