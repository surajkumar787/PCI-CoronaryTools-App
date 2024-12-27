// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "Sense/System/IScheduler.h"
#include "ITcpClient.h"

#include "CoRegistrationProtocol.h"
#include "CoRegistrationSystemInfo.h"
#include "CoRegistrationServerState.h"
#include "TimeStampValidator.h"

#include <functional>
#include <memory>
#include <vector>
#include <stdint.h>

namespace Sense{
    class Timer;
}

namespace CommonPlatform{
    class Log;
}

namespace CommonPlatform { namespace CoRegistration{

struct CoRegistrationPressureAndEcg;
struct CoRegistrationPdPaInfo;
struct CoRegistrationSpotMeasurement;
struct CoRegistrationIfrSpotCycle;
struct CoRegistrationIfrPullbackCycle;
struct CoRegistrationOptions;
struct CoRegistrationSystemInfo;
enum class CoRegistrationError;
enum class SystemCommand;

class CoRegistrationSource
{
public:

	struct TimeSyncInfo{
		double localTime;
		double serverTime;
		double calculatedTime;
		double offset;
		double latency;
	};

    enum class ConnectionStatus{
        ConnectedNoTimeSync,
        ConnectedWithTimeSync,
        Disconnected
    };

    explicit CoRegistrationSource(Sense::IScheduler &scheduler);
	CoRegistrationSource(Sense::IScheduler &scheduler, const std::shared_ptr<ITcpClient>& tcpClient, CommonPlatform::Log& logger);
    ~CoRegistrationSource();

    std::function<void(ConnectionStatus status)>                                    eventConnectionChanged;
	std::function<void(const TimeSyncInfo &timeSyncInfo)>							eventTimeSync;
	std::function<void()>                                                           eventCommandTimeout;
    std::function<void(const CoRegistrationServerState &coRegState)>                eventStateData;
    std::function<void(const CoRegistrationPressureAndEcg &coRegPressureAndEcg)>    eventPressureAndEcgData;
    std::function<void(const CoRegistrationPdPaInfo &coRegPdPaInfo)>                eventPdPaInfo;
    std::function<void(const CoRegistrationSpotMeasurement &coRegSpotMeasurement)>  eventSpotMeasurement;
    std::function<void(const CoRegistrationIfrSpotCycle &coRegIfrCycle)>            eventIfrSpotCycle;
	std::function<void(const float ifrSpotResult)>						            eventIfrSpotResult;
    std::function<void(const CoRegistrationIfrPullbackCycle &coRegPullbackCycle)>   eventIfrPullbackCycle;
    std::function<void(const CoRegistrationError &coRegError)>                      eventError;
    std::function<void(const double reviewTimestamp)>								eventReviewCursorPositionChanged;
	std::function<void(const CoRegistrationOptions &coRegOptions)>					eventOptions;
	std::function<void(const int runIndex)>											eventRunSelected;
	std::function<void(const CoRegistrationSystemInfo &coRegSystemInfo)>			eventSysInfo;

	void requestTimeSync();

    bool connect(const std::wstring &host, int port);
    void disconnect();

	//public method to set the connection monitor interval, so it can be used for unit-testing purposes
	void setNoConnectionMontitorInterval(double seconds);


private:
	void handleCommandRequest(SystemCommand command);
	void handleCommandResponse(SystemCommand command);

	void startTimeSync();
	void requestSystemInfo();
	void sendStateRequest();

	void setConnectionStatus(ConnectionStatus status);
	void setCurrentRunIndex(int runIndex);
    
    void onDataAvailable();
    void onSocketEmpty();
    void onTcpConnectionStatusChanged(bool isConnected);
    void onCommandResponseTimeout();
	void onConnectionMonitorTick();
	void onDelayedSystemInfoRequest();
	void handleConnectionFailed();

    void sendPacket(Tag tag, const void* data, int length);

    void parse                      ();
    void parse                      (Tag tag, char* data, int length);
    void parseStateData             (const StateData* data				, int count);
    void parsePressureAndEcgData    (const PressureAndEcgData* data		, int count);
    void parseMapData               (const MAPdata* data				, int count);
    void parseSpotMeasurement       (const FMSpotData* data				, int count);
    void parseIfrSpotCycleData      (const iFRSpotData* data			, int count);
	void parseIfrSpotResultData     (const iFRSpotResultData* data		, int count);
    void parseIfrPullbackCycleData  (const iFRPullbackData* data		, int count);
    void parseCurrentFrameMarker    (const FmCurrentFrameMarker* data	, int count);
    void parseErrorData             (const Error* data					, int count);
    void parseTimeSyncResponse      (const TimeSync* data				, int count);
	void parseScaleOptionsData      (const ScaleOptionsData* data		, int count);
	void parseSystemInfo            (const SysInfo* data				, int count);

	void sendCommand(FmCommands command);

	double getSyncedTimestamp(double timestamp);

	void startOfTimeStampValidation();
	void lastDataReceived(double serverTimeStamp);
	void stopTimeStampValidation();

    CoRegistrationServerState::FmState     getFmStateFromStateData(const StateData &stateData);
    CoRegistrationServerState::SystemState getSystemStateFromStateData(const StateData &stateData);

	void debugLogRunIndexChanges(int runIndex, std::wstring source);
	void debugLogStateChanges(CoRegistrationServerState::SystemState systemState,  CoRegistrationServerState::FmState fmState);
	bool checkNetworkLatency(double serverTimestamp);

	SystemCommand               activeCommand;
	SystemCommand               queuedCommand;
	Sense::IScheduler           &scheduler;
	Log                         &log;
	std::shared_ptr<ITcpClient> client;

	std::vector<char>   receivedData;
	std::vector<char>   sendData;
	int                 totalReceived;

	int currentRunIndex;

	ConnectionStatus          connectionStatus;
	CoRegistrationServerState coRegState;

	Sense::Timer        commandResponseTimeoutTimer;
	Sense::Timer		timeSyncTimer;
	Sense::Timer		connectionMonitorTimer;
	double			    connectionMonitorInterval;

	double				localTimeAtTimeSyncInMs;
	int					timeSyncResponseCount;
	TimeSyncInfo		bestTimeSync;
	CoRegistrationSystemInfo connectedSystemInfo;

	TimeStampValidator pressureTimeStampValidator;
	TimeStampValidator ffrTimeStampValidator;
	TimeStampValidator ifrTimeStampValidator;
};

}}

