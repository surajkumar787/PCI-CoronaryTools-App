// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "ITsmSource.h"

#include <CwisAlliance5.h>
#include <CwisToolsAlliance5.h>
#include <Sense/Common/Timer.h>

#include "Log.h"

namespace CommonPlatform { namespace TSM
{
	class TsmPage;

	class TsmSource : public ITsmSource
	{
	public:
		TsmSource(Sense::ITimerScheduler& scheduler, const wchar_t* description);
		~TsmSource();

		void connect(const wchar_t* host, unsigned short port);
		virtual void setPage(TsmPage* page) override;

		std::function<void(bool functionsAvailable)> eventFunctionAvailabilityChanged;
		std::function<void(Cwis::CwisConnectionType status)> eventConnectionChanged;

	private:
		void onButtonPressed();
		bool setData(const TsmPage& page);
		bool resetData();
		bool hasBeenActivated;

		bool hasClickableButtons() const;

		TsmPage splitButtonCenterTextIfNeeded(const TsmPage& page);

		void timerTick();

		CommonPlatform::Log& log;

		Cwis::CwisAlliance5Workstation      cwisAlliance;
		Cwis::CwisToolsAlliance5Workstation cwisTools;

		TsmPage* currentPage;

		Sense::Timer buttonRepeatTimer;
		unsigned int buttonRepeat;
	};

}}