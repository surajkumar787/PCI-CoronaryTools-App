// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>

namespace Sense
{
	struct Size;
}
namespace CommonPlatform { namespace Xray { enum class SystemType;} }

namespace Pci { namespace Core
{

	enum class ViewType;

	ViewType GetViewType(const std::wstring& systemType, const Sense::Size& screenSize);
	CommonPlatform::Xray::SystemType GetSystemType(const std::wstring& systemType);
}}