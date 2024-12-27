// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <memory>
#include <vector>

namespace CommonPlatform { namespace Xray { 
	struct XrayImage;
}}

namespace Pci { namespace Core {

class CrmRecording;
struct CrmOverlay;
class CrmRoadmap;

class ICrmAdministration
{
public:
	virtual ~ICrmAdministration() = default;

	// Return the current/last recording
	virtual std::shared_ptr<CrmRecording> getRecording() const = 0;

	virtual std::vector<std::shared_ptr<const CrmRoadmap>> getRoadmapsForCurrentGeo() const = 0;

	virtual void selectRoadmap(std::shared_ptr<const CrmRoadmap>& roadmap) = 0;

	virtual void lockRoadmap(std::shared_ptr<const CrmRoadmap>& roadmap) = 0;
	virtual void unlockRoadmap(std::shared_ptr<const CrmRoadmap>& roadmap) = 0;
};

}}
