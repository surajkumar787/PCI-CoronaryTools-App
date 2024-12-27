// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <cstdio>


namespace Pci {
	namespace Core
	{
		enum class LicenseStatus;
		enum class LicenseFeature;
	}

	namespace FlexLM{

#ifdef FLEXLM_EXPORTS
#define FLEXLM_FUNC_EXPORT __declspec(dllexport)
#else
#define FLEXLM_FUNC_EXPORT __declspec(dllimport)
#endif

extern "C" FLEXLM_FUNC_EXPORT void InitializeLicenseCheck(const char* licenseFile);
extern "C" FLEXLM_FUNC_EXPORT Pci::Core::LicenseStatus CheckLicenseStatus(Pci::Core::LicenseFeature feature);
extern "C" FLEXLM_FUNC_EXPORT void UninitializeLicenseCheck();

FILE _iob[] = { *stdin, *stdout, *stderr };

extern "C" FILE * __cdecl __iob_func(void)
{
	return _iob;
}
}}