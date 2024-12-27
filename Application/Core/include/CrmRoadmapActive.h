// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "CrmGuidanceBaseState.h"

namespace Pci{ namespace Core
{
	class CrmRoadmapActive : public CrmGuidanceBaseState
	{
		friend class TestCrmRoadmapActiveState;
	public:
		explicit CrmRoadmapActive(PciViewContext &viewContext);

		virtual void onEnter() override;
		virtual ViewState roadmapStatusChanged() override;
		virtual ViewState geometryChanged() override;

	private:
		void updateView();

		void onNavigateLeftDown();
		void onNavigateRightDown();

		void onLockUnlockRoadmap();

		void onRoadmapLocked();
		void onRoadmapUnlocked();


		void setSelection(int index);

		std::vector<std::shared_ptr<const CrmRoadmap> > m_roadmaps;
		int m_selectedRoadmap;
	};
}}

