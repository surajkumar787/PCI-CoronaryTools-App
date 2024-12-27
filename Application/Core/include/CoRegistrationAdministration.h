// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <vector>
#include <functional>
#include <memory>

#include "CoRegistrationServerState.h"
#include "XrayImage.h"
#include "CoRegParameters.h"

namespace CommonPlatform
{
	class Log;
	namespace Xray 
	{
		struct XrayEpx;
		struct XrayStudy;
	}
}



namespace CommonPlatform { namespace CoRegistration {
	struct CoRegistrationIfrPullbackCycle;
	struct CoRegistrationIfrSpotCycle;
	struct CoRegistrationPressureAndEcg;
	struct CoRegistrationPdPaInfo;
	enum class CoRegistrationError;
}}

namespace Pci { namespace Core {

	struct CrmOverlay;
	class PullbackRecording;
	template<typename T>
	class PciReviewPlayer;
    class  PciEpx;

	class CoRegistrationAdministration
	{
	public:
		explicit CoRegistrationAdministration(PciReviewPlayer<PullbackRecording> &player);

		//xray related setters
		void setImage   (const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image, const std::shared_ptr<const CrmOverlay> &roadmap);
		void setStudy	(const CommonPlatform::Xray::XrayStudy &study);
		void endLive	(CommonPlatform::Xray::XrayImage::Type acquisitionType);

		//coreg related setters
		void setIfrPullbackCycle(const ::CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle &cycle);
		void setIfrSpotCycle	(const ::CommonPlatform::CoRegistration::CoRegistrationIfrSpotCycle &cycle);
		void setCoRegState		(const ::CommonPlatform::CoRegistration::CoRegistrationServerState &coRegState);
		void setPressureAndEcg	(const ::CommonPlatform::CoRegistration::CoRegistrationPressureAndEcg &pressAndEcg);
		void setPdPaInfo		(const ::CommonPlatform::CoRegistration::CoRegistrationPdPaInfo &pdPaInfo);
		void setCoRegError		(	   ::CommonPlatform::CoRegistration::CoRegistrationError error);
		void setSelectedCoRegRun(int runIndex);
		void setCoRegReady		(bool ready);

		std::function<void(const std::shared_ptr<PullbackRecording>& recording)> eventReviewRecording;

		bool isRecording() const;
		std::shared_ptr<PullbackRecording> getRecording() const;

		double getLatencyCompensation() const;

		CoRegistrationAdministration(const CoRegistrationAdministration&) = delete;
		CoRegistrationAdministration& operator=(const CoRegistrationAdministration&) = delete;

	private:

		const int maxImagesInBuffer;

		void startRecording();
		void stopRecording();
		void stopAndClearRecording();

		bool recordingStarted;
		bool coRegReady;

		::CommonPlatform::CoRegistration::CoRegistrationServerState currentState;

		//only one active recording at a time
		std::shared_ptr<PullbackRecording> recording;
		PciReviewPlayer<PullbackRecording> &player;

		CoRegParameters coRegParams;
		::CommonPlatform::Log&      log;


	};
}}

