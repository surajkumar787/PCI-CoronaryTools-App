// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "StentMarkers.h"
#include "IpSimplicity.h"

namespace Pci {	namespace StentBoostAlgo {

//The StbLibSelect class tries to find an image out of all vessel images that would best match 
//with the boosted image (the reference). 

//Because marker detection in contrast is not as reliable,  we will have to avoid frames that have strange marker locations.
//The matching uses three criteria to give a score to each frame. Finally, 
//the frame with the highest score should be picked

//	*	(time)	Frames in the beginning of the contrast part have priority
//------------------------------------------------------------------------
//		This is because marker detection in contrast gets unreliable over time. Once the tracker has lost 
//		sight of the correct markers, it is unlikely that it will recover

//	*	(pos)	Markers that match the position of the reference frame have priority
//------------------------------------------------------------------------
//		If the location of the markers in the vessel frame, compared to the reference frame, 
//		is very different, probably the marker detection was wrong.

//	*	(delta) Markers that match the orientation of the reference frame have priority
//------------------------------------------------------------------------
//		If the orientation of the markers in the vessel frame, compared to the reference frame, 
//		is very different, probably the marker detection was wrong. Also, if the stent orientation
//		of the contrast frame closely matches the vessel frame, there is less change that the
//		vessel is "bent".



#define factorTime  0.05f		//the influence of the time criteria
#define factorDelta 1.0f		//the influence of the orientation criteria
#define factorPos   1.0f		//the influence of the position criteria


struct StbSelectDesc
{
	int imageNumber;
	//the scores for each criteria
	float scoreTime;
	float scoreDelta;
	float scorePos;

	//the total score. This is calculated as scoreTime * factorTime + ... + scorePos * factorPos
	float scoreTotal;

	//are the scores valid (because they are not, for non-contrast frames)
	bool valid;
};

struct StbContrastDesc;

class StbContrastSelect
{
public:
	StbContrastSelect(void);
	~StbContrastSelect(void);

	bool init();

	//pass all frames sequentially to the process function
	void process(const StentMarkers &markers, int index);
	//pass all frames sequentially to the postProcess function. 
	void postProcess(const StentMarkers &markers, StbContrastDesc &contrast, int index, bool contrastDetected);

private:

	Simplicity::NodeAffineTransform::Vector ref1, ref2;	    //the markers that are used as reference. They are copied from the first boosted image
	bool refValid;			//Are the reference markers found???

};
}}