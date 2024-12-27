// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "StateControlUtility.ui.h"

#include "CrmAdministration.h"

#include <map>
#include <string>
#include <memory>

namespace Pci { namespace Core
{
	enum class GuidanceState;
	class MainControllerTestServer;
	class CrmRoadmap;

	class UtilityCanvas : public Sense::ExperienceIdentity::Canvas
	{
	public:
		UtilityCanvas(Sense::Gpu& gpu, Sense::IScheduler& scheduler);

		virtual void onResize() override;

		UI::StateControlUtility Ui;
	};

	//
	// this for test purposes
	//
	class StateControlUtility
	{
	public:
		StateControlUtility(MainControllerTestServer& pciViewer, Sense::Gpu& gpu, Sense::IScheduler& scheduler);

		void onGuidanceStateChanged(GuidanceState state, CrmAdministration::Status adminStatus);

	private:
		void setSelectedState();

		struct GuidanceProperties
		{
			GuidanceProperties(GuidanceState state, bool roadmapAvailable = true) : State(state), RoadmapAvailable(roadmapAvailable)
			{}

			GuidanceState State;
			bool RoadmapAvailable;
		};

		MainControllerTestServer&					m_pciViewer;
		std::map<std::wstring, GuidanceProperties>	m_stateMap;
		std::shared_ptr<CrmRoadmap>					m_roadmap;
		UtilityCanvas								m_canvas;
		Sense::OutputWindow							m_output;
		Sense::UserMouseKeyboard					m_mouseKeyboard;
	};

}}