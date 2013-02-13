/***************************************************************************
****																	****
****	xplUDP.cpp														****
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
#include "xplCore.h"
#include "xplStringUtils.h"
#include "xplMsg.h"
#include "xplUDP.h"
// #include "EventLog.h"
// #include "RegUtils.h"

#include "Poco/SingletonHolder.h"

#include <iostream>

using namespace xpl;
using namespace Poco::Net;
using Poco::Net::NetworkInterface;

uint16 const xplUDP::c_xplHubPort = 3865;


/***************************************************************************
****																	****
****	xplUDP::Create													****
****																	****
***************************************************************************/

// xplUDP* xplUDP::Create
// (
//     bool const _bViaHub
// )
// {
//
//     // Create the xplUDP object
//     xplUDP* pObj = new xplUDP ( _bViaHub );
//     if ( !pObj->Connect() )
//     {
//         cout << "xplUDP fail destroy\n";
//         xplUDP::Destroy ( pObj );
//         pObj = NULL;
//     }
//
//     return ( pObj );
// }


namespace
{
static Poco::SingletonHolder<xplUDP> sh;
}

xplUDP* xplUDP::instance()
{
    return sh.get();
}



/***************************************************************************
****																	****
****	xplUDP constructor												****
****																	****
***************************************************************************/

xplUDP::xplUDP
(
    bool const _bViaHub
) :
    commsLog ( Logger::get ( "xplsdk.comms" ) ),
    xplComms(),
    m_bViaHub ( _bViaHub ),
    m_txPort ( c_xplHubPort ),
    //m_txAddr ( INADDR_BROADCAST ),
    //m_listenOnAddress ( INADDR_ANY ),
    m_bListenToFilter ( false )
{
    
    //commsLog.setLevel(Message::PRIO_TRACE );
    Logger::setLevel("xplsdk", Message::PRIO_NOTICE  );
    
    uint32 i;
//
//     // Build the list of local IP addreses

//
//     // If possible, set our IP (for use in heatbeats) to the
//     // first local IP that is not the loopback address.
    NetworkInterface::NetworkInterfaceList netlist = NetworkInterface::list();

    vector<NetworkInterface>::iterator nit = netlist.begin();
    for ( nit = netlist.begin(); nit != netlist.end(); ++nit )
    {
//         cout << "net: " << (*nit).address().toString() << " : " << (*nit).broadcastAddress().toString() << "\n";
        poco_information ( commsLog, "network interface: " + ( *nit ).address().toString() +" : " + ( *nit ).broadcastAddress().toString() );
        if ( ! ( *nit ).address().isLoopback() )
        {
            m_interface = ( *nit );
            break;
        }
    }
    poco_information ( commsLog, "our hbeat address is " + m_interface.address().toString() );

    Connect();

}


/***************************************************************************
****																	****
****	xplUDP::~xplUDP													****
****																	****
***************************************************************************/

xplUDP::~xplUDP()
{
//     cout << "destroying xplUDP\n";
    if ( IsConnected() )
    {
        Disconnect();
    }

    assert ( !IsConnected() );

    // Finished with the sockets

}


/***************************************************************************
****																	****
****	xplUDP::TxMsg													****
****																	****
***************************************************************************/

bool xplUDP::TxMsg
(
    xplMsg& _pMsg
)
{
    bool bRetVal = false;


    if ( IsConnected() )
    {

        IPAddress lbcast = m_interface.broadcastAddress();
         Poco::Net::SocketAddress destAddress ( lbcast, 3865 );
        poco_trace ( commsLog, "_pMsg.GetRawData()" );

        m_sock.sendTo ( _pMsg.GetRawData().c_str() , _pMsg.GetRawData().size(), destAddress );

    }

    return bRetVal;
}


/***************************************************************************
****																	****
****	xplUDP::RxMsg													****
****																	****
***************************************************************************/

xplMsg* xplUDP::RxMsg
(
    Poco::Event* exitevt,
    uint32 _timeout
)
{

    if ( exitevt != NULL && exitevt->tryWait ( 1 ) ) //TODO this is a hack... can't we wait on the socket and this somehow?
    {
        cout << "exitevt signaled\n";
    }
    cout << "exitevt said it's not signaled: " << exitevt <<"\n";


    //cout << "rx wait\n";
    if ( !incommingQueue.size() )
    {
        try
        {
            m_rxEvent.wait ( _timeout );
        }
        catch ( Poco::TimeoutException& e )
        {
            cout << "timed out on RX\n";
            return NULL;
        }
    }
    else
    {
        cout<<"have pkt waiting\n";
    }

    //cout << "rx gotten\n";

    incommingQueueLock.lock();
    xplMsg* toRet = incommingQueue.front();
    incommingQueue.pop();
    incommingQueueLock.unlock();
    m_rxEvent.reset();
    return toRet;

    return NULL;
}


/***************************************************************************
****																	****
****	xplUDP::Connect													****
****																	****
***************************************************************************/

bool xplUDP::Connect()
{
    if ( IsConnected() )
    {
        return true;
    }
    m_rxPort = c_xplHubPort;

    
//     Poco::Net::SocketAddress sa ( Poco::Net::IPAddress(), m_rxPort );
    Poco::Net::SocketAddress sa ( m_interface.broadcastAddress(), m_rxPort );
    poco_information ( commsLog, "Trying port " + NumberFormatter::format ( m_rxPort ) + " on IP " + sa.toString() );

//     // If we are not communicating via a hub (PocketPC app or the hub
//     // app itself, for example), then we bind directly to the standard
//     // xpl port.  Otherwise we try to bind to one numbered 50000+.
    bool bBound = false;

    try
    {
        m_sock = DatagramSocket ( sa,false );
        m_sock.setBroadcast ( true );
        bBound = true;
    }
    catch ( NetException & e )
    {
        poco_information ( commsLog, "Can't open port " + NumberFormatter::format ( m_rxPort )  + " on IP " + sa.toString());
    }

//     if ( !m_bViaHub )
//     {
//         // Not using a hub.  If we fail to bind to the hub port, then we will
//         // assume there is already one running, and bind to port 50000+ instead.
//     }
//
    if ( !bBound )
    {
//         // Try to bind to a port numbered 50000+

        m_rxPort = 50000;

        while ( !bBound )
        {
            sa = SocketAddress ( m_interface.address(), m_rxPort );
            try
            {
                m_sock = DatagramSocket ( sa,false );
                m_sock.setBroadcast ( true );
                bBound = true;
            }
            catch ( NetException & e )
            {
                poco_information ( commsLog, "Can't open port " + NumberFormatter::format ( m_rxPort )  + " on IP " + sa.toString() );
                m_rxPort += 1;
            }

        }
    }
    poco_information ( commsLog, "Opened port " + NumberFormatter::format ( m_rxPort ) );
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

//     // Call the base class
    commsLog.setLevel("trace");
    
    xplComms::Connect();

    listenAdapt = new RunnableAdapter<xplUDP> ( *this,&xplUDP::ListenForPackets );
    listenThread.setName ( "packet listen thread" );
    listenThread.start ( *listenAdapt );

    return true;
//
// ConnectError:
//     Disconnect();
    return false;
}


/***************************************************************************
****																	****
****	xplUDP::Disconnect 						  						****
****																	****
***************************************************************************/

void xplUDP::Disconnect()
{


    if ( IsConnected() )
    {
//         cout << "setting disconnect bool\n";
        xplComms::Disconnect();
        listenThread.join();

    }

    delete listenAdapt;

}




/***************************************************************************
****																	****
****	xplUDP::IsLocalIP						  						****
****																	****
***************************************************************************/

bool xplUDP::IsLocalIP
(
    IPAddress _ip
) const
{
    for ( int i=0; i<m_localIPs.size(); ++i )
    {
        if ( m_localIPs[i] == _ip )
        {
            return true;
        }
    }
    return false;
}


/***************************************************************************
****																	****
****	xplUDP::SendHeartbeat											****
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

void xplUDP::SendHeartbeat
(
    string const& _source,
    uint32 const _interval,
    string const& _version
)
{
    AutoPtr<xplMsg> pMsg = new xplMsg ( xplMsg::c_xplStat, _source, "*", "hbeat", "app" );
    cout << "ready to send\n";
    if ( pMsg )
    {
        int8 value[16];

        sprintf ( value, "%d", _interval );
        pMsg->AddValue ( "interval", value );

        sprintf ( value, "%d", m_rxPort );
        pMsg->AddValue ( "port", value );

        pMsg->AddValue ( "remote-ip", m_interface.address().toString() );
        pMsg->AddValue ( "version", _version );
        
        TxMsg ( *pMsg );
    }
}


/***************************************************************************
****																	****
****	xplUDP::SendConfigHeartbeat										****
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

void xplUDP::SendConfigHeartbeat
(
    string const& _source,
    uint32 const _interval,
    string const& _version
)
{
    AutoPtr<xplMsg> pMsg = new xplMsg ( xplMsg::c_xplStat, _source, "*", "config", "app" );
    if ( pMsg )
    {
        int8 value[16];

        sprintf ( value, "%d", _interval );
        pMsg->AddValue ( "interval", value );

        sprintf ( value, "%d", m_rxPort );
        pMsg->AddValue ( "port", value );

        pMsg->AddValue ( "remote-ip", m_interface.address().toString() );
        pMsg->AddValue ( "version", _version );

        TxMsg ( *pMsg );
    }
}


void xplUDP::ListenForPackets()
{

    poco_debug ( commsLog, "started listening" );
    Poco::Timespan timeout = Poco::Timespan ( 0,0,0,1,0 );    
    m_sock.setReceiveTimeout ( timeout );
    while ( this->IsConnected() ) //TODO locking here
    {
//         cout << "listening with timeout " << m_sock.getReceiveTimeout().totalMilliseconds() <<"\n";
        char buffer[2024];
        Poco::Net::SocketAddress sender;
        //int bytesRead = m_sock.receiveFrom(buffer, sizeof(buffer)-1, sender);
        bool ready = m_sock.poll ( timeout, Socket::SELECT_READ );
        if ( ! ready )
        {
            continue;
        }
        int bytesRead = m_sock.receiveFrom ( buffer, sizeof ( buffer )-1, sender );
//         cout << "got " << bytesRead << " bytes\n";
        if ( bytesRead == 0 )
        {
//             cout << "no bytes\n";
            continue;
        }
        buffer[bytesRead] = '\0';
        //std::cout << sender.toString() << ": " << buffer << std::endl;

        //     // Filter out messages from unwanted IPs
        if ( m_bListenToFilter )
        {
            uint32 i;
            for ( i=0; i<m_listenToAddresses.size(); ++i )
            {
                if ( m_listenToAddresses[i] == sender.host() )
                {
                    // Found a match.
                    break;
                }
            }

            if ( i == m_listenToAddresses.size() )
            {
                // We didn't find a match
                continue;
            }
        }

        //
        //     // Create an xplMsg object from the received data
        AutoPtr<xplMsg> pMsg = new xplMsg ( buffer );
//             incommingQueueLock.lock();
//             incommingQueue.push(pMsg);
//             cout << "added packet to queue of " << incommingQueue.size() << "\n";
//             incommingQueueLock.unlock();
//             m_rxEvent.set();
//

        rxNotificationCenter.postNotification ( new MessageRxNotification ( pMsg ) );
//             return ( pMsg );

    }
    poco_information(commsLog, "UDP rx thread stopped");
}

