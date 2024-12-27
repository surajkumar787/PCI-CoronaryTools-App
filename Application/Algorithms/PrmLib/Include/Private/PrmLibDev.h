// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_PRMLIBDEV_H
#define		_PRMLIBDEV_H

#include <ErrLib.h>

#define PRM_ABS(a) (((a)>=0)?(a):-(a))

typedef struct
{
	CPrmHdl     PrmHdl;                  // Copy of handle in object
	CErrHdl     ErrorHdl;                // Error handle
	CPrmPara    Para;                    // Parameters
	CPrmParaCtx Context;                 // Parameter contexts
	int         MaxNbCharInVersionInfo;  // Maximum number of characteres in version-info
	char*       VersionInfo;             // Version info string
	char*       VersionInfoCopy;         // Used when returned, as a pointer, to client
	int         WithinCreate;            // 1 if within PrmCreate

}CPrm;

int PrmClearCtx(CPrmHdl PrmHdl);
int PrmCheckParaCtx(CPrmHdl PrmHdl);
int PrmSetVersion(CPrm *This);

#endif   //_PRMLIBDEV_H