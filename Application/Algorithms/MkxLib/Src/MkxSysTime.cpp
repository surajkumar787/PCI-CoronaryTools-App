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
#define NOMINMAX
#include <windows.h> 

#include <MkxDev.h> 

/****************************************************************/
// Return time in ms between call(0) and call("chaine")         */
// call("chaine") print the same ms eval labelled with "chaine" */
/****************************************************************/

int MkxSysTimeMs(char *Name, int verbose)
{
	static LARGE_INTEGER temps1, temps2, freq;
	static int first=1;
	static double MyFreq;
	int time_ms=0;

	if(first) 
	{
		QueryPerformanceFrequency(&freq);
		MyFreq = (double)(freq.QuadPart/(double)1000000);
		first=0;
	}

	if(Name==NULL)
	{
		QueryPerformanceCounter(&temps1);
	}
	else 
	{
		QueryPerformanceCounter(&temps2);
		double Current = (double)((temps2.QuadPart - temps1.QuadPart)/MyFreq);
		time_ms = (int)(Current/1000.0 + 0.5);
		if(verbose)printf("TIMING of %s  = %d ms \n", Name, time_ms);
	}
	return time_ms;
}

int MkxSysTimeMsAndReset(char *Name, int verbose)
{
	int time_ms =MkxSysTimeMs(Name, verbose);
	MkxSysTimeMs(0,0);

	return time_ms;
}
