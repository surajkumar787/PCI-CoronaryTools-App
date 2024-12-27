// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.


#include "ViewStateHelper.h"

using namespace Pci::Core;

namespace Pci{ namespace Core{ namespace ViewStateHelper {

	ViewState  DetermineCrmState( CrmAdministration::Status adminStatus , PciSuite::CrmErrors::Error error)
	{
		ViewState newViewState = ViewState::CrmCreationGuidanceState;
		if( error == PciSuite::CrmErrors::Error::OK)
		{
			if(adminStatus == CrmAdministration::Status::Active)
			{
				newViewState = ViewState::CrmRoadmapActive;
			}else 
			if( adminStatus == CrmAdministration::Status::MoveArc)
			{
				newViewState = ViewState::CrmApcGuidanceState;
			}else 
			if(adminStatus == CrmAdministration::Status::MoveTable)
			{
				newViewState = ViewState::CrmTableGuidanceState;
			}else 
			if(	adminStatus == CrmAdministration::Status::NoRoadmap)
			{
				newViewState = ViewState::CrmCreationGuidanceState;
			}
		}
		else
		{
			newViewState = ViewState::CrmErrorGuidanceState;
		}

		return newViewState;
	}

}}}

