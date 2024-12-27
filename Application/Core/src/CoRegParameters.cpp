// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegParameters.h"

namespace Pci{ namespace Core{

const static float DefaultXrayLatency = 0.150f;
const static float DefaultPressureLatency = 0.020f;

CoRegParameters::CoRegParameters(const wchar_t *settingsIniFile)
:
	Settings                  (settingsIniFile),
	m_xrayLatency				  (get(L"CoReg",   L"xrayLatency",		DefaultXrayLatency)),
	m_pressureLatency			  (get(L"CoReg",   L"pressureLatency",  DefaultPressureLatency))
{
}

float Pci::Core::CoRegParameters::getXrayLatency() const
{
	return m_xrayLatency;
}

float Pci::Core::CoRegParameters::getPressureLatency() const
{
	return m_pressureLatency;
}
}}
