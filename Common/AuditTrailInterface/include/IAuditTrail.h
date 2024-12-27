// Copyright Koninklijke Philips N.V. 2016
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <sscfts1.h>

#include "AuditTrailConstants.h"

namespace AuditTrail { namespace Sscf {

#include "AuditSecurityOptions.h"

#define SSCFTS1_DECLARE_INTERFACE
#include "IAuditTrail.hpp"
#undef SSCFTS1_DECLARE_INTERFACE

#define SSCFTS1_DECLARE_PROXY
#include "IAuditTrail.hpp"
#undef SSCFTS1_DECLARE_PROXY

}}
