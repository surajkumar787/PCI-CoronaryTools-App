// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmErrors.h
#pragma once

namespace CommonPlatform
{
	class Log;
}

namespace PciSuite
{
	class CrmErrors
	{
	public:
		enum class Error
		{ 
			OK, 
			Unknown,
			SaveFailure,
			CarmMoved,
			PatientTableMoved,
			InjectionNotWellDefined,
			InjectionTooShort,
			NoInjection,
			AngioTooShort,
			NoHeartCycle,
			CarmAndTableMoved
		};

		CrmErrors();
		virtual ~CrmErrors();

		static bool mapOscErrorToCrmError(int result, Error &error, CommonPlatform::Log& logger);
		static bool mapOscErrorToCrmError(int result, Error &error, bool ignoreAngioErrors, CommonPlatform::Log& logger);
	};
}

