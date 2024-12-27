// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <IpSimplicity.h>
#include <Sense.h>
#include "XrayImage.h"
#include "StbMarkers.h"

namespace Pci {namespace Core {


using namespace Sense;

enum class StbImagePhase
{
	boostPhase,
	transitionPhase,
	contrastPhase
};

struct StbImage : public CommonPlatform::Xray::XrayImage
{
	StbImage();
	
	StbMarkers refMarkers;
	bool showWarning;
	StbImagePhase phase;

	StbImage(const StbImage &);
	StbImage & operator=(const StbImage &) = delete;

	std::shared_ptr<StbImage> invert() const;
};


}}