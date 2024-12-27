// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "CoRegistrationIfrPullbackCycle.h"
#include "CoRegistrationPressureAndEcg.h"
#include "CoRegistrationIfrSpotCycle.h"
#include "CoRegistrationPdPaInfo.h"
#include "CoRegistrationError.h"
#include "CrmRecording.h"

#include "Sense/Common/Timer.h"
#include "Sense/System/IScheduler.h"

#include <vector>
#include <stdint.h>

namespace CoReg = CommonPlatform::CoRegistration;

namespace Pci { namespace Core {

class PullbackRecording : public CrmRecording
{
public:
	explicit PullbackRecording(int maxImagesInBuffer);

	void  addiFrPullbackCycle(const CoReg::CoRegistrationIfrPullbackCycle &cycle);
	void  addPressureAndEcg	 (const CoReg::CoRegistrationPressureAndEcg &pressAndEcg);
	void  addiFrSpotCycle	 (const CoReg::CoRegistrationIfrSpotCycle &cycle);
	void  addPdPaInfo		 (const CoReg::CoRegistrationPdPaInfo &info);

	void  setCoRegError		 (CommonPlatform::CoRegistration::CoRegistrationError error);

	int	  findClosestIfrPullbackCycleIndexAfter(double timestamp);
	bool  getIfrTrendValueAt(double timestamp, float& ifrValue);

	bool  getDistalFm   	(float& distalFm);

    const std::vector<CoReg::CoRegistrationIfrPullbackCycle>& getAllIfrPullbackCycles()const;
    const std::vector<CoReg::CoRegistrationIfrSpotCycle>&	  getAllIfrSpotCycles()const;
    const std::vector<std::pair<CoReg::CoRegistrationPressureAndEcg, CoReg::CoRegistrationPdPaInfo>>& getAllPressureAndEcg()const;

	void  clear();
	bool  isPullback()const;
	bool  errorOccuredDuringMeasurement()const;
	CoReg::CoRegistrationError getMeasurementError() const;

private:
    std::vector<CoReg::CoRegistrationIfrPullbackCycle> ifrPullback;
    std::vector<CoReg::CoRegistrationIfrSpotCycle>	   ifrSpotMeasurement;
	std::vector<std::pair<CoReg::CoRegistrationPressureAndEcg, CoReg::CoRegistrationPdPaInfo>> pressureAndEcg;

	CoReg::CoRegistrationPdPaInfo currentPdPaInfo;

	CommonPlatform::CoRegistration::CoRegistrationError errorState;
};
}}

