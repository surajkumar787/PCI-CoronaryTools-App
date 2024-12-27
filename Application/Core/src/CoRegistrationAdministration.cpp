// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegistrationAdministration.h"
#include "PullbackRecording.h"
#include "CoRegistrationServerState.h"
#include "PciReviewPlayer.h"
#include "PciEpx.h"
#include "Log.h"


using namespace CommonPlatform::CoRegistration;
namespace Pci {	namespace Core {

static const int MaxImageInBuffer = 900;

CoRegistrationAdministration::CoRegistrationAdministration(PciReviewPlayer<PullbackRecording> &coRegPlayer) :
	maxImagesInBuffer       (MaxImageInBuffer),
	recordingStarted        (false),
	coRegReady              (false),
	currentState			(),
	recording				(nullptr),
	player                  (coRegPlayer),
	coRegParams             (L"../Configuration/settings.ini"),
	log(::CommonPlatform::Logger(::CommonPlatform::LoggerType::CoronaryRoadmap))
{
	log.DeveloperInfo(	L"CoRegistrationAdministration [Co-registration latency compensation=%g sec, XrayLatency=%g sec, PressureLatency=%g sec]",
						getLatencyCompensation(), coRegParams.getXrayLatency(), coRegParams.getPressureLatency());
}

void CoRegistrationAdministration::setCoRegState(const CoRegistrationServerState &newState)
{
	//always stop the review when switching states.
	player.stopReview();

	log.DeveloperInfo(	L"CoRegistrationAdministration::setCoRegState. System state changed [Current=%d, New=%d]",
						currentState.systemState, newState.systemState);

	if (((newState.systemState == CoRegistrationServerState::SystemState::Live) ||
		(newState.systemState == CoRegistrationServerState::SystemState::Idle)) ||
		((newState.systemState == CoRegistrationServerState::SystemState::Freeze) && recording))
	{
		stopAndClearRecording();
	}

	if ((newState.systemState == CoRegistrationServerState::SystemState::Pullback) ||
		(newState.systemState == CoRegistrationServerState::SystemState::SpotMeasurement))
	{
		if (currentState.systemState == CoRegistrationServerState::SystemState::Live)
		{
			startRecording();
		}
	}
	else
	{
		stopRecording();
	}

	if ((newState.systemState == CoRegistrationServerState::SystemState::Review) && recording)
	{
		// Only use recorded images that have time stamp that is in range of iFR pullback time stamps 
		if ( newState.fmState == CoRegistrationServerState::FmState::IFRPullback )
		{
			const auto& pressureData = recording->getAllPressureAndEcg();
			if (!pressureData.empty())
			{
				double startTimePressureData = pressureData.front().first.timestamp;
				recording->removeImagesBefore(startTimePressureData);
			}
		}

		if (eventReviewRecording)
		{
			eventReviewRecording(recording);
		}

		if ((newState.fmState == CoRegistrationServerState::FmState::IFRPullback) && (!recording->errorOccuredDuringMeasurement()))
		{
			player.startReview();
		}
	}

	currentState = newState;
}

void CoRegistrationAdministration::setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image,const std::shared_ptr<const CrmOverlay> &roadmap)
{
	player.stopReview();

	if ((recording && recordingStarted) && (currentState.fmState == CoRegistrationServerState::FmState::IFRPullback))
	{
		recording->addImage(image, roadmap);
	}
}

void CoRegistrationAdministration::endLive(CommonPlatform::Xray::XrayImage::Type acquisitionType)
{
	if ( currentState.fmState == CoRegistrationServerState::FmState::IFRPullback )
	{
		//add empty image/Roadmap to split subsequent fluoro runs in buffer.
		if ((currentState.systemState == CoRegistrationServerState::SystemState::Pullback) &&
			recording && recordingStarted && (acquisitionType == CommonPlatform::Xray::XrayImage::Type::Fluoro))
		{
			recording->addImage(nullptr, nullptr);
		}
	
		//resume review after end of run
		if ((currentState.systemState == CoRegistrationServerState::SystemState::Review)&&
			(recording && (!recording->errorOccuredDuringMeasurement())))
		{
			player.startReview();
		}
	}
}

void CoRegistrationAdministration::setStudy(const CommonPlatform::Xray::XrayStudy &)
{
	stopAndClearRecording();
}

void CoRegistrationAdministration::setIfrPullbackCycle(const CoRegistrationIfrPullbackCycle &cycle)
{
	// iFR pullback is a special case, filtered measurements will be received after recording stopped.
	// Todo: in a future design, IFR cycles can be recorded in individual runs, when for filtered data the run index is always reliable.
	if (coRegReady && recording && cycle.isFiltered)
	{
		// Put data in recording once. Filtered iFR cycles can be sent multiple times
		if ( recording->getAllIfrPullbackCycles().empty() ||
			(cycle.timestamp > recording->getAllIfrPullbackCycles().back().timestamp))
		{
			recording->addiFrPullbackCycle(cycle);
		}

		if (currentState.systemState == CoRegistrationServerState::SystemState::Review)
		{
			if (eventReviewRecording)
			{
				eventReviewRecording(recording);
			}
		}
	}
}

void CoRegistrationAdministration::setIfrSpotCycle(const CoRegistrationIfrSpotCycle &cycle)
{
	if (coRegReady && recording && recordingStarted)
	{
		recording->addiFrSpotCycle(cycle);
	}
}

void CoRegistrationAdministration::setPressureAndEcg(const CoRegistrationPressureAndEcg &pressAndEcg)
{
	if (coRegReady && recording && recordingStarted)
	{
		recording->addPressureAndEcg(pressAndEcg);
	}
}

void CoRegistrationAdministration::setPdPaInfo(const CoRegistrationPdPaInfo &pdPaInfo)
{
	if (coRegReady && recording && recordingStarted)
	{
		recording->addPdPaInfo(pdPaInfo);
	}
}

void CoRegistrationAdministration::startRecording()
{
	log.DeveloperInfo(L"CoRegistrationAdministration start recording");
	recording = std::make_shared<PullbackRecording>(maxImagesInBuffer);
	recordingStarted = true;
}

void CoRegistrationAdministration::stopRecording()
{
	log.DeveloperInfo(L"CoRegistrationAdministration stop recording");
	recordingStarted = false;
}

void CoRegistrationAdministration::stopAndClearRecording()
{
	stopRecording();
	player.stopReview();
	if (recording)
	{
		recording->clear();
		recording = nullptr;
	}
}

bool CoRegistrationAdministration::isRecording() const
{
	return recordingStarted;
}

std::shared_ptr<PullbackRecording> CoRegistrationAdministration::getRecording() const
{
	return recording;
}

void CoRegistrationAdministration::setCoRegReady(bool ready)
{
	if (!ready)
	{
		stopAndClearRecording();
	}

	coRegReady = ready;
}

void CoRegistrationAdministration::setSelectedCoRegRun(int runIndex)
{
	if (coRegReady && recording)
	{
		if (isRecording())
		{
			recording->setRunIndex(runIndex);
		}
		else if (runIndex != recording->getRunIndex())
		{
			stopAndClearRecording();
		}
	}
}

void CoRegistrationAdministration::setCoRegError(::CommonPlatform::CoRegistration::CoRegistrationError error)
{
	if (coRegReady && recording && recordingStarted)
	{
		recording->setCoRegError(error);
	}
}

double Pci::Core::CoRegistrationAdministration::getLatencyCompensation() const
{
	return static_cast<double>(coRegParams.getXrayLatency() - coRegParams.getPressureLatency());
}

}}