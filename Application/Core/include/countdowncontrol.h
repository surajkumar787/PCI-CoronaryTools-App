// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "countdowncontrol.ui.h"

namespace Pci { namespace Core {

class CountDownControl : public CountDownControlUi
{
public:
    explicit CountDownControl(Sense::Control &parent);
    virtual ~CountDownControl(void);

    virtual void onResize() override;
	void setDuration(double duration);

	void onTimeChanged(double remaining);

	void setCountDownToInProgress();
	void setCountDownToComplete();
	void setCountDownToNotStarted();

private:
    void showSecondsText(bool);

    void setTextColor(Sense::Color textColor);
    void setActiveTextSize();
    void setInactiveTextSize();

    std::wstring time2Text(double remainingTime, double duration);
    void SetSizes();

    double                    m_duration;
    double                    m_remainingTime;
    double                    m_progressValue;
    bool                      m_inProgress;
};

}}