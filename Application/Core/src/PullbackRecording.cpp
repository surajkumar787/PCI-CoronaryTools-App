// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <algorithm>
#include "PullbackRecording.h"

using namespace Pci::Core;
using namespace CommonPlatform::CoRegistration;

PullbackRecording::PullbackRecording(int maxImagesInBuffer) : CrmRecording(maxImagesInBuffer),
	errorState(CoRegistrationError::OK)
{
}

const std::vector<CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle>& Pci::Core::PullbackRecording::getAllIfrPullbackCycles()const
{
    return ifrPullback;
}

const std::vector<CommonPlatform::CoRegistration::CoRegistrationIfrSpotCycle>& Pci::Core::PullbackRecording::getAllIfrSpotCycles()const
{
	return ifrSpotMeasurement;
}

const std::vector<std::pair<CommonPlatform::CoRegistration::CoRegistrationPressureAndEcg, CommonPlatform::CoRegistration::CoRegistrationPdPaInfo>>& Pci::Core::PullbackRecording::getAllPressureAndEcg()const
{
	return pressureAndEcg;
}

CoReg::CoRegistrationError PullbackRecording::getMeasurementError() const
{
	return errorState;
}

bool PullbackRecording::errorOccuredDuringMeasurement()const
{
	return errorState != CoRegistrationError::OK;
}

void PullbackRecording::addiFrPullbackCycle(const CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle &cycle)
{
	ifrPullback.push_back(cycle);
}

void PullbackRecording::addiFrSpotCycle(const CommonPlatform::CoRegistration::CoRegistrationIfrSpotCycle &cycle)
{
	ifrSpotMeasurement.push_back(cycle);
}

void PullbackRecording::addPressureAndEcg(const CommonPlatform::CoRegistration::CoRegistrationPressureAndEcg &pressAndEcg)
{
	pressureAndEcg.emplace_back(pressAndEcg, currentPdPaInfo);
}

void PullbackRecording::addPdPaInfo(const CommonPlatform::CoRegistration::CoRegistrationPdPaInfo &info)
{
	currentPdPaInfo = info;
}

void PullbackRecording::setCoRegError(CommonPlatform::CoRegistration::CoRegistrationError error)
{
	errorState = error;
}

bool PullbackRecording::getIfrTrendValueAt(double timestamp, float &ifrValue)
{
	if(!ifrPullback.empty())
	{
		int i = findClosestIfrPullbackCycleIndexAfter(timestamp);

		if ( i < 0 )
		{
			// time stamp is before first IFR value
			return false;
		}

		if(i <  static_cast<int>(ifrPullback.size()))
		{
			if((std::abs( timestamp - ifrPullback[i].timestamp) < DBL_EPSILON) || i == 0)
			{
				ifrValue =  ifrPullback[i].ifrTrendValue;
			}
			else
			{
				double ifrDiff = ifrPullback[i].ifrTrendValue - ifrPullback[i-1].ifrTrendValue;
				double timestamps = ifrPullback[i].timestamp - ifrPullback[i-1].timestamp;

				double stepSize = ifrDiff / timestamps;
				double steps = timestamp - ifrPullback[i-1].timestamp;

				ifrValue =  static_cast<float>(ifrPullback[i-1].ifrTrendValue + (steps * stepSize));
			}
		}
		else 
		{
			// time stamp is after last IFR value
			ifrValue = ifrPullback.back().ifrTrendValue;
		}
		
		return true;
	}

	return false;
}

// Returns -1 when input timestamp is before timestamp of first IFR value
// Returns ifrPullback.size() when input timestamp is after timestamp of last IFR value
// Otherwise, returns the index of the first IFR value with a timestamp larger then input timestamp
int PullbackRecording::findClosestIfrPullbackCycleIndexAfter(double timestamp)
{
	if ( !ifrPullback.empty() && timestamp < ifrPullback.begin()->timestamp )
	{
		return -1;
	}

	auto lowerBoundTimestamp = std::lower_bound(ifrPullback.begin(), ifrPullback.end(), timestamp, 
		[&](const CoRegistrationIfrPullbackCycle &cycle, const double &time)
	{
		return cycle.timestamp < time;
	});

	return static_cast<int>(std::distance(ifrPullback.begin(), lowerBoundTimestamp));
}

bool PullbackRecording::getDistalFm(float& distalFm)
{
	if(!ifrPullback.empty() && ifrPullback[0].isValidCycle)
	{
		distalFm = ifrPullback[0].ifrTrendValue;
		return true;
	}

	return false;
}

void PullbackRecording::clear()
{
	ifrPullback.clear();
	ifrSpotMeasurement.clear();
	pressureAndEcg.clear();
	
	CrmRecording::clear();
}

bool PullbackRecording::isPullback() const
{
	return !ifrPullback.empty();
}
