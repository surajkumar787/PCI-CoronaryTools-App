// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "SmartSuiteLicenseAgreementDialog.ui.h"

namespace Pci { namespace Core
{
	class SmartSuiteLicenseAgreement : public SmartSuiteLicenseAgreementDialog
	{
	public:
		explicit SmartSuiteLicenseAgreement(Control &parent);

		void show();
		void hide();
	};
}}