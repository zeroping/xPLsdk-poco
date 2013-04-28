/***************************************************************************
****																	****
****	XplUDP.cpp														****
****																	****
****	xPL Communications												****
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

#include "Poco/Net/SocketAddress.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/NetworkInterface.h"
#include "XplCore.h"
#include "XplStringUtils.h"
#include "XplMsg.h"
#include "XplComms.h"
#include "XplUDP.h"
// #include "EventLog.h"
// #include "RegUtils.h"

#include "Poco/SingletonHolder.h"

#include <iostream>

using namespace xpl;
using namespace Poco::Net;
using Poco::Net::NetworkInterface;

uint16 const XplUDP::kXplHubPort = 3865;


namespace
{
static Poco::SingletonHolder<XplUDP> sh;
}

XplUDP* XplUDP::instance()
{
    return sh.get();
}


/***************************************************************************
****																	****
****	XplUDP constructor												****
****																	****
***************************************************************************/

XplUDP::XplUDP
(
    const bool viaHub
) :
    commsLog ( Logger::get ( "xplsdk.comms" ) ),
    viaHub_ ( viaHub ),
    txPort_ ( kXplHubPort ),
    listenToFilter_ ( false )
{

    Logger::setLevel("xplsdk", Message::PRIO_DEBUG  );
    
    uint32 i;
    // Build the list of local IP addreses
    // If possible, set our IP (for use in heatbeats) to the
    // first local IP that is not the loopback address.
    NetworkInterface::NetworkInterfaceList netlist = NetworkInterface::list();
    vector<NetworkInterface>::iterator nit = netlist.begin();
    for ( nit = netlist.begin(); nit != netlist.end(); ++nit )
    {
        poco_warning ( commsLog, "found network interface: " + ( *nit ).address().toString() +" : " + ( *nit ).broadcastAddress().toString() );
        if ( ! ( *nit ).address().isLoopback() )
        {
            interface_ = ( *nit );
            break;
        }
    }
    poco_information ( commsLog, "Our heartbeat address is " + interface_.address().toString() );

    Connect();
}


/***************************************************************************
****																	****
****	XplUDP::~XplUDP													****
****																	****
***************************************************************************/

XplUDP::~XplUDP()
{
    //can't use loggers when exiting
    //poco_debug ( commsLog, "disconnecting" );
    if ( IsConnected() )
    {
        Disconnect();
    }

    assert ( !IsConnected() );

}


/***************************************************************************
****																	****
****	XplUDP::TxMsg													****
****																	****
***************************************************************************/

bool XplUDP::TxMsg
(
    XplMsg& pMsg
)
{
    bool retVal = false;

    if ( IsConnected() )
    {

        IPAddress lbcast = interface_.broadcastAddress();
         Poco::Net::SocketAddress destAddress ( lbcast, 3865 );
        poco_trace ( commsLog, "_pMsg.GetRawData()" );

        int sentBytes = socket_.sendTo ( pMsg.GetRawData().c_str() , pMsg.GetRawData().size(), destAddress );
        retVal = (sentBytes == pMsg.GetRawData().size());

    }

    return retVal;
}



/***************************************************************************
****																	****
****	XplUDP::Connect													****
****																	****
***************************************************************************/

bool XplUDP::Connect()
{
    if ( IsConnected() )
    {
        return true;
    }
    rxPort_ = kXplHubPort;

    Poco::Net::SocketAddress sa ( interface_.broadcastAddress(), rxPort_ );
    poco_information ( commsLog, "Trying port " + NumberFormatter::format ( rxPort_ ) + " on IP " + sa.toString() );

    // If we are not communicating via a hub (PocketPC app or the hub
    // app itself, for example), then we bind directly to the standard
    // xpl port.  Otherwise we try to bind to one numbered 50000+.
    bool bound = false;
    try
    {
        socket_ = DatagramSocket ( sa,false );
        socket_.setBroadcast ( true );
        bound = true;
    }
    catch ( NetException & e )
    {
        poco_information ( commsLog, "Can't open port " + NumberFormatter::format ( rxPort_ )  + " on IP " + sa.toString() + "; trying next port.");
    }

    // Not using a hub.  If we fail to bind to the hub port, then we will
    // assume there is already one running, and bind to port 50000+ instead.
    if ( !bound )
    {
        // Try to bind to a port numbered 50000+
        rxPort_ = 50000;

        while ( !bound )
        {
            sa = SocketAddress ( interface_.address(), rxPort_ );
            try
            {
                socket_ = DatagramSocket ( sa,false );
                socket_.setBroadcast ( true );
                bound = true;
            }
            catch ( NetException & e )
            {
                poco_information ( commsLog, "Can't open port " + NumberFormatter::format ( rxPort_ )  + " on IP " + sa.toString() + "; trying next port.");
                //increment by one
                rxPort_ += 1;
            }
        }
    }
    poco_information ( commsLog, "Opened port " + NumberFormatter::format ( rxPort_ ) );
//     // Set the rx socket to be non blocking
//     {
//          nonBlock = 1;
//         if ( SOCKET_ERROR == ioctlsocket ( m_sock, FIONBIO, &nonBlock ) )
//         {
//             if ( EventLog::Get() )
//             {
//                 EventLog::Get()->ReportError ( "Unable to set the socket to be non-blocking" );
//             }
//             goto ConnectError;
//         }
//     }
//
    
    XplComms::Connect();
    listenAdapter_ = new RunnableAdapter<XplUDP> ( *this,&XplUDP::ListenForPackets );
    listenThread_.setName ( "packet listen thread" );
    listenThread_.start ( *listenAdapter_ );

    return true;
}


/***************************************************************************
****																	****
****	XplUDP::Disconnect 						  						****
****																	****
***************************************************************************/

void XplUDP::Disconnect()
{
    if ( IsConnected() )
    {
        XplComms::Disconnect();
        listenThread_.join();
    }

    delete listenAdapter_;
}




/***************************************************************************
****																	****
****	XplUDP::IsLocalIP						  						****
****																	****
***************************************************************************/

bool XplUDP::IsLocalIP
(
    IPAddress ip
) const
{
    for ( int i=0; i<localIPs_.size(); ++i )
    {
        if ( localIPs_[i] == ip )
        {
            return true;
        }
    }
    return false;
}


/***************************************************************************
****																	****
****	XplUDP::SendHeartbeat											****
****																	****
****	xpl-stat														****
****	{																****
****	hop=1															****
****	source=[VENDOR]-[DEVICE].[INSTANCE]								****
****	target=*														****
****	}																****
****	hbeat.app														****
****	{																****
****	interval=[interval in minutes]									****
****	port=[listening port]											****
****	remote-ip=[local IP address]									****
****	version=[version string]										****
****	(...additional info defined by the developer)					****
****	}																****
****																	****
***************************************************************************/

void XplUDP::SendHeartbeat
(
    string const& source,
    uint32 const interval,
    string const& version
)
{
    AutoPtr<XplMsg> pMsg = new XplMsg ( XplMsg::c_xplStat, source, "*", "hbeat", "app" );
    if ( pMsg )
    {
        pMsg->AddValue ( "interval", NumberFormatter::format(interval) );
        pMsg->AddValue ( "port", NumberFormatter::format(rxPort_) );
        pMsg->AddValue ( "remote-ip", interface_.address().toString() );
        pMsg->AddValue ( "version", version );
        
        TxMsg ( *pMsg );
    }
}


/***************************************************************************
****																	****
****	XplUDP::SendConfigHeartbeat										****
****																	****
****	xpl-stat														****
****	{																****
****	hop=1															****
****	source=[VENDOR]-[DEVICE].[INSTANCE]								****
****	target=*														****
****	}																****
****	config.app														****
****	{																****
****	interval=[interval in minutes]									****
****	port=[listening port]											****
****	remote-ip=[local IP address]									****
****	version=[version string]										****
****	(...additional info defined by the developer)					****
****	}																****
****																	****
***************************************************************************/

void XplUDP::SendConfigHeartbeat
(
    string const& source,
    uint32 const interval,
    string const& version
)
{
    AutoPtr<XplMsg> pMsg = new XplMsg ( XplMsg::c_xplStat, source, "*", "config", "app" );
    if ( pMsg )
    {
        pMsg->AddValue ( "interval", NumberFormatter::format(interval) );
        pMsg->AddValue ( "port", NumberFormatter::format(rxPort_) );
        pMsg->AddValue ( "remote-ip", interface_.address().toString() );
        pMsg->AddValue ( "version", version );

        TxMsg ( *pMsg );
    }
}


void XplUDP::ListenForPackets()
{

    poco_debug ( commsLog, "started listening" );
    Poco::Timespan timeout = Poco::Timespan ( 0,0,0,1,0 );    
    socket_.setReceiveTimeout ( timeout );
    while ( this->IsConnected() ) //we don't need locking here - connected is just a boolean
    {
        char buffer[2024];
        Poco::Net::SocketAddress sender;
        //int bytesRead = m_sock.receiveFrom(buffer, sizeof(buffer)-1, sender);
        bool ready = socket_.poll ( timeout, Socket::SELECT_READ );
        if ( ! ready )
        {
            continue;
        }
        int bytesRead = socket_.receiveFrom ( buffer, sizeof ( buffer )-1, sender );
//         cout << "got " << bytesRead << " bytes\n";
        if ( bytesRead == 0 )
        {
//             cout << "no bytes\n";
            continue;
        }
        buffer[bytesRead] = '\0';
        //std::cout << sender.toString() << ": " << buffer << std::endl;

        // Filter out messages from unwanted IPs
        if ( listenToFilter_ )
        {
            uint32 i;
            for ( i=0; i<listenToAddresses_.size(); ++i )
            {
                if ( listenToAddresses_[i] == sender.host() )
                {
                    // Found a match.
                    break;
                }
            }

            if ( i == listenToAddresses_.size() )
            {
                // We didn't find a match
                continue;
            }
        }

        // Create an XplMsg object from the received data
        try {
            AutoPtr<XplMsg> pMsg = new XplMsg ( buffer );

            rxNotificationCenter.postNotification ( new MessageRxNotification ( pMsg ) );
        } catch (XplMsgParseException e) {
            poco_warning ( commsLog, "cannot parse message: " + string(e.what()) );
        }    

    }
    //can't log here - the log may already have been taken down.
//     poco_information(commsLog, "UDP rx thread stopped");
}

