// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "Applications.h"

//This class provides interface to AppLauncherClient to restart or shutdown system as per state change
class IAppLauncherClient
{
public :
	virtual  ~IAppLauncherClient(){}
	virtual void ShutdownSystem() const=0;// this method shuts down the application , when state is received as "ShutDown"
	virtual void RebootSystem() const = 0;// this method reboots the application , when state is received as "Restart"
	virtual void WarmRestart() const =0;// this method restarts the application , when state is received as "WarmRestart"
};