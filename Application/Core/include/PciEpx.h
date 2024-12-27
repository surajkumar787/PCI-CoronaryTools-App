// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>

namespace CommonPlatform { namespace Xray
{
    struct XrayEpx;
}}

namespace Pci { namespace Core {

// This class maps the Cwis EPX model onto the PCI EPX model
class PciEpx
{
public:
    enum class Type	{ Roadmap, StentBoost, NonPci };
	enum class SubType { NotApplicable, Live, PostDeployment, Subtract };

    PciEpx();
    explicit PciEpx(const CommonPlatform::Xray::XrayEpx &Epx);
    explicit PciEpx(Type type);
    explicit PciEpx(SubType type);
	virtual ~PciEpx() {}
    
    bool changed(const PciEpx &epx) const;
	bool changed(const CommonPlatform::Xray::XrayEpx &epx);
	Type getType() const;
	SubType getSubType() const;

private:
    Type m_type;
	SubType m_subType;

protected:
    Type getPciExpType(const CommonPlatform::Xray::XrayEpx& xrayEpx);
	SubType getSubType(const CommonPlatform::Xray::XrayEpx& xrayEpx);
    bool isStentBoostAcitvityType(const std::wstring& activityType);
    bool isRoadmapAcitvityType(const std::wstring& activityType);
    bool stringContainsToken(const std::wstring& inputString, const wchar_t* token);
	bool isStentBoostPostDeploymentAcitvityType(const std::wstring& activityType);
	bool isStentBoostSubtractAcitvityType(const std::wstring& activityType);
	bool isStentBoostLiveAcitvityType(const std::wstring& activityType);
};

}}