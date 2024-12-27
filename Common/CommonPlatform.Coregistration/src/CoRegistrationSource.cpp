// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegistrationSource.h"
#include "TcpClient.h"
#include "Timing.h"
#include "Log.h"
#include "UtilizationLogEvent.h"

#include "CoRegistrationPressureAndEcg.h"
#include "CoRegistrationPdPaInfo.h"
#include "CoRegistrationSpotMeasurement.h"
#include "CoRegistrationIfrSpotCycle.h"
#include "CoRegistrationIfrPullbackCycle.h"
#include "CoRegistrationError.h"
#include "CoRegistrationOptions.h"
#include "CoRegistrationSystemInfo.h"

#include "Sense/System/IScheduler.h"
#include "Sense/Common/Timer.h"

#define NOMINMAX
#include <windows.h>
#include <string>
#include <assert.h>
#include "stlstring.h"

using namespace Sense;
using namespace CommonPlatform;

namespace CommonPlatform { namespace CoRegistration{

const double MaxCommandResponseTime     = 2.0;
const int	 minimalTimeSyncRetries		= 3;
const double minimalTimeSyncInterval	= 0.5;
const double maxTimeSyncLatency			= 0.05;
const int    maxNrOfPackageTooLate		= 20;
const size_t receiveBufferSize			= static_cast<size_t>(1024);
const double ConnectionMonitorInterval	= 3.0;
const double MaxContinousLatencyAllowed = 2.0;
const double maxTimeOffsetDifference	= 0.2;
const std::wstring SupportedProtocol    = L"1.";

enum class SystemCommand
{
	None,
	TimeSync,
	SystemState,
	SystemInfo
};

CoRegistrationSource::CoRegistrationSource(IScheduler &iScheduler) :
	CoRegistrationSource(iScheduler, std::make_shared<TcpClient>(L"CoRegistrationSource"), Logger(LoggerType::CoronaryRoadmap))
{
}

CoRegistrationSource::CoRegistrationSource(Sense::IScheduler &iScheduler, const std::shared_ptr<ITcpClient>& tcpClient, CommonPlatform::Log& logger) :
	activeCommand(SystemCommand::None),
	queuedCommand(SystemCommand::None),
	scheduler(iScheduler),
	connectionStatus(ConnectionStatus::Disconnected),
	totalReceived(0),
	receivedData(receiveBufferSize),
	sendData(0),
	currentRunIndex(0),
	localTimeAtTimeSyncInMs(0.0),
	timeSyncResponseCount(0),
	bestTimeSync(),
	log(logger),
	client(tcpClient),
	connectionMonitorInterval(ConnectionMonitorInterval),
	commandResponseTimeoutTimer(scheduler, [&] { onCommandResponseTimeout(); }),
	timeSyncTimer(scheduler, [&] { requestTimeSync(); }),
	connectionMonitorTimer(scheduler, [&] { onConnectionMonitorTick(); }),
	pressureTimeStampValidator(),
	ffrTimeStampValidator(),
	ifrTimeStampValidator()
{
	client->eventConnectionChanged = [this](bool isConnected) {onTcpConnectionStatusChanged(isConnected); };
	client->eventDataAvailable = [this]() {onDataAvailable(); };
}

CoRegistrationSource::~CoRegistrationSource()
{
	if(timeSyncTimer.isActive())
	{
		timeSyncTimer.stop();
	}

	disconnect();
}

bool CoRegistrationSource::connect(const std::wstring &host, int port)
{
	return client->connect(host, port);
}

void CoRegistrationSource::disconnect()
{
	client->disconnect();
}

void CoRegistrationSource::onTcpConnectionStatusChanged(bool isConnected)
{
	if (isConnected)
	{
		handleCommandRequest(SystemCommand::SystemInfo);
	}
	else
	{
		// Reset received data counter. There is no need to empty the receiveData buffer.
		totalReceived = 0;

		handleConnectionFailed();
	}
}

void CoRegistrationSource::onDataAvailable()
{
	bool allDataReceived = false;

	while(!allDataReceived)
	{
		if (totalReceived == receivedData.size()) 
		{
			receivedData.resize(receivedData.size() * 2);
		}

		int result = client->receive((char*)receivedData.data() + totalReceived, int(receivedData.size()-totalReceived));

		if(result > 0)
		{
			totalReceived += result;
			parse();
		}
		else
		{
			allDataReceived = true;
		}
	}
	allDataReceived = false;
}

void CoRegistrationSource::parse()
{
	int pos = 0;
	char* data = receivedData.data();

	while(pos + sizeof(PacketHeader) <= totalReceived)
	{
		PacketHeader &packet = *(PacketHeader*)(data + pos);
		if (pos + sizeof(PacketHeader) + packet.size > totalReceived) 
		{
			break;
		}

		parse(packet.tag, data + pos + sizeof(PacketHeader), packet.size);

		pos += sizeof(PacketHeader) + packet.size;
	}

	if (totalReceived > 0 && pos > 0)
	{
		totalReceived -= pos;
		memmove(receivedData.data(), receivedData.data() + pos, totalReceived);
	}
}

void CoRegistrationSource::parse(Tag tag, char* data, int length)
{
	if     (tag == StateDataTag          && length >= sizeof(StateData))             parseStateData             (reinterpret_cast<StateData*>              (data), length / sizeof(StateData)); 
	else if(tag == PressureAndEcgDataTag && length >= sizeof(PressureAndEcgData))    parsePressureAndEcgData    (reinterpret_cast<PressureAndEcgData*>     (data), length / sizeof(PressureAndEcgData));
	else if(tag == MAPTag                && length >= sizeof(MAPdata))               parseMapData               (reinterpret_cast<MAPdata*>                (data), length / sizeof(MAPdata));
	else if(tag == FMSpotTag             && length >= sizeof(FMSpotData))            parseSpotMeasurement       (reinterpret_cast<FMSpotData*>             (data), length / sizeof(FMSpotData));
	else if(tag == iFRSpotTag            && length >= sizeof(iFRSpotData))           parseIfrSpotCycleData      (reinterpret_cast<iFRSpotData*>            (data), length / sizeof(iFRSpotData));
	else if(tag == iFRSpotResultTag      && length >= sizeof(iFRSpotResultData))     parseIfrSpotResultData     (reinterpret_cast<iFRSpotResultData*>      (data), length / sizeof(iFRSpotResultData));
	else if(tag == iFRPullbackTag        && length >= sizeof(iFRPullbackData))       parseIfrPullbackCycleData  (reinterpret_cast<iFRPullbackData*>        (data), length / sizeof(iFRPullbackData));
	else if(tag == FmCurrentFrameDataTag && length >= sizeof(FmCurrentFrameMarker))  parseCurrentFrameMarker    (reinterpret_cast<FmCurrentFrameMarker*>   (data), length / sizeof(FmCurrentFrameMarker));
	else if(tag == ErrorDataTag          && length >= sizeof(Error))                 parseErrorData             (reinterpret_cast<Error*>                  (data), length / sizeof(Error));
	else if(tag == TimeSyncTag           && length >= sizeof(TimeSync))              parseTimeSyncResponse      (reinterpret_cast<TimeSync*>               (data), length / sizeof(TimeSync));
	else if(tag == ScaleOptionsTag		 && length >= sizeof(ScaleOptionsData))		 parseScaleOptionsData		(reinterpret_cast<ScaleOptionsData*>	   (data), length / sizeof(ScaleOptionsData));
	else if(tag == SysInfoTag			 && length >= sizeof(SysInfo))				 parseSystemInfo			(reinterpret_cast<SysInfo*>				   (data), length / sizeof(SysInfo));
	else log.DeveloperWarning(L"CoRegistrationSource:: Unknown package received. Packet info; Tag: %d, Packet size: %d", tag, length);
}

void CoRegistrationSource::parseStateData(const StateData* data, int count)
{
	for(int i = 0; i < count; i++)
	{
		coRegState.timestamp   = getSyncedTimestamp(data[i].timestamp);
		coRegState.fmState     = getFmStateFromStateData(data[i]);

		auto newState = getSystemStateFromStateData(data[i]);

		if ((coRegState.systemState != CoRegistrationServerState::SystemState::Pullback && newState == CoRegistrationServerState::SystemState::Pullback) ||
			(coRegState.systemState != CoRegistrationServerState::SystemState::SpotMeasurement && newState == CoRegistrationServerState::SystemState::SpotMeasurement))
			
		{
			requestTimeSync();
			startOfTimeStampValidation();
		}
		else if ((coRegState.systemState == CoRegistrationServerState::SystemState::Pullback && newState == CoRegistrationServerState::SystemState::Review) ||
				 (coRegState.systemState == CoRegistrationServerState::SystemState::SpotMeasurement && newState == CoRegistrationServerState::SystemState::Review))
		{
			lastDataReceived(coRegState.timestamp);
			stopTimeStampValidation();
		}

		if (coRegState.systemState != CoRegistrationServerState::SystemState::Normalization && newState == CoRegistrationServerState::SystemState::Normalization)
		{
			requestTimeSync();
		}

		coRegState.systemState = newState;

		coRegState.normalized               = (eNormalized              == (data[i].iStateBitmask & eNormalized));
		coRegState.ecgConnected             = (eEcgConnected            == (data[i].iStateBitmask & eEcgConnected));
		coRegState.demoModeActive           = (eDemoMode                == (data[i].iStateBitmask & eDemoMode));
		coRegState.pimConnected             = (ePimConnected            == (data[i].iStateBitmask & ePimConnected));
		coRegState.pressurePlugConnected    = (ePressurePlugConnected   == (data[i].iStateBitmask & ePressurePlugConnected));
		coRegState.wireConnected            = (eWireConnected           == (data[i].iStateBitmask & eWireConnected));
		coRegState.wireShorted				= (eWireShorted				== (data[i].iStateBitmask & eWireShorted));
		coRegState.wireReady				= (eWireReady				== (data[i].iStateBitmask & eWireReady));

		debugLogStateChanges(coRegState.systemState, coRegState.fmState);

		if(eventStateData)
		{
			eventStateData(coRegState);
		}
	}

	handleCommandResponse(SystemCommand::SystemState);
}

void CoRegistrationSource::parsePressureAndEcgData(const PressureAndEcgData* data, int count)
{
	for(int i = 0; i < count; i++)
	{
		CoRegistrationPressureAndEcg pressureAndEcg;
		
		pressureAndEcg.origTimestamp = data[i].timestamp;
		pressureAndEcg.timestamp	 = getSyncedTimestamp(data[i].timestamp);

		pressureAndEcg.paValue   = data[i].paVal;
		pressureAndEcg.pdValue   = data[i].pdVal;
		pressureAndEcg.ecgValue  = data[i].ecgVal;

		if ( !pressureTimeStampValidator.check(data[i].timestamp) )
		{
			log.DeveloperWarning(L"Timestamp validation failed on pressure and ecg data.");
			if(eventError) 
			{
				eventError(CoRegistrationError::UnreliableDataTransfer);
			}
		}

		if(eventPressureAndEcgData) 
		{
			eventPressureAndEcgData(pressureAndEcg);
		}
	}
}

void CoRegistrationSource::parseMapData(const MAPdata* data, int count)
{
	for(int i = 0; i < count; i++)
	{
		CoRegistrationPdPaInfo pdPaInfo;

		pdPaInfo.timestamp	= getSyncedTimestamp(data[i].timestamp);
		pdPaInfo.mapPaValue = data[i].mapPa;
		pdPaInfo.mapPdValue = data[i].mapPd;
		pdPaInfo.pdPaRatio	= data[i].pdpaRatio;

		if(eventPdPaInfo) 
		{
			eventPdPaInfo(pdPaInfo);
		}
	}
}

void CoRegistrationSource::parseSpotMeasurement(const FMSpotData* data, int count)
{
	for(int i = 0; i < count; i++)
	{
		CoRegistrationSpotMeasurement spotMeasurement;

		spotMeasurement.runIndex	  = data[i].whichRun;
		spotMeasurement.timestamp = getSyncedTimestamp(data[i].timestamp);
		spotMeasurement.fmValue   = data[i].fFm;

		debugLogRunIndexChanges(spotMeasurement.runIndex, L"FMSpotData");

		setCurrentRunIndex(spotMeasurement.runIndex);

		if ( !ffrTimeStampValidator.check(data[i].timestamp))
		{
			log.DeveloperWarning(L"Timestamp validation failed on FM spot measurement data.");
			if(eventError) 
			{
				eventError(CoRegistrationError::UnreliableDataTransfer);
			}
		}

		if(eventSpotMeasurement) 
		{
			eventSpotMeasurement(spotMeasurement);
		}
	}
}


void CoRegistrationSource::parseIfrSpotCycleData(const iFRSpotData* data, int count)
{
	for(int i = 0; i < count; i++)
	{
		CoRegistrationIfrSpotCycle iFrCycle;

		iFrCycle.runIndex			   = data[i].whichRun;
		iFrCycle.timestamp         = getSyncedTimestamp(data[i].timestamp);
		iFrCycle.isValidCycle      = data[i].validFlag;
		iFrCycle.waveFreeStartTime = getSyncedTimestamp(data[i].iWfp_start);
		iFrCycle.waveFreeEndTime   = getSyncedTimestamp(data[i].iWfp_end);

		debugLogRunIndexChanges(iFrCycle.runIndex, L"iFRSpotData");

		setCurrentRunIndex(iFrCycle.runIndex);

		if ( !ifrTimeStampValidator.check(data[i].timestamp))
		{
			log.DeveloperWarning(L"Timestamp validation failed on IFR cycle data.");
			if(eventError) 
			{
				eventError(CoRegistrationError::UnreliableDataTransfer);
			}
		}

		if(eventIfrSpotCycle) 
		{
			eventIfrSpotCycle(iFrCycle);
		}
	}
}

void CoRegistrationSource::parseIfrSpotResultData(const iFRSpotResultData* data , int count)
{
	for(int i = 0; i < count; i++)
	{
		float ifrSpotResult = data[i].iFr;

		if(eventIfrSpotResult) 
		{
			eventIfrSpotResult(ifrSpotResult);
		}
	}
}

void CoRegistrationSource::parseIfrPullbackCycleData(const iFRPullbackData* data, int count)
{
	for(int i = 0; i < count; i++)
	{
		CoRegistrationIfrPullbackCycle pullbackCycle;

		pullbackCycle.runIndex				= data[i].whichRun;
		pullbackCycle.timestamp         = getSyncedTimestamp(data[i].timestamp); 
		pullbackCycle.ifrTrendValue     = data[i].trendValue;
		pullbackCycle.ifrRawValue       = data[i].rawValue;
		pullbackCycle.isFiltered        = data[i].filtered;
		pullbackCycle.isValidCycle      = data[i].validFlag;
		pullbackCycle.waveFreeStartTime = getSyncedTimestamp(data[i].iWfp_start);
		pullbackCycle.waveFreeEndTime   = getSyncedTimestamp(data[i].iWfp_end);
		pullbackCycle.runStartedTime    = getSyncedTimestamp(data[i].iStartTime);

		debugLogRunIndexChanges(pullbackCycle.runIndex, L"iFRPullbackData\n");

		setCurrentRunIndex(pullbackCycle.runIndex);

		if ( !ifrTimeStampValidator.check(data[i].timestamp))
		{
			log.DeveloperWarning(L"Timestamp validation failed on IFR pullback data.");
			if(eventError)
			{
				eventError(CoRegistrationError::UnreliableDataTransfer);
			}
		}

		if(eventIfrPullbackCycle) 
		{
			eventIfrPullbackCycle(pullbackCycle);
		}
	}
}


void CoRegistrationSource::parseCurrentFrameMarker(const  FmCurrentFrameMarker* data, int count )
{
	for(int i = 0; i < count; i++)
	{
		if(eventReviewCursorPositionChanged)
		{
			eventReviewCursorPositionChanged(getSyncedTimestamp(data[i].timestamp));
		}
	}
}


void CoRegistrationSource::parseErrorData(const Error* data, int count )
{
	for(int i = 0; i < count; i++)
	{
		CoRegistrationError coRegError = CoRegistrationError::Unknown;

		switch (data[i].errorType)
		{
		case eNormalizationFailed:
			coRegError = CoRegistrationError::NormalizationError;
			break;
		case eFFRSpotFailed:
			coRegError = CoRegistrationError::FFRError;
			break;
		case eiFRSpotFailed:
		case eiFR_FailedConfidenceLevel:
		case eiFRPullbackFailed:
		case eiFRPullbackFailedFilter:
		case eiFRPullbackFailedShortRun:
			coRegError = CoRegistrationError::IFRError;
			break;
		case eCommandFailedWrongState:
			//TODO: this enum should be extended with specific command errors
			coRegError = CoRegistrationError::CommandError;
			break;
		default:
			break;
		}

		if(eventError) 
		{
			eventError(coRegError);
		}
	}
}

void CoRegistrationSource::parseTimeSyncResponse(const TimeSync* data, int count)
{
	commandResponseTimeoutTimer.stop();
	double latency = Timing::getTimeStamp() - localTimeAtTimeSyncInMs;
	if(count > 0 && activeCommand == SystemCommand::TimeSync)
	{
		timeSyncResponseCount++;
		if(localTimeAtTimeSyncInMs != 0)
		{
			bool isFirstTimeSyncResponse = !timeSyncTimer.isActive();
			if(isFirstTimeSyncResponse || latency < bestTimeSync.latency)
			{
				//we are only interested in the first timeSync packet
				double serverTimeAtTimeSyncInMs = data[0].timestamp;

				bestTimeSync.localTime		= localTimeAtTimeSyncInMs;
				bestTimeSync.serverTime		= serverTimeAtTimeSyncInMs;
				bestTimeSync.offset			= localTimeAtTimeSyncInMs - serverTimeAtTimeSyncInMs;
				bestTimeSync.latency		= latency;
				bestTimeSync.calculatedTime = getSyncedTimestamp(serverTimeAtTimeSyncInMs);
			}
		}

		if(timeSyncResponseCount < minimalTimeSyncRetries)
		{
			//the timesync needs to be performed multiple times to avoid latency issues caused by the OS
			timeSyncTimer.start(minimalTimeSyncInterval);
		}
		else
		{
			timeSyncTimer.stop();
			timeSyncResponseCount = 0;

			if(eventTimeSync)
			{
				eventTimeSync(bestTimeSync);
			}

			wchar_t timeOffset[32];
			swprintf_s(timeOffset, L"%.4f",bestTimeSync.offset);

			if(bestTimeSync.latency <= maxTimeSyncLatency)
			{
				setConnectionStatus(ConnectionStatus::ConnectedWithTimeSync);
				::UtilizationLogEvent::TimeSynchronizationFFRiFR(log, timeOffset);
			}
			else
			{
				log.DeveloperError(L"CoRegistrationSource: Latency problems detected in time synchronization. [Latency=%f]", bestTimeSync.latency);

				if(eventError)
				{
					eventError(CoRegistrationError::LatencyTooHigh);
				}

				setConnectionStatus(ConnectionStatus::ConnectedNoTimeSync);
				::UtilizationLogEvent::TimeSynchronizationFFRiFR_Failed(log, timeOffset);
			}
			connectionMonitorTimer.start(connectionMonitorInterval);
		}
	}
	handleCommandResponse(SystemCommand::TimeSync);
}

void CoRegistrationSource::parseScaleOptionsData(const ScaleOptionsData* data , int count)
{
	for(int i = 0; i < count; i++)
	{
		CoRegistrationOptions options;

		options.maxPressure			= data[i].pressMax;
		options.minPressure			= data[i].pressMin;
		options.maxIfr				= data[i].pullbackMax;
		options.minIfr				= data[i].pullbackMin;
		options.showIfrRawLine		= data[i].showRaw;
		options.showWaveFreePeriods	= data[i].showWfp;

		if(eventOptions) 
		{
			eventOptions(options);
		}
	}
}


void CoRegistrationSource::parseSystemInfo(const SysInfo* data , int count)
{
	for(int i = 0; i < count; i++)
	{
		CoRegistrationSystemInfo systemInfo;

		systemInfo.swVersion		= str_ext::stow(data[i].swVersion);
		systemInfo.protocolVersion	= str_ext::stow(data[i].protocolVersion);
		systemInfo.serialNumber		= str_ext::stow(data[i].serialNumber);
		systemInfo.license			= data[i].license;

		const auto& protocol = systemInfo.protocolVersion;
		if (protocol.size() >= SupportedProtocol.size() &&
			std::equal(std::begin(protocol), std::begin(protocol) + SupportedProtocol.size(), std::begin(SupportedProtocol)))
		{
			//only log the system info
			log.DeveloperInfo(L"CoRegistrationSource:: FFR/iFR System connected: SWVersion: %s, ProtocolVersion: %s, SerialNumber: %s, License available: %s",
				systemInfo.swVersion.c_str(),
				systemInfo.protocolVersion.c_str(),
				systemInfo.serialNumber.c_str(),
				systemInfo.license ? L"true" : L"false");

			connectedSystemInfo = systemInfo;
			setConnectionStatus(ConnectionStatus::ConnectedNoTimeSync);
			if (eventSysInfo)
			{
				eventSysInfo(systemInfo);
			}
			requestTimeSync();
		}
		else
		{
			// only report the error when it is a different system
			if (connectedSystemInfo != systemInfo)
			{
				connectedSystemInfo = systemInfo;
				log.DeveloperError(L"CoRegistrationSource:: FFR/iFR System connected with an incompatible protocol. Found '%s', Expected '%s' (SWVersion: %s, SerialNumber: %s, License available: %s)",
					systemInfo.protocolVersion.c_str(),
					SupportedProtocol.c_str(),
					systemInfo.swVersion.c_str(),
					systemInfo.serialNumber.c_str(),
					systemInfo.license ? L"true" : L"false");
				if (eventSysInfo)
				{
					eventSysInfo(systemInfo);
				}
				if (eventError)
				{
					eventError(CoRegistrationError::ProtocolMismatch);
				}
			}
			client->reconnect();
		}
	}
	handleCommandResponse(SystemCommand::SystemInfo);
}

CoRegistrationServerState::FmState CoRegistrationSource::getFmStateFromStateData(const StateData &stateData)
{
	CoRegistrationServerState::FmState returnState = coRegState.fmState;

	switch(stateData.state)
	{
	case FfrLive:
	case FfrSpotAnalyze:
	case FfrSpotReview:
		returnState = CoRegistrationServerState::FmState::FFR;
		break;
	case IfrPullbackAnalyze:
	case IfrPullbackReview:
		returnState = CoRegistrationServerState::FmState::IFRPullback;
		break;
	case IfrLive:
	case IfrSpotAnalyze:
	case IfrSpotReview:
		returnState = CoRegistrationServerState::FmState::IFRSpot;
		break;
	}

	return returnState;
}

CoRegistrationServerState::SystemState CoRegistrationSource::getSystemStateFromStateData( const StateData &stateData )
{
	CoRegistrationServerState::SystemState returnState = CoRegistrationServerState::SystemState::Idle;

	switch(stateData.state)
	{
	case FfrLive:
	case IfrLive:
		returnState = CoRegistrationServerState::SystemState::Live;
		break;
	case FfrSpotAnalyze:
	case IfrSpotAnalyze:
		returnState = CoRegistrationServerState::SystemState::SpotMeasurement;
		break;
	case IfrPullbackAnalyze:
		returnState = CoRegistrationServerState::SystemState::Pullback;
		break;
	case FfrSpotReview:
	case IfrSpotReview:
	case IfrPullbackReview:
		returnState = CoRegistrationServerState::SystemState::Review;
		break;
	case Normalization:
		returnState = CoRegistrationServerState::SystemState::Normalization;
		break;
	case Zero:
		returnState = CoRegistrationServerState::SystemState::Zero;
		break;
	case Freeze:
		returnState = CoRegistrationServerState::SystemState::Freeze;
		break;
	}

	return returnState;
}

void CoRegistrationSource::requestTimeSync()
{
	handleCommandRequest(SystemCommand::TimeSync);
}

void CoRegistrationSource::startTimeSync()
{
	log.DeveloperInfo(L"CoregSource: startTimeSync");
	localTimeAtTimeSyncInMs = Timing::getTimeStamp();
	sendCommand(FmCommands::eTimeSync);
}

void CoRegistrationSource::sendPacket( Tag tag, const void* data, int length )
{
	PacketHeader header;
	header.tag  = tag;
	header.size = length;

	if(sendData.size() > 0)
		sendData.clear();

	int sendLength = sizeof(PacketHeader) + header.size;
	sendData.resize(sendLength);

	memcpy(sendData.data(), &header, sizeof(PacketHeader));
	memcpy(sendData.data() + sizeof(PacketHeader), data, length);

	try{
		client->send(sendData.data(), sendLength);
	}
	catch(const TcpClient::SocketException& ex)
	{
		log.DeveloperError(L"CoRegistrationSource:: Error occurred while sending data. [error=%d]", ex.Error);
		client->reconnect();
	}
}

double CoRegistrationSource::getSyncedTimestamp(double serverTimestamp)
{
	return serverTimestamp + bestTimeSync.offset;
}

void CoRegistrationSource::setConnectionStatus(ConnectionStatus status)
{
	if (connectionStatus != status)
	{
		log.DeveloperInfo(L"CoregSource: setConnectionStatus changed [old status=%d; new status=%d]", connectionStatus, status);
		connectionStatus = status;
		if (eventConnectionChanged)
		{
			eventConnectionChanged(status);
		}

		if( status == ConnectionStatus::Disconnected)
		{
			::UtilizationLogEvent::ConnectionLostToFfrIfrSystem(log);
		}
	}
}

void CoRegistrationSource::setCurrentRunIndex(int runIndex)
{
	bool runIndexChanged = false;
	if(runIndex != currentRunIndex)
	{
		currentRunIndex = runIndex;
		runIndexChanged = true;
	}
	
	if(eventRunSelected && runIndexChanged)
	{
		eventRunSelected(currentRunIndex);
	}
}

void CoRegistrationSource::onConnectionMonitorTick()
{
	handleCommandRequest(SystemCommand::SystemState);
}

void CoRegistrationSource::sendStateRequest()
{
	sendCommand(FmCommands::eSysState);
}

void CoRegistrationSource::sendCommand(FmCommands command)
{
	FMCommand commandData;
	commandData.cAction = command;
	sendPacket(FMCommandTag, &commandData, sizeof(FMCommand));
}

void CoRegistrationSource::debugLogRunIndexChanges(int runIndex, std::wstring source)
{
	if(runIndex != currentRunIndex)
	{
		std::wstring str = L"Run index changed because of " + source + L" " + std::to_wstring(runIndex) + L"\n";
		OutputDebugStringW(str.c_str());
	}
}

void CoRegistrationSource::debugLogStateChanges(CoRegistrationServerState::SystemState systemState,  CoRegistrationServerState::FmState fmState)
{
	std::wstring systemStateStr;

	switch (systemState)
	{
	case CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState::Idle:
		systemStateStr = L"Idle";
		break;
	case CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState::Live:
		systemStateStr = L"Live";
		break;
	case CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState::SpotMeasurement:
		systemStateStr = L"SpotMeasurement";
		break;
	case CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState::Pullback:
		systemStateStr = L"Pullback";
		break;
	case CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState::Review:
		systemStateStr = L"Review";
		break;
	case CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState::Normalization:
		systemStateStr = L"Normalization";
		break;
	case CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState::Freeze:
		systemStateStr = L"Freeze";
		break;
	case CommonPlatform::CoRegistration::CoRegistrationServerState::SystemState::Zero:
		systemStateStr = L"Zero";
		break;
	default:
		break;
	}

	std::wstring fmStateStr;
	switch(fmState)
	{
	case CoRegistrationServerState::FmState::FFR:
		fmStateStr = L"FFR";
		break;
	case CoRegistrationServerState::FmState::IFRPullback:
		fmStateStr = L"IFRPullback";
		break;
	case CoRegistrationServerState::FmState::IFRSpot:
		fmStateStr = L"IFRSpot";
		break;
	case CoRegistrationServerState::FmState::Unknown:
		fmStateStr = L"Unknown";
		break;
	default:
		break;
	}

	std::wstring str = L"CoRegistration state changed to FmType: " + fmStateStr + L" SystemState: " + systemStateStr + L"\n";
	OutputDebugStringW(str.c_str());
}

bool CoRegistrationSource::checkNetworkLatency(double syncedTimeStamp)
{
	bool latencyCheckPass = true;

	if( coRegState.normalized ) 
	{
		double currentTime      = Timing::getTimeStamp();
		double offSetDifference = currentTime - syncedTimeStamp;

		// Check if the current offset used to synchronize server time stamp has not increased too much. Additional latency in connection is measured in currentTime.
		latencyCheckPass = offSetDifference <= maxTimeOffsetDifference;

		log.DeveloperInfo	(	L"CoRegistrationSource:: Network Latency check. [Result=%s, TimeSyncOffset=%f, SyncedServerTimeStamp=%f, LocalTimeStamp=%f, OffSetDifference=%f]", 
								latencyCheckPass ? L"Passed" : L"Failed", bestTimeSync.offset, syncedTimeStamp, currentTime, offSetDifference);
	}

	return latencyCheckPass;
}

void CoRegistrationSource::handleConnectionFailed()
{
	connectionMonitorTimer.stop();
	timeSyncTimer.stop();
	commandResponseTimeoutTimer.stop();
	activeCommand = SystemCommand::None;
	queuedCommand = SystemCommand::None;
	setConnectionStatus(ConnectionStatus::Disconnected);
}

void CoRegistrationSource::setNoConnectionMontitorInterval(double seconds)
{
	connectionMonitorInterval = seconds;
}

void CoRegistrationSource::handleCommandRequest(SystemCommand command)
{
	if (activeCommand != SystemCommand::None)
	{
		if (activeCommand != command) queuedCommand = command;
		return;
	}

	switch (command)
	{
	case SystemCommand::TimeSync:
		startTimeSync();
		break;
	case SystemCommand::SystemState:
		sendStateRequest();
		break;
	case SystemCommand::SystemInfo:
		requestSystemInfo();
		break;
	case SystemCommand::None:
	default:
		assert(false);
		return;
	}

	activeCommand = command;
	commandResponseTimeoutTimer.start(MaxCommandResponseTime);
}

void CoRegistrationSource::handleCommandResponse(SystemCommand command)
{
	assert (command != SystemCommand::None);
	if (command == activeCommand)
	{
		commandResponseTimeoutTimer.stop();
		activeCommand = SystemCommand::None;
		if (queuedCommand != SystemCommand::None)
		{
			auto newCommand = queuedCommand;
			queuedCommand = SystemCommand::None;
			handleCommandRequest(newCommand);
		}
	}
}

void CoRegistrationSource::onCommandResponseTimeout()
{
	switch (activeCommand)
	{
	case SystemCommand::TimeSync:
		timeSyncTimer.stop();
		log.DeveloperWarning(L"CoRegistrationSource: Server did not respond in time to TimeSync request command, trying to reconnect");
		::UtilizationLogEvent::TimeSynchronizationFFRiFR_Failed(log, L"TimeOut");
		break;
	case SystemCommand::SystemState:
		log.DeveloperWarning(L"CoRegistrationSource: Server did not respond in time to State request command, trying to reconnect");
		break;
	case SystemCommand::SystemInfo:
		log.DeveloperWarning(L"CoRegistrationSource: Server did not respond in time to SystemInfo request command, trying to reconnect");
		break;
	case SystemCommand::None:
	default:
		assert(false);
		return;
	}

	if (eventCommandTimeout)
	{
		eventCommandTimeout();
	}

	handleConnectionFailed();
	client->reconnect();
}

void CoRegistrationSource::requestSystemInfo()
{
	sendCommand(FmCommands::eSysInfo);
}

void CoRegistrationSource::startOfTimeStampValidation()
{
	pressureTimeStampValidator.start();
	ffrTimeStampValidator.start();
	ifrTimeStampValidator.start();
}

void CoRegistrationSource::lastDataReceived(double syncedTimeStamp)
{
	// Check the current network latency with help of the (synced) timestamp of a state package
	if ( !checkNetworkLatency(syncedTimeStamp) )
	{
		if(eventError)
		{
			eventError(CoRegistrationError::LatencyTooHigh);
		}
	}
}

void CoRegistrationSource::stopTimeStampValidation()
{
	pressureTimeStampValidator.stop();
	ffrTimeStampValidator.stop();
	ifrTimeStampValidator.stop();
}

}}