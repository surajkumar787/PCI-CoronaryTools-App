// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>

namespace CommonPlatform {	namespace Xray {

struct XrayStudy
{
	std::wstring studyId;
	std::wstring studyInstanceUid;
	std::wstring performedProcedureStepId;

	std::wstring referringPhysician;
	std::string studyDate;
	std::string studyTime;
	std::wstring accessionNumber;

	std::wstring patientName;
	std::wstring patientId;
	std::wstring patientBirthdate;
	std::wstring patientGender;
	std::wstring requestedProcedureId;
	std::wstring institutionName;	
	std::wstring otherPatientIds;	

    bool changed(const XrayStudy &study) const;
	bool equal( const XrayStudy &study) const;
    bool valid() const;
};

}}