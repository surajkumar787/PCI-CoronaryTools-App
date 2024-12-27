// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <memory>
#include "XrayImage.h"

namespace CommonPlatform {	namespace Xray {

class IXraySource
{
public:
	virtual ~IXraySource() {}

	virtual int getImageCount() const = 0;
	virtual std::shared_ptr<XrayImage> getImage(int index) = 0;
};

}}