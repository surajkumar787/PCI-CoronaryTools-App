/*-----------------------------------------------------------------------------
 | Copyright: Copyright(c) 2015, Philips Medical Systems Nederland B.V.       |
 | Author:    M. den Hartog, IGT Systems Innovation                           |
 -----------------------------------------------------------------------------*/
#pragma once

#include <memory>
#include <IpSimplicity.h>
#include <IpXres3.h>
#include <Sense.h>
#include <NodeMarkerExtraction.h>
#include "Settings.h"
#include "PciReviewParameters.h"

using namespace Simplicity;

namespace Pci {	namespace Core {

class StbParameters : public CommonPlatform::Settings
{
public:

    StbParameters(const wchar_t *stentboostInifile, const wchar_t *settingsInifile= L"");

	PciReviewParameters getReviewParameters() const;

	std::shared_ptr<NodeUnique::ParametersLut>			markersLut;
	std::shared_ptr<NodeUnique::Parameters>				markersUnique;
	std::shared_ptr<StentBoostAlgo::NodeMarkerExtraction::Parameters>	markerParams;

	std::shared_ptr<NodeUnique::Parameters>				boostUnique;	
	std::shared_ptr<NodeUnique::ParametersLut>			boostLut;

	// todo: for tuning purposes, the CBEH is currently used for live stentboost postprocessing: should be converted to 'normal parameters' 
	std::shared_ptr<NodeUnique::ParametersCBEH>			liveUniqueStentboost;
	std::shared_ptr<NodeUnique::ParametersLut>			liveLutStentboost;

	std::shared_ptr<NodeUnique::Parameters>				liveUnique;
	std::shared_ptr<NodeUnique::ParametersLut>			liveLut;
	

	bool												boostUseAtr;
	int													boostIntegrationCount;	
	int													boostBackground;
	float												boostZoomFactor;	
	float												boostStartEdgeValue;
	int													boostEdgeRampupTime;
	int													boostShutterMargin;	
	float												ortogonalScaleFactor;
	bool												showmask;
	std::shared_ptr<NodeAtr2Ex::Parameters>				boostAtr;

	bool												valid;

	std::shared_ptr<NodeLut::ParametersLut>				perceptionLut;

	int													nrStbLoops;
	double												loopDelay;
	int													loopMinNrFrames;
	int													loopMaxNrFrames;
    double                                              processingTimeout;

	Sense::Rect											regionOfInterest;
};

}}