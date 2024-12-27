// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


//
// Version history
//
#ifndef _ERRERSION_H
#define _ERRVERSION_H

#ifdef  _DEBUG
#define ERR_CONFIG "Debug"
#else
#define ERR_CONFIG "Release"
#endif

#ifdef  WIN32
#define ERR_PLATFORM "Win32"
#elif defined _X64
#define ERR_PLATFORM "x64"
#else
#define ERR_PLATFORM "Any Cpu"
#endif

#define ERR_VERSION_HEADER "ErrLib %s %s (Medisys Philips Paris)\n"

//#define ERR_VERSION_INFO "V.%d.%d.%d: No-interrupt, Fatal-error-indicator, ErrAdd-clash-check, ..."		// 1.1.0
//#define ERR_VERSION_INFO "V.%d.%d.%d: With version numbers in main header"		                        // 1.2.0
//#define ERR_VERSION_INFO "V.%d.%d.%d: Memory leak solved"		                                          // 1.2.1
//#define ERR_VERSION_INFO "V.%d.%d.%d: Deprecated calls removed"		                                    // 1.2.2
//#define ERR_VERSION_INFO "V.%d.%d.%d: some cleaning"		                                              // 1.2.3

#define ERR_VERSION_INFO "V.%d.%d.%d: 64 bits compatible"		// 1.3.0

#endif   // _ERRVERSION_H
