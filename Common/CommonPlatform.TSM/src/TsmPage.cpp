// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "TsmPage.h"

namespace CommonPlatform { namespace TSM
{

	TsmPage::TsmPage()
	{}

	TsmPage::~TsmPage()
	{}

	void TsmPage::onButtonPress(unsigned int button, bool repeated)
	{
		if (eventButtonPressed) eventButtonPressed(button, repeated);
	}

	void TsmPage::onButtonRelease(unsigned int button)
	{
		if (eventButtonReleased) eventButtonReleased(button);
	}

	void TsmPage::requestUpdate()
	{
		if (eventRequestUpdate) eventRequestUpdate();
	}

}}