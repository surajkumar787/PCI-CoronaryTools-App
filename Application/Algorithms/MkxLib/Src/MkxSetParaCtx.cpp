// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <ErrLib.h>
#include <PrmLib.h>
#include <MkxDev.h>

int MkxSetParaCtx(CPrmHdl P)
{
	int  k, E = 0;
	char FuncName[] ="MkxSetParaCtx";

#include <MkxParaContext.h>

	if(E) 
		ErrAppendErrorReportFromCaller(P.ErrHdl, FuncName, "PrmSetCtx???", 0, E);
	return E;
}

