// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

namespace Pci { namespace Core
{
	enum class ViewState
	{
		NonPciEpxState,
		CrmCreationGuidanceState,		
		CrmProcessingAngioState,	
		CrmErrorGuidanceState,		
		CrmApcGuidanceState,	
		CrmTableGuidanceState,		
		CrmRoadmapActive,
		CrmOverlayState,
		StbLiveGuidanceState,
		StbPostDeployGuidanceState,
		StbContrastGuidanceState,
		StbLiveProcessingState,
		StbLiveReviewState,
		StbPostDeployProcessingState,
		StbPostDeployReviewState,
		StbPostDeployRoiDefinitionState,
		StbContrastProcessingState,
		StbContrastReviewState,	
		StbContrastRoiDefinitionState,
		StbContrastFrameSelectionState,
		CoRegistrationState,
		CoRegistrationGuidanceState,
        StbBoostFrameSelectionState,
        NoConnection,
		NoLicense,
		Unknown
	};
}}