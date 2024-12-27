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


#ifndef		_OSCVERSION_H
#define		_OSCVERSION_H

#ifdef WIN32
	#ifdef _DEBUG
		#define OSC_CONFIG "Win32 Debug"
	#else
		#define OSC_CONFIG "Win32 Release"
	#endif
#else
	#ifdef _DEBUG
		#define OSC_CONFIG "x64 Debug"
	#else
		#define OSC_CONFIG "x64 Release"
	#endif
#endif

#define OSC_VERSION_HEADER "OscLib %s (Philips Reseach Medisys Paris)\n"

//#define OSC_VERSION_INFO "V.%d.%d.%d: The very first version delivered to Best (include rotation)" // 0.1.0						  	
//#define OSC_VERSION_INFO "V.%d.%d.%d: To sync with delivered OscWrapperLib"                        // 1.0.0		

//#define OSC_VERSION_INFO "V.%d.%d.%d: Multi-thread compliance + RdgLib absorbed in OscLib" // 1.0.1	
//#define OSC_VERSION_INFO "V.%d.%d.%d: User memory management (fluoro ini, handle to file), prototype needed data access, bugs correction" // 1.1.1	
//#define OSC_VERSION_INFO "V.%d.%d.%d: Checkbox to display test in windows in the Gui" // 1.1.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Shutter extraction restored, bug catheter body removal in DSA fixed" // 1.1.2
//#define OSC_VERSION_INFO "V.%d.%d.%d: User chooses nb proc, better settings for ROI detection, bugs corrections" // 1.2.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Multithread thinning confusion bug corrected" // 1.2.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Non-enough injected cycle detection" // 1.2.2
//#define OSC_VERSION_INFO "V.%d.%d.%d: ROI detection improvement: injection detection and specific twisted catheter injection tip detection" // 1.3.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Ridge background motion compensation, extra local motion exploration in fluoro" // 1.4.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Background motion compensation, diaphragm detection, clips detection" // 1.5.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Wedge detection, multithreading for fluoro MC (allowing some rotations)" // 1.6.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Wedge margin without map dilation" // 1.6.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Accelerated fluoro (mostly multicore tool extraction, no overlay computation, accelerated Gaussians) and color overlay in Console" // 1.7.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Optional guide wire extraction" // 1.8.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Access to angio vessel maps" // 1.8.2
//#define OSC_VERSION_INFO "V.%d.%d.%d: Check the shutter extraction values" // 1.8.3
//#define OSC_VERSION_INFO "V.%d.%d.%d: Wedge file bug" // 1.8.4
//#define OSC_VERSION_INFO "V.%d.%d.%d: Possibility to boost GW/catheter, new overlay, possible guidewire tip darkening in overlay; shutter position change in Gui; switch to visual 2005" // 1.9.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Shutter position change in Gui; switch to visual 2005" // 1.10.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Shutter position change in Gui; switch to visual 2005 - no hack" // 1.10.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Wedge extraction: no inversion, adapted search range" // 1.10.2
//#define OSC_VERSION_INFO "V.%d.%d.%d: Panning compensation in angio (incl. cath body deletion) and fluoro" // 1.11.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Improved angio sternal wire detection, possible fluoro sternal wire re-detection, vascular tree cleaning" // 1.12.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: No/unsufficient injection detection, different gammas in angio/fluoro, keep track cofficient in Viterbi, second type of spatial Viterbi term, Viterbi 2D1/2, Panning compensation coherent with system logs" // 1.13.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Skip frame, code reorganisation, multithread angiosequence, minor code change" // 1.14.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Skip frame bug correction (circular buffer)" // 1.14.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Skip frame bug correction (when resetting fluoro time)" // 1.14.2
//#define OSC_VERSION_INFO "V.%d.%d.%d: Correct multi-core with GW detection, Gw detection disabled by default" // 1.14.3
//#define OSC_VERSION_INFO "V.%d.%d.%d: Projection-based roadmapping" // 1.15.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Parameters reorganization, dependancy to pixel size, margin for shutter in OscLib" // 1.16.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Corrected the forced GW extraction in monothread, multithreaded background extraction" // 1.16.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Fluoro background extraction without MC, not MT anymore;" // 1.16.2
//#define OSC_VERSION_INFO "V.%d.%d.%d: Medisys overlay bug (large table panning) correction / internal bugs (backgd sub) correction/technical improvements (labelling)" // 1.16.3
//#define OSC_VERSION_INFO "V.%d.%d.%d: Final normalization correction bug corrected, angio memory management updated" // 1.16.4
//#define OSC_VERSION_INFO "V.%d.%d.%d: Overlay bug correction" // 1.16.5
//#define OSC_VERSION_INFO "V.%d.%d.%d: Inverse roadmapping, tipX integration, overlay computation externalization, bugs correction" // 2.0.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Merge OscLib and RdgLib" // 2.3.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Initial main memory assignement, new TipX lib integration" // 2.4.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: CBDLib integration, bugs correction (DiaLib, wedges), 3rd possible output (matching info)" // 2.5.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Integration of improved diaphragm detection (1 frame only), sternal wire detection with SWLib, fluoro diaphragm-catheter body-sternal wire detection better initialized from angio results" // 2.6.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Deilevry for Zwolle prototype integration. Larger kernels, no table panning correction wrt angulation" // 2.7.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Shutters allowed to move (and change between angio and fluoro).\\Motion compensation reorganized (previous table compensation cropped too much the FOV). Extended search range (discriminated in X and Y) if large table panning." // 2.7.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: No search range extension in case of table panning in X (too much confusion). Enlarged default serach range in Y (for breathing)." // 2.7.2
//#define OSC_VERSION_INFO "V.%d.%d.%d: Save and load corresponding angio ridge and masks when storing to disk. Do not delete temporary file anymore." // 2.7.3
//#define OSC_VERSION_INFO "V.%d.%d.%d: No limitation in the number of fluoro frames anymore, two little bugs corrected (cath body mask exiting when no shutter, ini of BM for background computation)." // 2.7.4
//#define OSC_VERSION_INFO "V.%d.%d.%d: No flag imposing Restore after saving the angio results, no initial memory reallocation in angio and fluoro, compilation with SSE2" // 2.7.5
//#define OSC_VERSION_INFO "V.%d.%d.%d: Bug incorrect behavior when non-default parameters solved" // 2.7.6
//#define OSC_VERSION_INFO "V.%d.%d.%d: Optimized code, bug corrected in SW detection, Viterbi different setting" // 2.8.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Write/read directional pyramids bug correction" // 2.8.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Memory leaks when writing/reading angio data (and desallocating/allocating)" // 2.8.2
//#define OSC_VERSION_INFO "V.%d.%d.%d: Error messages in case of allocation problems" // 2.8.3
//#define OSC_VERSION_INFO "V.%d.%d.%d: Creates TipX only when needed (reverse of Tip extraction)" // 2.8.4
//#define OSC_VERSION_INFO "V.%d.%d.%d: Search range adaptation when repeteadly bumps a search range border" // 2.9.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Report errors from SWLib and CBDLib. Incorrect memory allocation with some OscImageAlloc." // 2.10.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: x64 version. (+Tests on ostium)" // 2.11.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Substituted asserts with error message" // 2.11.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: OscLib recoded from scratch for product delivery" // 3.0.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Parallelization enabled in angio/fluoro matching, unused error messages removed, ability to accept any image size (to the potential cost of padding)" // 3.0.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Adaptations for padded cases in ShutterExtraction, GetRegisteredOverlay, and in case of frames skip" // 3.0.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Allocate once; save handle to memory (as opposed to disk); injection evaluation by comparison with active image surface, bug too long angio corrected" // 3.1.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Catheter injection ROI detection, complete IThreadPool separation, angio/fluoro image sizes can be different, reorganized error messages" // 3.2.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Bug fixed (limits in table panning compensation in background computation)" // 3.2.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Reduced memory load (Frg and Dir saved as UC), limited memory load when saving/loading after AngioProcessSequence, bugs fixed." // 3.3.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Bugs corrections (Vasculature cleaning passed on to Lab image, injection indicator discussion on cleaned image, warning in case of too long angio issued before time order...)." // 3.3.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Border handling (no ridge computation outside ROI), relaxed CBD condition (top and bottom border), internal variable SSFactor dependents, SSFactor 3 admissible." // 3.4.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: No C-arm CBD condition (top and bottom border). Injection condition in real world surface. Reinitializing panning shift and SW masks when processing new sequence." // 3.5.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Normalizing injection indicator. Default threshold is 0.5." // 3.5.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Integrated CBD.3.0 (CBD tracking and tube subtraction), corrected bug in cath inj contrast curve analysis, check maxAngioLengthAlloc compatibility when loading handle." // 3.6.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Bug corrected: if CBD rejected (after having been accepted), do not track and subtract it." // 3.6.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Distally boosted cycle choice, mask cleaning consolidation, parameters check when loading from memory, parameters hard-coded, catheter injection ROI not computed. SW detection disabled." // 3.7.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Bug fix in clean image: threshold badly defined before median computation kicks in." // 3.7.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Bug fix in memory load (OrigTablePosYPix)." // 3.7.2
//#define OSC_VERSION_INFO "V.%d.%d.%d: Integrating SWLib.3.0." // 3.8.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Removed some include MxLab." // 3.8.1
//#define OSC_VERSION_INFO "V.%d.%d.%d: Code CPPchecked. Removed a couple of unused functions and an unused error message." // 3.9.0
//#define OSC_VERSION_INFO "V.%d.%d.%d: Incorrect scope for dXYIni." // 3.9.1
#define OSC_VERSION_INFO "V.%d.%d.%d: Check for ROI large and wide enough; report if median with <=0 length asked; no not initialize clueter[0] if length <=0." // 3.9.2

#endif   // _OSCVERSION_H
