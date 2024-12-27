// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>

namespace Pci { namespace Core
{
	struct AboutInfo
	{
		std::wstring ProductName;
		std::wstring ReleaseIndication;

		std::wstring ManufactoringDate;

		std::wstring ReferenceNr;
		std::wstring LotNr;
		std::wstring Gs1Nr;

		std::wstring eula;
	};

	namespace ProductInfo
	{
		extern const AboutInfo AboutDcrCoreg;
		extern const AboutInfo AboutSbl;
	}

}}