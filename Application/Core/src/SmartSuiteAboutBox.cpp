// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "SmartSuiteAboutBox.h"
#include "Resources.h"
#include "Translation.h"
#include "AboutInfo.h"

using namespace Pci::Core;

SmartSuiteAboutBox::SmartSuiteAboutBox(Control &parent, const ILicenseCollection& licenseCollection) :
    SmartSuiteAboutBoxDialog(parent),
	m_licenseCollection(licenseCollection),
    eulaDialog(parent)
{
    close.eventClicked = [&] { hide(); };
	
    eulaDialog.visible  = false;
   
	ifuIndicatorLabel.image = PciSuite::Resources::IfuLight;
	warningLabel.image = PciSuite::Resources::warning;

    copyrightClause.text  = L"\u00A9 Koninklijke Philips N.V., 2016 - 2024. " + Localization::Translation::IDS_LegalAllRightsReserved;

    address.text          = L"Philips Medical Systems Nederland B.V.\nVeenpluis 6 5684 PC Best THE NETHERLANDS";
	manufacturingDate.image = PciSuite::Resources::manufacturer_gray;

    reproduceClause.text  = Localization::Translation::IDS_LegalCopyright + L" " +
                            Localization::Translation::IDS_LegalOwnership + L" " +
                            Localization::Translation::IDS_LegalReverseEngineer;

	Text t(gpu);
	t.type = reproduceClause.textType;
	t.text = reproduceClause.text;
	t.font = reproduceClause.font;
	
	auto requiredSize = t.measureSize(reproduceClause.getSpace(), reproduceClause.size->width, 0);
	auto currentSize = Size(reproduceClause.size->width, reproduceClause.size->height);

	auto diff = requiredSize - currentSize;

	//CVPRJ00441000: Eula Link has to be positioned based on the
	//reproduceClause text size, which is diffrent for diffrent language.
	t.text = L"View the End-User License Agreement";
	t.font = eulaStatement.font;

	eulaStatement.text = t.text;
	eulaStatement.size = t.measureSize(eulaStatement.getSpace(), eulaStatement.size->width, 0);
	eulaStatement.position = Point(size->width / 2 - eulaStatement.size->width / 2, eulaStatement.position->y + diff.height);
	eulaStatement.eventClicked = [this]() { onShowLicenseAgreement(); };
	eulaStatement.setFontColor(Palette::Blue050, Palette::Blue010);

	size = Size(size->width, size->height + diff.height);

	//Philips Logo
	imageBox.image = PciSuite::Resources::PhilipsShield;

	// "#" label caption
	label.image = PciSuite::Resources::BatchCode;

	// "UDI" label caption
	label2.image = PciSuite::Resources::MDR_UDIMarking;

	// "LOT" label caption
	label4.image = PciSuite::Resources::ModelNumber;

	// "MD" label caption
	label5.image = PciSuite::Resources::MDR_MDMarking;
	label5.imageSize = label5.size;

    visible = false;
}

void SmartSuiteAboutBox::setContents(const AboutInfo& info)
{
	constexpr int NAME_MARGIN = 2 * 22;

	productName.text = info.ProductName;
	panelBadge.size = Sense::Size(productName.measureSize().width + NAME_MARGIN, panelBadge.size->height);
	releaseLabel.text = info.ReleaseIndication;

	manufacturingDate.text = info.ManufactoringDate;

	refNumberLabel.text = info.ReferenceNr;
	lotNumberLabel.text = info.LotNr;
	udiNumberLabel.text = info.Gs1Nr;

	eulaDialog.eulaText.text = info.eula;

	ceLabel.image = getDynamicLabel(m_licenseCollection);
}

void SmartSuiteAboutBox::show()
{
    auto parentRect = getParentSpace().bounds;
    position = Point( (parentRect.width()-size->width)/2,(parentRect.height()-size->height)/2);
    eulaDialog.hide();
    visible = true;
}

void SmartSuiteAboutBox::hide()
{
    eulaDialog.hide();
    visible = false;
}

void SmartSuiteAboutBox::onShowLicenseAgreement()
{
    eulaDialog.bringToFront();
    eulaDialog.show();
}

const Sense::Image SmartSuiteAboutBox::getDynamicLabel(const ILicenseCollection& licenseCollection)
{
	int gap = 15;							//gap between two label
	bool showCELabel = false;
	bool showUKCALabel = false;
	Sense::Image image = Image();
	
	ceLabel.imageSize = Size(0, 0);			//Placeholder to hold Dynamic label
	logoSectionPanel.size = Size(0, 0);		//Placeholder to hold Dynamic label + MD Label

	if (productName.text == L"StentBoost Live")
	{
		if (licenseCollection.isActive(LicenseFeature::stentBoostLiveCE))
		{
			showCELabel = true;
		}
		
		if (licenseCollection.isActive(LicenseFeature::stentBoostLiveUKCA))
		{
			showUKCALabel = true;
		}
	}
	else if (productName.text == L"Dynamic Coronary Roadmap")
	{
		if (licenseCollection.isActive(LicenseFeature::roadmapCE))
		{
			showCELabel = true;
		}
		
		if (licenseCollection.isActive(LicenseFeature::roadmapUKCA))
		{
			showUKCALabel = true;
		}
	}	
	
	if (showCELabel && showUKCALabel)
	{
		image = PciSuite::Resources::CE_UKCA;
	}
	else if (showUKCALabel)
	{
		image = PciSuite::Resources::UKCA;
	}
	else if (showCELabel)
	{
		image = PciSuite::Resources::CE;
	}

	//Set the size of the lable place holder
	ceLabel.imageSize = image.size();

	//Calculate the exact width of the placeholder
	double width = panel8.size->width + (ceLabel.imageSize->width ? ceLabel.imageSize->width + gap : 0);
	
	//Set the size of the place holder based on the image selected
	logoSectionPanel.size = Size(width, 40);

	return image;
}