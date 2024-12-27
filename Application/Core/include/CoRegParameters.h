// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "Settings.h"

namespace Pci { namespace Core {

class CoRegParameters : public ::CommonPlatform::Settings
{
public:
	explicit CoRegParameters(const wchar_t *settingsIniFile);
	virtual ~CoRegParameters() {}


	float getXrayLatency() const;
	float getPressureLatency() const;
	// System latencies at moment of time-stamping since the data was acquired.
private:
	float m_xrayLatency;
	float m_pressureLatency;
};

}}