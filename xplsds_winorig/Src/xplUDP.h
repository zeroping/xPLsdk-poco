/***************************************************************************
****																	****
****	xplUDP.h														****
****																	****
****	UDP Communications												****
****																	****
****	Copyright (c) 2005 Mal Lansell.									****
****    Email: xpl@lansell.org                                          ****
****																	****
****	Permission is hereby granted, free of charge, to any person		****
****	obtaining a copy of this software and associated documentation	****
****	files (the "Software"), to deal in the Software without			****
****	restriction, including without limitation the rights to use,	****
****	copy, modify, merge, publish, distribute, sublicense, and/or	****
****	sell copies of the Software, and to permit persons to whom the	****
****	Software is furnished to do so, subject to the following		****
****	conditions:														****
****																	****
****	The above copyright notice and this permission notice shall		****
****	be included in all copies or substantial portions of the		****
****	Software.														****
****																	****
****	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY		****
****	KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE		****
****	WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR			****
****	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR	****
****	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER		****
****	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR			****
****	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE		****
****	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.			****
****																	****
***************************************************************************/

#ifndef _xplUDP_H
#define _xplUDP_H

#pragma once

#include <winsock2.h>
#include <vector>
#include <string>
#include "xplCore.h"
#include "xplComms.h"

namespace xpl
{

/**
 * xPL communications over a LAN or the Internet.
 * This class enables xPL messages to be sent and recieved over a LAN
 * or Internet connection.
 * <p>
 * The only method of any real interest to the xPL developer will be
 * xplUDP::Create.  The Destroy method is implemented in the base class,
 * xplComms.  All the other public methods exist to enable other xPL
 * classes to carry out their work, and should not need to be called
 * directly by the application.
 */
class xplUDP: public xplComms
{
public:
    /**
     * Creates a UDP communications object.
     * Creates an xplUDP object that can be passed into xplDevice::Create
     * @param _bViaHub set this to false to bind directly to the xPL port
     * rather than sending messages via the hub.  This should only be
     * done when writing a hub application.
     * @return Pointer to a new xplUDP object, or NULL if an error occured
     * during initialisation.
     * @see xplComms::Destroy, xplDevice::Create
     */
    static xplUDP* Create ( bool const _bViaHub = true );

    /**
     * Checks an IP address to see if it is local.
     * @param _ip 32bit IP address to check.
     * @return true if the address is a local one.
     */
    bool IsLocalIP ( uint32 const _ip ) const;

    /**
     * Sets the port used for sending messages.
     * This method is provided only so a hub can forward messages to
     * clients (who each specify a unique port).  There should be no
     * reason for it to be called from anywhere else.
     * @param _port index of port to use.
     */
    void SetTxPort ( uint16 _port )
    {
        m_txPort = _port;
    }

    /**
     * Sets the ip address used as the message destination.
     * This method is provided only so a hub can forward messages to
     * clients (who each specify a unique addr).  There should be no
     * reason for it to be called from anywhere else.
     * @param _addr 32bit IP address to use.
     */
    void SetTxAddr ( uint32 _addr )
    {
        m_txAddr = _addr;
    }

    /**
     * Gets the local IP address that is used in xPL heartbeat messages.
     * @return the heartbeat IP address.
     */
    string GetHeartbeatIP() const
    {
        return m_ip;
    }

    // Overrides of xplComms' methods.  See xplComms.h for documentation.
    virtual bool TxMsg ( xplMsg* _pMsg );
    virtual xplMsg* RxMsg ( HANDLE _hInterrupt = INVALID_HANDLE_VALUE, uint32 _timeout = 0 );
    virtual void SendHeartbeat ( string const& _source, uint32 const _interval, string const& _version );

protected:
    /**
     * Constructor.  Only to be called via the static Create method
     * @see xplUDP::Create
     */
    xplUDP ( bool const _bViaHub );

    /**
     * Destructor.  Only to be called via the xplComms::Destroy method
     * @see xplComms::Destroy
     */
    virtual ~xplUDP();

    virtual void SendConfigHeartbeat ( string const& _source, uint32 const _interval, string const& _version );
    virtual bool Connect();
    virtual void Disconnect();

private:
    /**
     * Creates a list of all the local IP addresses.
     */
    bool GetLocalIPs();

    uint16						m_rxPort;				// Port on which we are listening for messages
    uint16						m_txPort;				// Port on which we are sending messages
    string						m_ip;					// Local IP address to use in xPL heartbeats
    bool						m_bViaHub;				// If false, bind directly to port 3865

    SOCKET						m_sock;					// Socket used to send and receive xpl Messages
    WSAEVENT					m_rxEvent;				// Event used to wait for received data

    uint32						m_txAddr;				// IP address to which we send our messages.  Defaults to the broadcast address.
    uint32						m_listenOnAddress;		// IP address on which we listen for incoming messages

    bool						m_bListenToFilter;		// True to enable filtering of IP addresses from which we can receive messages.
    vector<uint32>				m_listenToAddresses;	// List of IP addresses that we accept messages from when m_bListenToFilter is true.
    vector<uint32>				m_localIPs;				// List of all local IP addresses for this machine

    static uint16 const			c_xplHubPort;			// Standard port assigned to xPL traffic
};

}	// namespace xpl

#endif // _xplUDP_H

