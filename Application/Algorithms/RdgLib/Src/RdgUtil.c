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
#include <windows.h> 
#include <malloc.h>
#include <RdgDev.h> 

#define MEM_AVOID_64K_ALIASING




/*************************************************************************************/
// Image allocation function for Rdg internals (no high-level error reporting)
/*************************************************************************************/
int RdgImAlloc(void **Pt1, void ***Pt2, int NbByte, int Iw, int Ih)
{
	char       *Relai;
	int        i;
#ifdef MEM_AVOID_64K_ALIASING
   int        padding;
#endif
	
	if(Pt1 == 0 && Pt2 == 0)
	{
		return(RDG_ERR_MEM_FUNC_ARG);
	}

#ifdef MEM_AVOID_64K_ALIASING
   // alloc more space with a random offset
   // will return Relai+padding as the pointer
   // the returned Relai-1 will hold the address of the allocated pointer, to be able to correctly free this space
   padding = 128*(rand()%(65535/128));
   if (padding<sizeof(void *))
      padding = sizeof(void *);
   Relai = (char *) calloc((Iw * Ih)*NbByte + padding, 1); 
   ((void **) (((char *)Relai)+padding))[-1] = Relai;
   // *(((void **) (Relai+padding))-1) = (void *) Relai;
   Relai = Relai+padding;
#else
   Relai = (char *) calloc(Iw * Ih, NbByte);
#endif
	if(Relai == NULL)
	{
		return(RDG_ERR_MEM_ALLOC);
	}
	if(Pt1 != 0) *Pt1 = (void *) Relai;

	if(Pt2 != 0)
	{
#ifdef MEM_AVOID_64K_ALIASING
      padding = 128*(rand()%(65535/128));
      if (padding<sizeof(void *))
         padding = sizeof(void *);
      *Pt2 = (void **) malloc(Ih * sizeof(void *)+padding);
      ((void **) (((char *)*Pt2)+padding))[-1] = *Pt2;
      // *(((void **) (((char *) *Pt2)+padding))-1) = *Pt2; 
      *Pt2 = (void **) (((char *) *Pt2)+padding);
#else
      *Pt2 = (void **) malloc(Ih * sizeof(void *));
#endif
		if (*Pt2 == NULL)
		{
			return(RDG_ERR_MEM_ALLOC);
		}

		for (i = 0; i < Ih; i++)
			*((*Pt2)+i) = (void *) (Relai + (i * Iw * NbByte));
	}

	return(0);
}

/*************************************************************************************/
// Image freeing function for Rdg internals (no high-level error reporting)
/*************************************************************************************/
int RdgImFree(void *Pt1, void **Pt2)
{	
	if(Pt1 == 0 && Pt2 == 0) 
		return(0);	

#ifdef MEM_AVOID_64K_ALIASING
	if(Pt1 != 0)
	{
		free(((void **)Pt1)[-1]);
		if (Pt2 != 0) free(((void **)Pt2)[-1]);
	}
	else
	{
		free(((void **)Pt2[0])[-1]);
		free(((void **)Pt2)[-1]);
	}
#else
	if(Pt1 != 0)
	{
		free(Pt1);
		if (Pt2 != 0) free(Pt2);
	}
	else
	{
		free(Pt2[0]);
		free(Pt2);
	}
#endif
	
	return(0);
}


/*************************************************************************************/
// Image allocation function for Rdg internals (no high-level error reporting)
/*************************************************************************************/
int RdgImAlignedAlloc(void **Pt1, void ***Pt2, int NbByte, int Iw, int Ih, int Alignment)
{
    char       *Relai;
    int        i;

    if(Pt1 == 0 && Pt2 == 0)
    {
        return(RDG_ERR_MEM_FUNC_ARG);
    }

    Relai = (char *) _aligned_malloc(Iw * Ih * NbByte, Alignment);
    memset(Relai, 0, Iw * Ih * NbByte);
    if(Relai == NULL)
    {
       return(RDG_ERR_MEM_ALLOC);
    }
    if(Pt1 != 0) *Pt1 = (void *) Relai;

    if(Pt2 != 0)
    {
        *Pt2 = (void **) _aligned_malloc(Ih * sizeof(void *), Alignment);
        if (*Pt2 == NULL)
        {
            return(RDG_ERR_MEM_ALLOC);
        }

        for (i = 0; i < Ih; i++)
            *((*Pt2)+i) = (void *) (Relai + (i * Iw * NbByte));
    }

    return(0);
}

/*************************************************************************************/
// Image freeing function for Rdg internals (no high-level error reporting)
/*************************************************************************************/
int RdgImAlignedFree(void *Pt1, void **Pt2)
{	
	if(Pt1 == 0 && Pt2 == 0) 
		return(0);	

	if(Pt1 != 0)
	{
		_aligned_free(Pt1);
		if (Pt2 != 0) _aligned_free(Pt2);
	}
	else
	{
		_aligned_free(Pt2[0]);
		_aligned_free(Pt2);
	}
	
	return(0);
}


/****************************************************************/
// From rfSystem.c
// Return time in ms between call(0) and call("chaine")         */
// call("chaine") print the same ms eval labelled with "chaine" */
/****************************************************************/
int RdgSysTimeMs(char *Name)
{
    static LARGE_INTEGER temps1;
    static int first=1;
    static double MyFreq;
    double Current;
    int time_ms = 0;

    if(first) 
    {
        LARGE_INTEGER freq;

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
        LARGE_INTEGER temps2;

        QueryPerformanceCounter(&temps2);
        Current = (double)((temps2.QuadPart - temps1.QuadPart)/MyFreq);
        time_ms = (int)(Current/1000.0 + 0.5);
        printf("TIMING of %s  = %d ms \n", Name, time_ms);
    }

    return time_ms;
}


/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
