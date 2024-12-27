// **********************************************************************
// *  Copyright (c) 2011 Medisys Research Laboratory, Philips France.   * 
// *                                                                    *
// * All rights reserved. Reproduction or disclosure to third parties   * 
// * in whole or in part is prohibited without the prior written        * 
// * consent of the copyright owner.                                    * 
// *                                                                    *
// **********************************************************************


#ifndef		_MKXPARACODES_H
#define		_MKXPARACODES_H

typedef enum  {

	MKX_DEFROIFLAG_I=0,                     
	MKX_INITROIFLAG_I,                 
	MKX_BLOBENHSUBSAMPLINGMODE_I,             
	MKX_BLOBENHRINGRADIUS_F,                      
	MKX_BLOBENHRINGSIG_F,                  
	MKX_BLOBENHRINGNBDIR_I, 
	MKX_BLOBENHINTENSITYFACTOR_F,             
	MKX_CPLSELECTNBLABELSKEPT_I,                
	MKX_CPLSELECTSWLibFLAG_I, 
    MKX_SWSUBSAMPLINGMODE_I,
	MKX_CPLSELECTWIREFLAG_I,    
	MKX_CPLSELECTMAXNBWIRES_I,
	MKX_TRACKONFLAG_I,                 
	MKX_TRMAXNBCOUPLE_I,               
	MKX_TRINITTIME_I,                  
	MKX_TRVIRTUALPENALIZE_I,           
	MKX_TRLOCKCOUNT_I,                 
	MKX_TRLOCKMAXCONSKIP_I,            
	MKX_TRALPHA_F,                     
	MKX_TRANGLEHIGH_F,                 
	MKX_TRANGLESKIP_F,                 
	MKX_TRBREADTHHIGH_F,               
	MKX_TRBREADTHSKIP_F,               
	MKX_TRCENTROIDHIGH_F,              
	MKX_TRCENTROIDSKIP_F,              
	MKX_TRSTRENGTHIMPACT_F,    
	MKX_ADAPTIVEROIFLAG_I,             
	MKX_ADAPTIVEROIFACTOR_F,              
	MKX_ADAPTIVENBLABELSKEPTFLAG_I,    
	MKX_ADAPTIVEMARKERSDISTFLAG_I,  
	MKX_ADAPTIVEWCPLSELECTFLAG_I,      
	MKX_WIREFLAG_I,                    
	MKX_HISTORYFLAG_I,                    
	MKX_HISTORYDEPTH_I,                    
	MKX_HISTORYFACTOR_F,
	MKX_HISTORYVANISHINGFACTOR_F,
	MKX_HistoryCentroidXLow_I ,  
	MKX_HistoryCentroidXHigh_I ,  
	MKX_HistoryCentroidYLow_I , 
	MKX_HistoryCentroidYHigh_I ,  
	MKX_HistoryBreadthLow_F ,  
	MKX_HistoryBreadthHigh_F ,  
	MKX_HistoryAngleLow_F ,  
	MKX_HistoryAngleHigh_F , 
	MKX_PARA_DarkPixelsPenalize_I,

	MKX_NBPARA                         //Last of enum

} CMkxExtractParaCodes;

#endif   //_MKXPARACODES_H

