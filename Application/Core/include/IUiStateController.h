// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <Sense.h>
#include "ViewState.h"

namespace Pci {	namespace Core
{
	class IUiStateController
	{
	public:
		virtual ~IUiStateController() {};

		virtual void switchState(ViewState nextState) = 0;
		virtual void setStbRegionOfInterest(Sense::Rect roi) = 0;

		virtual void startReplayRun(bool loop) = 0;
		virtual void stopReplayRun() = 0;
		virtual bool isReplayActive() = 0;

	};
}}

