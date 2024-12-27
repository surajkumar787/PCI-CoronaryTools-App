// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbContrastSelect.h"
#include "StbcontrastDetect.h"

namespace Pci {	namespace StentBoostAlgo {

using namespace Simplicity;

StbContrastSelect::StbContrastSelect(void)
{
}

StbContrastSelect::~StbContrastSelect(void)
{
}

bool StbContrastSelect::init()
{	
	refValid = false;	//we did not find the reference markers...
	return true;
}

void StbContrastSelect::process(const StentMarkers &markers, int index)
{
	if (index == 0)	//lets store the markers so we have at least valid markers...
	{
		ref1 = markers.p1;
		ref2 = markers.p2;
	}
	else if (markers.valid &&								//if the markers for the current frame are found
			!refValid)										//if we did not find any earlier markers...
	{
		//then store the markers as reference.
		ref1 = markers.p1;
		ref2 = markers.p2;
		refValid = true;
	}
}


float getDist(NodeAffineTransform::Vector p1, NodeAffineTransform::Vector p2)
{
	return sqrtf((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
}


void StbContrastSelect::postProcess(const StentMarkers &markers, StbContrastDesc &contrast, int index, bool contrastDetected)
{
	bool isContrast = contrast.hasContrast && contrastDetected;
	if (!markers.valid)	isContrast = false;

	//because the markers might be swapped, we calculate both distances.
	//we will simply select the smallest distance as a measure
	float dist1 = getDist(markers.p1, ref1) + getDist(markers.p2, ref2);
	float dist2 = getDist(markers.p1, ref2) + getDist(markers.p2, ref1);

	//calculate the orientation of the reference
	NodeAffineTransform::Vector delta1 = { ref2.x - ref1.x, ref2.y - ref1.y };
	//because the markers might be swapped, we calculate both possible orientations for the current marker pair.
	//we will simply select the smallest orientation difference as a measure
	NodeAffineTransform::Vector delta2 = { markers.p1.x - markers.p2.x, markers.p1.y - markers.p2.y };
	NodeAffineTransform::Vector delta3 = { markers.p2.x - markers.p1.x, markers.p2.y - markers.p1.y };

	//note that we take the negative of all indications, so that the higher the value, the lower the score
	contrast.stbSelect.imageNumber = index;
	contrast.stbSelect.scoreTime = -(float)index;
	contrast.stbSelect.scorePos = -std::min(dist1, dist2);
	contrast.stbSelect.scoreDelta = -std::min(getDist(delta1, delta2), getDist(delta1, delta3));

	// TODO
	// update formula with correction factor to normalise frame rate and mm/pixel
	// time * (15/fps)
	// distance * (mm.pixel/0.11) 
	// angle * 1

	// finally, calculate the final score and assign it to the desc
	contrast.stbSelect.scoreTotal = contrast.stbSelect.scoreTime*factorTime + contrast.stbSelect.scorePos*factorPos + contrast.stbSelect.scoreDelta*factorDelta;
	contrast.stbSelect.valid = isContrast;
}
}}