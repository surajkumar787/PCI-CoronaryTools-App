
// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.


#include "MessageTester.h"

#include "MainModel.h"
#include "MainViewer.h"
#include "IArchivingStatus.h"
#include "CoRegistrationServerState.h"

#include "UserMouseKeyboardMonitor.h"
#include "Translation.h"
#include "TranslationHelper.h"

#include "ViewType.h"

#include <Sense/Common/Size.h>
#include <Sense/Common/Rect.h>
#include <Sense/Common/Types.h>
#include <Sense/Common/Timer.h>
#include <Sense/Controls/Control.h>
#include <Sense/System/Scheduler.h>

#include <ExperienceIdentity/Controls/Label.h>
#include <ExperienceIdentity/Controls/Canvas.h>

#include <vector>

using namespace ::Pci::Core;
using namespace CommonPlatform::CoRegistration;

namespace
{
	Sense::Size getLabelTextMargin(const Sense::ExperienceIdentity::Label& label)
	{
		Sense::Size size = label.size;
		Sense::Size requiredSize = label.measureTextSize();

		return size - requiredSize;
	}

	bool labelTextFits(const Sense::ExperienceIdentity::Label& label)
	{
		auto margin = getLabelTextMargin(label);
		auto fits = margin.width >= -2 && margin.height >= -2;
		return fits;
	}

	Sense::Rect getAbsoluteRect(const Sense::Control& parent, const Sense::Control& child)
	{
		return parent.getSpace().mapFrom(child.getSpace(), child.getBounds());
	}

	Sense::Rect getBoundingBoxLabelText(const Sense::Control& parent, const Sense::ExperienceIdentity::Label& label)
	{
		auto margin = getLabelTextMargin(label);

		double textVOffset = 0;
		switch (label.verticalAlign)
		{
		case Sense::VAlign::Top:    textVOffset = 0;                 break;
		case Sense::VAlign::Bottom: textVOffset = margin.height;     break;
		case Sense::VAlign::Middle: textVOffset = margin.height / 2; break;
		}

		Sense::Rect labAbsBounds = getAbsoluteRect(parent, label);
		Sense::Rect textAbsBounds = labAbsBounds + Sense::Point(0, textVOffset);

		return textAbsBounds;
	}

	bool labelTextsOverlap(const Control& parent, const Sense::ExperienceIdentity::Label& label1, const Sense::ExperienceIdentity::Label& label2)
	{
		return getBoundingBoxLabelText(parent, label1).contains(getBoundingBoxLabelText(parent, label2));
	}

	template<typename View>
	bool allLabelTextsFit(const View& view)
	{
		bool fits = true;
		fits &= labelTextFits(view.labelCoregInfo);
		fits &= labelTextFits(view.labelMessageArea1);
		fits &= labelTextFits(view.labelMessageArea2);

		fits &= !labelTextsOverlap(view, view.labelCoregInfo, view.labelMessageArea1);
		fits &= !labelTextsOverlap(view, view.labelMessageArea1, view.labelMessageArea2);

		return fits;
	}
}

struct MessageTester::Private
{
	struct TestCase
	{
		ViewType        view;
		std::wstring    language;
		ArchivingStatus snapshotStatus;
		CaptureArea     area;
		ArchivingStatus movieStatus;
		bool            showCoregStatus;
	};

	Private(Sense::Scheduler& scheduler, Sense::ExperienceIdentity::Canvas& canvas, MainModel& model, Config config, UserMouseKeyboardMonitor* user)
		: m_scheduler(scheduler)
		, m_canvas(canvas)
		, m_model(model)
		, m_config(config)
		, m_user(user)
		, m_testTimer(scheduler, [this] { runNextTest(); })
		, m_reviewTimer(scheduler, [this] { reviewNextFailure(); })
	{
		initTestVector();
		startTests();
	}

	void initTestVector()
	{
		auto views = { ViewType::Allura, ViewType::SmartSuiteFlexVision, ViewType::SmartSuite19Inch, ViewType::SmartSuiteFullHD };

		auto languages = { L"da", L"de", L"en", L"es", L"fr", L"it", L"nl", L"no", L"sv", L"ja", L"zh-Hans", L"zh-Hant" };

		auto status = { ArchivingStatus::None, ArchivingStatus::Ready, ArchivingStatus::Busy,
		                ArchivingStatus::Success, ArchivingStatus::Failure };

		auto areas = { CaptureArea::MainView, CaptureArea::MainAndSideView };

		for (const auto& view     : views)
		for (const auto& language : languages)
		for (const auto& snapshot : status)
		for (const auto& area     : areas)
		for (const auto& movie    : status)
		{
			m_tests.push_back({ view, language, snapshot, area, movie, true });
		}
	}

	void startTests()
	{
		m_next = m_tests.begin();
		if (m_user && m_config.manualStepping)
		{
			m_user->onKeyPress = std::bind(&Private::runNextTest, this);
		}
		else
		{
			m_testTimer.start(Sense::Timer::Priority::Fast);
		}
	}

	void runNextTest()
	{
		if (m_next == m_tests.end())
		{
			m_testTimer.stop();
			onTestsDone();
		}
		else
		{
			runTest(*m_next);
			++m_next;
		}
	}

	void runTest(const TestCase& test)
	{
		bool fits = applyTest(test);

		if (!fits)
		{
			m_failed.push_back(test);
		}
	}

	bool applyTest(const TestCase& test)
	{
		bool  languagePackEnabled = true;
		Localization::Translation::setLanguage( Localization::getLanguageId( test.language, languagePackEnabled));

		m_model.setTsmFunctionsAvailable(true);

		m_model.setSnapshotStatus(test.snapshotStatus, test.area);
		m_model.setMovieStatus   (test.movieStatus);

		m_model.setCoregStatusText(test.showCoregStatus
								   ? Localization::Translation::IDS_UnreliableIFRConnection + L". " + Localization::Translation::IDS_ContactServiceProvider
								   : L"");

		setView(test.view);

		bool fits = true;
		switch (test.view)
		{
		case ViewType::Allura:
			m_view->updateAllura();
			fits &= allLabelTextsFit(m_view->alluraView);
			break;
		case ViewType::SmartSuiteFlexVision:
			m_view->updateSmartSuite(m_view->smsFlexvisionView, m_model);
			fits &= allLabelTextsFit(m_view->smsFlexvisionView);
			break;
		case ViewType::SmartSuite19Inch:
			m_view->updateSmartSuite(m_view->sms19InchView, m_model);
			fits &= allLabelTextsFit(m_view->sms19InchView);
			break;
		case ViewType::SmartSuiteFullHD:
			m_view->updateSmartSuite(m_view->smsFullHdView, m_model);
			fits &= allLabelTextsFit(m_view->smsFullHdView);
			break;
		}

		return fits;
	}

	void setView(ViewType viewType)
	{
		m_view = std::make_unique<MainViewer>(m_canvas, m_model, viewType);
		m_canvas.size = m_view->size;
		m_view->visible = true;

		m_view->coRegControl.showFmDisplay();
		m_view->coRegControl.showIfrSpotReviewPanel();
		m_view->coRegControl.showIfrReviewPanel();
		m_view->coRegControl.showPopupPanel(CoRegistrationServerState::FmState::IFRPullback);
	}

	void onTestsDone()
	{
		if (m_config.reviewFailures)
		{
			clearView();
			startReview();
		}
		else
		{
			m_scheduler.exit((int)m_failed.size());
		}
	}

	void clearView()
	{
		applyTest({ViewType::SmartSuiteFlexVision, L"en",
				   ArchivingStatus::None, CaptureArea::MainAndSideView, ArchivingStatus::None,
				   false });
	}

	void startReview()
	{
		m_next = m_failed.begin();

		if (m_user)
		{
			m_user->onKeyPress = std::bind(&Private::reviewNextFailure, this);
		}
		else
		{
			m_reviewTimer.start(Sense::Timer::Priority::Slow);
		}
	}

	void reviewNextFailure()
	{
		if (m_next == m_failed.end())
		{
			m_next = m_failed.begin();
		}
		else
		{
			applyTest(*m_next);
			++m_next;
		}
	}

	Sense::Scheduler&                  m_scheduler;
	Sense::ExperienceIdentity::Canvas& m_canvas;
	MainModel&                         m_model;
	std::unique_ptr<MainViewer>        m_view;
	Config						       m_config;

	UserMouseKeyboardMonitor*          m_user;
	Sense::Timer                       m_testTimer;
	Sense::Timer                       m_reviewTimer;

	std::vector<TestCase>				  m_tests;
	std::vector<TestCase>::const_iterator m_next;
	std::vector<TestCase>				  m_failed;
};


MessageTester::MessageTester(Sense::Scheduler& scheduler, Sense::ExperienceIdentity::Canvas& canvas,
							 MainModel& model, Config config, UserMouseKeyboardMonitor* user)
	: m_p(new Private(scheduler, canvas, model, config, user))
{
}

MessageTester::~MessageTester()
{
}
