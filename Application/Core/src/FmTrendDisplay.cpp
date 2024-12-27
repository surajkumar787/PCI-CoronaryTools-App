// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "FmTrendDisplay.h"
#include "ExperienceIdentity.h"
#include "PullbackRecording.h"
#include "Timing.h"

#include <windows.h>
#include <algorithm>

using namespace Sense;
using namespace Sense::ExperienceIdentity;
using namespace CommonPlatform::CoRegistration;

namespace Pci { namespace Core {

static const double MaxIfr							= 1.0;
static const double MinIfr							= 0.4;
static const double IfrScale						= 0.1;
static const double ShortRecordingMaxSeconds		= 10.0;
static const double LongRecordingMaxSeconds			= 30.0;
static const int    MinSeconds						= 0;
static const double	ShortRecordingScaleSeconds		= 2.0;
static const double	LongRecordingScaleSeconds		= 5.0;
static const double DefaultRecordingScaleSeconds	= 10.0;
static const Color  PullbackLineColor				= Color(0.0, 1.0, 1.0);
static const Color  InvalidCycleColor				= Color(1.0, 0.5, 0.0);
static const Color  XrayRunActiveColor				= Color(0.4, 0.4, 0.4);
static const int	SecondsAddedAtEndOfDisplay		= 15;
static const double	OneSecondMargin					= 1.0;
static const double LowValueIfrGraph				= 0.4;
static const double MediumValueIfrGraph				= 0.6;
static const double HighValueIfrGraph				= 0.8;
static const double VerticalAxesOffset				= 31.0; // from GraphDisplay with additional pixel

FmTrendDisplay::FmTrendDisplay(Control &parent, _locale_t &locale) :
	GraphDisplay(parent, MaxIfr, MinIfr, IfrScale, LongRecordingMaxSeconds, MinSeconds, LongRecordingScaleSeconds, locale, true),
	dot(gpu),  
	lastReceivedTimestamp(0.0),	
	drawing(gpu),
	pullbackTrendLineSegments(),
	pullbackRawLineSegments(),
	runs(),
	liveIfrPullbackCycles(),
	invalidCyclesTrend(),
	invalidCyclesRaw(),
	rawLineVisible(true),
	useStaticScale(false)
{
	RunSpan span;
	span.startTime = 0.0;
	span.endTime = 0.0;

	runs.push_back(span);
}

FmTrendDisplay::~FmTrendDisplay()
{
}


void FmTrendDisplay::render(IRenderer &renderer) const
{
	GraphDisplay::render(renderer);

    Space space = getSpace();

	if (!dot.isValid())
	{
		unsigned pixels[] = { 0xFFFFFFFF, 0x00000000 };
		dot.set(ITexture::Lock(ITexture::Format::Rgba32, reinterpret_cast<char*>(pixels), 2, 1));
	}

    if(isInvalidated())
    {
        drawing.open(space /*,L"fmTrendLine"*/);

		for(const RunSpan &run : runs)
		{
			if(run.startTime > 0.0)
			{
				Point pStart = transformDoubleToPoint(run.startTime, minY);
				if ( pStart.x < VerticalAxesOffset)
				{
					pStart.x = VerticalAxesOffset;
				}

				if(run.endTime == 0.0)
				{
					double endX = 0.0;
					if(!pullbackTrendLineSegments.empty() && pullbackTrendLineSegments.back().getLength() > 0)
					{
						endX = pullbackTrendLineSegments.back().getLastPoint().x;
					}

					renderer.render(getSpace(),Rect(pStart.x, bounds.top, endX, bounds.bottom),XrayRunActiveColor);
				}
				else
				{
					Point pEnd = transformDoubleToPoint(run.endTime, minY);

					renderer.render(getSpace(),Rect(pStart.x, bounds.top, pEnd.x, bounds.bottom),XrayRunActiveColor);
				}
			}
		}

		if(rawLineVisible)
		{
			for(const Line &rawLineSegment : pullbackRawLineSegments)
			{
				rawLineSegment.drawLine(renderer, space, dot);
			}
		}

		for(const Line &trendLineSegment : pullbackTrendLineSegments)
		{
			trendLineSegment.drawLine(renderer, space, dot);
		}

		cursor.drawPolyLine(drawing);

		drawInvalidCycleCrosses(renderer, invalidCyclesTrend, false);

		if (rawLineVisible )
		{
			drawInvalidCycleCrosses(renderer, invalidCyclesRaw, false);
		}

        drawing.close();
    }

    drawing.render(space, renderer);
}

void FmTrendDisplay::setTimestampStart(double timestamp)
{
    startOffset = timestamp;
}

void FmTrendDisplay::update(const std::vector<CoRegistrationIfrPullbackCycle> &cycles)
{
	pullbackTrendLineSegments.clear();
	pullbackRawLineSegments.clear();
	invalidCyclesTrend.clear();
	invalidCyclesRaw.clear();

	if(cycles.empty())
	{
		//TODO LOG;
		return;
	}

	if(!useStaticScale)
	{
		setDynamicIfrScale(cycles);
	}

	bool currentCilceValid = cycles.empty()? false:cycles.at(0).isValidCycle;
	Line trendSegment(1.4);
	Line rawSegment(0.7);
	trendSegment.setLineType( currentCilceValid? Line::LineType::Solid:Line::LineType::Dashed);
	trendSegment.setColor(currentCilceValid? PullbackLineColor : InvalidCycleColor);
	rawSegment.setColor(currentCilceValid? PullbackLineColor : InvalidCycleColor);
	rawSegment.setLineType(Line::LineType::Dashed);

	for(int i = 0 ; i < cycles.size();i++)
	{
		const CoRegistrationIfrPullbackCycle &cycle = cycles.at(i);

		Point currentTrendPoint = transformDoubleToPoint(cycle.timestamp, static_cast<double>(cycle.ifrTrendValue));
		Point currentRawPoint   = transformDoubleToPoint(cycle.timestamp, static_cast<double>(cycle.ifrRawValue));		

		bool nextCycleValid = cycles.at(i).						isValidCycle &&
							  cycles.at(min(i+1,cycles.size()-1)).isValidCycle;

		if(currentCilceValid != nextCycleValid)
		{
			trendSegment.addPoint(currentTrendPoint);
			rawSegment.addPoint(currentRawPoint);
			pullbackTrendLineSegments.push_back(trendSegment);
			pullbackRawLineSegments.push_back(rawSegment);
			trendSegment.clear();
			rawSegment.clear();
			currentCilceValid = nextCycleValid;		
		}

		trendSegment.addPoint(currentTrendPoint);
		rawSegment.addPoint(currentRawPoint);

		trendSegment.setLineType( currentCilceValid? Line::LineType::Solid:Line::LineType::Dashed);
		trendSegment.setColor(currentCilceValid? PullbackLineColor : InvalidCycleColor);
		rawSegment.setColor(currentCilceValid? PullbackLineColor : InvalidCycleColor);
		rawSegment.setLineType(Line::LineType::Dashed);

		if(!cycle.isValidCycle)
		{
			invalidCyclesTrend.push_back(transformDoubleToPoint(cycle.timestamp, static_cast<double>( cycle.ifrTrendValue)));
			invalidCyclesRaw.push_back(transformDoubleToPoint(cycle.timestamp, static_cast<double>(cycle.ifrRawValue)));
		}
	}

	if( trendSegment.getLength() > 0)
	{
		pullbackTrendLineSegments.push_back(trendSegment);
		pullbackRawLineSegments.push_back(rawSegment);
	}

    invalidate();
}

void FmTrendDisplay::setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image)
{
	lastReceivedTimestamp = image->timestamp;
	if(runs.back().startTime == 0.0)
	{
		runs.back().startTime = lastReceivedTimestamp;
	}

	invalidate();
}

void FmTrendDisplay::endLive()
{
	runs.back().endTime = lastReceivedTimestamp;

	RunSpan newSpan;
	newSpan.endTime = 0.0;
	newSpan.startTime = 0.0;

	runs.push_back(newSpan);

	invalidate();
}

void FmTrendDisplay::enableDynamicScaling(bool enable)
{
	useStaticScale = !enable;
}

void FmTrendDisplay::reset()
{
	runs.clear();
	cursor.clear();
	pullbackTrendLineSegments.clear();
	pullbackRawLineSegments.clear();
	liveIfrPullbackCycles.clear();
	invalidCyclesTrend.clear();
	invalidCyclesRaw.clear();

	RunSpan span;
	span.startTime = 0.0;
	span.endTime = 0.0;

	runs.push_back(span);

	maxX = LongRecordingMaxSeconds;
	horizontalSampleSize = 0.0;
	verticalSampleSize	 = 0.0;

	scaleX = LongRecordingScaleSeconds;

	rawLineVisible = true;
}

bool FmTrendDisplay::isEmpty() const
{
	return pullbackTrendLineSegments.size() == 0 && pullbackRawLineSegments.size() == 0 &&
	       liveIfrPullbackCycles.size() == 0 &&
	       invalidCyclesTrend.size() == 0 && invalidCyclesRaw.size() == 0;
}

void FmTrendDisplay::setRecording(const std::shared_ptr<PullbackRecording> &recording)
{
	rescaleToFullGraph(recording);
	
	update(recording->getAllIfrPullbackCycles());
	
	RunSpan span = {0,0};
	runs.push_back(span);

	for(const std::pair<std::shared_ptr<const ::CommonPlatform::Xray::XrayImage>, std::shared_ptr<const CrmOverlay>> &imageWithRoadmap : recording->getAllImages())
	{
		if(!imageWithRoadmap.first)
		{
			span.startTime  = 0.0;
			span.endTime	= 0.0;
			runs.push_back(span);
		}
		else
		{
			if(runs.back().startTime == 0.0)
			{
				runs.back().startTime = imageWithRoadmap.first->timestamp;
			}

			runs.back().endTime = imageWithRoadmap.first->timestamp;
		}
	}
}
void FmTrendDisplay::setIfrPullbackCycle(const CoRegistrationIfrPullbackCycle &iFrCycle)
{
	if(endOfDisplayReached(iFrCycle.timestamp))
	{
		maxX += static_cast<double>(SecondsAddedAtEndOfDisplay);

		if ( maxX > LongRecordingMaxSeconds)
		{
			rescaleTimeAxisTo(maxX);
		}

		resetSampleSize();
	}
	liveIfrPullbackCycles.push_back(iFrCycle);

	update(liveIfrPullbackCycles);
}

bool FmTrendDisplay::endOfDisplayReached(double timestamp) const
{
	return timestamp >= (maxX + startOffset);
}

void FmTrendDisplay::rescaleToFullGraph(const std::shared_ptr<PullbackRecording> & recording)
{
	if((recording != nullptr) && !recording->getAllIfrPullbackCycles().empty())
	{
		maxX = (recording->getAllIfrPullbackCycles().back().timestamp - recording->getAllIfrPullbackCycles().back().runStartedTime) + OneSecondMargin; 
		resetSampleSize();
		rescaleTimeAxisTo(maxX);
	}
}

void FmTrendDisplay::resetSampleSize()
{
	horizontalSampleSize = 0.0; //reset the sample size so it gets recalculated
	verticalSampleSize	 = 0.0;
}

void FmTrendDisplay::showRawLine(bool show)
{
	rawLineVisible = show;

	invalidate();
}

void FmTrendDisplay::setStaticIfrScale(float maxIfrValue, float minIfrValue)
{
	maxY = maxIfrValue;
	minY = minIfrValue;

	resetSampleSize();

	useStaticScale = true;

	invalidate();
}

void FmTrendDisplay::setDynamicIfrScale(const std::vector<CoRegistrationIfrPullbackCycle> &cycles)
{
	auto minTrendElement = std::min_element(cycles.begin(), cycles.end(), [&](const CoRegistrationIfrPullbackCycle &cycleA, const CoRegistrationIfrPullbackCycle &cycleB)
	{
		return cycleA.ifrTrendValue < cycleB.ifrTrendValue;
	});

	auto maxTrendElement = std::max_element(cycles.begin(), cycles.end(), [&](const CoRegistrationIfrPullbackCycle &cycleA, const CoRegistrationIfrPullbackCycle &cycleB)
	{
		return cycleA.ifrTrendValue > cycleB.ifrTrendValue;
	});

	auto minRawElement = std::min_element(cycles.begin(), cycles.end(), [&](const CoRegistrationIfrPullbackCycle &cycleA, const CoRegistrationIfrPullbackCycle &cycleB)
	{
		return cycleA.ifrRawValue < cycleB.ifrRawValue;
	});

	auto maxRawElement = std::max_element(cycles.begin(), cycles.end(), [&](const CoRegistrationIfrPullbackCycle &cycleA, const CoRegistrationIfrPullbackCycle &cycleB)
	{
		return cycleA.ifrRawValue > cycleB.ifrRawValue;
	});


	auto highestIfr = maxTrendElement->ifrTrendValue > maxRawElement->ifrRawValue ? maxTrendElement->ifrTrendValue : maxRawElement->ifrRawValue;

	float lowestIfr = minTrendElement->ifrTrendValue < minRawElement->ifrRawValue ? minTrendElement->ifrTrendValue : minRawElement->ifrRawValue;

	if(highestIfr > MaxIfr )
	{
		if( highestIfr <= MaxIfr + IfrScale)
		{	
			maxY =  MaxIfr + IfrScale;
		}
		else
		{
			maxY = ceil(highestIfr / IfrScale) *IfrScale;
		}
	}


	if(lowestIfr < LowValueIfrGraph)									 {minY = 0.0;}
	if(lowestIfr < MediumValueIfrGraph && lowestIfr >= LowValueIfrGraph) {minY = LowValueIfrGraph;}
	if(lowestIfr < HighValueIfrGraph && lowestIfr >= MediumValueIfrGraph) {minY = MediumValueIfrGraph;}
	if(lowestIfr >= HighValueIfrGraph)					{minY = HighValueIfrGraph;}

	resetSampleSize();
}

void FmTrendDisplay::rescaleTimeAxisTo(double maxTimeInSec)
{
	if ( maxTimeInSec < ShortRecordingMaxSeconds )
	{
		scaleX = ShortRecordingScaleSeconds;
	}
	else if ( maxTimeInSec < LongRecordingMaxSeconds )
	{
		scaleX = LongRecordingScaleSeconds;
	}
	else
	{
		// First calculate the number of intervals that would be required with the default scale.
		// Then calculate a multiplier to reduce the number of intervals and adapt the scale to 2x,3x,4x,etc the default scale.
		const double MaximumNumberOfLabels = 6.0;
		const double numberOfIntervals				= std::floor(maxTimeInSec / DefaultRecordingScaleSeconds);
		const double additionalMultiplierForScaling	= std::floor(1 + numberOfIntervals / MaximumNumberOfLabels);
		scaleX										= DefaultRecordingScaleSeconds * additionalMultiplierForScaling;
	}
}


}}
