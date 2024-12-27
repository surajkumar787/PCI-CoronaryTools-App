// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <string>
#include "Sense/Common/Color.h"
#include "XrayGeometry.h"
#include "CrmRoadmap.h"
#include "PciXrayPositionModel.h"

namespace Sense { class Image; }

namespace Pci { namespace Core
{

class PciGuidanceModel : public PciXrayPositionModel
{
public:
	// Guidance block 
	PropertyNotifier<bool> showMessagePanel;
	PropertyNotifier<std::wstring> messageHeaderText;
	PropertyNotifier<Sense::Color> messageHeaderColor;
	PropertyNotifier<const Sense::Image*> messageHeaderIcon;
	PropertyNotifier<std::wstring> messageText;
	PropertyNotifier<Sense::Color> messageColor;
	PropertyNotifier<std::wstring> guidanceText;

	PropertyNotifier<bool> showMessagePicturePanel;
	PropertyNotifier<const Sense::Image*> messagePicture;
	PropertyNotifier<std::wstring> messagePictureText;

	// area
	PropertyNotifier<std::wstring>status;
	PropertyNotifier<bool>roadmapActive;
	PropertyNotifier<std::shared_ptr<const CrmRoadmap>> roadMapImage;
	PropertyNotifier<const Sense::Image*> statusIcon;
	PropertyNotifier<bool> showPositionPanel;
	PropertyNotifier<double> currentRoadmapAngulation;
	PropertyNotifier<double> currentRoadmapRotation;
	PropertyNotifier<bool> showProcessingPanel;

	PropertyNotifier<bool> showSBSGuidance;
	PropertyNotifier<double> stentBoostPhaseDuration;
	PropertyNotifier<double> stentBoostTimeRemaining;
	PropertyNotifier<bool> stentBoostPhaseActive;
	PropertyNotifier<bool> contrastPhaseActive;
};
}}
