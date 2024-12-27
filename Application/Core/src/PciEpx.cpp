// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PciEpx.h"
#include "XrayEpx.h"

#include <string>
#include <vector>

namespace Pci{ namespace Core{
using namespace CommonPlatform::Xray;

static const wchar_t ActivityTypeSeperator = L',';

PciEpx::PciEpx()
:	m_type(Type::NonPci),
	m_subType(SubType::NotApplicable)
{
}

PciEpx::PciEpx(const XrayEpx &Epx)
:	m_type(getPciExpType(Epx)),
	 m_subType(getSubType(Epx))
{
}

Pci::Core::PciEpx::PciEpx(Type type)
{
	m_type = type;
	m_subType = SubType::NotApplicable;
}

Pci::Core::PciEpx::PciEpx(Pci::Core::PciEpx::SubType subType)
{
	m_type = Type::StentBoost;
	m_subType = subType;
}

bool PciEpx::changed(const PciEpx &epx)const
{
    return (epx.getType() != m_type) || (epx.getSubType() !=  m_subType) ;
}

bool PciEpx::changed(const XrayEpx &epx)
{
	return (getPciExpType(epx) != m_type ) || ( getSubType(epx) != m_subType );
}

Pci::Core::PciEpx::Type Pci::Core::PciEpx::getType() const
{
	return m_type;
}

Pci::Core::PciEpx::SubType PciEpx::getSubType() const
{
	return m_subType;
}
PciEpx::Type PciEpx::getPciExpType(const XrayEpx& xrayEpx)
{
    Type epxType;
    epxType = Type::NonPci;

    if ( isStentBoostAcitvityType   (xrayEpx.activityType)) epxType = Type::StentBoost;
    if ( isRoadmapAcitvityType      (xrayEpx.activityType)) epxType = Type::Roadmap;

    return epxType;
}

bool PciEpx::isStentBoostAcitvityType(const std::wstring& activityType)
{
    if (stringContainsToken(activityType, L"SBLive"))      return true;
    if (stringContainsToken(activityType, L"StbPciSuite")) return true;
	if (stringContainsToken(activityType, L"SBLivePost"))     return true;
	if (stringContainsToken(activityType, L"SBLiveSubtract"))     return true;
	return false;
}

bool PciEpx::isRoadmapAcitvityType(const std::wstring& activityType)
{
    if (stringContainsToken(activityType, L"LCA"        )) return true;
    if (stringContainsToken(activityType, L"RCA"        )) return true;
    if (stringContainsToken(activityType, L"CrmPciSuite")) return true;
    return false;
}

bool PciEpx::stringContainsToken(const std::wstring& inputString, const wchar_t* token)
{
	std::vector<wchar_t> string(inputString.begin(), inputString.end());
	string.push_back(L'\0');

	wchar_t* buffer = nullptr;
	wchar_t* inputToken = nullptr;
	inputToken = wcstok_s(&string[0], L", ", &buffer);
	while (inputToken != nullptr)
	{
		if (_wcsicmp(inputToken, token) == 0)
		{
			return true;
		}
		inputToken = wcstok_s(nullptr, L", ", &buffer);
	}
	return false;
}

Pci::Core::PciEpx::SubType Pci::Core::PciEpx::getSubType(const XrayEpx& xrayEpx)
{
	SubType subtype(SubType::NotApplicable);
	if (isStentBoostPostDeploymentAcitvityType(xrayEpx.activityType))
	{
		subtype = SubType::PostDeployment;
	}
	else if ( isStentBoostSubtractAcitvityType(xrayEpx.activityType))
	{
		subtype = PciEpx::SubType::Subtract;
	}
	else if ( isStentBoostLiveAcitvityType(xrayEpx.activityType))
	{
		subtype = PciEpx::SubType::Live;
	}
	return subtype;
}

bool Pci::Core::PciEpx::isStentBoostPostDeploymentAcitvityType(const std::wstring& activityType)
{
	return (stringContainsToken(activityType, L"SBLivePost"));
}

bool Pci::Core::PciEpx::isStentBoostSubtractAcitvityType(const std::wstring& activityType)
{
	return (stringContainsToken(activityType, L"SBLiveSubtract"));
}

bool Pci::Core::PciEpx::isStentBoostLiveAcitvityType(const std::wstring& activityType)
{
	return (stringContainsToken(activityType, L"SBLive"));
}


}}
