// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "Settings.h"
#include <IpSimplicity.h>
#include <memory>

namespace Pci { namespace Core {

class CrmParameters : public ::CommonPlatform::Settings
{
public:
	CrmParameters(const wchar_t *roadmapInifile, const wchar_t *settingsIniFile = L"");
	virtual ~CrmParameters() {}
	
	int  angleMinimumDistance;

	int  angleThresholdActive;
	int  angleThresholdRelease;
	int  angleThresholdAvailable;

	int  panningThresholdActive;
	int  panningThresholdRelease;
	int  panningThresholdAvailable;

	float thresholdAcceptNew;
	float thresholdAcceptOverwrite;

	bool ignoreAngioErrors;
	float blendFactor;

    std::wstring storeRoadmapFolder;
	std::shared_ptr<Simplicity::NodeLut::ParametersLut>	perceptionLut;
};

}}