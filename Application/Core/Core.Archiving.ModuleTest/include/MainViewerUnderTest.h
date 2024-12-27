// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "MainViewer.h"

class MainViewerUnderTest : public ::Pci::Core::MainViewer
{
public:
	MainViewerUnderTest(Sense::Control& parent, Pci::Core::MainModel& model, Pci::Core::ViewType type) :
		MainViewer(parent, model, type)
	{
	}

	// Test functions to access protected data of MainViewer class
	std::wstring getPatientName()
	{
		if (alluraView.visible)
		{
			return alluraView.patientName.text;
		}
		
		if (smsFlexvisionView.visible)
		{
			return smsFlexvisionView.patientBar.patientData->name;
		}
		
		if (smsFullHdView.visible)
		{
			return smsFullHdView.patientBar.patientData->name;
		}

		return L"";
	}

};