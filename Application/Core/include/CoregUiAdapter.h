// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "ViewType.h"
#include "RefProperty.h"
#include "PciProgressBar.h"

#include "ExperienceIdentity/Controls/Label.h"
#include "ExperienceIdentity/Controls/Panel.h"
#include "ExperienceIdentity/Controls/Separator.h"

//TICS -INT#002  UI needs public property to subscribe to..

namespace Pci { namespace Core
{
	namespace Util = ::CommonPlatform::Utilities;

	class CoregUiAdapter
	{
	public:
		CoregUiAdapter(Sense::Control& parent, ViewType type);

		std::unique_ptr<Sense::Control> gui;
		Util::RefProperty<Sense::ExperienceIdentity::Label> fmTypeLabel;
		Util::RefProperty<Sense::ExperienceIdentity::Label> fmTypeValue;
		Util::RefProperty<Sense::ExperienceIdentity::Panel> fmTypePanel;
		Util::RefProperty<Sense::ExperienceIdentity::Panel> ifrProgressPanel;
		Util::RefProperty<PciProgressBar>                   ifrProgressBar;

		Util::RefProperty<Sense::ExperienceIdentity::Label> pdPaLabel;
		Util::RefProperty<Sense::ExperienceIdentity::Label> pdPaValue;
		Util::RefProperty<Sense::ExperienceIdentity::Panel> pdPaPanel;
		Util::RefProperty<Sense::ExperienceIdentity::Label> pdLabel;
		Util::RefProperty<Sense::ExperienceIdentity::Label> pdValue;
		Util::RefProperty<Sense::ExperienceIdentity::Label> paLabel;
		Util::RefProperty<Sense::ExperienceIdentity::Label> paValue;

		Util::RefProperty<Sense::ExperienceIdentity::Label> ifrReviewFmTypeLabel;
		Util::RefProperty<Sense::ExperienceIdentity::Label> ifrReviewTrendValue;
		Util::RefProperty<Sense::ExperienceIdentity::Label> ifrReviewTrendLabel;
		Util::RefProperty<Sense::ExperienceIdentity::Label> ifrReviewDistalFmValue;
		Util::RefProperty<Sense::ExperienceIdentity::Panel> ifrReview;

		Util::RefProperty<Sense::ExperienceIdentity::Panel> ifrSpotReviewPanel;
		Util::RefProperty<Sense::ExperienceIdentity::Label> ifrSpotReviewValue;
		Util::RefProperty<Sense::ExperienceIdentity::Label> ifrSpotReviewLabel;

		Util::RefProperty<Sense::ExperienceIdentity::Panel> graphPanel;
		Util::RefProperty<Sense::Control>					IfrValuePanel;

		Util::RefProperty<Sense::ExperienceIdentity::Panel>	popupPannel;
		Util::RefProperty<Sense::ExperienceIdentity::Panel> popupHeader;
		Util::RefProperty<Sense::ExperienceIdentity::Label> popupHeaderText;
		
		Util::RefProperty<Sense::ExperienceIdentity::Panel> topPannel;
	private:
		template<typename GuiType>
		GuiType* getGui();

		Sense::Control* createGui(Sense::Control& parent, ViewType type);
	};
}}

//TICS +INT#002