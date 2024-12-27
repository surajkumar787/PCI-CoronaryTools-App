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

/***************************************************************************/
// MkxApplyRigidMotionMask :                                               */
// Input Image : Enh contains the intensity of the dark spots in the image.*/
//               Mask contains the binary mask of rigid motion pixels      */
// OutPut Image : Enh has pixels zeroed out at rigid motion locations      */
/***************************************************************************/
int  MkxApplyRigidMotionMask(
  float *Enh, int IwEnh, int IhEnh,
  CMkxRoiCoord UsefulRoiCoord, 
  unsigned char *Mask, int IwMask, int IhMask
  )
{
	int localSSFactor = IwEnh/IwMask;
	int  iMin, iMax, jMin, jMax;

	iMin = UsefulRoiCoord.YMin;
	iMax = UsefulRoiCoord.YMax;
	jMin = UsefulRoiCoord.XMin;
	jMax = UsefulRoiCoord.XMax;

	if (IwEnh/IwMask!=IhEnh/IhMask || localSSFactor<1)
	{
		assert(0 && "Invalid size for rigid motion mask\n"); 
		return 1;
	}

	// Get Min and Max
	if (localSSFactor==1)
	{
		for(int i = iMin; i < iMax; i++)
			for(int j = jMin; j < jMax; j++)
				if (Mask[j+i*IwMask])
					Enh[j+i*IwEnh] = 0;
	}
	else if (localSSFactor==2)
	{
		for(int i = iMin; i < iMax; i++)
			for(int j = jMin; j < jMax; j++)
				if (Mask[j/2+(i/2)*IwMask])
					Enh[j+i*IwEnh] = 0;
	}
	else
	{
		for(int i = iMin; i < iMax; i++)
			for(int j = jMin; j < jMax; j++)
				if (Mask[j/localSSFactor+(i/localSSFactor)*IwMask])
					Enh[j+i*IwEnh] = 0;
	}

	return 0;
}





/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/


