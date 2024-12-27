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
	class StbRoiTsmPage : public StbTsmPage
	{
	public:
		// numbering on the Tsm (1..20)
		static const int SizeUpButton = 11;
		static const int SizeDownButton = 16;

		static const int PanLeftButton = 17;
		static const int PanRightButton = 19;
		static const int PanUpButton = 13;
		static const int PanDownButton = 18;

		static const int ApplyButton = 20;

		StbRoiTsmPage();

		Util::RefProperty<TSM::TsmButtonProperties> sizeUpButton;
		Util::RefProperty<TSM::TsmButtonProperties> sizeDownButton;

		Util::RefProperty<TSM::TsmButtonProperties> panLeftButton;
		Util::RefProperty<TSM::TsmButtonProperties> panRightButton;
		Util::RefProperty<TSM::TsmButtonProperties> panUpButton;
		Util::RefProperty<TSM::TsmButtonProperties> panDownButton;

		Util::RefProperty<TSM::TsmButtonProperties> applyButton;
	};
}}