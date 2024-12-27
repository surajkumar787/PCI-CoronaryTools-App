// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "countdowncontrol.h"
#include "Translation.h"

namespace Pci { namespace Core {

const static double Epsilon = 0.01;
const static double ToProcent = 100.0;
const static double MinimumDelta = 0.05;
const static int    TimeTextLength = 20;
const static double OneDecimalShift = 10.0;
const static double ProgressCircleMarginPercentageActive   = 0.00;
const static double ProgressCircleMarginPercentageInactive = 0.05;

const static Sense::Color& InProgressTextColor = Sense::ExperienceIdentity::Palette::Gray000;
const static Sense::Color& CompleteTextColor   = Sense::ExperienceIdentity::Palette::Gray100;
const static Sense::Color& NotStartedTextColor = Sense::ExperienceIdentity::Palette::Gray100;

const static Font& FontTimerName             = Font(L"Philips Healthcare Font", 26);
const static Font& FontSecondsValueInactive  = Font(L"Philips Healthcare Font", 32);
const static Font& FontSecondsValueActive    = Font(L"Philips Healthcare Font", 40);
const static Font& FontSecondsLabelInactive  = Font(L"Philips Healthcare Font", 18);
const static Font& FontSecondsLabelActive    = Font(L"Philips Healthcare Font", 20);

CountDownControl::CountDownControl(Sense::Control& parent) :
    CountDownControlUi(parent),  
    m_duration(0.0),
    m_remainingTime(0.0),
    m_inProgress(false)
{
	secondsLabel.text = Localization::Translation::IDS_Seconds;
	panelDisableCircleOverlay.visible = false;

	setCountDownToNotStarted();
	showSecondsText(false);
}

CountDownControl::~CountDownControl(void)
{
}

void CountDownControl::setCountDownToNotStarted()
{
	progressCircle.determinate = true;
    setTextColor(NotStartedTextColor);
    setInactiveTextSize();
	if (m_duration > 0)
	{
		showSecondsText(true);
		progressCircle.value = 100;
		onTimeChanged(m_duration);
		panelDisableCircleOverlay.visible = false;
	}
	else
	{
		progressCircle.value = 0;
	}
    
    m_inProgress = false;
    SetSizes();
}

void CountDownControl::setCountDownToInProgress()
{
    setTextColor(InProgressTextColor);
    setActiveTextSize();
	if (m_duration > 0.0)
	{
		showSecondsText(true);
		progressCircle.value = m_progressValue;
		panelDisableCircleOverlay.visible = false;
	}
	else
	{
		progressCircle.determinate = false;
		panelDisableCircleOverlay.visible = false;
	}

    m_inProgress = true;
    SetSizes();
}

void CountDownControl::setCountDownToComplete()
{
    setTextColor(CompleteTextColor);
    setInactiveTextSize();
    showSecondsText(false);
    progressCircle.value = 0;
    panelDisableCircleOverlay.visible = true;
    m_inProgress = false;
    SetSizes();
}

void CountDownControl::onTimeChanged(double remaining)
{
	m_remainingTime = remaining;
    if ((m_duration > MinimumDelta) && (m_duration > 0.0))
    {
        m_progressValue = ToProcent * m_remainingTime / m_duration;
    }

    if (m_inProgress)
    {
        progressCircle.value = m_progressValue;
    }

    secondsValue.text = time2Text(m_remainingTime, m_duration);
}

std::wstring CountDownControl::time2Text(double remainingTime, double duration)
{
    // When duration is 2.5s, 
    // the control counts down in these steps:  
    //   2.5  >  2  >  1  >  0

    std::wstring format;
    double ceilingTime = 0.0;

    if(remainingTime > std::floor(duration) )
    {
        // Display with one decimal when remainingTime > floor(duration)
        format = L"%.1f";
        ceilingTime = duration;
    }
    else
    {
        // Display with no decimals when remainingTime <= floor(duration)
        format = L"%.0f";
        ceilingTime = std::ceil(remainingTime);
    }

    wchar_t  value[TimeTextLength];
    swprintf_s(value, TimeTextLength, format.c_str(), ceilingTime);

    return value;
}

void CountDownControl::setTextColor(Sense::Color textColor)
{
    timerName.color    = textColor;
    secondsValue.color = textColor;
    secondsLabel.color = textColor;
}

void CountDownControl::setActiveTextSize()
{
    timerName.font    = FontTimerName;
    secondsValue.font = FontSecondsValueActive;
    secondsLabel.font = FontSecondsLabelActive;
}

void CountDownControl::setInactiveTextSize()
{
    timerName.font    = FontTimerName;
    secondsValue.font = FontSecondsValueActive;
    secondsLabel.font = FontSecondsLabelActive;
}

void CountDownControl::showSecondsText(bool show)
{
    secondsValue.visible = show;
    secondsLabel.visible = show;
}

void CountDownControl::onResize()
{
    CountDownControlUi::onResize();
    SetSizes();
}

void CountDownControl::setDuration(double duration)
{
	m_duration = duration;
	m_remainingTime = duration;
	m_progressValue = 0.0;
	containerSeconds.visible = m_duration > 0.0;
	setCountDownToNotStarted();
	onTimeChanged(m_remainingTime);
}

void CountDownControl::SetSizes()
{
    // Set overlays to same size as circle control 
    auto bounds = progressCircle.getBounds();
    containerSeconds.setBounds(bounds);
    panelDisableCircleOverlay.setBounds(bounds);
}

}}