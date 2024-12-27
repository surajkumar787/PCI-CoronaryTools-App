// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "XrayStudy.h"

namespace CommonPlatform {	namespace Xray {


bool XrayStudy::changed(const XrayStudy &study) const
{
	return study.studyId != studyId || 
        study.patientId != patientId || 
        study.patientName != patientName ||
        study.patientBirthdate != patientBirthdate ||
        study.patientGender != patientGender;
}

bool XrayStudy::equal(const XrayStudy &study) const
{
	return ( study.studyId == studyId &&
			 study.studyInstanceUid == studyInstanceUid &&
			 study.performedProcedureStepId == performedProcedureStepId &&
			 
			 study.referringPhysician == referringPhysician &&
			 study.studyDate == studyDate &&
			 study.studyTime == studyTime &&
			 study.accessionNumber == accessionNumber &&			 
			 study.patientName == patientName &&
			 study.patientId == patientId &&
			 study.patientBirthdate == patientBirthdate &&
			 study.patientGender == patientGender &&
			 study.requestedProcedureId == requestedProcedureId  &&	
			 study.otherPatientIds == otherPatientIds &&
			 study.institutionName == institutionName );
}


bool XrayStudy::valid() const
{
	return !studyId.empty() && !patientId.empty();
}

}}