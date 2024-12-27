// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


// Documents the parameter context: 
// Aka:       As-known-as: a textual identifier
// Txt:       A small descriptive text about the parameter meaning
// Tag:       Critical: the parameter cannot changed after t=0, Normal: changed whenever
// Min / Max: Min and Max values for this parameter (validity range)
// Acc:       Recommended accuracy (for instance to program a slider)
// Def:       Default value used by the library if not otherwise specified by a SetPara call
	

	k = MKX_DEFROIFLAG_I ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "DefRoiFlag");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Region of Interest flag (Default ROI)");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  MKX_CST_EXTRACT_ROI_NODEF);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  MKX_CST_EXTRACT_ROI_DEF);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  MKX_CST_EXTRACT_ROI_NODEF);
											  
	k =   MKX_INITROIFLAG_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "InitRoiFlag");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Region of Interest flag (Init ROI)");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  MKX_CST_EXTRACT_ROI_NODEF);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  MKX_CST_EXTRACT_ROI_DEF);
	if (!E) E = PrmSetCtxInt(P, k, PRM_CST_ACC, 1);// , 0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  MKX_CST_EXTRACT_ROI_NODEF);
											  
	k =   MKX_BLOBENHSUBSAMPLINGMODE_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "BlobEnhSubsamplingMode");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Blob enhancement SubsamplingMode (0: auto, 1: 1, 2: 2, 3: 4) ");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_CRITICAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  MKX_CST_SUBSAMPLING_AUTO);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  MKX_CST_SUBSAMPLING_4);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if (!E) E = PrmSetCtxInt(P, k, PRM_CST_DEF, MKX_CST_SUBSAMPLING_AUTO);// MKX_CST_SUBSAMPLING_2);
											  
	k =   MKX_BLOBENHRINGRADIUS_F ;    
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "BlobEnhRingRadius");           
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Radius in mm for blob ring enhancement filter (scaling will be applied for subsampling)");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  0.0f);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  10.0f);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  0.1f);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_DEF,  1.0f);
											  
	k =   MKX_BLOBENHRINGSIG_F ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "BlobEnhRingSig");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Blob ring filter : Sigma of the Gaussian filter (in ratio of the radius) (scaling will be applied for subsampling)");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  0.0f);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  1.0f);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  0.05f);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_DEF,  0.2f);
											  
	k =   MKX_BLOBENHRINGNBDIR_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "BlobEnhRingNbDir");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Nb of directions for blob ring enhancement filter");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  2);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  36);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  16);
											  
	k =  MKX_BLOBENHINTENSITYFACTOR_F ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "BlobEnhIntensityFactor");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Intensity factor for point enhancement (0: no infuence)");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  (float)0.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  (float)10.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  (float)0.1);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_DEF,  (float)0.);
											  
	k =   MKX_CPLSELECTNBLABELSKEPT_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "CplSelectNbLabelsKept");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Number of labels to be kept");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  100);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  15);
											  
	k =   MKX_CPLSELECTSWLibFLAG_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "CplSelectSWLibFlag");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Sternal wires removal (0: not-active, 1: active at start)");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_CRITICAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  1);
											  							  
  k =   MKX_SWSUBSAMPLINGMODE_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "SWSubsamplingMode");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Sternal wire removal SubsamplingMode (0: auto, 1: 1, 2: 2, 3: 4) ");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_CRITICAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  MKX_CST_SUBSAMPLING_AUTO);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  MKX_CST_SUBSAMPLING_4);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if (!E) E = PrmSetCtxInt(P, k, PRM_CST_DEF,  MKX_CST_SUBSAMPLING_AUTO);//2);

	k =   MKX_CPLSELECTWIREFLAG_I ;    
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "CplSelectWireFlag");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Wire-based couple selection flag (0: none, 1: extra wire-based measures");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_CRITICAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  1);
											  
	k =   MKX_CPLSELECTMAXNBWIRES_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "CplSelectMaxNbWires");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Maximum number of wire-tested marker couples");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  100);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  15);
											  
	k =   MKX_TRACKONFLAG_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrackOnFlag");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Tracking flag (0: no tracking, 1: tracking is used)");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  1);
											  
	k =   MKX_TRMAXNBCOUPLE_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrMaxNbCouple");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Maximum number of tracked marker couples");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  30);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  5);
											  
	k =  MKX_TRINITTIME_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrInitTime");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Tracker init time to get trustworthy results");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  20);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  4);
											  
	k =  MKX_TRVIRTUALPENALIZE_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrVirtualPenalize");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Penalization (in %) for consecutive virtual tracks (cumulative)");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  20);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  1);
											  
	k =   MKX_TRLOCKCOUNT_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrLockCount");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "For locking: Nb of consecutive times when best alarm is tracked by same track");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  20);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  5);
											  
	k =   MKX_TRLOCKMAXCONSKIP_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrLockMaxConSkip");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "For locking: maximum number of consecutive skips within the locking period");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  10);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  4);
											  
	k =  MKX_TRALPHA_F ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrAlpha");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Tracker integration coefficient");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  (float)0.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  (float)1.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  (float)0.1);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_DEF,  (float)0.9);
											
//default values for TrxxHigh and TrxxSkip have been obtained from statistics on groundtruth
//for playlists Direct_not_challenging and Contrast_not_challenging on 26/07/2011
//by the following rules:
// TrxxHigh = 0.75 * Q99
// TrxxSkip = 1.0 * Q99
// where Q99 is the value of the quantile for 99% of the values of the histogram

	k =  MKX_TRANGLEHIGH_F ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrAngleHigh");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Value in degree per second above which angle variation is considered high");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  (float)0.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  (float)500.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  (float)5.);
	if (!E) E = PrmSetCtxFlt(P, k, PRM_CST_DEF, (float)80);// = ~0.75 * 110 // , (float)180);
											  
	k =  MKX_TRANGLESKIP_F ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrAngleSkip");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Value in degree per second above which angle variation is considered too high");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  (float)0.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  (float)500.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  (float)5.);
	if (!E) E = PrmSetCtxFlt(P, k, PRM_CST_DEF, (float)110); // = ~1.0 * 110,  (float)240);
											  
	k =  MKX_TRBREADTHHIGH_F ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrBreadthHigh");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Value above which breadth relative variation  (in %/sec) is considered high");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  (float)0.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  (float)1000.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  (float)10.);
	if (!E) E = PrmSetCtxFlt(P, k, PRM_CST_DEF, (float)110);// = ~0.75 * 150 //, (float)240);
											  
	k =   MKX_TRBREADTHSKIP_F ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrBreadthSkip");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Value above which breadth relative variation (in %/sec)is considered too high");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  (float)0.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  (float)1000.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  (float)10.);
	if (!E) E = PrmSetCtxFlt(P, k, PRM_CST_DEF, (float)150);// = ~0.1 * 150 // , (float)321);
											  
	k =  MKX_TRCENTROIDHIGH_F ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrCentroidHigh");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Value above which centroid translation (in mm per second) is considered high");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  (float)0.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  (float)500.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  (float)5.);
	if (!E) E = PrmSetCtxFlt(P, k, PRM_CST_DEF, (float)60);// = ~0.75 * 80 //, (float)118);
											  
	k =  MKX_TRCENTROIDSKIP_F ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrCentroidSkip");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Value above which centroid translation (in mm per second) is considered too high");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  (float)0.);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  (float)500.); 
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  (float)5.);
	if (!E) E = PrmSetCtxFlt(P, k, PRM_CST_DEF, (float)80); // = ~0.1 * 80 // , (float)157);
											  
	k =   MKX_TRSTRENGTHIMPACT_F ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "TrStrengthImpact");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Fraction of couple strength (vs predictability) for track evaluation");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  0.f);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  1.f);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  0.05f);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_DEF,  0.5f);
											  
	k =   MKX_ADAPTIVEROIFLAG_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "AdaptiveRoiFlag");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Adaptive Region of Interest flag (0: no adapt, 1: adapt with one roi, 2: adapt with 2 rois) ");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  MKX_CST_NOADAPT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  MKX_CST_ADAPT_2ROIS);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  MKX_CST_ADAPT_2ROIS);
											  
	k =   MKX_ADAPTIVEROIFACTOR_F ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "AdaptiveRoiFactor");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Adaptivity Factor for Adaptive ROI sizing: 0 minimum, 1 maximum");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  (float)0);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  (float)1);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  (float)0.1);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_DEF,  (float)0.7);
											  
	k =   MKX_ADAPTIVENBLABELSKEPTFLAG_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "AdaptiveNbLabelsKeptFlag");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Adaptive Nb of Labels kept flag");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  MKX_CST_NOADAPT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  MKX_CST_ADAPT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  MKX_CST_ADAPT);
											  
	k =   MKX_ADAPTIVEMARKERSDISTFLAG_I ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "AdaptiveMarkersDistFlag");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Adaptive MarkersDist flag");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  MKX_CST_NOADAPT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  MKX_CST_ADAPT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  MKX_CST_ADAPT);
											  
	k =  MKX_ADAPTIVEWCPLSELECTFLAG_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "AdaptiveWCplSelectFlag");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Adaptive max Nb cpl + adaptive pruning in wire-based selection (0: not adaptive, 1: adaptive)");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  MKX_CST_NOADAPT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  MKX_CST_ADAPT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  MKX_CST_ADAPT);

								  
	k =  MKX_WIREFLAG_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "WireFlag");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Wire flag (0: not extracted, 1: extracted, 2: extracted with possible downsampling)");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  2);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  0);
											  
	k = MKX_HISTORYFLAG_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "HistoryFlag");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "History mode (0: not used, 1: update,2: apply but no update, 3: apply and update) ");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  MKX_CST_HISTORY_NOTUSED);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  MKX_CST_HISTORY_APPLYANDUPDATE);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  MKX_CST_HISTORY_UPDATEONLY);
											  
	k =   MKX_HISTORYDEPTH_I ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "HistoryDepth");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "History depth: number of consecutive past images in which to look for building the history filter ");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  100);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  20);
											  
	k =  MKX_HISTORYFACTOR_F ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "HistoryFactor");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Modulation factor for applying the history filter (0: loose filtering, 1: tight filtering)");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  (float)0);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  (float)1);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  (float)0.05);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_DEF,  (float)0.5);
											  
	k =  MKX_HISTORYVANISHINGFACTOR_F ;
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "HistoryVanishingFactor");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "Modulation factor when skipping (0: no vanishing, 1: get to 0 after a time equal to HistoryDepth)");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  (float)0);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  (float)1);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  (float)0.05);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_DEF,  (float)0.5);
											  
	k = MKX_HistoryCentroidXLow_I ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "HistoryCentroidXLow");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "(only for HistoryFlag=3) Value below which distance between observed and predicted from history centroid X coordinate (in mm) is considered low");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  130);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  5);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  0);
											  
	k = MKX_HistoryCentroidXHigh_I ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "HistoryCentroidXHigh");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "(only for HistoryFlag=3) Value above which distance between observed and predicted from history centroid X coordinate (in mm) is considered high");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  130);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  5);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  10);
											  
	k = MKX_HistoryCentroidYLow_I ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "HistoryCentroidYLow");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "(only for HistoryFlag=3) Value below which distance between observed and predicted from history centroid Y coordinate (in mm) is considered low");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  130);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  5);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  0);
											  
	k = MKX_HistoryCentroidYHigh_I ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "HistoryCentroidYHigh");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "(only for HistoryFlag=3) Value above which distance between observed and predicted from history centroid Y coordinate (in mm) is considered high");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  130);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  5);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  10);
											  
	k = MKX_HistoryBreadthLow_F ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "HistoryBreadthLow");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "(only for HistoryFlag=3) Value below which difference between observed and predicted from history breadth (in % of  breadth) is considered low");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  100);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  5);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_DEF,  20);
											  
	k = MKX_HistoryBreadthHigh_F ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "HistoryBreadthHigh");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "(only for HistoryFlag=3) Value above which difference between observed and predicted from history breadth (in % of  breadth) is considered high");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  100);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  5);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_DEF,  100);
											  
	k = MKX_HistoryAngleLow_F ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "HistoryAngleLow");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "(only for HistoryFlag=3) Value below which difference between observed and predicted from history angle (in degree) is considered low");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  100);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  5);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_DEF,  20);
											  
	k = MKX_HistoryAngleHigh_F ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "HistoryAngleHigh");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "(only for HistoryFlag=3) Value above which difference between observed and predicted from history angle (in degree) is considered high");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_NORMAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_FLOAT);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_MAX,  100);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_ACC,  5);
	if(!E) E=PrmSetCtxFlt(P, k, PRM_CST_DEF,  60);	

	k = MKX_PARA_DarkPixelsPenalize_I ;  
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_AKA,  "DarkPixelsPenalize");
	if(!E) E=PrmSetCtxStr(P, k, PRM_CST_TXT,  "0: no, 1: yes");
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TAG,  MKX_CST_PARA_CRITICAL);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_TYP,  PRM_CST_INT);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MIN,  0);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_MAX,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_ACC,  1);
	if(!E) E=PrmSetCtxInt(P, k, PRM_CST_DEF,  1);

										  