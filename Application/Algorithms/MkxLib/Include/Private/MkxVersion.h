// ***************************************************************************
// * Copyright (c) 2002-2016 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_MKXVERSION_H
#define		_MKXVERSION_H

#ifdef _DEBUG
#define MKX_CONFIG "Debug"
#else
#define MKX_CONFIG "Release"
#endif

#if defined  _WIN64
#define MKX_PLATFORM "x64"
#else
#define MKX_PLATFORM "Win32"
#endif


#define MKX_VERSION_HEADER "MkxLib %s %s (Medisys Philips Paris)\n"

//#define STB_VERSION_INFO "PRF StbLib: version 0.1 : First version"
//#define STB_VERSION_INFO "PRF StbLib: version 0.5 : Crude error mechanism, no verbose filter"
//#define STB_VERSION_INFO "PRF StbLib: version 0.9 : Crude error mechanism"
//#define STB_VERSION_INFO "PRF StbLib: version 0.95: With Error mechanism"
//#define STB_VERSION_INFO "PRF StbLib: version 1.00: Error mechanism + Opaqueness"
//#define STB_VERSION_INFO "PRF StbLib: version 1.10 (%s) : Diff in StbIsFatalError + Debug/Release config"
//#define STB_VERSION_INFO "PRF StbLib: version 1.20 (%s) : Longi / TransFacToOri + Roi % 4"
//#define STB_VERSION_INFO "PRF StbLib: version 2.0  (%s) : Tracker + improved blob enhancer"
//#define STB_VERSION_INFO "PRF StbLib: version 3.00 (%s) : 1st delivery for Fresco product"
//#define STB_VERSION_INFO "PRF StbLib: version 3.10 (%s) : With Ori<->Boosted image transforms"
//#define STB_VERSION_INFO "PRF StbLib: version 3.20 (%s) : With moving ROI"
//#define STB_VERSION_INFO "PRF StbLib: version 3.22 (%s) : With marker radius"
//#define STB_VERSION_INFO "PRF StbLib: version 3.22.1 (%s) : With marker radius"
//#define STB_VERSION_INFO "PRF StbLib: version 3.22.1 (%s) : With marker radius"
//#define STB_VERSION_INFO "PRF StbLib: version 3.30 (%s) : Wire based registration"
//#define MKX_VERSION_INFO "PRF MkxLib: version 4.0 beta (%s): StbLib = MkxLib + BooLib"
//#define MKX_VERSION_INFO "PRF MkxLib: version 4.0 (%s): StbLib = MkxLib + BooLib"
//#define MKX_VERSION_INFO "PRF MkxLib: version 4.10 beta (%s): With classifier mechanism"
//#define MKX_VERSION_INFO "PRF MkxLib: version 4.12 (%s)" // including specs of 24/02/2006 meeting
//#define MKX_VERSION_INFO "PRF MkxLib: version 4.13 (%s)" //with history mechanism
//#define MKX_VERSION_INFO "PRF MkxLib: version 4.14 (%s)" //modif on tracker; new adaptive roi when locked & skipped
//#define MKX_VERSION_INFO "PRF MkxLib: version 4.20 (%s)" //compatible with subtract sequences 13/07/2006
//#define MKX_VERSION_INFO "PRF MkxLib: version 4.20.1 (%s)" //27/07/2006
//#define MKX_VERSION_INFO "PRF MkxLib: version 4.20.2 (%s)" //18-09-2006 memory leak fixed
//#define MKX_VERSION_INFO "PRF MkxLib: version 4.20.2 modified for 3D (%s)" 
//#define MKX_VERSION_INFO "V.%d.%d.%d modified for 3D + ErrLib & PrmLib" //4.30.0
//#define MKX_VERSION_INFO "V.%d.%d.%d modified tracker + modified 2 rois behaviour" //4.32.0
//#define MKX_VERSION_INFO "V.%d.%d.%d modified refining" //4.33.0
//#define MKX_VERSION_INFO "V.%d.%d.%d new history" //4.34.0
//#define MKX_VERSION_INFO "V.%d.%d.%d " //4.35.0 modified polarize
//#define MKX_VERSION_INFO "V.%d.%d.%d " //4.36.0 increased speed through roi limited calculations
//#define MKX_VERSION_INFO "V.%d.%d.%d " //4.37.0 updated history parameters
//#define MKX_VERSION_INFO "V.%d.%d.%d new default para" //5.0.1 
//#define MKX_VERSION_INFO "V.%d.%d.%d  with MkxMarkerBasedWarp + MkxWireBasedWarp" //5.1.0 
//#define MKX_VERSION_INFO "V.%d.%d.%d  some bug fixed" //5.1.1 bug fixed in BxLib
//#define MKX_VERSION_INFO "V.%d.%d.%d  with MkxGetWireFromMarkers + MkxGetWireFromUser" //5.2.0 
//#define MKX_VERSION_INFO "V.%d.%d.%d  " //5.2.1 to synchronize with S3dLib 1.6.2
//#define MKX_VERSION_INFO "V.%d.%d.%d  " //5.3 to synchronize with S3dLib 2.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  " //5.3.1 little cleaning

//5.4.0  protection against memory overflow in BxLib, HistoryRoi, MarkersDist deactivation option, speed optimisation in MkxRing, 
// output merits for debug, HistoryFactor activ, HistoryVanishingFactor introduced, correlation deactivation option,
//5.5.0  speed optimisation in Tr1CalcCorrel
//5.6.0  introduction of "mm per pixel" parameter
//#define MKX_VERSION_INFO "V.%d.%d.%d  : Consolidate; Params in physical units" //6.0.0 MkxConsolidate, mmPerpixel, fps
//#define MKX_VERSION_INFO "V.%d.%d.%d  : interface modified (in CMkxAdvanced)" //6.1.0 
//#define MKX_VERSION_INFO "V.%d.%d.%d  : bug fixed for TrCorrelFlag=0" //6.1.1 
//#define MKX_VERSION_INFO "V.%d.%d.%d  : interface modified" //6.2.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : Consolidate with status update" //6.2.1
//#define MKX_VERSION_INFO "V.%d.%d.%d  : Consolidate with status update; modified interface" //6.3.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : Consolidate with status update; modified interface" //6.3.1
//#define MKX_VERSION_INFO "V.%d.%d.%d  : Min/max instead of MarkersDist/toler" //6.4.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : CMkxSeqPara, MkxGetSkipFromConfidence" //6.5.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : New SkipFromConfidence parameters" //6.6.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : new patch" //6.6.1 
//#define MKX_VERSION_INFO "V.%d.%d.%d  : historyStat instead of historyPredict for historyFlag=2" //6.7.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : WireCpl reintroduced" //6.7.1
//#define MKX_VERSION_INFO "V.%d.%d.%d  : GetSkipFromConfidence with parameters, negative wire correlation values clipped, new field StatusAfterTracking " //6.8.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : protection against particular situations (MkxGetSkipFromConfidence)" //6.8.1
//#define MKX_VERSION_INFO "V.%d.%d.%d  : some cleaning" //6.8.2
//#define MKX_VERSION_INFO "V.%d.%d.%d  : bug fixed in MkxGetSkipFromConfidence" //6.8.3
//#define MKX_VERSION_INFO "V.%d.%d.%d  : cplIntensityWrtWire improved" //6.8.4 penalize modified, one call to MkxMarkerOrderByIntensity suppressed, MkxMarkerCoupleOrderByIntensity fixed, some cleaning
//#define MKX_VERSION_INFO "V.%d.%d.%d  : better processing of special cases in marker refinement" //6.8.5 MkxMarkerDirection: better processing of special cases
//#define MKX_VERSION_INFO "V.%d.%d.%d  : last version before freezing study on 17-11-2011" //6.8.6 some changes in MkxAdapt : MKX_ADAPTIVEWCPLSELECTFLAG_I disabled
//#define MKX_VERSION_INFO "V.%d.%d.%d  : Optimized version" //6.8.100
//#define MKX_VERSION_INFO "V.%d.%d.%d  : Dark pixels penalize" //6.8.101
//#define MKX_VERSION_INFO "V.%d.%d.%d  : few warnings removed + bug in MkxRdg fixed" //6.8.102
//#define MKX_VERSION_INFO "V.%d.%d.%d  : StentBoost+ (synchronised with BooLib.7.0.0)" //7.0.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : memory leaks removed" //7.0.1
//#define MKX_VERSION_INFO "V.%d.%d.%d  : double pointers removed" //8.0.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : refactoring in progress, client functions related to wire removed" //8.1.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : MkxSwapMarkers corrected" //8.1.1
//#define MKX_VERSION_INFO "V.%d.%d.%d  : sternal wire masking " //8.2.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : sternal wire masking with threadpool" //8.3.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : new parameter: max number of cores for SWLib" //8.3.1
//#define MKX_VERSION_INFO "V.%d.%d.%d  : ThreadPool outside MkxLib" //8.3.2
//#define MKX_VERSION_INFO "V.%d.%d.%d  : new args in MkxCreate, process image size in ExtractResults, new version SWLib.3.1.0" //8.4.0

//#define MKX_VERSION_INFO "V.%d.%d.%d  : output Sternal Wire status in ExtractResults + SWLib.3.1.2" //8.5.0
//#define MKX_VERSION_INFO "V.%d.%d.%d  : version CPPChecked" //8.6.0
#define MKX_VERSION_INFO "V.%d.%d.%d  : independant subsampling factors for SWLib and for extraction process" //8.7.0


#endif   // _MKXVERSION_H
