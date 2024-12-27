/*!
* \file PlatformCPUDetect.h
*
* \author
* Emmanuel Attia (emmanuel.attia@philips.com)
* The REACT Project - MedISys Research Lab - Philips HealthCare
*
* \brief
* REACT internal productivity tool: Platform CPU detector.
*
* \date 24 sept 2009: Initial header
*
*/
//TICS -*
#ifndef PLATFORMCPUDETECT_H_
#define PLATFORMCPUDETECT_H_

enum Platform_CPU_Ability
{
    Platform_CPU_Ability_MMX = 1,
    Platform_CPU_Ability_SSE = 2,
    Platform_CPU_Ability_SSE2 = 4,
    Platform_CPU_Ability_SSE3 = 8,
    Platform_CPU_Ability_SSSE3 = 16,
    Platform_CPU_Ability_SSE41 = 32,
    Platform_CPU_Ability_SSE42 = 64,
    Platform_CPU_Ability_AVX = 128
};

#ifdef _MSC_VER
#include <intrin.h>
#endif

#ifdef __GNUC__
//#define __cpuid__(func,ax,bx,cx,dx)\
//    __asm__ __volatile__ ("cpuid":\
//    "=a" (ax), "=b" (bx), "=c" (cx), "=d" (dx) : "a" (func));
FORCEINLINE void __cpuid(int (& CPUInfo)[4], int InfoType)
{
    __asm__ __volatile__ ("cpuid": "=a" (CPUInfo[0]), "=b" (CPUInfo[1]), "=c" (CPUInfo[2]), "=d" (CPUInfo[3]) : "a" (InfoType));
}

#endif

FORCEINLINE unsigned int Platform_CPU_DetectAbilities()
{
    unsigned int result = 0;
    int CPUInfo[4];
    __cpuid(CPUInfo, 1);
    if (CPUInfo[3] & (1<<23))
        result |= Platform_CPU_Ability_MMX;
    if (CPUInfo[3] & (1<<25))
        result |= Platform_CPU_Ability_SSE;
    if (CPUInfo[3] & (1<<26))
        result |= Platform_CPU_Ability_SSE2;
    if (CPUInfo[2] & (1<<0))
        result |= Platform_CPU_Ability_SSE3;
    if (CPUInfo[2] & (1<<9))
        result |= Platform_CPU_Ability_SSSE3;
    if (CPUInfo[2] & (1<<19))
        result |= Platform_CPU_Ability_SSE41;
    if (CPUInfo[2] & (1<<20))
        result |= Platform_CPU_Ability_SSE42;
    if ((CPUInfo[2] & ((1<<28)|(1<<27))) == ((1<<28)|(1<<27)))
    {
        //GetThreadContext(NULL, NULL);
        //int64_t out;
        //if (get_xgetbv_info(out, 0))
        //{
            //if ((out & ((1<<1)|(1<<2))) == ((1<<1)|(1<<2)))
                result |= Platform_CPU_Ability_AVX;
        //}
    }

    return result;
}

#endif /* PLATFORMCPUDETECT_H_ */
