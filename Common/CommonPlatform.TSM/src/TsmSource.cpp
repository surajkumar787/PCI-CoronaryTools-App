// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "TsmSource.h"
#include "TsmPage.h"
#include "UtilizationLogEvent.h"

static const double ButtonRepeatFrequency = 1.0 / 10;

namespace CommonPlatform { namespace TSM
{
	TsmSource::TsmSource(Sense::ITimerScheduler& scheduler, const wchar_t* description) :
		log(CommonPlatform::Logger(LoggerType::Platform)),
		cwisAlliance(description),
		cwisTools(description),
		currentPage(nullptr),
		buttonRepeatTimer(scheduler, [this]() { timerTick(); }),
		buttonRepeat(0),
		hasBeenActivated(false)
	{
		cwisAlliance.eventConnectionChanged = [this](Cwis::CwisConnectionType conn)
		{
			if (conn == Cwis::CwisConnectionType::Connected)
			{
				setPage(currentPage);
				if (conn == cwisTools.isConnected())
				{
					eventConnectionChanged(conn);
				}
			}
			if(conn == Cwis::CwisConnectionType::Active)
			{
				hasBeenActivated = true;
			}
			if (conn == Cwis::CwisConnectionType::Disconnected)
			{
				resetData();
				if(!hasBeenActivated)
				{
					::CommonPlatform::UtilizationLogEvent::XrayLicenseMissing(log,L"PkgTSMControl3rdParty5");
				}
			}
		};
		cwisTools.eventConnectionChanged = [this](Cwis::CwisConnectionType conn)
		{
			if (conn == Cwis::CwisConnectionType::Connected)
			{
				cwisTools.workstation.application   = L"PCIe";
				cwisTools.workstation.text          = L"Coronary Tools";
				cwisTools.workstation.visible       = true;
				cwisTools.workstation.enabled       = true;
				cwisTools.commit();

				if (conn == cwisAlliance.isConnected())
				{
					eventConnectionChanged(conn);
				}
			}
		};
	}

	TsmSource::~TsmSource(void)
	{
	}

	void TsmSource::timerTick()
	{
		if (currentPage && buttonRepeat) currentPage->onButtonPress(buttonRepeat, true);
	}

	void TsmSource::setPage(TsmPage* page)
	{
		// disable changed events to prevent a button press from the old page turning up in the new page
		cwisAlliance.eventModelChanged = nullptr;
		if (currentPage) currentPage->eventRequestUpdate = nullptr;
		bool changed = false;
		currentPage = page;
		if (currentPage)
		{
			changed = setData(*currentPage);
			cwisAlliance.workstation.upperMenuStyleIsMainStyle = true;
			cwisAlliance.workstation.lowerMenuIsTouchPad = false;
			cwisAlliance.workstation.mouseEnabled = false;
			cwisAlliance.workstation.enabled = true;
		}
		else
		{
			changed = resetData();
			cwisAlliance.workstation.enabled = false;
		}

		if (changed)
		{
			cwisAlliance.commit();

			if (eventFunctionAvailabilityChanged) eventFunctionAvailabilityChanged(hasClickableButtons());
		}

		if (currentPage)
		{
			cwisAlliance.eventModelChanged = [this]() {  onButtonPressed(); };
			currentPage->eventRequestUpdate = [this] 
			{ 
				if (setData(*currentPage)) cwisAlliance.commit(); 
				if (eventFunctionAvailabilityChanged) eventFunctionAvailabilityChanged(hasClickableButtons());
			};
		}
	}

	void TsmSource::onButtonPressed()
	{
		unsigned int pressed = 0;
		if		(cwisAlliance.modality.button01Pressed)	pressed = 1;
		else if (cwisAlliance.modality.button02Pressed)	pressed = 2;
		else if (cwisAlliance.modality.button03Pressed)	pressed = 3;
		else if (cwisAlliance.modality.button04Pressed)	pressed = 4;
		else if (cwisAlliance.modality.button05Pressed)	pressed = 5;
		else if (cwisAlliance.modality.button06Pressed)	pressed = 6;
		else if (cwisAlliance.modality.button07Pressed)	pressed = 7;
		else if (cwisAlliance.modality.button08Pressed)	pressed = 8;
		else if (cwisAlliance.modality.button09Pressed)	pressed = 9;
		else if (cwisAlliance.modality.button10Pressed)	pressed = 10;
		else if (cwisAlliance.modality.button11Pressed)	pressed = 11;
		else if (cwisAlliance.modality.button12Pressed)	pressed = 12;
		else if (cwisAlliance.modality.button13Pressed)	pressed = 13;
		else if (cwisAlliance.modality.button14Pressed)	pressed = 14;
		else if (cwisAlliance.modality.button15Pressed)	pressed = 15;
		else if (cwisAlliance.modality.button16Pressed)	pressed = 16;
		else if (cwisAlliance.modality.button17Pressed)	pressed = 17;
		else if (cwisAlliance.modality.button18Pressed)	pressed = 18;
		else if (cwisAlliance.modality.button19Pressed)	pressed = 19;
		else if (cwisAlliance.modality.button20Pressed)	pressed = 20;

		if (pressed)
			log.DeveloperInfo(L"TSM: pressed button %u", pressed);
		else if (buttonRepeat)
			log.DeveloperInfo(L"TSM: released button %u", buttonRepeat);

		if (buttonRepeat && pressed != buttonRepeat && currentPage)
		{
			currentPage->eventButtonReleased(buttonRepeat);
		}

		buttonRepeat = pressed;
		if (pressed)
		{
			if (currentPage && !buttonRepeatTimer.isActive()) currentPage->onButtonPress(pressed);

			buttonRepeatTimer.start(ButtonRepeatFrequency);
		}
		else
		{
			// set all data again in case of a button appearance update
			if (setData(*currentPage)) cwisAlliance.commit();
			if (eventFunctionAvailabilityChanged) eventFunctionAvailabilityChanged(hasClickableButtons());
			buttonRepeatTimer.stop();
		}
	}

	void TsmSource::connect(const wchar_t* host, unsigned short port)
	{
		cwisAlliance.connect(host, port);
		cwisTools.connect(host, port);
	}

#define ASSIGN(property, value) \
	if (property != value)    \
	{                         \
		property  = value;    \
		changed = true;       \
	}

#define SET_DATA(buttonNr, buttonProp)                                                      \
	ASSIGN(cwisAlliance.workstation.button ## buttonNr ## Visible,    buttonProp.visible)   \
	ASSIGN(cwisAlliance.workstation.button ## buttonNr ## Enabled,    buttonProp.enabled)   \
	ASSIGN(cwisAlliance.workstation.button ## buttonNr ## ButtonType, buttonProp.buttonType)\
	ASSIGN(cwisAlliance.workstation.button ## buttonNr ## Uppertext,  buttonProp.upperText) \
	ASSIGN(cwisAlliance.workstation.button ## buttonNr ## LowerText,  buttonProp.lowerText) \
	ASSIGN(cwisAlliance.workstation.button ## buttonNr ## IconName,   buttonProp.icon)      \
	ASSIGN(cwisAlliance.workstation.button ## buttonNr ## CenterText, buttonProp.centerText)\
	ASSIGN(cwisAlliance.workstation.button ## buttonNr ## LabelText,  buttonProp.label)     \
	ASSIGN(cwisAlliance.workstation.button ## buttonNr ## Selected,   buttonProp.selected)  \
	ASSIGN(cwisAlliance.workstation.button ## buttonNr ## Blinking,   buttonProp.blinking)

	bool TsmSource::setData(const TsmPage& page)
	{
		bool changed = false;

		auto formattedPage = splitButtonCenterTextIfNeeded(page);

		SET_DATA(01, formattedPage.buttonProperties[0]);
		SET_DATA(02, formattedPage.buttonProperties[1]);
		SET_DATA(03, formattedPage.buttonProperties[2]);
		SET_DATA(04, formattedPage.buttonProperties[3]);
		SET_DATA(05, formattedPage.buttonProperties[4]);
		SET_DATA(06, formattedPage.buttonProperties[5]);
		SET_DATA(07, formattedPage.buttonProperties[6]);
		SET_DATA(08, formattedPage.buttonProperties[7]);
		SET_DATA(09, formattedPage.buttonProperties[8]);
		SET_DATA(10, formattedPage.buttonProperties[9]);
		SET_DATA(11, formattedPage.buttonProperties[10]);
		SET_DATA(12, formattedPage.buttonProperties[11]);
		SET_DATA(13, formattedPage.buttonProperties[12]);
		SET_DATA(14, formattedPage.buttonProperties[13]);
		SET_DATA(15, formattedPage.buttonProperties[14]);
		SET_DATA(16, formattedPage.buttonProperties[15]);
		SET_DATA(17, formattedPage.buttonProperties[16]);
		SET_DATA(18, formattedPage.buttonProperties[17]);
		SET_DATA(19, formattedPage.buttonProperties[18]);
		SET_DATA(20, formattedPage.buttonProperties[19]);

		return changed;
	}

	bool TsmSource::resetData()
	{
		bool changed = false;
		TsmButtonProperties prop;
		SET_DATA(01, prop);
		SET_DATA(02, prop);
		SET_DATA(03, prop);
		SET_DATA(04, prop);
		SET_DATA(05, prop);
		SET_DATA(06, prop);
		SET_DATA(07, prop);
		SET_DATA(08, prop);
		SET_DATA(09, prop);
		SET_DATA(10, prop);
		SET_DATA(11, prop);
		SET_DATA(12, prop);
		SET_DATA(13, prop);
		SET_DATA(14, prop);
		SET_DATA(15, prop);
		SET_DATA(16, prop);
		SET_DATA(17, prop);
		SET_DATA(18, prop);
		SET_DATA(19, prop);
		SET_DATA(20, prop);
		return changed;
	}

	bool TsmSource::hasClickableButtons() const
	{
		bool clickableButtons(false);

		if (currentPage != nullptr)
		{
			for (auto& button : currentPage->buttonProperties)
			{
				if (button.visible && button.enabled)
				{
					clickableButtons = true;
					break;
				}
			}
		}

		return clickableButtons;
	}

	TsmPage TsmSource::splitButtonCenterTextIfNeeded(const TsmPage& page)
	{
		TsmPage formattedPage = page;
		for (int i = 0; i < 20; i++)
		{
			size_t newlineIndex = 0;
			if ((newlineIndex = formattedPage.buttonProperties[i].centerText.find(L'\n')) != std::wstring::npos)
			{
				formattedPage.buttonProperties[i].upperText = page.buttonProperties[i].centerText.substr(0, newlineIndex);
				formattedPage.buttonProperties[i].lowerText = page.buttonProperties[i].centerText.substr(newlineIndex + 1, std::wstring::npos);
				formattedPage.buttonProperties[i].centerText = L"";
			}
		}

		return formattedPage;
	}

}}