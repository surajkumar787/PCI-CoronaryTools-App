// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "LicensesCollection.h"
#include "SmartSuiteAboutBoxDialog.ui.h"
#include "SmartSuiteLicenseAgreement.h"

namespace Pci { namespace Core {
	
using namespace Sense;
using namespace Sense::ExperienceIdentity;

struct AboutInfo;

// This class is a base class for about boxes.
class SmartSuiteAboutBox : public SmartSuiteAboutBoxDialog
{
public:
	explicit SmartSuiteAboutBox(Control &parent, const ILicenseCollection& licenseCollection);

	void setContents(const AboutInfo& info);

	void show();
    void hide();

protected:
    void onShowLicenseAgreement();
	const Sense::Image getDynamicLabel(const ILicenseCollection& licenseCollection);

private:
    SmartSuiteLicenseAgreement eulaDialog;
	const ILicenseCollection&	m_licenseCollection;
};

}}