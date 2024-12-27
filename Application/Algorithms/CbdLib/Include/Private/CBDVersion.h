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

#ifndef		_CBDVERSION_H
#define		_CBDVERSION_H

#ifdef WIN32
	#ifdef _DEBUG
		#define CBD_CONFIG "Win32 Debug"
	#else
		#define CBD_CONFIG "Win32 Release"
	#endif
#else
	#ifdef _DEBUG
		#define CBD_CONFIG "x64 Debug"
	#else
		#define CBD_CONFIG "x64 Release"
	#endif
#endif

#define CBD_VERSION_HEADER "SWLib %s (Philips Reseach Medisys Paris)\n"

//#define CBD_VERSION_INFO "V.%d.%d.%d: First deployed version" // 1.0.0
//#define CBD_VERSION_INFO "V.%d.%d.%d: Once allocation (worst case) possible" // 2.1.0
//#define CBD_VERSION_INFO "V.%d.%d.%d: Bug fixed (pointer not set to NULL)" // 2.1.1
//#define CBD_VERSION_INFO "V.%d.%d.%d: Bug fixed (pointer not set to NULL)" // 2.2.0
//#define CBD_VERSION_INFO "V.%d.%d.%d: Tube estimation and subtraction, CBD temporal tracking, polynomial fit" // 3.0.0
//#define CBD_VERSION_INFO "V.%d.%d.%d: Bug corrected in the computation of the Tukey parameter in profile background estimation, in the case where samples are missing" // 3.0.1
#define CBD_VERSION_INFO "V.%d.%d.%d: Reorganized the header file." // 3.0.2

#endif   // _CBDVERSION_H