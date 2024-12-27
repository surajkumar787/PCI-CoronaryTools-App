// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "SmartSuiteLicenseAgreement.h"

using namespace Sense;
using namespace Sense::ExperienceIdentity;

namespace Pci { namespace Core
{
	SmartSuiteLicenseAgreement::SmartSuiteLicenseAgreement(Control &parent) : SmartSuiteLicenseAgreementDialog(parent)
	{
		close.eventClicked = [&] { hide(); };
	}

	void SmartSuiteLicenseAgreement::show()
	{
		auto parentRect = getParentSpace().bounds;
		position = Sense::Point( (parentRect.width()-size->width)/2.0,(parentRect.height()-size->height)/2.0);
		visible = true;
	}

	void SmartSuiteLicenseAgreement::hide()
	{
		visible = false;
	}
}}