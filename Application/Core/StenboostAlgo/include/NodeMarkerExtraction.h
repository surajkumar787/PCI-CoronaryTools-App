// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <IpSimplicity.h>
#include <memory>
#include "StentMarkers.h"
#include "PrmLib.h"
#include "mkx.h"

namespace Pci {	namespace StentBoostAlgo {

class StbThreadPool;
class NodeMarkerExtraction
{
public:
		
	enum class StentType:int {manual=-1, cardiac = 0, head = 1 , abdomen = 2, thorax = 3, peripheral = 4 ,cardiac2 = 5,cardiac3  = 6};

	struct Parameters: Simplicity::ParameterList
	{
		Parameters();	
		int lostMarkerWarningThreshold;
		int maxLostMarkerCount;
		StentType	stentType;	
		bool	cardiac_movement;		
		float	minimal_length_mm_try1;
		float	maximum_length_mm_try1;
		float	radius_mm_try1;	
		float	tracker_angle_high_level;
		float	tracker_angle_skip_level;
		float	tracker_breadth_high_level;
		float	tracker_breadth_skip_level;
		float	tracker_centroid_high_level;
		float	tracker_centroid_skip_level;			
	};
	
	explicit NodeMarkerExtraction(Simplicity::ThreadPool &threadPool);
	virtual ~NodeMarkerExtraction();

	void reset();
	void process(const Simplicity::Metadata &meta, const Parameters &params, const Simplicity::Region &roi, float fps, float mmPerPixel, const Simplicity::Image<short> &input, StentMarkers &output, bool useHistory);

	NodeMarkerExtraction(const NodeMarkerExtraction&) = delete;
	NodeMarkerExtraction& operator=(const NodeMarkerExtraction&) = delete;

protected:
				
	bool init(const Parameters &params, const Simplicity::Image<short> &input);
	bool check(int error, const std::string &mainError);
	void exit();

	std::unique_ptr<StbThreadPool> _threadPool;

	Parameters	_current;
	
	std::unique_ptr<CMkxHdl> _mkxHandle;
	std::unique_ptr<CPrmPara>_mkxParams;
	std::unique_ptr<CPrmParaCtx>_mkxContext;
	
	short       *_mkxImage;

	int			_width;
	int			_height;
	int			_frameIndex;
	bool		_reset;
};

}}

