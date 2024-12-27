// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <Sense.h>
#include <CwisEpx.h>
#include <CwisGeometry.h>
#include <CwisRun.h>
#include <CwisBiopsy.h>
#include <CwisExam.h>
#include <deque> 
#include <functional> 
#include <thread.h>
#include "XrayImage.h"
#include "SystemType.h"
#include "IXraySourceRto.h"
#include "DvlpReceiverMonitor.h"


 
namespace CommonPlatform {	

class Log;

namespace Xray {

using namespace Sense;

class XraySourceRto : public IAsync, public CommonPlatform::Xray::IXraySourceRto
{
public:

	explicit XraySourceRto(IScheduler &scheduler, SystemType systemType, const std::wstring &description = L"PciExpressXraySourceRto");
	explicit XraySourceRto(IScheduler &scheduler, SystemType systemType, CommonPlatform::Log& logger, const std::wstring &description = L"PciExpressXraySourceRto");
	virtual ~XraySourceRto();

    // IXraySourceRto interface members
	virtual void connect(const std::wstring &dvlpAdapter, const std::wstring &dvlpMulticast, int dvlpPort, const std::wstring &cwisHost, int cwisPort) override;
	virtual void disconnect()   override;
    virtual bool isConnected()  override;
    virtual DvlpStatus getDvlpConnectionStatus() override;
    virtual CwisStatus getCwisConnectionStatus() override;

	// Error handling
	std::function<void(const wchar_t* modelName, const wchar_t* errorDescr)> eventCwisError;

	XraySourceRto(const XraySourceRto &) = delete;
	XraySourceRto &operator=(const XraySourceRto &) = delete;

private:

	void asyncInvoke(const std::function<void()> &command) const override;
	void dvlpLoop();
	bool dvlpConnect();
    std::shared_ptr<XrayImage> receiveXrayImage();
	static unsigned long __stdcall threadProc(void *param);

    void onDvlp();
	void onCwisGeometry();
    void onCwisRun();
    void onCwisBiopsy();
	void onCwisExam();
	void onCwisEpx();
    void onCwisConnectionStatusChanged();
    
    // Conenction status handling
	bool synchronizeImageWithMetaData(std::shared_ptr<XrayImage> &image);
    void updateConnectionStatus();
    void updateXrayGeometry();
    bool areCwisAndDvlpConnected;
    CwisStatus cwisConnectionStatus;
    DvlpStatus dvlpConnectionStatus;
    bool allCwisClientsActive() const;     
    bool atLeastOneClientActive() const;  
    bool atLeastOneClientConnected() const;

    // Image handling
    bool isImageQueueEmpty()const;
    std::shared_ptr<XrayImage> getImageFromQueue();
	bool isEndOfRun();
    void HandleEndOfRun();
    void HandleAbortedRun();
    bool isRunAborted(std::shared_ptr<XrayImage> &image);
    void SyncIncompleteImages();
    bool canSyncImage(std::shared_ptr<XrayImage> &image)const;
    bool fillMetaData(std::shared_ptr<XrayImage> &image);
    bool requiredCwisModelsAreValid() const;
    void readCwisGeometryModel();
	void readCwisTableGeometryModel();
	void readCwisFrontalStandGeometryModel();
    void readCwisRunModel();
    void readCwisBiopsyModel();

	void onCwisError(const wchar_t* modelName, Cwis::CwisErrorType error) const;

	IScheduler  &scheduler;
	void 	    *thread;
	std::unique_ptr<CRITICAL_SECTION>	section;

	std::deque<std::shared_ptr<XrayImage>> images;			//images to be processed by the UI thread
	std::deque<std::shared_ptr<XrayImage>> incomplete;		//images already processed but no CWIS info received yet...

	Cwis::CwisEpxWorkstation		cwisEpx;
	Cwis::CwisGeometryWorkstation	cwisGeometry;
	Cwis::CwisRunWorkstation		cwisRun;
	Cwis::CwisBiopsyWorkstation		cwisBiopsy;
	Cwis::CwisExamWorkstation		cwisExam;
	
    DvlpReceiverMonitor             dvlp;
	int								dvlpPort;
	std::wstring					dvlpAdapter;
	std::wstring					dvlpMulticast;
	Timer							dvlpTimer;

	XrayImage::Type	                type;
    XrayEpx                         epx;
	XrayGeometry	                geometry;
	Timer			                geoTimer;

	int				imageIndex;
	int				lastReceivedRunIndex;
	bool			runActive;

    const int    DvlpImagePollingInterval; 
    const int    DvlpImageReceiveTimeOut;
    const double GeometryMovementTimeOut;

	CommonPlatform::Log&			m_log;
	SystemType						m_systemType;
};

}}