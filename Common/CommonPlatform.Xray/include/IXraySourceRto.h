// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "XrayImage.h"
#include "XrayStudy.h"

namespace CommonPlatform { namespace Xray {

class IXraySourceRto
{
public:

    /// <summary>
    /// Cwis Connection Status
    /// </summary>
    enum class CwisStatus
    {
        Connected,
        PartiallyConnected,
        NotConnectedSinceNoActiveConnections,
        NotConnectedSinceNoCwisServer,
    };

    /// <summary>
    /// Dvlp Connection Status
    /// </summary>
    enum class DvlpStatus
    {
        Connected,
        NotConnected,
        NotConnectedSinceNoCable,
        NotConnectedSinceOpeningSocketFailed,
        NotConnectedSinceNoAdapterDefined
    };

    virtual ~IXraySourceRto() {}

    virtual void connect(const std::wstring &dvlpAdapter, const std::wstring &dvlpMulticast, int dvlpPort, const std::wstring &cwisHost, int cwisPort) = 0;
	virtual void disconnect()   = 0;
    virtual bool isConnected()  = 0;
    
    virtual DvlpStatus getDvlpConnectionStatus() = 0;
    virtual CwisStatus getCwisConnectionStatus() = 0;

   	std::function<void(const std::shared_ptr<XrayImage> &, bool skip)>	eventImage;
	std::function<void(const XrayGeometry&)>							eventGeometry;
	std::function<void(const XrayEpx     &)>							eventEpx;
	std::function<void(const XrayStudy   &)>							eventStudy;
	std::function<void(XrayImage::Type)>								eventImageBegin;
	std::function<void()>												eventImageEnd;
	std::function<void()>												eventGeometryEnd;
    std::function<void(bool,CwisStatus,DvlpStatus)>                     eventConnectionChanged;

};

}}