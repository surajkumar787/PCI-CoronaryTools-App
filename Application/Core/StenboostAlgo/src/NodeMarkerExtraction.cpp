// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.


#include <NodeMarkerExtraction.h>
#include <Mkx.h>
#include <cassert>
#include <algorithm>
#include "StentMarkers.h"
#include <iostream>

#undef min
#undef max

namespace Pci {	namespace StentBoostAlgo {
using namespace Simplicity;

static Region getRoi(const Metadata &meta, const NodeMarkerExtraction::Parameters &params, const Region &roi, const Image<short> &input)
{
	Region rect;
	
	//get region from shutters and validrect
	rect.top    = std::max(meta.shutters.top,	 meta.validrect.top);
	rect.left   = std::max(meta.shutters.left,	 meta.validrect.left);
	rect.bottom = std::min(meta.shutters.bottom, meta.validrect.bottom);
	rect.right  = std::min(meta.shutters.right,	 meta.validrect.right);

	//add custom roi to it...
	if (roi.right > roi.left && roi.bottom > roi.top)
	{
		rect.left	= std::max(rect.left,	roi.left);
		rect.top	= std::max(rect.top,	roi.top);
		rect.right	= std::min(rect.right,	roi.right);
		rect.bottom = std::min(rect.bottom,	roi.bottom);
	}
		
	rect.left	= std::min(std::max(rect.left,	 0), input.width);
	rect.top	= std::min(std::max(rect.top,	 0), input.height);
	rect.right	= std::min(std::max(rect.right,	 0), input.width);
	rect.bottom = std::min(std::max(rect.bottom, 0), input.height);

	if (rect.right-rect.left < 64 || rect.bottom-rect.top < 64)
	{
		rect.left   = 0;
		rect.top    = 0;
		rect.right  = input.width;
		rect.bottom = input.height;
	}

	return rect;
}

class StbThreadPool : public IThreadPool
{
public:
	
	StbThreadPool(Simplicity::ThreadPool &pool) : pool(pool) {}

	void execute(const std::function<void(int, int)> &function) override
	{
		pool.execute([&](Simplicity::Thread &thread)
		{
			function(thread.getIndex(),  thread.getCount());
		});
	}

	Simplicity::ThreadPool &pool;	

	StbThreadPool(const StbThreadPool&) = delete;
	StbThreadPool& operator=(const StbThreadPool&) = delete;
};

// This struct defines marker extraction parameters that depend on the type of stent/procedure.
struct MarkerInfo
{
	NodeMarkerExtraction::StentType type;
	const char*	name;
	bool		cardiac_movement;
	int			number_of_tries;
	float		minimal_length_mm_try1;
	float		maximum_length_mm_try1;
	float		radius_mm_try1;
	float		minimal_length_mm_try2;
	float		maximum_length_mm_try2;
	float		radius_mm_try2;
	float		tracker_angle_high_level;
	float		tracker_angle_skip_level;
	float		tracker_breadth_high_level;
	float		tracker_breadth_skip_level;
	float		tracker_centroid_high_level;
	float		tracker_centroid_skip_level;
};


	
const MarkerInfo markerInfo[] = 
{	
	{ NodeMarkerExtraction::StentType::cardiac	 ,	"Cardiac",		true,	1,	  5,  40, 1.0f,	0,   0,  0,		80, 110,	110,150, 60, 80 },
	{ NodeMarkerExtraction::StentType::head		 ,	"Head",			false,	1,	  5,  60, 1.0f,	0,   0,  0,		23, 30,		37,  49, 12, 16 },
	{ NodeMarkerExtraction::StentType::abdomen	 ,	"Abdomen",		false,	1,	  5,  85, 1.0f,	0,   0,  0,	    16, 21,		31,  41, 11, 14 },
	{ NodeMarkerExtraction::StentType::thorax	 ,	"Thorax",		false,	1,	 10,  40, 1.0f,	0,   0,  0,		29, 38,		39,  51, 16, 21 },
	{ NodeMarkerExtraction::StentType::peripheral,	"Peripheral",	false,	1,	  5, 150, 1.4f,	0,   0,  0,		 8, 12,		18,  24,  7,  9 },
	{ NodeMarkerExtraction::StentType::cardiac2  ,	"Cardiac2",		true,	1,	  5,  40, 1.0f,	0,   0,  0,	   180, 240,   240, 321,118, 157},
	{ NodeMarkerExtraction::StentType::cardiac3  ,	"Cardiac3",		true,	1,	  5,  40, 1.0f,	0,   0,  0,	    80, 140, 80, 150, 60, 118 },
	
};																												    
																												    
NodeMarkerExtraction::Parameters::Parameters()																	    
{																												    
	add("markersStentType", reinterpret_cast<int&>(stentType),		0, -1, 6);	
	add("lostMarkerWarningThreshold", lostMarkerWarningThreshold, 6, 1, 100);
	add("maxLostMarkerCount", maxLostMarkerCount,		6, 1, 100);	
	
	add("cardiac_movement",				cardiac_movement,               true);		
	add("minimal_length_mm_try1",		minimal_length_mm_try1	,		 5.0f,	0.0f, 200.0f);	
	add("maximum_length_mm_try1",		maximum_length_mm_try1	,		40.0f,	0.0f, 200.0f);	
	add("radius_mm_try1",				radius_mm_try1,					1.0f,	1.0f,   5.0f);	
	add("tracker_angle_high_level",		tracker_angle_high_level	,	180.0f, 0.0f, 500.0f);	
	add("tracker_angle_skip_level",		tracker_angle_skip_level	,	240.0f, 0.0f, 500.0f);	
	add("tracker_breadth_high_level",	tracker_breadth_high_level	,	240.0f, 0.0f,1000.0f);	
	add("tracker_breadth_skip_level",	tracker_breadth_skip_level	,	321.0f, 0.0f,1000.0f);	
	add("tracker_centroid_high_level",	tracker_centroid_high_level	,	118.0f, 0.0f, 500.0f);	
	add("tracker_centroid_skip_level",	tracker_centroid_skip_level	,	157.0f, 0.0f, 500.0f);	
	reset();																									

}

NodeMarkerExtraction::NodeMarkerExtraction(ThreadPool &threadPool)
:	
	_threadPool			(std::unique_ptr<StbThreadPool>(new StbThreadPool(threadPool))),
	_mkxImage			(nullptr),
	_width				(0),
	_height				(0),
	_frameIndex			(0),
	_mkxHandle			(new CMkxHdl()),
	_mkxParams			(new CPrmPara()),
	_mkxContext			(new CPrmParaCtx()),
	_reset				(true)
{
}

NodeMarkerExtraction::~NodeMarkerExtraction()
{			
	exit();
}

void NodeMarkerExtraction::reset()
{
	_frameIndex = 0;
}

static const int ImageDymanicRange = 14;

bool NodeMarkerExtraction::init(const Parameters &params, const Image<short> &input)
{
	_frameIndex = 0;

	if ((input.width != _width) || (input.height != _height) || (_mkxHandle->Pv == nullptr))
	{
		exit();
	
		_width	= input.width;
		_height	= input.height;

		if (!check(MkxCreate(_mkxHandle.get(), ImageDymanicRange, _width, _height,_threadPool.get()), "Error creating Mkx library")) return false;
		_mkxImage = new short[_width*_height];
	}
		
	if (!(PrmGetDefaultPara(_mkxHandle->PrmHdl, _mkxParams.get()),  "Could not get parameters"))					return false;
	if (!(PrmGetParaContext(_mkxHandle->PrmHdl, _mkxContext.get()), "Parameter context could not be extracted"))	return false;
	
	//if the user has overriden the parameters, don't apply them...
	MarkerInfo info;
	if(params.stentType == StentType::manual)
	{
		info = markerInfo[static_cast<int>(StentType::cardiac)]; // take remaining defaults from first entry
		info.cardiac_movement				=	params.cardiac_movement;
		info.number_of_tries				=	1;
		info.minimal_length_mm_try1			=	params.minimal_length_mm_try1;
		info.maximum_length_mm_try1			=	params.maximum_length_mm_try1;
		info.radius_mm_try1					=	params.radius_mm_try1;
		info.tracker_angle_high_level		=	params.tracker_angle_high_level;
		info.tracker_angle_skip_level		=	params.tracker_angle_skip_level;
		info.tracker_breadth_high_level		=	params.tracker_breadth_high_level;
		info.tracker_breadth_skip_level		=	params.tracker_breadth_skip_level;
		info.tracker_centroid_high_level	=	params.tracker_centroid_high_level;
		info.tracker_centroid_skip_level	=	params.tracker_centroid_skip_level;
	}
	else
	{
		info = markerInfo[static_cast<int>(params.stentType)];
	}

	float marker_radius	= std::min(info.radius_mm_try1,	    _mkxContext->Ctx[MKX_BLOBENHRINGRADIUS_F].Max.Float);
	marker_radius		= std::max(marker_radius, 		    _mkxContext->Ctx[MKX_BLOBENHRINGRADIUS_F].Min.Float);
	marker_radius		= std::max(0.0f,					marker_radius);

	_mkxParams->Val[MKX_DEFROIFLAG_I].Int					= MKX_CST_EXTRACT_ROI_DEF;			// enable the use of a default roi (coordinates are sequence parameters)
	_mkxParams->Val[MKX_INITROIFLAG_I].Int				= MKX_CST_EXTRACT_ROI_NODEF;		// disabled
	_mkxParams->Val[MKX_BLOBENHRINGNBDIR_I].Int			= 16;    	
	_mkxParams->Val[MKX_CPLSELECTMAXNBWIRES_I].Int		= 15;
	_mkxParams->Val[MKX_TRACKONFLAG_I].Int				= 1;								// enabled
	_mkxParams->Val[MKX_TRMAXNBCOUPLE_I].Int				= 5;
	_mkxParams->Val[MKX_TRINITTIME_I].Int					= 4;								// allow 4 frames to find track of markers
	_mkxParams->Val[MKX_TRVIRTUALPENALIZE_I].Int			= 1;
	_mkxParams->Val[MKX_TRLOCKCOUNT_I].Int				= 5;
	_mkxParams->Val[MKX_TRLOCKMAXCONSKIP_I].Int			= 4;	
	_mkxParams->Val[MKX_TRALPHA_F].Float					= 0.9f;
	_mkxParams->Val[MKX_TRANGLEHIGH_F].Float				= info.tracker_angle_high_level;	// degree per second		
	_mkxParams->Val[MKX_TRANGLESKIP_F].Float				= info.tracker_angle_skip_level;	// degree per second
	_mkxParams->Val[MKX_TRBREADTHHIGH_F].Float			= info.tracker_breadth_high_level;	// % length difference/sec
	_mkxParams->Val[MKX_TRBREADTHSKIP_F].Float			= info.tracker_breadth_skip_level;	// % length difference/sec
	_mkxParams->Val[MKX_TRCENTROIDHIGH_F].Float			= info.tracker_centroid_high_level;	// mm per second
	_mkxParams->Val[MKX_TRCENTROIDSKIP_F].Float			= info.tracker_centroid_skip_level;	// mm per second
	_mkxParams->Val[MKX_TRSTRENGTHIMPACT_F].Float			= 0.5;								// who do we trust more? tracker of blob enhancer?  
	_mkxParams->Val[MKX_ADAPTIVEROIFLAG_I].Int			= MKX_CST_ADAPT_2ROIS;				// a roi for each marker            
	_mkxParams->Val[MKX_ADAPTIVEROIFACTOR_F].Float		= 0.7f;
	_mkxParams->Val[MKX_ADAPTIVENBLABELSKEPTFLAG_I].Int	= MKX_CST_ADAPT;
	_mkxParams->Val[MKX_ADAPTIVEMARKERSDISTFLAG_I].Int	= MKX_CST_ADAPT;
	_mkxParams->Val[MKX_WIREFLAG_I].Int					= 0;             
	_mkxParams->Val[MKX_HISTORYFLAG_I].Int				= MKX_CST_HISTORY_UPDATEONLY;		// only learn, do not apply by default
	_mkxParams->Val[MKX_HISTORYDEPTH_I].Int				= 20;
	_mkxParams->Val[MKX_HISTORYFACTOR_F].Float			= 0.5;     
	_mkxParams->Val[MKX_HISTORYVANISHINGFACTOR_F].Float	= 0.5;                    
	_mkxParams->Val[MKX_HistoryCentroidXLow_I].Int		= 0;
	_mkxParams->Val[MKX_HistoryCentroidXHigh_I].Int		= 10;
	_mkxParams->Val[MKX_HistoryCentroidYLow_I].Int		= 0;
	_mkxParams->Val[MKX_HistoryCentroidYHigh_I].Int		= 10;
	_mkxParams->Val[MKX_HistoryBreadthLow_F].Float		= 20.0;
	_mkxParams->Val[MKX_HistoryBreadthHigh_F].Float		= 100.0;
	_mkxParams->Val[MKX_HistoryAngleLow_F].Float			= 20.0;
	_mkxParams->Val[MKX_HistoryAngleHigh_F].Float			= 60.0;					
	_mkxParams->Val[MKX_BLOBENHRINGRADIUS_F].Float		= marker_radius; 
	_mkxParams->Val[MKX_CPLSELECTSWLibFLAG_I].Int			= 1;
	_mkxParams->Val[MKX_PARA_DarkPixelsPenalize_I].Int	= 0;

	// for vascular stent, set different parameters
	if (info.type == NodeMarkerExtraction::StentType::cardiac || int(info.type) >=5 || info.type ==NodeMarkerExtraction::StentType::manual)
	{
		// Cardio options (defaults from MkxLib)
		_mkxParams->Val[MKX_BLOBENHSUBSAMPLINGMODE_I].Int		= MKX_CST_SUBSAMPLING_NONE;		
		_mkxParams->Val[MKX_BLOBENHRINGSIG_F].Float			= 0.2f;  
		_mkxParams->Val[MKX_BLOBENHINTENSITYFACTOR_F].Float	= 0.0;							// amplification of contrast around pixels      
		_mkxParams->Val[MKX_CPLSELECTNBLABELSKEPT_I].Int		= 15;
		_mkxParams->Val[MKX_CPLSELECTWIREFLAG_I].Int			= 1;
		_mkxParams->Val[MKX_SWSUBSAMPLINGMODE_I].Int			= MKX_CST_SUBSAMPLING_2;
	}
	else
	{
		// 6.0.3 Go back to fixed size sigma for vascular subtract to get prototype performance with MkxLib 6.8.3
		_mkxParams->Val[MKX_BLOBENHSUBSAMPLINGMODE_I].Int		= MKX_CST_SUBSAMPLING_2;		// disable the supsampling
		_mkxParams->Val[MKX_BLOBENHRINGSIG_F].Float			= 0.15f / info.radius_mm_try1; 
		_mkxParams->Val[MKX_BLOBENHINTENSITYFACTOR_F].Float	= 3;							// amplification of contrast around pixels   
		_mkxParams->Val[MKX_CPLSELECTWIREFLAG_I].Int			= 0;							// not 
		_mkxParams->Val[MKX_CPLSELECTNBLABELSKEPT_I].Int		= 30; 
	}

	
	if (!check(PrmSetPara(_mkxHandle->PrmHdl, _mkxParams.get()), "Could not set parameters")) return false;

	_current = params;
	return true;
}

void NodeMarkerExtraction::exit()
{
	delete [] _mkxImage;
	_mkxImage = nullptr;

	if(_mkxHandle->Pv)
	{
		MkxDelete(*_mkxHandle);				
		_mkxHandle->Pv = nullptr;
	}
}

void NodeMarkerExtraction::process(const Metadata &meta, const Parameters &params, const Region &roi, float fps, float mmPerPixel, const Image<short> &input, StentMarkers &output, bool useHistory)
{
	if (!(_mkxHandle->Pv))					_reset = true;
	if (input.width			 != _width)					_reset = true;
	if (input.height		 != _height)				_reset = true;
	if (params.stentType	 != _current.stentType)		_reset = true;

	if (_reset)
	{
		if (input.width < MKX_CST_MIN_IMAWIDTH)
		{
			system().traceMsg("Image width is too small ( %d < %d )", input.width, MKX_CST_MIN_IMAWIDTH);
			return;
		}
		if (input.height < MKX_CST_MIN_IMAHEIGHT)
		{
			system().traceMsg("Image height is too small ( %d < %d )", input.height, MKX_CST_MIN_IMAHEIGHT);
			return;
		}
	}
	if (_reset)
	{
		init(params, input);
	}

	_reset = false;

	if (!(_mkxHandle->Pv)) return;

	Region rect            = getRoi(meta, params, roi, input);
		//if the user has overriden the parameters, don't apply them...
	MarkerInfo info;
	if(params.stentType == StentType::manual)
	{
		info = markerInfo[(int)StentType::cardiac]; // take remaining defaults from first entry
		info.cardiac_movement				=	params.cardiac_movement;		
		info.minimal_length_mm_try1			=	params.minimal_length_mm_try1;
		info.maximum_length_mm_try1			=	params.maximum_length_mm_try1;
		info.radius_mm_try1					=	params.radius_mm_try1;
		info.tracker_angle_high_level		=	params.tracker_angle_high_level;
		info.tracker_angle_skip_level		=	params.tracker_angle_skip_level;
		info.tracker_breadth_high_level		=	params.tracker_breadth_high_level;
		info.tracker_breadth_skip_level		=	params.tracker_breadth_skip_level;
		info.tracker_centroid_high_level	=	params.tracker_centroid_high_level;
		info.tracker_centroid_skip_level	=	params.tracker_centroid_skip_level;
	}
	else
	{
		info = markerInfo[(int)params.stentType];
	}

	CMkxSeqPara seqParams;

	seqParams.MarkersDistMinMm	= info.minimal_length_mm_try1;
	seqParams.MarkersDistMaxMm  = info.maximum_length_mm_try1;
	seqParams.Fps				= fps; 
	seqParams.MmPerPixel		= mmPerPixel;
	seqParams.InitRoiFacXMin	= float(rect.left)		/ float(_width-1);
	seqParams.InitRoiFacYMin	= float(rect.top)		/ float(_height-1);
	seqParams.InitRoiFacXMax	= float(rect.right-1)	/ float(_width-1);
	seqParams.InitRoiFacYMax	= float(rect.bottom-1)	/ float(_height-1);
	seqParams.DefRoiFacXMin		= seqParams.InitRoiFacXMin;
	seqParams.DefRoiFacYMin		= seqParams.InitRoiFacYMin;
	seqParams.DefRoiFacXMax		= seqParams.InitRoiFacXMax;
	seqParams.DefRoiFacYMax		= seqParams.InitRoiFacYMax;


	if (useHistory) 
	{
		_mkxParams->Val[MKX_HISTORYFLAG_I].Int = MKX_CST_HISTORY_APPLYONLY;		// use history for marker location prediction 
	}	
	else
	{
		_mkxParams->Val[MKX_HISTORYFLAG_I].Int = MKX_CST_HISTORY_UPDATEONLY;		// only learn
	}

	if (!check(PrmSetPara(_mkxHandle->PrmHdl, _mkxParams.get()), "Could not set parameters")) return;

	if (!check(MkxSetSeqPara(*(_mkxHandle), seqParams), "Unable to set sequence specific paramater")) return;
	

	for(int y = 0; y < _height;y++)
	{
		memcpy( ( _mkxImage + y*_width), input.pointer + y *input.stride,_width *sizeof(_mkxImage[0]));
	}		

	CMkxExtractResults result;		
	memset(&result, 0,sizeof(result));
	memset(&output, 0, sizeof(output));

	_threadPool->pool.profileBegin("MarkerExtraction.Extract");		
	
	bool success = check(MkxExtract(*_mkxHandle, _frameIndex, _mkxImage, &result), "Could not extract markers");
	_threadPool->pool.profileEnd();

	if (success)
	{
		output.valid			 = result.Locked == MKX_CST_EXTRACT_RES_LOCKED && result.Status == MKX_CST_EXTRACT_RES_OK;
		output.stitchDetected	 = result.SwStatus == MKX_CST_EXTRACT_RES_STERNALWIRE_DETECTED;
		output.p1.x				 = result.X1Refined;
		output.p1.y				 = result.Y1Refined;
		output.p2.x				 = result.X2Refined;
		output.p2.y				 = result.Y2Refined;
		output.confidence        = result.Confidence;
	}

	_frameIndex++;
}


bool NodeMarkerExtraction::check(int error, const std::string &message)
{
	if(error == 0)					return true;
	if(!(_mkxHandle->Pv))	return false;

	char *errMsg;		ErrGetErrorMessage		(_mkxHandle->ErrHdl,error,	&errMsg);
	char *errReport;	ErrGetLastErrorReport	(_mkxHandle->ErrHdl,		&errReport);

	std::string str = message + "(" + errMsg + ")";

	system().traceMsg(str.c_str());
	system().traceMsg("Additional information:");
	system().traceMsg(errReport);
		
	if(error >= 7) _reset = true;
	return false;					
}

}}