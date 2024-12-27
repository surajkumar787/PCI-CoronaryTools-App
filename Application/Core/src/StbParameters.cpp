// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "StbParameters.h"
#include <assert.h>

namespace Pci {	namespace Core {

using namespace StentBoostAlgo;

static const int MaxNumberOfBands = 6;
static const int DefauntMinNrFrames =5;
static const int DefauntMaxNrFrames = 300;
static const double DefaultLoopDalay = 1.0;
static const int DefaultNrStbLoops = 3;
static const double DefaultProcessingTimeout = 0.2;
static const int DefaultBoostIntegrationCount = 5;
static const int DefaultBoostShutterMargin = 20;
static const float DefaultZoomFactor = 2.0f;
static const float DefaultBoostStartEdgeValue = 0.8f;

	
StbParameters::StbParameters(const wchar_t *stentboostInifile, const wchar_t *settingsInifile)
:
	Settings(stentboostInifile),
	markersLut				(std::make_shared<NodeUnique	::ParametersLut>()),
	markersUnique			(std::make_shared<NodeUnique	::Parameters>()),
	markerParams			(std::make_shared<NodeMarkerExtraction::Parameters>()),
	boostUnique				(std::make_shared<NodeUnique	::Parameters>()),
	boostLut				(std::make_shared<NodeUnique	::ParametersLut>()),	
	liveUniqueStentboost	(std::make_shared<NodeUnique	::ParametersCBEH>()),
	liveLutStentboost		(std::make_shared<NodeUnique	::ParametersLut>()),
	liveUnique				(std::make_shared<NodeUnique	::Parameters>()),
	liveLut					(std::make_shared<NodeUnique	::ParametersLut>()),
	boostUseAtr				(false),
	boostIntegrationCount   (DefaultBoostIntegrationCount),
	boostBackground         (8192),
	boostZoomFactor         (DefaultZoomFactor),
	boostStartEdgeValue     (DefaultBoostStartEdgeValue),
	boostEdgeRampupTime     (4),
	boostShutterMargin      (DefaultBoostShutterMargin),
	ortogonalScaleFactor    (0.0),
	showmask                (true),
	boostAtr				(std::make_shared<NodeAtr2Ex	::Parameters>()),
	valid					(true),
	perceptionLut			(std::make_shared<NodeLut		::ParametersLut>()),
	nrStbLoops				(3),
	loopDelay				(0.0),
	loopMinNrFrames			(DefauntMinNrFrames),
	loopMaxNrFrames			(DefauntMaxNrFrames),
    processingTimeout       (DefaultProcessingTimeout),
	regionOfInterest		()
{
	boostUseAtr					= get(L"boost",   L"useAtr",				false);
	boostIntegrationCount		= get(L"boost",   L"integrationCount",      DefaultBoostIntegrationCount);
	boostBackground				= get(L"boost",   L"background",			8192);
	boostZoomFactor				= get(L"boost",   L"zoomFactor",		    DefaultZoomFactor);
	boostStartEdgeValue			= get(L"boost",   L"startEdgeValue",		DefaultBoostStartEdgeValue);			boostStartEdgeValue	= std::max(boostStartEdgeValue,0.0f);
	boostEdgeRampupTime			= get(L"boost",   L"edgeRampupTime",		4);	
	boostShutterMargin			= get(L"boost",   L"shutterMargin",         DefaultBoostShutterMargin);
	ortogonalScaleFactor		= get(L"boost",	  L"ortogonalScaleFactor",  0.0f);
	ortogonalScaleFactor		= std::max(0.0f, std::min(ortogonalScaleFactor,1.0f));
	showmask					= get(L"mask",    L"showmask",				true);
	

	// Innova compatible luts are not supported
	valid = valid && (!get(L"boostUnique",			L"innovacompat",false));
	valid = valid && (!get(L"markersUnique",		L"innovacompat",false));
	valid = valid && (!get(L"liveUnique",			L"innovacompat",false));
	valid = valid && (!get(L"liveUniqueStentboost",	L"innovacompat",false));
	
		
	if(get(L"boostUnique",L"cbeh", false))
	{
		NodeUnique::ParametersCBEH params;
		valid = valid && params.load(stentboostInifile,"boostUnique", MaxNumberOfBands);
		NodeUnique::computeFromCBEH(params,*boostUnique);
	}
	else
	{
		valid = valid && boostUnique->load(stentboostInifile,"boostUnique", MaxNumberOfBands);
	}
	
	if(get(L"markersUnique",L"cbeh",false))
	{
		// load CBEH and convert
		NodeUnique::ParametersCBEH params;
		valid = valid && params.load(stentboostInifile,"markersUnique", MaxNumberOfBands);
		NodeUnique::computeFromCBEH(params,*markersUnique);
	}
	else
	{
		valid = valid && markersUnique->load(stentboostInifile,"markersUnique", MaxNumberOfBands);
	}

	
	if(get(L"liveUnique",L"cbeh",false))
	{
		// load CBEH and convert
		NodeUnique::ParametersCBEH params;
		valid = valid && params.load(stentboostInifile,"liveUnique", MaxNumberOfBands);
		NodeUnique::computeFromCBEH(params,*liveUnique);
	}
	else
	{
		valid = valid && liveUnique->load(stentboostInifile,"liveUnique", MaxNumberOfBands);
	}

		// TODO: must be CBEH and not innova compatible for tuning purposes	
	valid = valid &&  get(L"liveUniqueStentboost",	L"cbeh",true);

	valid = valid && liveUniqueStentboost	->load(stentboostInifile,"liveUniqueStentboost", MaxNumberOfBands);


	// load unique luts
	valid = valid && markersLut			->load(stentboostInifile,"markersUnique");
	valid = valid && liveLut			->load(stentboostInifile,"liveUnique");
	valid = valid && liveLutStentboost	->load(stentboostInifile,"liveUniqueStentboost");
	valid = valid && boostLut			->load(stentboostInifile,"boostUnique");
	
	// load atr3
	valid = valid && boostAtr		->load(stentboostInifile,"atr",3);
	valid = valid && markerParams	->load(stentboostInifile,"markers");
	
    bool traceWarning = false;
    const char* iniGroup  = "perception";
    const char* iniName   = "lut";

    // by default use lut from Settings INI, but it can be overridden with a lut from stentboost ini (for testing/tuning purposes)
    if(!perceptionLut->load(stentboostInifile,iniGroup, iniName, traceWarning))	
    {
        perceptionLut->load(settingsInifile,iniGroup, iniName, traceWarning);
    }

	nrStbLoops		= get(L"boost",	L"nrStbLoops", DefaultNrStbLoops);
	loopDelay		= get(L"boost",	L"loopDelay", DefaultLoopDalay);
	loopMinNrFrames	= get(L"boost",	L"loopMinNrFrames", DefauntMinNrFrames);
	loopMaxNrFrames	= get(L"boost",	L"loopMaxNrFrames", DefauntMaxNrFrames);

    processingTimeout = get(L"boost",	L"processingTimeout", DefaultProcessingTimeout);

#if defined(_DEBUG) // time between frames exceeds the normal value in debug mode
    processingTimeout *= 2;
#endif

	assert(valid);
}

Pci::Core::PciReviewParameters StbParameters::getReviewParameters() const
{
	return Pci::Core::PciReviewParameters(nrStbLoops, loopDelay, loopMinNrFrames, loopMaxNrFrames);
}
}}