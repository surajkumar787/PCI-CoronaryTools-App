// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <algorithm>

#include "PressureDisplay.h"
#include "ExperienceIdentity.h"
#include "PullbackRecording.h"
#include "Timing.h" 

#include <windows.h>

using namespace Sense;
using namespace Sense::ExperienceIdentity;
using namespace CommonPlatform::CoRegistration;

namespace Pci { namespace Core {

static const int	MaxPressure	   = 200;
static const int	MinPressure	   = 0;
static const int	PressureScale  = 50;
static const int	MaxSeconds	   = 5;
static const int	MinSeconds	   = 0;
static const int	SecondScale	   = 1;

// axis scale on the Allura UI goes from 0 to 5 over 240 pixels, this serves as a basis for pressure graphs which are wider
static const double PixelsPerSecond = 240.0 / 5.0;

static const Color  PaLineColor    = Color(1,0,0);
static const Color  PdLineColor    = Color(1,1,0);
static const Color  WaveFreeColor  = Color(0,1,0);

PressureDisplay::PressureDisplay(Control &parent, _locale_t &locale ) :
	GraphDisplay(parent, MaxPressure, MinPressure, PressureScale, MaxSeconds, MinSeconds, SecondScale, locale),
	m_log(CommonPlatform::Logger(::CommonPlatform::LoggerType::CoronaryRoadmap)),
	m_pressureDrawing(gpu),
	m_paSubsampler([&](const TimedPoint &point){ addPaPoint(point); }),
	m_pdSubsampler([&](const TimedPoint &point){ addPdPoint(point); }),
	m_paRatioSubsampler([&](const TimedPoint &point){ addPaRatioPoint(point); }),
	m_pdRatioSubsampler([&](const TimedPoint &point){ addPdRatioPoint(point); }),
	m_waveFreePaSubsampler([&](const TimedPoint &point){ addWaveFreePaPoint(point); }),
	m_waveFreePdSubsampler([&](const TimedPoint &point){ addWaveFreePdPoint(point); }),
	m_paLine(),
	m_pdLine(),
	m_paRatioLine(),
	m_pdRatioLine(),
	m_invalidCycles(),
	m_waveFreeCount(0),
	m_waveFreePa(),
	m_waveFreePd(),
	m_recording(nullptr),
	m_isFfr(false),
	m_pressureVisible(true),
	m_waveFreeVisible(true),
	m_resetStartTimeStamp(true),
	m_lastPullbackLinePoint(),
	m_racerBarColor(Palette::Black)
{
	m_paLine.setColor(PaLineColor);
	m_pdLine.setColor(PdLineColor);

	m_paRatioLine.setColor(PaLineColor);
	m_pdRatioLine.setColor(PdLineColor);

	onResize();
}

PressureDisplay::~PressureDisplay()
{
}

void PressureDisplay::onResize()
{
	GraphDisplay::onResize();

	maxX = bounds.width() / PixelsPerSecond;
}

void PressureDisplay::render(IRenderer &renderer) const
{
	GraphDisplay::render(renderer);
    Space space = getSpace();

	if(isInvalidated())
	{
		m_pressureDrawing.open(space/*,L"pressureLine"*/);
		
		if(m_pressureVisible)
		{
			m_paLine.drawPolyLine(m_pressureDrawing);
			m_pdLine.drawPolyLine(m_pressureDrawing);

			if(m_waveFreeVisible)
			{
				for(const Line &segPa : m_waveFreePa)
				{
					segPa.drawPolyLine(m_pressureDrawing);
				}

				for(const Line &segPd : m_waveFreePd)
				{
					segPd.drawPolyLine(m_pressureDrawing);
				}
			}
		}

		if(m_isFfr)
		{
			m_paRatioLine.drawPolyLine(m_pressureDrawing);
			m_pdRatioLine.drawPolyLine(m_pressureDrawing);
		}

		//the racer bar
		if(m_paLine.getPosition() > 0)
		{

			Rectangle::draw(m_pressureDrawing, Rect(m_paLine.getLastPoint().x, bounds.top, m_paLine.getLastPoint().x +10, bounds.bottom - 1), m_racerBarColor);
		}

		cursor.drawPolyLine(m_pressureDrawing);
		
        m_pressureDrawing.close();
    }

	m_pressureDrawing.render(space, renderer);
	drawInvalidCycleCrosses(renderer, m_invalidCycles, true);

	Control::render(renderer);
}

void PressureDisplay::setRacerBarColor(const Sense::Color &color)
{
	m_racerBarColor = color;
}


void PressureDisplay::setTimestampStart(double timestamp)
{
    startOffset = timestamp;
}

void PressureDisplay::reset()
{
	onResize();

	resetTemporaryLines();

	m_invalidCycles.clear();

	cursor.clear();

	m_isFfr = false;
	m_waveFreeVisible = true;

	maxX = bounds.width() / PixelsPerSecond;
	resetScale();
	invalidate();
}

bool PressureDisplay::addLiveSample(const CoRegistrationPressureAndEcg &sample, const CoRegistrationPdPaInfo &pdPaInfo)
{
	if( m_resetStartTimeStamp)
	{
		setTimestampStart(sample.timestamp);
		m_resetStartTimeStamp = false;
	}

	TimedPoint pa = transformValueToTimedPoint(sample.timestamp, sample.paValue);
	TimedPoint pd = transformValueToTimedPoint(sample.timestamp, sample.pdValue);

	//TODO: the pdPaInfo timestamp is not correct. use the one from the sample (FM2.5 build 40)
	TimedPoint paRatio = transformValueToTimedPoint(sample.timestamp, pdPaInfo.mapPaValue);
	TimedPoint pdRatio = transformValueToTimedPoint(sample.timestamp, pdPaInfo.mapPdValue);

	bool endReached = endOfDisplayReached(pa, pd, paRatio, pdRatio);

	if(endReached)
	{
		m_paLine.wrap();
		m_pdLine.wrap();
		m_paRatioLine.wrap();
		m_pdRatioLine.wrap();

		m_paSubsampler.reset();
		m_pdSubsampler.reset();
		m_paRatioSubsampler.reset();
		m_pdRatioSubsampler.reset();

		setTimestampStart(sample.timestamp);

		return endReached;
	}
	
	//TODO: the pdPaInfo timestamp is not correct. use the one from the sample (FM2.5 build 40)
	paRatio = transformValueToTimedPoint(sample.timestamp, pdPaInfo.mapPaValue);
	pdRatio = transformValueToTimedPoint(sample.timestamp, pdPaInfo.mapPdValue);

	auto paSubsampleNeeded		= m_paLine.getPosition()		> 0 && abs(pa.point.x		- m_paLine.getLastPoint().x)		< 1;
	auto pdSubsampleNeeded		= m_pdLine.getPosition()		> 0 && abs(pd.point.x		- m_pdLine.getLastPoint().x)		< 1;
	auto paRatioSubsampleNeeded = m_paRatioLine.getPosition() > 0 && abs(paRatio.point.x  - m_paRatioLine.getLastPoint().x) < 1;
	auto pdRatioSubsampleNeeded = m_pdRatioLine.getPosition() > 0 && abs(pdRatio.point.x  - m_pdRatioLine.getLastPoint().x) < 1;

	if(paSubsampleNeeded)		m_paSubsampler.start();	   else m_paSubsampler.stop();
	if(pdSubsampleNeeded)		m_pdSubsampler.start();	   else m_pdSubsampler.stop();
	if(paRatioSubsampleNeeded)	m_paRatioSubsampler.start(); else m_paRatioSubsampler.stop();
	if(pdRatioSubsampleNeeded)	m_pdRatioSubsampler.start(); else m_pdRatioSubsampler.stop();

	m_paSubsampler.addPoint(pa);
	m_pdSubsampler.addPoint(pd);
	m_paRatioSubsampler.addPoint(paRatio);
	m_pdRatioSubsampler.addPoint(pdRatio);

	return endReached;
}


void PressureDisplay::addPaPoint(const TimedPoint &paPoint)
{
	m_paLine.addPoint(paPoint.point);
	invalidate();
}

void PressureDisplay::addPdPoint(const TimedPoint &pdPoint)
{
	m_pdLine.addPoint(pdPoint.point);
	invalidate();
}

void PressureDisplay::addPaRatioPoint(const TimedPoint &paRatioPoint)
{
	m_paRatioLine.addPoint(paRatioPoint.point);
	invalidate();
}

void PressureDisplay::addPdRatioPoint(const TimedPoint &pdRatioPoint)
{
	m_pdRatioLine.addPoint(pdRatioPoint.point);
	invalidate();
}

void PressureDisplay::addWaveFreePaPoint(const TimedPoint &waveFreePoint)
{
	m_waveFreePa[m_waveFreeCount].addPoint(waveFreePoint.point);
}

void PressureDisplay::addWaveFreePdPoint(const TimedPoint &waveFreePoint)
{
	m_waveFreePd[m_waveFreeCount].addPoint(waveFreePoint.point);
}

void PressureDisplay::showPdPaRatio()
{
	m_isFfr = true;
}

void PressureDisplay::setRecording(const std::shared_ptr<PullbackRecording> &newRecording, bool showFullRecording)
{
	auto pressAndEcg = newRecording->getAllPressureAndEcg();

	// Clear subsampler internals and temporary lines
	resetTemporaryLines();

	if(pressAndEcg.empty())
	{	
		return;
	}

	if(showFullRecording)
	{
		setTimestampStart(pressAndEcg[0].first.timestamp);

		maxX = pressAndEcg.back().first.timestamp - startOffset;
		resetScale();

	}

	std::vector<WaveFreePeriod> waveFreePeriods;
	if(newRecording->isPullback())
	{
		waveFreePeriods = convertIfrPullbackCyclesToWaveFree(newRecording->getAllIfrPullbackCycles());
	}
	else
	{
		waveFreePeriods = convertIfrSpotCyclesToWaveFree(newRecording->getAllIfrSpotCycles());
	}

	if(waveFreePeriods.empty())
	{
		for(const auto &pressurePoint : pressAndEcg)
		{
			if(pressurePoint.first.timestamp >= startOffset)
			{
				bool endOfDisplayReached = addLiveSample(pressurePoint.first, pressurePoint.second);

				if(!showFullRecording && endOfDisplayReached)
				{
					break;
				}
			}
		}
	}
	else
	{
		m_waveFreeCount = 0;

		WaveFreePeriod currentWaveFreePeriod = waveFreePeriods[m_waveFreeCount];
		m_waveFreePa.resize(waveFreePeriods.size(), createEmptyWaveFreePeriodLine());
		m_waveFreePd.resize(waveFreePeriods.size(), createEmptyWaveFreePeriodLine());

		bool invalidCycleAddedForThisPeriod = false;

		for(const auto &pressureData : pressAndEcg)
		{
			if(pressureData.first.timestamp >= startOffset)
			{
				bool endOfDisplayReached = addLiveSample(pressureData.first, pressureData.second);

				if (!showFullRecording && endOfDisplayReached)
				{
					break;
				}

				if((pressureData.first.timestamp >= currentWaveFreePeriod.start) && (pressureData.first.timestamp <= currentWaveFreePeriod.end))
				{
					if(!currentWaveFreePeriod.validCycle && !invalidCycleAddedForThisPeriod)
					{
						m_invalidCycles.push_back(transformDoubleToPoint(currentWaveFreePeriod.start, static_cast<double>(pressureData.second.mapPaValue)));
						invalidCycleAddedForThisPeriod = true;
					}
					
					subSampleWaveFreePeriods(pressureData);
				}
				else if(pressureData.first.timestamp > currentWaveFreePeriod.end)
				{	
					m_waveFreePaSubsampler.reset();
					m_waveFreePdSubsampler.reset();
					invalidCycleAddedForThisPeriod = false;

					m_waveFreeCount++;
					if(m_waveFreeCount < static_cast<int>(waveFreePeriods.size()))
					{
						currentWaveFreePeriod = waveFreePeriods[m_waveFreeCount];
					}
				}
			}
		}
	}
}

void PressureDisplay::setRecording(const std::shared_ptr<PullbackRecording> &recording)
{
	setRecording(recording, false);
}

Line PressureDisplay::createEmptyWaveFreePeriodLine()
{
	Line wavefreeSegment;
	wavefreeSegment.setColor(WaveFreeColor);

	return wavefreeSegment;
}

void PressureDisplay::subSampleWaveFreePeriods(const std::pair<CoRegistrationPressureAndEcg, CoRegistrationPdPaInfo> &pressureData)
{
	if(m_waveFreeCount >= m_waveFreePa.size() && m_waveFreeCount >= m_waveFreePd.size())
	{
		m_log.DeveloperWarning(L"PressureDisplay: Could not subsample WaveFree periods, incorrect size");
		return;
	}

	auto waveFreePaPoint = transformValueToTimedPoint(pressureData.first.timestamp, pressureData.first.paValue);
	auto waveFreePdPoint = transformValueToTimedPoint(pressureData.first.timestamp, pressureData.first.pdValue);

	auto paSubsampleNeeded = m_waveFreePa[m_waveFreeCount].getLength() > 0 && abs(waveFreePaPoint.point.x - m_waveFreePa[m_waveFreeCount].getLastPoint().x) < 1;
	auto pdSubsampleNeeded = m_waveFreePd[m_waveFreeCount].getLength() > 0 && abs(waveFreePdPoint.point.x - m_waveFreePd[m_waveFreeCount].getLastPoint().x) < 1;

	if(paSubsampleNeeded)
	{ 
		m_waveFreePaSubsampler.start();
	}
	else 
	{
		m_waveFreePaSubsampler.stop();
	}

	if(pdSubsampleNeeded) 
	{
		m_waveFreePdSubsampler.start();
	}
	else 
	{
			m_waveFreePdSubsampler.stop();
	}

	m_waveFreePaSubsampler.addPoint(waveFreePaPoint);
	m_waveFreePdSubsampler.addPoint(waveFreePdPoint);
}

TimedPoint PressureDisplay::transformValueToTimedPoint(double timestamp, float value)
{
	TimedPoint timedPoint;
	timedPoint.point	 = transformDoubleToPoint(timestamp, static_cast<double>(value));
	timedPoint.timestamp = timestamp;

	return timedPoint;
}

std::vector<PressureDisplay::WaveFreePeriod> PressureDisplay::convertIfrSpotCyclesToWaveFree(const std::vector<CoRegistrationIfrSpotCycle> &cycles)
{
	std::vector<WaveFreePeriod> waveFreePeriods(cycles.size());
	std::transform(cycles.begin(), cycles.end(), waveFreePeriods.begin(), 
		[&](const CoRegistrationIfrSpotCycle &cycle)
			{
				WaveFreePeriod period = {cycle.waveFreeStartTime, cycle.waveFreeEndTime, cycle.isValidCycle}; 
				return period;
			});

	return waveFreePeriods;
}

std::vector<PressureDisplay::WaveFreePeriod> PressureDisplay::convertIfrPullbackCyclesToWaveFree(const std::vector<CoRegistrationIfrPullbackCycle> &cycles)
{
	std::vector<WaveFreePeriod> waveFreePeriods(cycles.size());

	std::transform(cycles.begin(), cycles.end(), waveFreePeriods.begin(), 
		[&](const CoRegistrationIfrPullbackCycle &cycle)
	{
		WaveFreePeriod period = {cycle.waveFreeStartTime, cycle.waveFreeEndTime, cycle.isValidCycle}; 
		return period;
	});

	return waveFreePeriods;
}

bool PressureDisplay::endOfDisplayReached(const TimedPoint &pa, const TimedPoint &pd, const TimedPoint &paRatio, const TimedPoint &pdRatio) const
{
	return ((pa.point.x > bounds.right) || 
		    (pd.point.x > bounds.right) || 
		    (paRatio.point.x > bounds.right) || 
		    (pdRatio.point.x > bounds.right));
}

void PressureDisplay::hidePressure()
{
	m_pressureVisible = false;
}

double PressureDisplay::getTimestampStart() const
{
	return startOffset;
}

void PressureDisplay::showWaveFreePeriods(bool show)
{
	m_waveFreeVisible = show;
	invalidate();
}

static const double EqualPressureThreshold = 0.01;

void PressureDisplay::setStaticPressureScale(float maxPressure, float minPressure)
{
	double maxPress = static_cast<double> (maxPressure);
	double minPress = static_cast<double> (minPressure);

	if( ( std::abs(maxY - maxPress) > EqualPressureThreshold) ||
		( std::abs(minY - minPress) > EqualPressureThreshold))
	{
		maxY = maxPress;
		minY = minPress;

		resetTemporaryLines();
		m_invalidCycles.clear();

		m_resetStartTimeStamp = true;
		resetScale();
		invalidate();	
	}
}

void PressureDisplay::resetScale()
{
	horizontalSampleSize = 0.0; //reset the sample size so it gets recalculated
	verticalSampleSize	 = 0.0;
}

void PressureDisplay::resetTemporaryLines()
{
	m_pdSubsampler.reset();
	m_paSubsampler.reset();
	m_pdRatioSubsampler.reset();
	m_paRatioSubsampler.reset();
	m_waveFreePaSubsampler.reset();
	m_waveFreePdSubsampler.reset();


	m_paLine.reset();
	m_pdLine.reset();
	m_paRatioLine.reset();
	m_pdRatioLine.reset();
	m_waveFreePa.clear();
	m_waveFreePd.clear();
	m_waveFreeCount = 0;
}

bool PressureDisplay::isEmpty() const
{
	return (m_paLine.getLength() == 0) && ( m_pdLine.getLength() == 0) &&
		   (m_paRatioLine.getLength() == 0) && (m_pdRatioLine.getLength() == 0) &&
		   (m_waveFreePa.size() == 0) && (m_waveFreePd.size() == 0) &&
		   (m_invalidCycles.size() == 0) &&
		   !m_paSubsampler.isStarted() && !m_pdSubsampler.isStarted() &&
		   !m_paRatioSubsampler.isStarted() && !m_pdRatioSubsampler.isStarted() &&
		   !m_waveFreePaSubsampler.isStarted() && !m_waveFreePdSubsampler.isStarted();
}

}}