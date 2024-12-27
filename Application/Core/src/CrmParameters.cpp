// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmParameters.h"

using namespace Pci::Core;
	
CrmParameters::CrmParameters(const wchar_t *roadmapInifile, const wchar_t *settingsIniFile)
:
	Settings                  (roadmapInifile),
	angleMinimumDistance	  (get(L"Roadmap", L"angleMinimumDistance",			2)),
	angleThresholdActive	  (get(L"Roadmap", L"angleThresholdActive", 		2)),
	angleThresholdRelease	  (get(L"Roadmap", L"angleThresholdRelease", 		2)),
	angleThresholdAvailable	  (get(L"Roadmap", L"angleThresholdAvailable",		10)),
	panningThresholdActive	  (get(L"Roadmap", L"panningThresholdActive",		30)),
	panningThresholdRelease	  (get(L"Roadmap", L"panningThresholdRelease",		30)),
	panningThresholdAvailable (get(L"Roadmap", L"panningThresholdAvailable",	200)),
    thresholdAcceptNew	   	  (get(L"Roadmap", L"thresholdAcceptNew",			0.5f)),
    thresholdAcceptOverwrite  (get(L"Roadmap", L"thresholdAcceptOverwrite",		0.5f)),
	ignoreAngioErrors		  (get(L"Roadmap", L"ignoreAngioErrors",			false)),
	blendFactor				  (get(L"Roadmap", L"blendFactor",					0.2f)),
    storeRoadmapFolder        (get(L"Roadmap", L"storeRoadmapFolder",           L"")),
	perceptionLut			  (std::make_shared<Simplicity::NodeLut		::ParametersLut>())
{
    bool traceWarning = false;
    const char* iniGroup  = "perception";
    const char* iniName   = "lut";

    // by default use lut from Settings INI, but it can be overriden with a lut from crm/roadmap ini (for testing/tuning purposes)
    if(!perceptionLut->load(roadmapInifile,iniGroup, iniName, traceWarning ))
    {
        perceptionLut->load(settingsIniFile,iniGroup, iniName, traceWarning);
    }
}