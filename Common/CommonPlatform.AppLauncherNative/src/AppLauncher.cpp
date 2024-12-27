// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "AppLauncher.h"

namespace CommonPlatform { namespace AppLauncher { namespace Native
{

#define SSCFTS1_DEFINE_INTERFACE
#include "IAppLauncher.hpp"
#undef SSCFTS1_DEFINE_INTERFACE

#define SSCFTS1_DEFINE_PROXY
#include "IAppLauncher.hpp"
#undef SSCFTS1_DEFINE_PROXY


#define SSCFTS1_DEFINE_INTERFACE
#include "IAppExitNotifier.hpp"
#undef SSCFTS1_DEFINE_INTERFACE

#define SSCFTS1_DEFINE_PROXY
#include "IAppExitNotifier.hpp"
#undef SSCFTS1_DEFINE_PROXY

}}}
