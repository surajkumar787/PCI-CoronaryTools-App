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

// Parameter nature
enum  {
	OSC_CST_CRITICAL = 0,  // Critical parameter
	OSC_CST_KEY = 1,	    // Key parameter
	OSC_CST_COMMON = 2,	    // Angio/Fluoro parameter
	OSC_CST_ANGIO = 3,      // Angio parameter
	OSC_CST_FLUORO = 4      // Fluoro parameter	
};

/*************************************************************************************/
// Impacting extraction process fields with new external parameters (passed through PrmSetPara)
// If (LockCriticalParamsFlag==true), critical params are allowed to change, 
// else it raises an error
/*************************************************************************************/
int OscSetExtractProcFromExternalParam(COsc *This, int LockCriticalParamsFlag)
{
	int  Error = 0;
	int  CriticalParamsHaveChanged;

	// *** Check whether critical parameters have been changed.
	// If so and not authorized, error issued
   Error  = PrmHasAnyTaggedParaChanged(This->PrmHdl, &This->ExtractParam, OSC_CST_CRITICAL,	&CriticalParamsHaveChanged);
   if (Error)
   {
      ErrAppendErrorReportFromCaller(This->ErrHdl, "OscSetExtractProcFromExternalParam", "PrmHasAnyTaggedParaChanged", 0, Error);
      return Error;
   }
   if (LockCriticalParamsFlag && (CriticalParamsHaveChanged!=-1) )	// -1 when no critical parameter has been changed
   {
	  char Reason[128];
      sprintf_s(Reason, 128,"Critical Extract-parameters changed at t=%d != 0", This->ExtractProc.t);
      ErrStartErrorReportFromLeaf(This->ErrHdl, "OscSetExtractProcFromParam", OSC_ERR_CRITICAL_PARA, Reason);
      return OSC_ERR_CRITICAL_PARA;
   }

   // *** Get external parameters
   Error = PrmGetPara(This->PrmHdl, &This->ExtractParam);
   if (Error)
   {
      ErrAppendErrorReportFromCaller(This->ErrHdl, "OscSetExtractProcFromExternalParam", "PrmGetPara", 0, Error);
      return Error;
   }

	return 0;
}