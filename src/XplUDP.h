/***************************************************************************
****																	****
****	XplUDP.h														****
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

#ifndef _XplUDP_H
#define _XplUDP_H

#pragma once

//#include <winsock2.h>
#include <vector>
#include <string>
#include "Poco/Event.h"
#include "Poco/Net/IPAddress.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/NetworkInterface.h"
#include "Poco/Thread.h"
#include "Poco/RunnableAdapter.h"
#include "Poco/Mutex.h"
#include "Poco/Logger.h"
#include "Poco/NumberFormatter.h"


#include <queue>

#include "XplCore.h"
#include "XplComms.h"

using Poco::Mutex;
using Poco::Net::DatagramSocket;
using Poco::RunnableAdapter;
using Poco::Thread;

namespace xpl
{

/**
 * xPL communications over a LAN or the Internet.
 * This class enables xPL messages to be sent and recieved over a LAN
 * or Internet connection.
 * <p>
 * The only method of any real interest to the xPL developer will be
 * XplUDP::Create.  The Destroy method is implemented in the base class,
 * XplComms.  All the other public methods exist to enable other xPL
 * classes to carry out their work, and should not need to be called
 * directly by the application.
 */
class XplUDP: public XplComms
{
public:
// 	/**
// 	 * Creates a UDP communications object.
// 	 * Creates an XplUDP object that can be passed into XplDevice::Create
// 	 * @param _bViaHub set this to false to bind directly to the xPL port
// 	 * rather than sending messages via the hub.  This should only be
// 	 * done when writing a hub application.
// 	 * @return Pointer to a new XplUDP object, or NULL if an error occured
// 	 * during initialisation.
// 	 * @see XplComms::Destroy, XplDevice::Create
// 	 */
// 	static XplUDP* Create( bool const _bViaHub = true );
    static XplUDP* instance();

    /**
     * Destructor.  Only to be called via the XplComms::Destroy method
     * @see XplComms::Destroy
     */
    virtual ~XplUDP();


    /**
     * Checks an IP address to see if it is local.
     * @param _ip 32bit IP address to check.
     * @return true if the address is a local one.
     */
    bool IsLocalIP ( Poco::Net::IPAddress ip ) const;

    /**
     * Sets the port used for sending messages.
     * This method is provided only so a hub can forward messages to
     * clients (who each specify a unique port).  There should be no
     * reason for it to be called from anywhere else.
     * @param _port index of port to use.
     */
    void SetTxPort ( uint16 port )
    {
        txPort_ = port;
    }

    /**
     * Sets the ip address used as the message destination.
     * This method is provided only so a hub can forward messages to
     * clients (who each specify a unique addr).  There should be no
     * reason for it to be called from anywhere else.
     * @param _addr 32bit IP address to use.
     */
    void SetTxAddr ( uint32 addr )
    {
        txAddr_ = addr;
    }

    /**
     * Gets the local IP address that is used in xPL heartbeat messages.
     * @return the heartbeat IP address.
     */
    string GetHeartbeatIP() const
    {
        return interface_.address().toString();
    }

    // Overrides of XplComms' methods.  See XplComms.h for documentation.
    virtual bool TxMsg ( XplMsg& pMsg );

    virtual void SendHeartbeat ( string const& source, uint32 const interval, string const& version );

    /**
     * Constructor.  Only to be called via the static Create method
     * @see XplUDP::Create
     */
    XplUDP ( bool const viaHub = false );


protected:


    virtual void SendConfigHeartbeat ( string const& source, uint32 const interval, string const& version );
    virtual bool Connect();
    virtual void Disconnect();

private:
    /**
     * Creates a list of all the local IP addresses.
     */
    bool GetLocalIPs();

    /**
     * target for the the listenAdapter thread to sit and listen for packets.
     */
    void ListenForPackets();

    uint16						rxPort_;				// Port on which we are listening for messages
    uint16						txPort_;				// Port on which we are sending messages
    Poco::Net::NetworkInterface						interface_;					// Local IP address to use in xPL heartbeats
    bool						viaHub_;				// If false, bind directly to port 3865

    //SOCKET						m_sock;					// Socket used to send and receive xpl Messages
    DatagramSocket socket_;
    RunnableAdapter<XplUDP>* listenAdapter_;
    Thread  listenThread_;
    queue<XplMsg*> incommingQueue_;
    Mutex incommingQueueLock_;
    //WSAEVENT					m_rxEvent;				// Event used to wait for received data
    Poco::Event   rxEvent_;


    uint32						txAddr_;				// IP address to which we send our messages.  Defaults to the broadcast address.
    uint32						listenOnAddress_;		// IP address on which we listen for incoming messages

    bool						listenToFilter_;		// True to enable filtering of IP addresses from which we can receive messages.
    vector<Poco::Net::IPAddress>				listenToAddresses_;	// List of IP addresses that we accept messages from when m_bListenToFilter is true.
    vector<Poco::Net::IPAddress>				localIPs_;				// List of all local IP addresses for this machine

    static uint16 const			kXplHubPort;			// Standard port assigned to xPL traffic
    Logger& commsLog;
};

}	// namespace xpl

#endif // _XplUDP_H

