// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "GraphDisplay.h"
#include "XrayImage.h"
#include "Line.h"

#include <stdint.h>

namespace CommonPlatform { namespace CoRegistration {
    struct CoRegistrationIfrPullbackCycle;
}}

namespace Pci { namespace Core {

    class PullbackRecording;

    class FmTrendDisplay : public GraphDisplay
    {
    public:
        FmTrendDisplay(Sense::Control &parent, _locale_t &locale);
        ~FmTrendDisplay();

        void setTimestampStart(double timestamp);
		void setRecording(const std::shared_ptr<PullbackRecording> &recording);

		void setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image);
		void endLive();

		void setIfrPullbackCycle(const CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle &iFrCycle);
		void reset();
		bool isEmpty() const;

		void showRawLine(bool show);
		void setStaticIfrScale(float maxIfr, float minIfr);
		void enableDynamicScaling(bool enable);
    protected:
        virtual void render (Sense::IRenderer &renderer) const override;

    private:
		mutable Sense::TextureInput dot;

		struct RunSpan{
			double startTime;
			double endTime;
		};

		double lastReceivedTimestamp;

		mutable Sense::Drawing drawing;
        mutable std::vector<Line> pullbackTrendLineSegments;
		mutable std::vector<Line> pullbackRawLineSegments;

		std::vector<RunSpan> runs;
		std::vector<CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle> liveIfrPullbackCycles;
		mutable std::vector<Sense::Point> invalidCyclesTrend;
		mutable std::vector<Sense::Point> invalidCyclesRaw;

		bool rawLineVisible;
		bool useStaticScale;

		void update(const std::vector<CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle> &cycles);

		bool endOfDisplayReached(double timestamp) const;
		void rescaleToFullGraph(const std::shared_ptr<PullbackRecording> & recording);
		void setDynamicIfrScale(const std::vector<CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle> &cycles);
		void resetSampleSize();
		void rescaleTimeAxisTo(double maxTimeInSec);
	};

}}

