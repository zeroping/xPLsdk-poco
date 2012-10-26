/***************************************************************************
****																	****
****	W800.cpp														****
****																	****
****	xpl-enables the W800RF32 X10 rf receiver						****
****																	****
****	Copyright (c) 2005 Mal Lansell.									****
****																	****
****	SOFTWARE NOTICE AND LICENSE										****
****																	****
****	This work (including software, documents, or other related		****
****	items) is being provided by the copyright holders under the		****
****	following license. By obtaining, using and/or copying this		****
****	work, you (the licensee) agree that you have read, understood,	****
****	and will comply with the following terms and conditions:		****
****																	****
****	Permission to use, copy, and distribute this software and its	****
****	documentation, without modification, for any purpose and		****
****	without fee or royalty is hereby granted, provided that you		****
****	include the full text of this NOTICE on ALL copies of the		****
****	software and documentation or portions thereof.					****
****																	****
****	THIS SOFTWARE AND DOCUMENTATION IS PROVIDED "AS IS," AND		****
****	COPYRIGHT HOLDERS MAKE NO REPRESENTATIONS OR WARRANTIES,		****
****	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO, WARRANTIES OF	****
****	MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT	****
****	THE USE OF THE SOFTWARE OR DOCUMENTATION WILL NOT INFRINGE ANY	****
****	THIRD PARTY PATENTS, COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS.	****
****																	****
****	COPYRIGHT HOLDERS WILL NOT BE LIABLE FOR ANY DIRECT, INDIRECT,	****
****	SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF ANY USE OF THE	****
****	SOFTWARE OR DOCUMENTATION.										****
****																	****
****	The name and trademarks of copyright holders may NOT be used in	****
****	advertising or publicity pertaining to the software without		****
****	specific, written prior permission.  Title to copyright in this ****
****	software and any associated documentation will at all times		****
****	remain with copyright holders.									****
****																	****
***************************************************************************/

#include "xplDevice.h"
#include "xplUDP.h"
#include "xplConfigItem.h"
#include "xplMsg.h"
#include "Service.h"
#include "W800.h"
#include "EventLog.h"

using namespace xpl;

string const W800::c_version = "6.0.0";


/***************************************************************************
****																	****
****	main															****
****																	****
****	Parses the command line and installs, uninstalls or runs the	****
****	Service service as appropriate	 								****
****																	****
****	8th August 2004													****
****																	****
***************************************************************************/

int main ( int argc, char* argv[] )
{
    // Create the EventLog
    EventLog::Create ( "xPLW800" );

    // Create the application
    W800* pApp = new W800();

    // Wrap the application in a service
    Service* pService = Service::Create ( "xPLW800", "xPL gateway for the W800RF32 receiver", W800::MainProc, ( void* ) pApp );

    // Install, Uninstall or Run the service as determined by the command line
    int res = pService->ProcessCommandLine ( argc, argv );

    // Clean up
    Service::Destroy();
    delete pApp;
    EventLog::Destroy();

    return ( res );
}


/***************************************************************************
****																	****
****	W800 Constructor												****
****																	****
***************************************************************************/

W800::W800() :
    m_pDevice ( NULL ),
    m_pComms ( NULL ),
    m_pW800 ( NULL ),
    m_comPort ( -1 ),
    m_pMsg ( NULL ),
    m_bBlockDuplicates ( false ),
    m_timeout ( 200 )
{
    // Create an event that will be signalled when a messsage has been received from the W800RF32
    m_hRxNotify = CreateEvent ( NULL, TRUE, FALSE, NULL );
}


/***************************************************************************
****																	****
****	W800 Destructor													****
****																	****
***************************************************************************/

W800::~W800()
{
    CloseHandle ( m_hRxNotify );
}


/***************************************************************************
****																	****
****	W800::Run														****
****																	****
***************************************************************************/

void W800::Run
(
    HANDLE _hActive,
    HANDLE _hExit
)
{
    // Init

    m_pW800 = NULL;
    m_comPort = -1;
    m_pMsg = NULL;
    m_bBlockDuplicates = false;
    m_timeout = 200;

    // Create the xPL communications object
    m_pComms = xplUDP::Create ( true );
    if ( NULL == m_pComms )
    {
        return;
    }

    //Create the xPL Device
    m_pDevice = xplDevice::Create ( "mal", "w800rf32", c_version, true, true, m_pComms );
    if ( NULL == m_pDevice )
    {
        return;
    }

    // Get the message and config events
    HANDLE xplMsgEvent = m_pDevice->GetMsgEvent();
    HANDLE configEvent = m_pDevice->GetConfigEvent();

    // Create the additional config items
    // Com port to which the W800RF32 is connected
    xplConfigItem* pItem = new xplConfigItem ( "comport", "reconf" );
    if ( NULL != pItem )
    {
        pItem->AddValue ( "1" );
        m_pDevice->AddConfigItem ( pItem );
    }

    // Timeout for filtering duplicate messages
    pItem = new xplConfigItem ( "timeout", "reconf" );
    if ( NULL != pItem )
    {
        pItem->AddValue ( "200" );
        m_pDevice->AddConfigItem ( pItem );
    }

    // Init the xplDevice
    // Note that all config items and callbacks must
    // have been set up before Init() is called.
    m_pDevice->Init();

    // Main loop
    while ( 1 )
    {
        // Wait for a message to be received from the W800RF32,
        // or for the exit event to occur.
        HANDLE handles[4];
        handles[0] = _hExit;
        handles[1] = m_hRxNotify;
        handles[2] = xplMsgEvent;
        handles[3] = configEvent;

        DWORD trigger = WaitForMultipleObjects ( 4, handles, FALSE, INFINITE );
        if ( WAIT_OBJECT_0 == trigger )
        {
            // Exit event was signalled
            break;
        }

        // Check we're not meant to be paused
        handles[1] = _hActive;
        if ( WAIT_OBJECT_0 == WaitForMultipleObjects ( 2, handles, FALSE, INFINITE ) )
        {
            // Exit event was signalled
            break;
        }

        // Deal with any received xPL messages
        if ( ( WAIT_OBJECT_0+2 ) == trigger )
        {
            HandleMessages();
        }

        // Is configuration required?
        if ( ( WAIT_OBJECT_0+3 ) == trigger )
        {
            Configure();
        }

        // Deal with any received W800 messages
        if ( ( WAIT_OBJECT_0+1 ) == trigger )
        {
            // Check the timeout used to filter out the duplicate
            // messages that most X10 remote controls send
            if ( m_bBlockDuplicates )
            {
                int64 currentTime;
                GetSystemTimeAsFileTime ( ( FILETIME* ) &currentTime );
                if ( currentTime > m_nextMessageTime )
                {
                    m_bBlockDuplicates = false;
                }
            }

            // Check for new messages from the W800RF32
            xplMsg* pMsg = NULL;
            if ( m_pW800 )
            {
                pMsg = m_pW800->Poll();
            }

            if ( NULL != pMsg )
            {
                if ( m_bBlockDuplicates )
                {
                    // We do not send the same message again ( unless it is a DIM or BRIGHT )
                    // Compare the message to the last one
                    if ( ( *pMsg ) == ( *m_pMsg ) )
                    {
                        string command = pMsg->GetValue ( "command" );
                        if ( ( command != "bright" ) && ( command != "dim" ) )
                        {
                            // Skip this duplicate message, but reset the timer.
                            // This will stop another command being sent until
                            // the remote control button has been released and
                            // pressed again.
                            pMsg->Release();
                            GetSystemTimeAsFileTime ( ( FILETIME* ) &m_nextMessageTime );
                            m_nextMessageTime += ( m_timeout*10000 );	// *10000 converts from milliseconds of m_timeout to the 100 nanosecond intervals of FILETIME.
                            continue;
                        }
                    }
                }

                // Delete any stored message, and save the new one.
                if ( NULL != m_pMsg )
                {
                    m_pMsg->Release();
                }
                m_pMsg = pMsg;

                // Send the message.
                m_pDevice->SendMsg ( pMsg );

                // Do not release the message - it's ref will now be held by m_pMsg.

                m_bBlockDuplicates = true;
                GetSystemTimeAsFileTime ( ( FILETIME* ) &m_nextMessageTime );
                m_nextMessageTime += ( m_timeout*10000 );	// *10000 converts from milliseconds of m_timeout to the 100 nanosecond intervals of FILETIME.
            }
        }
    }

    // Clean up and exit

    // Destroy the xPL Device
    // This also deletes any config items we added
    xplDevice::Destroy ( m_pDevice );
    m_pDevice = NULL;

    // Destroy the comms object
    xplUDP::Destroy ( m_pComms );
    m_pComms = NULL;

    // Destroy the W800RF32 object
    W800RF32::Destroy ( m_pW800 );
    m_pW800 = NULL;

    // Delete any stored event
    m_pMsg->Release();
    m_pMsg = NULL;
    m_bBlockDuplicates = false;

    return;
}


/***************************************************************************
****																	****
****	W800::HandleMessages											****
****																	****
***************************************************************************/

void W800::HandleMessages()
{
    xplMsg* pMsg = NULL;
    while ( pMsg = m_pDevice->GetMsg() )
    {
        // We do nothing with any xPL messages
        pMsg->Release();
    }
}


/***************************************************************************
****																	****
****	W800::Configure													****
****																	****
***************************************************************************/

void W800::Configure()
{
    // Com port index
    xplConfigItem const* pItem = m_pDevice->GetConfigItem ( "comport" );
    string value = pItem->GetValue();

    if ( !value.empty() )
    {
        int comPort = atol ( value.c_str() );

        // Value must be between 1 and 256.  If not, we default to 1
        if ( ( comPort < 0 ) || ( comPort > 256 ) )
        {
            comPort = 1;
        }

        // If the com port number has changed, we will need to recreate the W800RF32 object
        if ( ( NULL == m_pW800 ) || ( comPort != m_comPort ) )
        {
            m_comPort = comPort;
            W800RF32::Destroy ( m_pW800 );
            m_pW800 = W800RF32::Create ( m_comPort, m_hRxNotify, m_pDevice->GetCompleteId() );
        }
    }

    // Duplicate message timeout
    pItem = m_pDevice->GetConfigItem ( "timeout" );
    value = pItem->GetValue();
    if ( !value.empty() )
    {
        //Value must be less than 1000 (representing one second)
        int timeout = atol ( value.c_str() );

        if ( timeout < 0 )
        {
            timeout = 0;
        }

        if ( timeout > 1000 )
        {
            timeout = 1000;
        }

        m_timeout = ( unsigned long ) timeout;
    }
}


/***************************************************************************
****																	****
****	W800::MainProc													****
****																	****
***************************************************************************/

void W800::MainProc
(
    HANDLE _hActive,
    HANDLE _hExit,
    void* pContext
)
{
    W800* pW800 = ( W800* ) pContext;
    pW800->Run ( _hActive, _hExit );
}






