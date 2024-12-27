// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//

#include "CrmErrors.h"
#include "Log.h"
#include <Osc.h>

using namespace PciSuite;

CrmErrors::CrmErrors()
{
}


CrmErrors::~CrmErrors()
{
}

bool PciSuite::CrmErrors::mapOscErrorToCrmError(int result, Error &error, CommonPlatform::Log& logger)
{
	return mapOscErrorToCrmError(result, error, false, logger);
}

bool CrmErrors::mapOscErrorToCrmError(int result, Error &error, bool ignoreAngioErrors, CommonPlatform::Log& logger)
{
	bool errorIsWarning = false;

	if (result == 0)
	{
		error = Error::OK;
	}
	else if (error == Error::OK && result != 0) switch(result)
	{
	case OSC_INFO_CARM_MOVED_INSEQ:				  logger.DeveloperInfo(L"CrmErrors: OSC_INFO_CARM_MOVED_INSEQ\n");		break;
	case OSC_INFO_PIXSIZE_CHANGED:				  logger.DeveloperInfo(L"CrmErrors: OSC_INFO_PIXSIZE_CHANGED\n");		break;
	case OSC_INFO_CARM_MOVED_ANGIOFLUORO:		  logger.DeveloperInfo(L"CrmErrors: OSC_INFO_CARM_MOVED_ANGIOFLUORO\n");break;
	case OSC_INFO_SHUTTERINFO_CHANGE:			  logger.DeveloperInfo(L"CrmErrors: OSC_INFO_SHUTTERINFO_CHANGE\n");	break;
	case OSC_WNG_TOO_LONG_ANGIO:				  logger.DeveloperInfo(L"CrmErrors: OSC_WNG_TOO_LONG_ANGIO\n");			break;

	case OSC_WRN_INHOMOGENEOUS_EXTRACTED_CYCLE:	  logger.DeveloperInfo(L"CrmErrors: OSC_WRN_INHOMOGENEOUS_EXTRACTED_CYCLE\n");							if (!ignoreAngioErrors) error = Error::InjectionTooShort;		break;
	case OSC_WRN_NONINJECTED_ANGIO:				  logger.DeveloperInfo(L"CrmErrors: OSC_WRN_NONINJECTED_ANGIO\n");				errorIsWarning = true;	if (!ignoreAngioErrors) error = Error::NoInjection;				break;
	case OSC_WRN_BADLY_INJECTED_ANGIO:			  logger.DeveloperInfo(L"CrmErrors: OSC_WRN_BADLY_INJECTED_ANGIO\n");				errorIsWarning = true;	if (!ignoreAngioErrors) error = Error::InjectionNotWellDefined;	break;
	case OSC_WRN_IMSIZE_INCOMP_WITH_PARAMS:		  logger.DeveloperWarning(L"CrmErrors: OSC_WRN_IMSIZE_INCOMP_WITH_PARAMS\n");		break;

	case OSC_ERR_DLL_VERSION:					  logger.DeveloperError(L"OSC_ERR_DLL_VERSION\n");						error = Error::Unknown;			break;
	case OSC_ERR_IMADYN_RANGE:					  logger.DeveloperError(L"OSC_ERR_IMADYN_RANGE\n");						error = Error::Unknown;			break;
	case OSC_ERR_INCORRECT_TIME:				  logger.DeveloperError(L"OSC_ERR_INCORRECT_TIME\n");					error = Error::Unknown;			break;
	case OSC_ERR_SYSTEM_INFO_MISSING:			  logger.DeveloperError(L"OSC_ERR_SYSTEM_INFO_MISSING\n");				error = Error::Unknown;			break;
	case OSC_ERR_INCORRECT_SHUTTERINFO:			  logger.DeveloperError(L"OSC_ERR_INCORRECT_SHUTTERINFO\n");			error = Error::Unknown;			break;
	case OSC_ERR_MEM_ALLOC:						  logger.DeveloperError(L"OSC_ERR_MEM_ALLOC\n");						error = Error::Unknown;			break;
	case OSC_ERR_CRITICAL_PARA:					  logger.DeveloperError(L"OSC_ERR_CRITICAL_PARA\n");					error = Error::Unknown;			break;
	case OSC_ERR_INCOHERENT_SUBFUNCTION_ARGUMENT: logger.DeveloperError(L"OSC_ERR_INCOHERENT_SUBFUNCTION_ARGUMENT\n");	error = Error::Unknown;			break;
	case OSC_ERR_DIALIB:						  logger.DeveloperError(L"OSC_ERR_DIALIB\n");							error = Error::Unknown;			break;
	case OSC_ERR_CBDLIB:						  logger.DeveloperError(L"OSC_ERR_CBDLIB\n");							error = Error::Unknown;			break;
	case OSC_ERR_INTERNAL:						  logger.DeveloperError(L"OSC_ERR_INTERNAL\n");							error = Error::Unknown;			break;
	case OSC_ERR_SWLIB:							  logger.DeveloperError(L"OSC_ERR_SWLIB\n");							error = Error::Unknown;			break;
	case OSC_ERR_LABELING:						  logger.DeveloperError(L"OSC_ERR_LABELING\n");							error = Error::Unknown;			break;
	case OSC_ERR_ORIGIN:						  logger.DeveloperError(L"OSC_ERR_ORIGIN\n");							error = Error::Unknown;			break;
	case OSC_ERR_SHORT_ANGIOSEQ:				  logger.DeveloperInfo(L"OSC_ERR_SHORT_ANGIOSEQ\n");					error = Error::AngioTooShort;	break;
	case OSC_ERR_NO_CARDIAC_CYCLE:				  logger.DeveloperInfo(L"OSC_ERR_NO_CARDIAC_CYCLE\n");					error = Error::NoHeartCycle;	break;
	case OSC_ERR_ANGIO_UNPROCESSED:				  logger.DeveloperError(L"OSC_ERR_ANGIO_UNPROCESSED\n");				error = Error::Unknown;			break;
	case OSC_ERR_EMPTY_POINTER:					  logger.DeveloperError(L"OSC_ERR_EMPTY_POINTER\n");					error = Error::Unknown;			break;
	case OSC_ERR_IMSIZEEXCEEDSALLOCATION:		  logger.DeveloperError(L"OSC_ERR_IMSIZEEXCEEDSALLOCATION\n");			error = Error::Unknown;			break;
	case OSC_ERR_PREVIOUS_FATAL_ERROR:			  logger.DeveloperError(L"OSC_ERR_PREVIOUS_FATAL_ERROR\n");				error = Error::Unknown;			break;
	case OSC_ERR_LAST:							  logger.DeveloperError(L"OSC_ERR_LAST");								error = Error::Unknown;			break;
	
	default:									  logger.DeveloperError(L"CrmErrors: Unknown OSC Error : %d\n", result);error = Error::Unknown;			break;
	};
	return errorIsWarning || error == Error::OK;
}
