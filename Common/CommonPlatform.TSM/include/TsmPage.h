// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <functional>
#include <string>

namespace CommonPlatform { namespace TSM
{

	struct TsmButtonProperties
	{
		TsmButtonProperties() :
			visible(false),
			enabled(false),
			selected(false),
			blinking(false)
		{}

		bool visible;
		bool enabled;
		bool selected;
		bool blinking;
		std::wstring upperText;
		std::wstring lowerText;
		std::wstring centerText;
		std::wstring icon;
		std::wstring buttonType;
		std::wstring label;
	};

	class TsmPage
	{
	public:
		TsmPage();
		virtual ~TsmPage();

		virtual void onButtonPress(unsigned int button, bool repeated = false);
		virtual void onButtonRelease(unsigned int button);

		void requestUpdate();

		std::function<void()> eventRequestUpdate;
		std::function<void(unsigned int, bool repeated)> eventButtonPressed;
		std::function<void(unsigned int)> eventButtonReleased;
		TsmButtonProperties buttonProperties[20];
	};

}}