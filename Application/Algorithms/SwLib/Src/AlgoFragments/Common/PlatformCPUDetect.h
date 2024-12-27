/*
 * Copyright (c)2009-2014 Koninklijke Philips Electronics N.V.,
 * All Rights Reserved.
 *
 * This source code and any compilation or derivative thereof is the
 * proprietary information of Koninklijke Philips Electronics N.V.
 * and is confidential in nature.
 * Under no circumstances is this software to be combined with any Open Source
 * Software in any way or placed under an Open Source License of any type 
 * without the express written permission of Koninklijke Philips 
 * Electronics N.V.
 *
 * Author: Emmanuel Attia - emmanuel.attia@philips.com - Philips Research Paris - Medisys
 *
 */
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
    Platform_CPU_Ability_AVX = 128,
    Platform_CPU_Ability_AVX2 = 256
};

#define PLATFORM_CPU_UNTIL(x)   (((x)<<1)-1)

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

#include <stdlib.h>
#include <string.h>

namespace
{
    FORCEINLINE unsigned int Platform_CPU_DetectAbilities(char const * env)
    {
        unsigned int result = 0;
        unsigned int mask = (unsigned int)~0;
        if (env != NULL)
        {
#define Platform_CPU_DetectAbilities_TestFOR(x) \
        else if (strcmp(env, #x) == 0) { mask = PLATFORM_CPU_UNTIL(Platform_CPU_Ability_##x); }
            if (0);
            Platform_CPU_DetectAbilities_TestFOR(MMX)
            Platform_CPU_DetectAbilities_TestFOR(SSE)
            Platform_CPU_DetectAbilities_TestFOR(SSE2)
            Platform_CPU_DetectAbilities_TestFOR(SSE3)
            Platform_CPU_DetectAbilities_TestFOR(SSSE3)
            Platform_CPU_DetectAbilities_TestFOR(SSE41)
            Platform_CPU_DetectAbilities_TestFOR(SSE42)
            Platform_CPU_DetectAbilities_TestFOR(AVX)
            Platform_CPU_DetectAbilities_TestFOR(AVX2)
#undef Platform_CPU_DetectAbilities_TestFOR
        }

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
#if PLATFORM_INTEL_SSE_COMPILABLE >= 50
        if ((CPUInfo[2] & ((1<<28)|(1<<27))) == ((1<<28)|(1<<27)))
        {
#if __ICL >= 1300 // Intel C++ 12.0 (2011) bug workaround see http://software.intel.com/en-us/forums/topic/276970
            if ((_xgetbv(0) & 6) == 6) // Check for OS support of ymm registers
#endif
            {
                result |= Platform_CPU_Ability_AVX;
                {
                    // AVX2 detection
                    // Source: https://software.intel.com/en-us/articles/how-to-detect-new-instruction-support-in-the-4th-generation-intel-core-processor-family
                    int avx2_bmi12_mask = (1 << 5) | (1 << 3) | (1 << 8);
                    int fma_movbe_osxsave_mask = ((1 << 12) | (1 << 22) | (1 << 27));

                    if ((CPUInfo[2] & fma_movbe_osxsave_mask) == fma_movbe_osxsave_mask)
                    {
                        __cpuid(CPUInfo, 7);

                        if ((CPUInfo[1] & avx2_bmi12_mask) == avx2_bmi12_mask)
                            result |= Platform_CPU_Ability_AVX2;
                    }
                }
            }
        }
#endif

        return result & mask;
    }

    FORCEINLINE unsigned int Platform_CPU_DetectAbilities()
    {
#pragma warning(push)
#pragma warning(disable:4996)
        return Platform_CPU_DetectAbilities(getenv("PLATFORM_CPU_LIMIT"));
#pragma warning(pop)
    }

    // Check abilities, using PLATFORM_INTEL_SSE values
    // Pass zero to get true (for default behaviors)
    FORCEINLINE bool Platform_CPU_IsSSEAbove(int platformIntelSSE)
    {
        int abilities = Platform_CPU_DetectAbilities();
#define Platform_CPU_IsSSEAbove_IMPL(LEVEL, x) \
    if ((platformIntelSSE >= LEVEL) && (0 == (abilities & Platform_CPU_Ability_##x))) { return false; }

        Platform_CPU_IsSSEAbove_IMPL(10, SSE)
        Platform_CPU_IsSSEAbove_IMPL(20, SSE2)
        Platform_CPU_IsSSEAbove_IMPL(30, SSE3)
        Platform_CPU_IsSSEAbove_IMPL(35, SSSE3)
        Platform_CPU_IsSSEAbove_IMPL(40, SSE41)
        Platform_CPU_IsSSEAbove_IMPL(42, SSE42)
        Platform_CPU_IsSSEAbove_IMPL(50, AVX)
        Platform_CPU_IsSSEAbove_IMPL(60, AVX2)

#undef Platform_CPU_IsSSEAbove_IMPL

        return true;
    }
}

// If a custom "promotability map" is needed, please redefine this macro before including "Platform.h"
#ifndef PXDISPATCH_DO_FOR_SSE

#if PLATFORM_INTEL_SSE >= 50
#define PXDISPATCH_DO_FOR_SSE(X, Y, ...) X(50, Y, __VA_ARGS__)
#elif PLATFORM_INTEL_SSE >= 42
#define PXDISPATCH_DO_FOR_SSE(X, Y, ...) X(50, Y, __VA_ARGS__) X(42, Y, __VA_ARGS__)
#elif PLATFORM_INTEL_SSE >= 41
#define PXDISPATCH_DO_FOR_SSE(X, Y, ...) X(50, Y, __VA_ARGS__) X(42, Y, __VA_ARGS__) X(41, Y, __VA_ARGS__)
#elif PLATFORM_INTEL_SSE >= 35
#define PXDISPATCH_DO_FOR_SSE(X, Y, ...) X(50, Y, __VA_ARGS__) X(42, Y, __VA_ARGS__) X(41, Y, __VA_ARGS__) X(35, Y, __VA_ARGS__)
#elif PLATFORM_INTEL_SSE >= 30
#define PXDISPATCH_DO_FOR_SSE(X, Y, ...) X(50, Y, __VA_ARGS__) X(42, Y, __VA_ARGS__) X(41, Y, __VA_ARGS__) X(35, Y, __VA_ARGS__) X(30, Y, __VA_ARGS__)
#elif PLATFORM_INTEL_SSE >= 20
#define PXDISPATCH_DO_FOR_SSE(X, Y, ...) X(50, Y, __VA_ARGS__) X(42, Y, __VA_ARGS__) X(41, Y, __VA_ARGS__) X(35, Y, __VA_ARGS__) X(30, Y, __VA_ARGS__) X(20, Y, __VA_ARGS__)
#else
#define PXDISPATCH_DO_FOR_SSE(X, Y, ...) X(50, Y, __VA_ARGS__) X(42, Y, __VA_ARGS__) X(41, Y, __VA_ARGS__) X(35, Y, __VA_ARGS__) X(30, Y, __VA_ARGS__) X(20, Y, __VA_ARGS__) X(10, Y, __VA_ARGS__)
#endif

#endif

// You can customize it too by redefining it before including "Platform.h"
#ifndef PXDISPATCH_IMPLEMENT
// Some macros to help implement writing SSE promotion dispatcher
#define PXDISPATCH_IMPLEMENT(LEVEL, FUNCTION,  ...) \
    else if (Platform_CPU_IsSSEAbove(LEVEL)) FUNCTION##LEVEL (__VA_ARGS__);

#endif

#define PXDISPATCH_IMPLEMENT_FOR_SSE(FUNCTION, ...) \
    if (0); PXDISPATCH_DO_FOR_SSE(PXDISPATCH_IMPLEMENT, FUNCTION, __VA_ARGS__)

#endif /* PLATFORMCPUDETECT_H_ */
