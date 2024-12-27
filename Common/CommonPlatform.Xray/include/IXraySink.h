// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "XrayImage.h"


namespace CommonPlatform {	namespace Xray {

class IXraySink
{
    public:
	virtual ~IXraySink() {}

	virtual void addImage(const std::shared_ptr<const XrayImage> &image) = 0;
};

}}
