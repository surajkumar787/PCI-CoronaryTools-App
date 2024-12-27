// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>

namespace CommonPlatform {	namespace Xray {

// This struct hold the all data that is recorded from an incoming Cwis EPX model.
struct XrayEpx
{
	bool changed(const XrayEpx &epx) const;
    std::wstring activityType;
};

}}