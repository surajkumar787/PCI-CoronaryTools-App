// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "StbTsmPage.h"
#include "log.h"
#include "RefProperty.h"

namespace Pci {	namespace Core
{
	class StbImageSelectTsmPage : public StbTsmPage
	{
	public:
		// numbering on the Tsm (1..20)
		static const int NavigateLeftButton = 17;
		static const int NavigateRightButton = 19;

		static const int ResetDefaultButton = 15;
		static const int ApplyButton = 20;

        StbImageSelectTsmPage();

		Util::RefProperty<TSM::TsmButtonProperties> navigateLeftButton;
		Util::RefProperty<TSM::TsmButtonProperties> navigateRightButton;

		Util::RefProperty<TSM::TsmButtonProperties> resetDefaultButton;
		Util::RefProperty<TSM::TsmButtonProperties> applyButton;
	};
}}