// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "GraphDisplay.h"
#include "LineSubsampler.h"
#include "Line.h"
#include "Log.h"
#include <stdint.h>

namespace CommonPlatform { namespace CoRegistration {
    struct CoRegistrationPressureAndEcg;
	struct CoRegistrationPdPaInfo;
	struct CoRegistrationIfrPullbackCycle;
	struct CoRegistrationIfrSpotCycle;
}}

namespace Pci { namespace Core {

    class PullbackRecording;

    class PressureDisplay : public GraphDisplay
    {
    public:
		PressureDisplay(Sense::Control &parent, _locale_t &locale);
        ~PressureDisplay();

        void   setTimestampStart(double timestamp);
		double getTimestampStart() const;

		void reset();
		bool isEmpty() const;

		void showPdPaRatio();
		void hidePressure();

		void showWaveFreePeriods(bool show);
		void setStaticPressureScale(float maxPressure, float minPressure);
		void setRacerBarColor(const Sense::Color &color);
        
		bool addLiveSample(const ::CommonPlatform::CoRegistration::CoRegistrationPressureAndEcg &sample, const ::CommonPlatform::CoRegistration::CoRegistrationPdPaInfo &pdPaInfo);

		void setRecording(const std::shared_ptr<PullbackRecording> &recording);
		void setRecording(const std::shared_ptr<PullbackRecording> &recording, bool showFullRecording);

		void resetScale();

		virtual void onResize() override;

    protected:
        virtual void render (Sense::IRenderer &renderer) const override;

    private:
		::CommonPlatform::Log& m_log;

		struct WaveFreePeriod{
			double start;
			double end;
			bool   validCycle;
		};
		
		mutable Sense::Drawing m_pressureDrawing;

		LineSubsampler m_paSubsampler;
		LineSubsampler m_pdSubsampler;
		LineSubsampler m_paRatioSubsampler;
		LineSubsampler m_pdRatioSubsampler;
		LineSubsampler m_waveFreePaSubsampler;
		LineSubsampler m_waveFreePdSubsampler;

		mutable Line m_paLine;
		mutable Line m_pdLine;
		mutable Line m_paRatioLine;
		mutable Line m_pdRatioLine;

		mutable std::vector<Sense::Point> m_invalidCycles;

		int m_waveFreeCount;
		mutable std::vector<Line> m_waveFreePa;
		mutable std::vector<Line> m_waveFreePd;

		std::shared_ptr<PullbackRecording> m_recording;
		
		bool m_isFfr;
		bool m_pressureVisible;
		bool m_waveFreeVisible;
		bool m_resetStartTimeStamp;

        Sense::Point m_lastPullbackLinePoint;
		Sense::Color m_racerBarColor;

		TimedPoint   transformValueToTimedPoint(double timestamp, float value);

		void addPaPoint		   (const TimedPoint &paPoint);
		void addPdPoint		   (const TimedPoint &pdPoint);
		void addPaRatioPoint   (const TimedPoint &paRatioPoint);
		void addPdRatioPoint   (const TimedPoint &pdRatioPoint);
		void addWaveFreePaPoint(const TimedPoint &waveFreePoint);
		void addWaveFreePdPoint(const TimedPoint &waveFreePoint);

		void subSampleWaveFreePeriods(const std::pair<CommonPlatform::CoRegistration::CoRegistrationPressureAndEcg, CommonPlatform::CoRegistration::CoRegistrationPdPaInfo> &pressureData);
		Line createEmptyWaveFreePeriodLine();

		bool endOfDisplayReached(const TimedPoint &pa, const TimedPoint &pd, const TimedPoint &paRatio, const TimedPoint &pdRatio)const;

		std::vector<WaveFreePeriod> convertIfrSpotCyclesToWaveFree(const std::vector<CommonPlatform::CoRegistration::CoRegistrationIfrSpotCycle> &cycles);
		std::vector<WaveFreePeriod> convertIfrPullbackCyclesToWaveFree(const std::vector<CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle> &cycles);

		void resetTemporaryLines();
	};

}}

