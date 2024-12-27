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

	class CrmTsmPage : public PciTsmPage
	{
	public:
		// numbering on the Tsm (1..20)

		static const int RoadmapSelectionButton = 3;

		static const int FrameBackButton = 17;
		static const int FrameForwardButton = 19;
		static const int PlayPauseButton = 18;

		// numbering on the Tsm (1..20)
		static const int NavigateLeftButton = 17;
		static const int NavigateRightButton = 19;

		static const int LockRoadmapButton = 20;

		CrmTsmPage();

		Util::RefProperty<TSM::TsmButtonProperties> roadmapOptionsButton;

		Util::RefProperty<TSM::TsmButtonProperties> frameBackwardButton;
		Util::RefProperty<TSM::TsmButtonProperties> frameForwardButton;
		Util::RefProperty<TSM::TsmButtonProperties> playPauseButton;

		Util::RefProperty<TSM::TsmButtonProperties> navigateLeftButton;
		Util::RefProperty<TSM::TsmButtonProperties> navigateRightButton;

		Util::RefProperty<TSM::TsmButtonProperties> lockRoadmapButton;
	};



}}