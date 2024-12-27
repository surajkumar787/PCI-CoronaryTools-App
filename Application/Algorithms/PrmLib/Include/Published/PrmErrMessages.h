// ***************************************************************************
// * Copyright (c) 2002-2014 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef _PRMERRMESSAGES_H
#define _PRMERRMESSAGES_H

#include <ErrLib.h>

static CErrError __PrmErrorArray[] =
{
	//     Error Code         IsFatal            Message
	{PRM_WRN_INT_RANGE,         0,     "PRM_WRN_INT_RANGE: Int param value does not match with context range" }, 
	{PRM_WRN_FLOAT_RANGE,       0,     "PRM_WRN_FLOAT_RANGE: Float param value does not match with context range" },
	{PRM_WRN_IVEC_RANGE,        0,     "PRM_WRN_IVEC_RANGE: Int vector param value does not match with context range" },
	{PRM_WRN_FVEC_RANGE,        0,     "PRM_WRN_FVEC_RANGE: Float vector param value does not match with context range" },
	{PRM_WRN_NOT_IN_CREATE,     0,     "PRM_WRN_NOT_IN_CREATE: function should be called from PrmCreate via func-pointer" },
	{PRM_WRN_INCORRECT_PARANB,  0,     "PRM_WRN_INCORRECT_PARANB: Wrong parameter number" },
	{PRM_ERR_INDEX_RANGE,       1,     "PRM_ERR_INDEX_RANGE: Parameter index out of range" }, 
	{PRM_ERR_FIELD_RANGE,       1,     "PRM_ERR_FIELD_RANGE: Parameter field out of range" }, 
	{PRM_ERR_TYP_RANGE,         1,     "PRM_ERR_TYP_RANGE: Parameter type out of range" }, 
	{PRM_ERR_AKA_RANGE,         1,     "PRM_ERR_AKA_RANGE: Aka field of parameter context too long" }, 
	{PRM_ERR_TXT_RANGE,         1,     "PRM_ERR_TXT_RANGE: Txt field of parameter context too long" }, 
	{PRM_ERR_MINMAX_RANGE,      1,     "PRM_ERR_MINMAX_RANGE: Min, Max fields of parameter context incompatible" }, 
	{PRM_ERR_DEF_RANGE,         1,     "PRM_ERR_DEF_RANGE: Def field of parameter context out of range" }, 
	{PRM_ERR_ACC_RANGE,         1,     "PRM_ERR_ACC_RANGE: Acc field of parameter context out of range" },
	{PRM_ERR_OVERFLOW,          1,     "PRM_ERR_OVERFLOW: Parameter number overflow (at PrmCreate time)" },
	{PRM_ERR_MISSING_CTX,       1,     "PRM_ERR_MISSING_CTX: missing context initialisation" },
	{PRM_ERR_INTERNAL,          1,     "PRM_ERR_INTERNAL: internal error (development-level)" }
};

#endif   // _PRMERRMESSAGES_H