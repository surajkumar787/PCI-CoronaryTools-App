// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "PciTsmPage.h"

namespace Pci { namespace Core
{

	namespace TSM = ::CommonPlatform::TSM;
	namespace Util = ::CommonPlatform::Utilities;

	class StbTsmPage : public PciTsmPage
	{
	public:
		// numbering on the Tsm (1..20)

		static const int CopyToRefButton = 3;
		static const int DefineROIButton = 4;
        static const int ContrastFrameSelectButton = 7;
        static const int BoostFrameSelectButton = 6;
        static const int InvertBoostButton = 5;
		static const int ToBoostButton = 16;
		static const int ToContrastButton = 20;
		static const int StopStartButton = 18;

		StbTsmPage();

		Util::RefProperty<TSM::TsmButtonProperties> defineRoiButton;
		Util::RefProperty<TSM::TsmButtonProperties> copyToRefButton;
        Util::RefProperty<TSM::TsmButtonProperties> contrastFrameSelectButton;
        Util::RefProperty<TSM::TsmButtonProperties> boostFrameSelectButton;
        Util::RefProperty<TSM::TsmButtonProperties> invertBoostButton;
		Util::RefProperty<TSM::TsmButtonProperties> toBoostButton;
		Util::RefProperty<TSM::TsmButtonProperties> toContrastButton;
		Util::RefProperty<TSM::TsmButtonProperties> stopStartButton;
	};



}}