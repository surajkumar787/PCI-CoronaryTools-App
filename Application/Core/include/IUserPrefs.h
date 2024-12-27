// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include "IArchivingSettings.h"

namespace Pci {	namespace Core {

class IUserPrefs
{
public:
	virtual ~IUserPrefs() = default;

	virtual const IArchivingSettings& getArchivingSettings() const = 0 ;
	virtual bool getDefaultShowInvertedStent() const  = 0;
	virtual bool getApcAutoActivation() const = 0;
};

}}
