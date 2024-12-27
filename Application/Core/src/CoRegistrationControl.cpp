// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegistrationControl.h"
#include "Sense.h"
#include "ExperienceIdentity.h"
#include "ViewType.h"
#include "AlluraColors.h"

#include "CoRegistrationPressureAndEcg.h"
#include "CoRegistrationIfrPullbackCycle.h"
#include "CoRegistrationPdPaInfo.h"
#include "CoRegistrationServerState.h"
#include "CoRegistrationSpotMeasurement.h"
#include "CoRegistrationOptions.h"
#include "CoRegistrationError.h"

#include "Translation.h"
#include "Resources.h"


using namespace Sense;
using namespace ExperienceIdentity;
using namespace CommonPlatform::CoRegistration;

namespace Pci { namespace Core {

static const Color		  SpotColor	       = Color(1.0, 1.0, 0.0);
static const Color		  PullbackColor    = Color(0.0, 1.0, 1.0);
static const int		  MaxProgress	   = 10;
static const int		  StrBufferSize    = 32;
static const int		  ProgressInterval = 1;
static const std::wstring InvalidFmValue   = L"---";
static const float		  MaxIfrTrend	   = 1.0f;
static const float		  MinIfrTrend	   = 0.4f;

CoRegistrationControl::CoRegistrationControl(Control &parent, ViewType type) :
	PciBaseViewer(parent, type),
	gui(*this, type),
	userLocale(_create_locale(LC_ALL, "")),
	fmDisplay(*this, userLocale),
	pdPaDisplay(*this, userLocale),
	totalPdPaDisplay(*this, userLocale),
	recording(nullptr),
	currentPdPaInfo(),
	currentState(),
	m_coregConnected(false),
	m_lastError(CoRegistrationError::OK),	
	m_enableValueUpdates(true),
	progressTimer(scheduler, [&]{ onProgressTimerTick(); })
{
	hidePopupPanel();
	hideIfrReviewPanel();
	hideIfrSpotReviewPanel();

	setGraphFull(pdPaDisplay);
	setGraphTop(fmDisplay);
	setGraphTop(totalPdPaDisplay);

	totalPdPaDisplay.visible = false;
	totalPdPaDisplay.hidePressure();
	totalPdPaDisplay.showPdPaRatio();

	gui.ifrReviewFmTypeLabel->text	= Localization::Translation::IDS_IFR + L" " + Localization::Translation::IDS_Distal;
	gui.ifrReviewTrendLabel->text	= Localization::Translation::IDS_IFR +  L" " + Localization::Translation::IDS_Trend;
	gui.ifrSpotReviewLabel->text    = Localization::Translation::IDS_IFR;
	gui.ifrSpotReviewValue->color	= viewType == ViewType::Allura? Palette::Black : PullbackColor;
	
	gui.ifrReviewDistalFmValue->color =  viewType == ViewType::Allura? Palette::Black:PullbackColor;
	gui.ifrReviewTrendValue->color =  viewType == ViewType::Allura? Palette::Black:PullbackColor;
	gui.popupHeader->color = Palette::Gray140;

	gui.fmTypeLabel->text = Localization::Translation::IDS_FFR;
	gui.pdLabel->text	  = Localization::Translation::IDS_Pd;
	gui.pdValue->color    = viewType == ViewType::Allura? Palette::Gray040:SpotColor;	
	gui.paLabel->text	  = Localization::Translation::IDS_Pa;
	gui.pdPaLabel->text	  = Localization::Translation::IDS_Pd + L"/" + Localization::Translation::IDS_Pa;
	gui.pdPaPanel->color  = Palette::Transparent;
	gui.ifrReview->color  = viewType == ViewType::Allura? PullbackColor:Palette::Transparent;

	gui.pdLabel->color = viewType == ViewType::Allura? Palette::Gray040: SpotColor;
	gui.fmTypeValue->color = viewType == ViewType::Allura ? Palette::Gray040 : PullbackColor;
	gui.fmTypePanel->color = Palette::Transparent;
	gui.graphPanel->color =  viewType == ViewType::Allura ? Palette::Black :Palette::Transparent;
	gui.topPannel->color = Palette::Transparent;

	gui.ifrProgressPanel->visible = false;
	gui.ifrProgressBar->max = MaxProgress;
	gui.ifrProgressBar->color = viewType == ViewType::Allura ? SpotColor:
															   Gradient(Palette::White,PullbackColor,Point(0.0,0.0) ,Point(0.0,18.0));

	invalidateValues();
}

CoRegistrationControl::~CoRegistrationControl()
{
	_free_locale(userLocale);
}

void CoRegistrationControl::setGraphTop(Control& graph)
{
	auto parentSpace = getSpace();
	auto graphSpace = gui.graphPanel->getSpace();
	auto halfHeight = gui.graphPanel->size->height / 2.0;

	auto graphTopSize = gui.graphPanel->getBounds();

	graphTopSize.bottom -= halfHeight;
	graphTopSize = parentSpace.mapFrom(graphSpace, graphTopSize);

	graph.setBounds(graphTopSize);
}

void CoRegistrationControl::setGraphBottom(Control& graph)
{
	auto parentSpace = getSpace();
	auto graphSpace = gui.graphPanel->getSpace();
	auto halfHeight = gui.graphPanel->size->height / 2.0;

	auto graphBottomSide = gui.graphPanel->getBounds();
	graphBottomSide.top += halfHeight;

	graphBottomSide = parentSpace.mapFrom(graphSpace, graphBottomSide);
	graph.setBounds(graphBottomSide);
}

void CoRegistrationControl::setGraphFull(Control& graph)
{
	auto parentSpace = getSpace();
	auto graphSpace = gui.graphPanel->getSpace();
	auto graphBounds = gui.graphPanel->getBounds();
	graphBounds =  parentSpace.mapFrom(graphSpace,graphBounds);
	graph.setBounds(graphBounds);
}


void CoRegistrationControl::setPressureAndEcg(const CoRegistrationPressureAndEcg &pressureAndEcg)
{
	if (notConnecedOrUpdatesDisabled())
	{
		return;
	}

	if (pdPaDisplay.isEmpty())
	{
		pdPaDisplay.setTimestampStart(pressureAndEcg.timestamp);
	}
	
	if (currentState.systemState != CoRegistrationServerState::SystemState::Review) 
	{
		pdPaDisplay.addLiveSample(pressureAndEcg, currentPdPaInfo);
	}

	if (currentState.fmState == CoRegistrationServerState::FmState::FFR)
	{
		pdPaDisplay.showPdPaRatio();
	}
}

void CoRegistrationControl::setIfrPullbackCycle(const CoRegistrationIfrPullbackCycle &cycle)
{
	if (notConnecedOrUpdatesDisabled())
	{
		return;
	}

	if (currentState.systemState == CoRegistrationServerState::SystemState::Pullback)
	{
		gui.fmTypeValue->text = ifrFloatToString(cycle.ifrRawValue);
		fmDisplay.setTimestampStart(cycle.runStartedTime);
		fmDisplay.setIfrPullbackCycle(cycle);
	}
}

void CoRegistrationControl::setSpotMeasurement(const CoRegistrationSpotMeasurement &spot)
{
	if (notConnecedOrUpdatesDisabled())
	{
		return;
	}

	gui.fmTypeValue->text = ifrFloatToString(spot.fmValue);
	setFfrRecording(recording, spot.timestamp);
}

void CoRegistrationControl::setPdPaInfo(const CoRegistrationPdPaInfo &pdPaInfo)
{
	if (notConnecedOrUpdatesDisabled())
	{
		return;
	}

	currentPdPaInfo = pdPaInfo;

	gui.pdValue->text = std::to_wstring( static_cast<int>(pdPaInfo.mapPdValue+0.5));
	gui.paValue->text = std::to_wstring( static_cast<int>(pdPaInfo.mapPaValue+0.5));
	gui.pdPaValue->text = pdPaDoubleToString(pdPaInfo.pdPaRatio);
}

void CoRegistrationControl::clearGraphs()
{
	pdPaDisplay.reset();
	totalPdPaDisplay.reset();
	fmDisplay.reset();
}

bool CoRegistrationControl::allGraphsEmpty() const
{
	return pdPaDisplay.isEmpty() && totalPdPaDisplay.isEmpty() && fmDisplay.isEmpty();
}

bool CoRegistrationControl::allValuesInvalid() const
{
	return gui.ifrReviewDistalFmValue->text == InvalidFmValue &&
           gui.ifrReviewTrendValue->text == InvalidFmValue &&
           gui.ifrSpotReviewValue->text == InvalidFmValue &&
           gui.fmTypeValue->text == InvalidFmValue &&
           gui.pdPaValue->text == InvalidFmValue &&
           gui.pdValue->text == InvalidFmValue &&
           gui.paValue->text == InvalidFmValue;
}

void CoRegistrationControl::clearGraphsAndValues()
{
	clearGraphs();
	invalidateValues();
	enableValueUpdates(false);
}

void CoRegistrationControl::setCoRegState(const ::CommonPlatform::CoRegistration::CoRegistrationServerState & nextState)
{
	if (currentState != nextState)
	{
		switch (nextState.fmState)
		{
		case CoRegistrationServerState::FmState::FFR:
			gui.fmTypeLabel->text = Localization::Translation::IDS_FFR;
			gui.fmTypeLabel->color = viewType == ViewType::Allura ?Palette::Black: Palette::Gray070;
			gui.fmTypeValue->color = viewType == ViewType::Allura ?Palette::Black: Palette::Gray070;
			gui.fmTypePanel->color = viewType == ViewType::Allura ? SpotColor :Palette::Transparent;			
			break;
		case CoRegistrationServerState::FmState::IFRPullback:
		case CoRegistrationServerState::FmState::IFRSpot:
			gui.fmTypeLabel->text = Localization::Translation::IDS_IFR;
			if (nextState.systemState == CoRegistrationServerState::SystemState::SpotMeasurement)
			{
				gui.fmTypeLabel->color = viewType == ViewType::Allura ?Palette::Black: Palette::Gray070;
				gui.fmTypeValue->color = viewType == ViewType::Allura ?Palette::Black: Palette::Gray070;
				gui.fmTypePanel->color = viewType == ViewType::Allura ? SpotColor :Palette::Transparent;	
			}
			else
			{
				gui.fmTypeLabel->color = viewType == ViewType::Allura ? Palette::Gray040  : Palette::Gray070;
				gui.fmTypeValue->color = viewType == ViewType::Allura ? Palette::Gray040  : PullbackColor;
				gui.fmTypePanel->color = Palette::Transparent;
			}

			break;
		case CoRegistrationServerState::FmState::Unknown:
			break;
		default:
			break;
		}

		if (currentState.systemState != CoRegistrationServerState::SystemState::Freeze)
		{
			clearGraphs();
		}

		switch (nextState.systemState)
		{
		case CoRegistrationServerState::SystemState::SpotMeasurement:
			hidePopupPanel();
			showValuePanel(nextState);
			hideIfrSpotReviewPanel();
			hideIfrReviewPanel();
			invalidateValues();
			enableValueUpdates(currentState.systemState == CoRegistrationServerState::SystemState::Live);
			break;
		case CoRegistrationServerState::SystemState::Pullback:
			hidePopupPanel();
			resetFmDisplayScale();
			showFmDisplay();
			showValuePanel(nextState);
			hideIfrSpotReviewPanel();
			hideIfrReviewPanel();
			invalidateValues();
			enableValueUpdates(currentState.systemState == CoRegistrationServerState::SystemState::Live);
			break;
		case CoRegistrationServerState::SystemState::Live:
			hidePopupPanel();
			recording.reset();
			hideTotalPdPaDisplay();
			hideFmDisplay();
			hideIfrSpotReviewPanel();
			hideIfrReviewPanel();
			showValuePanel(nextState);
			invalidateValues();
			enableValueUpdates(true);
			if (nextState.fmState == CoRegistrationServerState::FmState::FFR)
			{
				pdPaDisplay.showPdPaRatio();
			}
			break;
		case CoRegistrationServerState::SystemState::Review:
			showPopupPanel(nextState.fmState);
			if (nextState.fmState == CoRegistrationServerState::FmState::IFRSpot)
			{ 
				showIfrSpotReviewPanel();
				hideIfrReviewPanel();
				hideTotalPdPaDisplay();
				hideValuePanel();
			}
			else if (nextState.fmState == CoRegistrationServerState::FmState::IFRPullback)
			{
				showIfrReviewPanel();
				hideIfrSpotReviewPanel();
				hideTotalPdPaDisplay();
				showFmDisplay();
				hideValuePanel();
			}
			else if (nextState.fmState == CoRegistrationServerState::FmState::FFR)
			{
				showTotalPdPaDisplay();
				hideIfrReviewPanel();
				hideIfrSpotReviewPanel();
				showValuePanel(nextState);
			}

			if ((m_lastError == CoRegistrationError::LatencyTooHigh) ||
				(m_lastError == CoRegistrationError::UnreliableDataTransfer))
			{
				clearGraphsAndValues();
			}

			break;
		case (CoRegistrationServerState::SystemState::Zero):
		case (CoRegistrationServerState::SystemState::Freeze):
		case (CoRegistrationServerState::SystemState::Idle):
		case (CoRegistrationServerState::SystemState::Normalization):
		default:
			hidePopupPanel();
			break;
		}

		currentState = nextState;
	}
}

void CoRegistrationControl::resetFmDisplayScale()
{
	fmDisplay.setStaticIfrScale(MaxIfrTrend,MinIfrTrend);
	fmDisplay.enableDynamicScaling(true);
}

void CoRegistrationControl::setCoRegOptions(const ::CommonPlatform::CoRegistration::CoRegistrationOptions &options)
{
	double cursorPosition = pdPaDisplay.getCursorPosition();

	if (currentState.systemState == CoRegistrationServerState::SystemState::Pullback)
	{
		fmDisplay.setStaticIfrScale(options.maxIfr, options.minIfr);
	}

	fmDisplay.showRawLine(options.showIfrRawLine);
	totalPdPaDisplay.setStaticPressureScale(options.maxPressure, options.minPressure);
	pdPaDisplay.setStaticPressureScale(options.maxPressure, options.minPressure);
	pdPaDisplay.showWaveFreePeriods(options.showWaveFreePeriods);

	if (recording)
	{
		setRecording(recording);
		setFfrRecording(recording, cursorPosition);
	}

}

void CoRegistrationControl::setReview(double timestamp)
{
	fmDisplay.setCursor(timestamp);

	float correspondingIfrValue = 0.0f;
	bool validIFR = recording->getIfrTrendValueAt(timestamp, correspondingIfrValue);
	gui.ifrReviewTrendValue->text	   = validIFR ? ifrFloatToString(correspondingIfrValue) : InvalidFmValue;
	gui.ifrSpotReviewValue->text	   = validIFR ? ifrFloatToString(correspondingIfrValue) : InvalidFmValue;

	pdPaDisplay.reset();
	pdPaDisplay.setTimestampStart(timestamp - (pdPaDisplay.getMaxX() / 2.0));
	pdPaDisplay.setCursor(timestamp);
	pdPaDisplay.setRecording(recording);

	totalPdPaDisplay.reset();
	totalPdPaDisplay.showPdPaRatio();
	totalPdPaDisplay.setRecording(recording, true);
	totalPdPaDisplay.setCursor(timestamp);
}

std::wstring CoRegistrationControl::ifrFloatToString(float ifrValue)
{
	wchar_t str[StrBufferSize];
	_swprintf_s_l(str, StrBufferSize, L"%.2f", userLocale, ifrValue);
	return str;
}

std::wstring CoRegistrationControl::pdPaDoubleToString(double pdPaValue)
{
	wchar_t str[StrBufferSize];
	_swprintf_s_l(str, StrBufferSize, L"%.2f", userLocale, pdPaValue);
	return str;
}

void CoRegistrationControl::setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image)
{
	if (currentState.systemState == CoRegistrationServerState::SystemState::Pullback)
	{
		fmDisplay.setImage(image);
	}
}

void CoRegistrationControl::endLive()
{
	fmDisplay.endLive();
}

void CoRegistrationControl::showFmDisplay()
{
	fmDisplay.visible = true;
	setGraphTop(fmDisplay);
	setGraphBottom(pdPaDisplay);
}

void CoRegistrationControl::hideFmDisplay()
{
	fmDisplay.visible = false;
	setGraphFull(pdPaDisplay);
}

void CoRegistrationControl::showValuePanel(const CoRegistrationServerState& coRegState)
{
	gui.IfrValuePanel->visible = true;

	if (coRegState.fmState == CoRegistrationServerState::FmState::FFR)
	{
		gui.ifrProgressPanel->visible = false;
		gui.fmTypeValue->visible = true;
	}

	if (coRegState.fmState == CoRegistrationServerState::FmState::IFRSpot)
	{
		if (coRegState.systemState == CoRegistrationServerState::SystemState::SpotMeasurement)
		{
			//during an IFR spot measurement we show the progress bar.
			gui.ifrProgressPanel->visible = true;
			gui.fmTypeValue->visible = false;
			gui.ifrProgressBar->value		= MaxProgress;
			progressTimer.start(ProgressInterval);
		}
		else
		{
			gui.ifrProgressBar->value		= 0;
			progressTimer.stop();

			gui.ifrProgressPanel->visible = false;
			gui.fmTypeValue->visible = true;
		}
	}

}

void CoRegistrationControl::hideValuePanel()
{
	gui.IfrValuePanel->visible = false;
}

void CoRegistrationControl::showTotalPdPaDisplay()
{
	totalPdPaDisplay.visible = true;
	setGraphTop(totalPdPaDisplay);
	setGraphBottom(pdPaDisplay);

	fmDisplay.visible = false;
}

void CoRegistrationControl::hideTotalPdPaDisplay()
{
	totalPdPaDisplay.visible = false;
	setGraphFull(pdPaDisplay);
}

void CoRegistrationControl::setIfrSpotResult(float iFrSpotResult)
{
	if (notConnecedOrUpdatesDisabled())
	{
		return;
	}

	gui.fmTypeValue->text             = ifrFloatToString(iFrSpotResult);
	gui.ifrSpotReviewValue->text      = ifrFloatToString(iFrSpotResult);
}

void CoRegistrationControl::showIfrReviewPanel()
{
	gui.ifrReview->visible = true;
}

void CoRegistrationControl::hideIfrReviewPanel()
{
	gui.ifrReview->visible = false;
}

void CoRegistrationControl::showIfrSpotReviewPanel()
{
	gui.ifrSpotReviewPanel->visible = true;
}

void CoRegistrationControl::hideIfrSpotReviewPanel()
{
	gui.ifrSpotReviewPanel->visible = false;
}

void CoRegistrationControl::showPopupPanel(CoRegistrationServerState::FmState state)
{
	gui.popupPannel->color = Palette::Gray170;
	gui.popupPannel->colorEdge = Palette::Gray140;
	gui.popupHeader->color = Palette::Gray140;
	gui.popupHeaderText->visible = true;
	gui.popupHeaderText->imageSize = Size(20.0,20.0);
	pdPaDisplay.setRacerBarColor(viewType == ViewType::Allura? Palette::Black : UI::PanelDropShadow);
	totalPdPaDisplay.setRacerBarColor(viewType == ViewType::Allura? Palette::Black : UI::PanelDropShadow);
	
	switch (state)
	{
	case CommonPlatform::CoRegistration::CoRegistrationServerState::FmState::FFR:
		gui.popupHeaderText->text   = L"FFR";
		gui.popupHeaderText->image  = ::PciSuite::Resources::ffrReview;
		break;
	case CommonPlatform::CoRegistration::CoRegistrationServerState::FmState::IFRSpot:
		gui.popupHeaderText->text   = L"iFR Spot";
		gui.popupHeaderText->image  = ::PciSuite::Resources::IfrReview;
		break;
	case CommonPlatform::CoRegistration::CoRegistrationServerState::FmState::IFRPullback:
		gui.popupHeaderText->text   = L"iFR Pullback";
		gui.popupHeaderText->image  = ::PciSuite::Resources::IfrReview;
		break;
	case CommonPlatform::CoRegistration::CoRegistrationServerState::FmState::Unknown:
	default:
		gui.popupHeaderText->text   = L"";
		break;
	}

}

void CoRegistrationControl::hidePopupPanel()
{
	gui.popupPannel->color = Palette::Transparent;
	gui.popupPannel->colorEdge = Palette::Transparent;
	gui.popupHeader->color = Palette::Transparent;
	pdPaDisplay.setRacerBarColor(Palette::Black);
	totalPdPaDisplay.setRacerBarColor(Palette::Black);
	gui.popupHeaderText->visible = false;
}

void CoRegistrationControl::setRecording(const std::shared_ptr<PullbackRecording> &rec)
{
	if (!rec)
	{
		clearGraphsAndValues();
		return;
	}

	recording = rec;

	fmDisplay.setRecording(recording);

	if ((recording->getMeasurementError() != CoRegistrationError::IFRError) &&
		(recording->getMeasurementError() != CoRegistrationError::FFRError))
	{
		float distalIFR;
		bool validIFR = recording->getDistalFm(distalIFR);
		gui.ifrReviewDistalFmValue->text = validIFR ? ifrFloatToString(distalIFR) : InvalidFmValue;
	}

	if (!recording->getAllPressureAndEcg().empty())
	{
		// Set start point of IFR trend graph equal to start of pressure data
		fmDisplay.setTimestampStart(recording->getAllPressureAndEcg().front().first.timestamp);
	}

	if (currentState.fmState == CoRegistrationServerState::FmState::IFRPullback)
	{
		if (!recording->getAllPressureAndEcg().empty())
		{
			pdPaDisplay.setTimestampStart(recording->getAllPressureAndEcg()[0].first.timestamp);
		}

		pdPaDisplay.setRecording(recording, false);
	}
	else if (currentState.fmState == CoRegistrationServerState::FmState::IFRSpot)
	{
		pdPaDisplay.setRecording(recording, true);
	}
	else if (currentState.fmState == CoRegistrationServerState::FmState::FFR)
	{
		totalPdPaDisplay.setRecording(recording, true);
		pdPaDisplay.setRecording(recording, false);
	}
}

void CoRegistrationControl::setFfrRecording(const std::shared_ptr<PullbackRecording> &newRecording, double cursorPosition)
{
	if ((currentState.systemState == CoRegistrationServerState::SystemState::Review) && 
		(currentState.fmState == CoRegistrationServerState::FmState::FFR) && newRecording)
	{
		double timestamp = cursorPosition;
		pdPaDisplay.reset();
		pdPaDisplay.setTimestampStart(timestamp - (pdPaDisplay.getMaxX() / 2.0));
		pdPaDisplay.setCursor(timestamp);
		pdPaDisplay.showPdPaRatio();
		pdPaDisplay.setRecording(newRecording);

		totalPdPaDisplay.reset();
		totalPdPaDisplay.showPdPaRatio();
		totalPdPaDisplay.setRecording(newRecording, true);
		totalPdPaDisplay.setCursor(timestamp);
	}
}

void CoRegistrationControl::setCoRegConnection(bool connected)
{
	if (!connected)
	{
		clearGraphsAndValues();
	}

	m_coregConnected = connected;
}

void CoRegistrationControl::setCoRegError(CoRegistrationError error)
{
	m_lastError = error;

	if ((error == CoRegistrationError::IFRError) || (error == CoRegistrationError::FFRError))
	{
		enableValueUpdates(false);
	}
}

void CoRegistrationControl::invalidateValues()
{
	gui.ifrReviewDistalFmValue->text = InvalidFmValue;
	gui.ifrReviewTrendValue->text	 = InvalidFmValue;
	gui.ifrSpotReviewValue->text	 = InvalidFmValue;
	gui.fmTypeValue->text			 = InvalidFmValue;

	gui.pdPaValue->text				 = InvalidFmValue;
	gui.pdValue->text				 = InvalidFmValue;
	gui.paValue->text				 = InvalidFmValue;
}

void CoRegistrationControl::enableValueUpdates(bool enable)
{
	m_enableValueUpdates = enable;
}

void CoRegistrationControl::onProgressTimerTick()
{
	//use a timer for the progressbar updates in IFRSpot, 
	//because if pressure values don't change during a spot measurement no updates are received.
	if (currentState.fmState == CoRegistrationServerState::FmState::IFRSpot)
	{
		gui.ifrProgressBar->value = gui.ifrProgressBar->value - 1; 
	}
}


bool CoRegistrationControl::isIfrValuesHighLighted() const
{
	// The type of color is a gradient. In this case we know that it is a single color.
	// Therefore we use the color1 member of this gradient to test if it is not transparent.
	return gui.fmTypePanel->color->color1.a > 0.0;
}

bool CoRegistrationControl::notConnecedOrUpdatesDisabled() const
{
	return !m_coregConnected || !m_enableValueUpdates;
}

}}