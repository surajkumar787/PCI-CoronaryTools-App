// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "TsmPage.h"
#include "log.h"
#include "RefProperty.h"

namespace Pci { namespace Core
{
	namespace TSM = ::CommonPlatform::TSM;
	namespace Util = ::CommonPlatform::Utilities;

	class PciTsmPage : public TSM::TsmPage
	{
	public:
		// numbering on the Tsm (1..20)
		static const int SnapshotButton = 1;
		static const int MovieButton = 2;

		PciTsmPage();

		virtual void onButtonPress(unsigned int button, bool repeated = false) override;

		CommonPlatform::Log& log;

		Util::RefProperty<TSM::TsmButtonProperties> storeSnapshotButton;
		Util::RefProperty<TSM::TsmButtonProperties> storeMovieButton;
	};

}}