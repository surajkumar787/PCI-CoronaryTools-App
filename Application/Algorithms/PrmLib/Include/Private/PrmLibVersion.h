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
#ifndef _PRMVERSION_H
#define _PRMVERSION_H

#ifdef  _DEBUG
#define PRM_CONFIG "Debug"
#else
#define PRM_CONFIG "Release"
#endif

#ifdef  WIN32
#define PRM_PLATFORM "Win32"
#elif defined _X64
#define PRM_PLATFORM "x64"
#else
#define PRM_PLATFORM "Any Cpu"
#endif

#define PRM_VERSION_HEADER "PrmLib %s %s (Medisys Philips Paris)\n"

//#define PRM_VERSION_INFO "V.1.0.0: First-version"
//#define PRM_VERSION_INFO "V.1.1.0: First-version + some interface changing + error-code-changes"  // 1.1.0
//#define PRM_VERSION_INFO "V.%d.%d.%d: Version-number-in-header + tag-field"	                    // 1.2.0
//#define PRM_VERSION_INFO "V.%d.%d.%d: Memory leak solved"	                                        // 1.2.1
//#define PRM_VERSION_INFO "V.%d.%d.%d: Deprecated calls removed"	                                // 1.2.2
//#define PRM_VERSION_INFO "V.%d.%d.%d: Several instances can share same Err-Handle"	                // 1.3.0
//#define PRM_VERSION_INFO "V.%d.%d.%d: some cleaning"	                // 1.3.1

#define PRM_VERSION_INFO "V.%d.%d.%d: 64 bits compatible"	  // 1.4.0

#endif   // _PRMVERSION_H
