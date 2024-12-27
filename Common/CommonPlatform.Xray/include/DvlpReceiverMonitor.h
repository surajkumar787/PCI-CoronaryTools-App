// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <Dvlp/DvlpReceiver.h>

namespace CommonPlatform {	namespace Xray {

// This class add network connection monitor functionality to Dvlp::Receiver
class DvlpReceiverMonitor : public Dvlp::Receiver
{
public:
    DvlpReceiverMonitor();
    ~DvlpReceiverMonitor();

    /// <summary>
    /// Dvlp Connection Status
    /// </summary>
    enum class DvlpConnectionStatus
    {
        Connected,
        NotConnectedSinceNoCable,
        NotConnectedSinceOpeningSocketFailed,
        NotConnectedSinceNoAdapterDefined,
    };

    /*! Opens a new network socket for reading.
        \param adapter      The network interface to bind (0 for any).
        \param port         The port number to bind.
        \param bufferSize   The network socket buffer size, 0 = default.
        \returns            true if the network socket could be opened, false otherwise.
     */
	bool open(uint32_t adapter, uint16_t port);
	bool open(uint32_t adapter, uint16_t port, int bufferSize);
	
    /*! Opens a new network socket for reading.
        \param group        The multicast group IP address.
        \param adapter      The network interface to bind.
        \param port         The port number to bind.
        \param bufferSize   The network socket buffer size, 0 = default.
        \returns            true if the network socket could be opened, false otherwise.
     */
	bool open(uint32_t group, uint32_t adapter, uint16_t port);
	bool open(uint32_t group, uint32_t adapter, uint16_t port, int bufferSize);

    // Get the current status of the dvlp connection
    DvlpConnectionStatus  checkConnection();

protected:
    static const int JumboFrameSize = 9000; // Minimum MTU for DVLP connection
    uint32_t dvlpAdapter;

    DvlpConnectionStatus getStatusOfAdapter(uint32_t adapter) const;

};

}}