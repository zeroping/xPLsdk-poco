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
#include "xplCore.h"
#include "xplStringUtils.h"
#include "xplMsg.h"
#include "xplUDP.h"
// #include "EventLog.h"
// #include "RegUtils.h"

#include <iostream>

using namespace xpl;
using namespace Poco::Net;

uint16 const xplUDP::c_xplHubPort = 3865;


/***************************************************************************
****																	****
****	xplUDP::Create													****
****																	****
***************************************************************************/

xplUDP* xplUDP::Create
(
    bool const _bViaHub
)
{
// 	uint32_t wVersionRequested = MAKEWORD( 2, 2 );
// 	WSADATA wsaData;
// 	uint32 err = WSAStartup( wVersionRequested, &wsaData );
// 	if ( err != 0 )
// 	{
// 		// No suitable Winsock DLL found
// 		assert( 0 );
// 		return( NULL );
// 	}
//
// 	// Confirm that the Winsock DLL supports 2.2.*/
// 	if( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 )
// 	{
// 		assert( 0 );
// 		WSACleanup();
// 		return( NULL );
// 	}


    cout << "xplUDP create\n";
    
    // Create the xplUDP object
    xplUDP* pObj = new xplUDP ( _bViaHub );
    if ( !pObj->Connect() )
    {
        xplUDP::Destroy ( pObj );
        pObj = NULL;
    }

    return ( pObj );
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
    xplComms(),
    m_bViaHub ( _bViaHub ),
    m_txPort ( c_xplHubPort ),
    //m_txAddr ( INADDR_BROADCAST ),
    //m_listenOnAddress ( INADDR_ANY ),
    m_bListenToFilter ( false ),
    m_ip ( "127.0.0.1" )
    //m_rxEvent ( WSA_INVALID_EVENT )
{
    uint32 i;
// 
//     // Build the list of local IP addreses
     GetLocalIPs();
// 
//     // If possible, set our IP (for use in heatbeats) to the
//     // first local IP that is not the loopback address.
    for ( i=0; i<m_localIPs.size(); ++i )
    {
//         if ( INADDR_LOOPBACK != m_localIPs[i] )
        //cout << "testing : " << m_localIPs[i].toString() << " : " << m_localIPs[i].isLinkLocal() << "\n";
        if (!m_localIPs[i].isLoopback())
        {
//             sockaddr_in addr;
//             memset ( &addr, 0, sizeof ( addr ) );
//             addr.sin_family = AF_INET;
//             addr.sin_addr.s_addr = m_localIPs[i];
//             char* ip = inet_ntoa ( addr.sin_addr );
//             m_ip = ip;
            m_ip = m_localIPs[i];
            break;
        }
    }
    cout << "our hbeat address is " << m_ip.toString() << "\n";
// 
//     // Read the xpl settings from the registry
// 
//     HKEY hKey;
//     if ( RegOpen ( HKEY_LOCAL_MACHINE, "SOFTWARE\\xPL", &hKey ) )
//     {
//         // Broadcast Address
//         string broadcastAddress;
//         if ( RegRead ( hKey, "BroadcastAddress", &broadcastAddress ) )
//         {
//             uint32 ip = inet_addr ( broadcastAddress.c_str() );
//             if ( INADDR_NONE != ip )
//             {
//                 m_txAddr = ip;
//             }
//         }
// 
//         // Listen-On Address
//         string listenOnAddress;
//         if ( RegRead ( hKey, "ListenOnAddress", &listenOnAddress ) )
//         {
//             if ( listenOnAddress != string ( "ANY_LOCAL" ) )
//             {
//                 // If the setting is not "ANY LOCAL" we assume that it is an address instead
//                 // The address must be local.
//                 uint32 ip = inet_addr ( listenOnAddress.c_str() );
//                 if ( ( INADDR_NONE != ip ) && IsLocalIP ( ip ) )
//                 {
//                     m_listenOnAddress = ip;
//                     m_ip = listenOnAddress;
//                 }
//             }
//         }
// 
//         // Listen-To Addresses
//         string listenToAddress;
//         if ( RegRead ( hKey, "ListenToAddress", &listenToAddress ) )
//         {
//             if ( listenToAddress != "ANY" )
//             {
//                 // Assume a comma-separated list of IPs
//                 m_bListenToFilter = true;
// 
//                 uint32 start = 0;
//                 uint32 pos = 0;
//                 uint32 length = ( uint32 ) listenToAddress.size();
//                 int8* pBuffer = new int8[length+1];
//                 memcpy ( pBuffer, listenToAddress.c_str(), length );
//                 pBuffer[length] = 0;
//                 while ( pBuffer[pos++] )
//                 {
//                     int8 ch = pBuffer[pos];
//                     if ( !ch || ( ch==',' ) )
//                     {
//                         // Extract the string
//                         pBuffer[pos] = 0;
// 
//                         uint32 ip = inet_addr ( &pBuffer[start] );
//                         if ( INADDR_NONE != ip )
//                         {
//                             m_listenToAddresses.push_back ( ip );
//                         }
// 
//                         pBuffer[pos] = ch;
//                         start = pos+1;
//                     }
//                 }
//                 delete [] pBuffer;
//             }
//         }
// 
//         // Close the key
//         RegClose ( hKey );
//     }
}


/***************************************************************************
****																	****
****	xplUDP::~xplUDP													****
****																	****
***************************************************************************/

xplUDP::~xplUDP()
{
    assert ( !IsConnected() );

    // Finished with the sockets
//     WSACleanup();
}


/***************************************************************************
****																	****
****	xplUDP::TxMsg													****
****																	****
***************************************************************************/

bool xplUDP::TxMsg
(
    xplMsg* _pMsg
)
{
    bool bRetVal = false;
    

    if ( IsConnected() )
    {
        // Build the complete message
        int8* pMsgBuffer;
        uint32 dataLength = _pMsg->GetRawData ( &pMsgBuffer );

        string lbcastaddr = m_ip.toString();
        int lastd = lbcastaddr.find_last_of('.');
        lbcastaddr = lbcastaddr.substr(0,lastd+1) + "255";

        IPAddress lbcast = Poco::Net::IPAddress(lbcastaddr);

        
        Poco::Net::SocketAddress sourceAddress(m_ip, 50000);
        Poco::Net::SocketAddress destAddress(lbcast, 3865);
        DatagramSocket dgs(sourceAddress,true);
        dgs.setBroadcast(true);
        cout << "tx message from " << sourceAddress.toString();
        cout << "  to " << destAddress.toString() << "\n";
        dgs.sendTo(pMsgBuffer,dataLength, destAddress);
        
        //m_sock.sendTo(pMsgBuffer,dataLength, broad);
        
        
        
//         if ( SOCKET_ERROR != sendto ( m_sock, pMsgBuffer, dataLength, 0, ( SOCKADDR* ) &txAddr, sizeof ( txAddr ) ) )
//         {
//             // Success
//             bRetVal = true;
//         }
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
    //cout << "rx wait\n";
    if (!incommingQueue.size()) {
        try {
            m_rxEvent.wait(_timeout);
        } catch (Poco::TimeoutException& e) { 
            cout << "timed out on RX\n";
            return NULL;
        }
    } else {
        cout<<"have pkt waiting\n";
    }
        
    //cout << "rx gotten\n";
    
    incommingQueueLock.lock();
    xplMsg* toRet = incommingQueue.front();
    incommingQueue.pop();
    incommingQueueLock.unlock();
    m_rxEvent.reset();
    return toRet;
    
    // Wait if required
//     if ( _timeout )
//     {
//         HANDLE handles[2];
//         handles[0] = m_rxEvent;		//Network event signalled on FD_
//         handles[1] = _hInterrupt;
// 
//          numHandles = 2;
//         if ( INVALID_HANDLE_VALUE == _hInterrupt )
//         {
//             numHandles = 1;
//         }
// 
//         // Wait
//          res = WaitForMultipleObjects ( numHandles, handles, FALSE, _timeout );
// 
//         // Check if the wait timed-out, or the interrupt event was signalled
//         if ( WAIT_OBJECT_0 != res )
//         {
//             return NULL;
//         }
// 
//         // Reset the message event
//         WSAResetEvent ( m_rxEvent );
//     }
// 
//     // Read any incoming messages
//     
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

    cout << "trying port " << m_rxPort << "\n";
    Poco::Net::SocketAddress sa(Poco::Net::IPAddress(), m_rxPort);


//     dgs.setBroadcast(true);
//     
//     char buffer[1024];
//     for (;;)
//     {
//         Poco::Net::SocketAddress sender;
//         int n = dgs.receiveFrom(buffer, sizeof(buffer)-1, sender);
//         buffer[n] = '\0';
//         std::cout << sender.toString() << ": " << buffer << std::endl;
//         
//         std::string syslogMsg;
//         Poco::Timestamp now;
//         syslogMsg = Poco::DateTimeFormatter::format(now,
//                                                     "<14>%w %f %H:%M:%S Hello, world!");
//         
//         Poco::Net::SocketAddress broad("255.255.255.255", 12344);
//         dgs.sendTo(syslogMsg.data(),syslogMsg.size(), broad);
//     }


//     // Create a socket for sending messages
//     if ( INVALID_SOCKET == ( m_sock = socket ( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) )
//     {
//         if ( EventLog::Get() )
//         {
//             EventLog::Get()->ReportError ( "Unable to create socket" );
//         }
//         goto ConnectError;
//     }
// 
//     // Enable broadcast transmission
//     {
//         uint32 enable = 1;
//         if ( setsockopt ( m_sock, SOL_SOCKET, SO_BROADCAST, ( int8* ) &enable, sizeof ( enable ) ) )
//         {
//             if ( EventLog::Get() )
//             {
//                 EventLog::Get()->ReportError ( "Cannot enable broadcast mode" );
//             }
//             goto ConnectError;
//         }
//     }
// 
//     // If we are not communicating via a hub (PocketPC app or the hub
//     // app itself, for example), then we bind directly to the standard
//     // xpl port.  Otherwise we try to bind to one numbered 50000+.
     bool bBound = false;
     
     try { 
         m_sock = DatagramSocket(sa,false);
         m_sock.setBroadcast(true);
         bBound = true;
     } catch (NetException & e) {
         cout << "can't open port " << m_rxPort << "\n"; 
     }
     
//     if ( !m_bViaHub )
//     {
//         // Not using a hub.  If we fail to bind to the hub port, then we will
//         // assume there is already one running, and bind to port 50000+ instead.
//         m_rxPort = c_xplHubPort;
//         sockaddr_in rxAddr;
//         rxAddr.sin_family = AF_INET;
//         rxAddr.sin_addr.s_addr = m_listenOnAddress;
//         rxAddr.sin_port = htons ( m_rxPort );
//         if ( SOCKET_ERROR != bind ( m_sock, ( SOCKADDR* ) &rxAddr, sizeof ( rxAddr ) ) )
//         {
//             bBound = true;
// 
//             if ( EventLog::Get() )
//             {
//                 char str[64];
//                 sprintf ( str, "Listening to port %d on %s", m_rxPort, inet_ntoa ( rxAddr.sin_addr ) );
//                 EventLog::Get()->ReportInformation ( str );
//             }
//         }
//     }
// 
     if ( !bBound )
     {
//         // Try to bind to a port numbered 50000+
//         sockaddr_in rxAddr;
//         rxAddr.sin_family = AF_INET;
//         rxAddr.sin_addr.s_addr = m_listenOnAddress;
         m_rxPort = 50000;
// 
         while ( !bBound )
         {
             sa = SocketAddress(Poco::Net::IPAddress(), m_rxPort);
             try { 
                 m_sock = DatagramSocket(sa,true);
                 bBound = true;
             } catch (NetException & e) {
                 cout << "can't open port " << m_rxPort << "\n";
                 m_rxPort += 1;
             }
//             rxAddr.sin_port = htons ( m_rxPort );
//             if ( SOCKET_ERROR != bind ( m_sock, ( SOCKADDR* ) &rxAddr, sizeof ( rxAddr ) ) )
//             {
//                 if ( EventLog::Get() )
//                 {
//                     char str[64];
//                     sprintf ( str, "Listening to port %d on %s", m_rxPort, inet_ntoa ( rxAddr.sin_addr ) );
//                     EventLog::Get()->ReportInformation ( str );
//                 }
//                 break;
//             }
// 
//             if ( WSAEADDRINUSE == WSAGetLastError() )
//             {
//                 // Port in use.  Try the next one
//                 ++m_rxPort;
//                 continue;
//             }
// 
//             if ( EventLog::Get() )
//             {
//                 char str[64];
//                 sprintf ( str, "Unable to bind to a port on %s", inet_ntoa ( rxAddr.sin_addr ) );
//                 EventLog::Get()->ReportError ( str );
//             }
// 
//              errorCode = WSAGetLastError();
//             goto ConnectError;
         }
     }
     cout << "open on port " << m_rxPort << "\n";
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
//     // Create the event to use to wait for received data.
//     m_rxEvent = WSACreateEvent();
//     if ( WSA_INVALID_EVENT != m_rxEvent )
//     {
//         WSAEventSelect ( m_sock, m_rxEvent, FD_READ );
//     }
// 
//     // Call the base class
     
     xplComms::Connect();
     cout << "comms connected\n";
     
     listenAdapt = new RunnableAdapter<xplUDP>(*this,&xplUDP::ListenForPackets);
     listenThread.setName("packet listen thread");
     listenThread.start(*listenAdapt);

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
//         if ( INVALID_SOCKET != m_sock )
//         {
//             closesocket ( m_sock );
//             m_sock = INVALID_SOCKET;
//         }
// 
//         if ( WSA_INVALID_EVENT != m_rxEvent )
//         {
//             WSACloseEvent ( m_rxEvent );
//             m_rxEvent = WSA_INVALID_EVENT;
//         }
// 


         xplComms::Disconnect();
        listenThread.join();
    
     }
    

}


/***************************************************************************
****																	****
****	xplUDP::GetLocalIPs						  						****
****																	****
***************************************************************************/

bool xplUDP::GetLocalIPs()
{
    // Remove any old addresses
    m_localIPs.clear();
//
//     int8 hostName[81];
//     if ( gethostname ( hostName, 80 ) )
//     {
//         // Error (no network card?)
//         goto GetLocalIPsError;
//     }
//
//     // Build the list of IP addresses
//     {
//         struct hostent* pHostEnt = gethostbyname ( hostName );
//         if ( NULL == pHostEnt )
//         {
//             // Error (no network card?)
//             goto GetLocalIPsError;
//         }
//
//         int8* pAddr = NULL;
//         uint32 i = 0;
//         while ( pAddr = pHostEnt->h_addr_list[i++] )
//         {
//             m_localIPs.push_back ( * ( unsigned long* ) pAddr );
//         }
//
//         if ( m_localIPs.size() > 0 )
//         {
//             // List built successfully.
//             return true;
//         }
//     }
//
//     // No IPs found

// horrible hack for linux

#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netdevice.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
    int s;
    struct ifconf ifconf;
    struct ifreq ifr[50];
    int ifs;
    int i;

    s = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( s < 0 )
    {
        perror ( "socket" );
        return 0;
    }

    ifconf.ifc_buf = ( char * ) ifr;
    ifconf.ifc_len = sizeof ifr;

    if ( ioctl ( s, SIOCGIFCONF, &ifconf ) == -1 )
    {
        perror ( "ioctl" );
        return 0;
    }

    ifs = ifconf.ifc_len / sizeof ( ifr[0] );
    printf ( "interfaces = %d:\n", ifs );
    for ( i = 0; i < ifs; i++ )
    {
        char ip[INET_ADDRSTRLEN];
        struct sockaddr_in *s_in = ( struct sockaddr_in * ) &ifr[i].ifr_addr;

        if ( !inet_ntop ( AF_INET, &s_in->sin_addr, ip, sizeof ( ip ) ) )
        {
            perror ( "inet_ntop" );
            return 0;
        }

        printf ( "%s - %s\n", ifr[i].ifr_name, ip );
        m_localIPs.push_back ( IPAddress ( ip ) );
    }

    close ( s );





// GetLocalIPsError:
//     // There was a problem, but we can at least
//     // have the loopback address in the list.
    //Poco::Net::IPAddress local = Poco::Net::IPAddress("127.0.0.1" );
    //m_localIPs.push_back (local);
    return true;
}


/***************************************************************************
****																	****
****	xplUDP::IsLocalIP						  						****
****																	****
***************************************************************************/

bool xplUDP::IsLocalIP
(
    uint32 const _ip
) const
{
//     for ( int i=0; i<m_localIPs.size(); ++i )
//     {
//         if ( m_localIPs[i] == _ip )
//         {
//             return true;
//         }
//     }

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
    xplMsg* pMsg = xplMsg::Create ( xplMsg::c_xplStat, _source, "*", "hbeat", "app" );
    if ( pMsg )
    {
        int8 value[16];

        sprintf ( value, "%d", _interval );
        pMsg->AddValue ( "interval", value );

        sprintf ( value, "%d", m_rxPort );
        pMsg->AddValue ( "port", value );

        pMsg->AddValue ( "remote-ip", m_ip.toString() );
        pMsg->AddValue ( "version", _version );

        TxMsg ( pMsg );
        pMsg->Release();
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
    xplMsg* pMsg = xplMsg::Create ( xplMsg::c_xplStat, _source, "*", "config", "app" );
    if ( pMsg )
    {
        int8 value[16];

        sprintf ( value, "%d", _interval );
        pMsg->AddValue ( "interval", value );

        sprintf ( value, "%d", m_rxPort );
        pMsg->AddValue ( "port", value );

        pMsg->AddValue ( "remote-ip", m_ip.toString() );
        pMsg->AddValue ( "version", _version );

        TxMsg ( pMsg );
        pMsg->Release();
    }
}


void xplUDP::ListenForPackets() {
    cout << "started listening\n";
    //m_sock.setReceiveTimeout(1000);
    while( this->IsConnected()){//TODO locking here
        //cout << "listening\n";
        char buffer[2024];
        Poco::Net::SocketAddress sender;
        int bytesRead = m_sock.receiveFrom(buffer, sizeof(buffer)-1, sender);
        
        cout << "got " << bytesRead << " bytes\n";
        if (bytesRead == 0) {
            
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
            xplMsg* pMsg = xplMsg::Create ( buffer );
            incommingQueueLock.lock();
            incommingQueue.push(pMsg);
            cout << "added packet to queue of " << incommingQueue.size() << "\n";
            incommingQueueLock.unlock();
            m_rxEvent.set();
            
//             return ( pMsg );
        
    }
    cout << "udp rx thread stopped\n";
}

