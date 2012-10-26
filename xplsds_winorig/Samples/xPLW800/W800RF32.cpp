/***************************************************************************
****																	****
****	W800RF32.cpp													****
****																	****
****	Support for X10 W800RF32 device									****
****																	****
****	Copyright (c) 2005 Mal Lansell.									****
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

#include <assert.h>
#include <atlbase.h>
#include <comutil.h>
#include <msxml2.h>
#include <xplMsg.h>
#include "W800RF32.h"
#include "SensorManager.h"

string const W800RF32::c_houseCodes[16] =
{
    "M",
    "E",
    "C",
    "K",
    "O",
    "G",
    "A",
    "I",
    "N",
    "F",
    "D",
    "L",
    "P",
    "H",
    "B",
    "J"
};

/***************************************************************************
****																	****
****	W800RF32::Create												****
****																	****
***************************************************************************/

W800RF32* W800RF32::Create
(
    int _comPortIndex,
    HANDLE _hRxNotify,
    string const& _msgSource
)
{
    // Get COM port name
    char comPortName[8];
    sprintf ( comPortName, "COM%d", _comPortIndex );

    // Open COM port
    HANDLE hComPort = CreateFile (	comPortName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_FLAG_OVERLAPPED,
                                    NULL );

    if ( INVALID_HANDLE_VALUE == hComPort )
    {
        //Error
        printf ( "Cannot open %s: Error code %d\n", comPortName, GetLastError() );
        return NULL;
    }

    // Finish configuring the serial device parameters
    // Build on the current configuration
    DCB dcb;
    if ( !GetCommState ( hComPort, &dcb ) )
    {
        //Error.  Clean up and exit
        CloseHandle ( hComPort );
        return NULL;
    }

    // Fill in the DCB: baud=4800 bps, 8 data bits, no parity, and 1 stop bit.
    dcb.BaudRate = CBR_4800;		//Set the baud rate
    dcb.ByteSize = 8;				//Data size, xmit, and rcv
    dcb.Parity = NOPARITY;			//No parity bit
    dcb.StopBits = ONESTOPBIT;		//One stop bit

    if ( !SetCommState ( hComPort, &dcb ) )
    {
        //Error. Clean up and exit
        CloseHandle ( hComPort );
        return NULL;
    }

    // Set the timeouts for the com port
    {
        COMMTIMEOUTS commTimeouts;
        commTimeouts.ReadIntervalTimeout = MAXDWORD;
        commTimeouts.ReadTotalTimeoutConstant = 0;
        commTimeouts.ReadTotalTimeoutMultiplier = 0;
        commTimeouts.WriteTotalTimeoutConstant = 0;
        commTimeouts.WriteTotalTimeoutMultiplier = 0;
        if ( !SetCommTimeouts ( hComPort, &commTimeouts ) )
        {
            // Error.  Clean up and exit
            CloseHandle ( hComPort );
            return NULL;
        }
    }

    // Set the com port to signal when data is received
    if ( !SetCommMask ( hComPort, EV_RXCHAR ) )
    {
        //Error.  Clean up and exit
        CloseHandle ( hComPort );
        return NULL;
    }

    printf ( "Serial port successfully configured\n" );

    // Finally, create the W800RF32 object
    W800RF32* pW800RF32 = new W800RF32 ( hComPort, _hRxNotify, _msgSource );

    if ( pW800RF32 )
    {
        // Create a thread to manage communications with the W800RF32 interface
        HANDLE hThread = ::CreateThread ( NULL, 0, W800RF32::ReadRFThreadProc, pW800RF32, CREATE_SUSPENDED, NULL );
        pW800RF32->m_hThread = hThread;
        ::ResumeThread ( hThread );
    }

    return ( pW800RF32 );
}


/***************************************************************************
****																	****
****	W800RF32 Constructor											****
****																	****
***************************************************************************/

W800RF32::W800RF32
(
    HANDLE _hComPort,
    HANDLE _hRxNotify,
    string const& _msgSource
) :
    m_hComPort ( _hComPort ),
    m_hRxNotify ( _hRxNotify ),
    m_msgSource ( _msgSource )
{
    LoadSecurityCodes();
    LoadPCRemoteCodes();

    SensorManager::Create();

    // Create an event allowing us to tell the RFRead Thread to exit
    m_hExitEvent = CreateEvent ( NULL, TRUE, FALSE, NULL );

    // Initialise the CriticalSection object that we will use to
    // serialise access to the thread data.
    InitializeCriticalSection ( &m_criticalSection );
}


/***************************************************************************
****																	****
****	W800RF32 Destructor												****
****																	****
***************************************************************************/

W800RF32::~W800RF32()
{
    // Terminate the W800RF32_IO_Thread
    SetEvent ( m_hExitEvent );

    // Wait for the thread to exit
    WaitForSingleObject ( m_hThread, INFINITE );

    // Close the handles
    CloseHandle ( m_hThread );
    CloseHandle ( m_hExitEvent );
    CloseHandle ( m_hComPort );

    //Delete the CriticalSection object
    DeleteCriticalSection ( &m_criticalSection );

    // Delete the Security code map
    {
        map<uint8,SecurityCode*>::iterator iter = m_securityMap.begin();
        while ( iter != m_securityMap.end() )
        {
            delete iter->second;
            iter = m_securityMap.erase ( iter );
        }
    }

    // Delete the PC Remote code map
    {
        map<uint8,PCRemoteCode*>::iterator iter = m_pcRemoteMap.begin();
        while ( iter != m_pcRemoteMap.end() )
        {
            delete iter->second;
            iter = m_pcRemoteMap.erase ( iter );
        }
    }

    SensorManager::Destroy();
}


/***************************************************************************
****																	****
****	W800RF32::Poll													****
****																	****
***************************************************************************/

xplMsg* W800RF32::Poll()
{
    xplMsg* pMsg = NULL;

    //If there are any events in the RxEvents buffer, remove
    //the first one and return it to the caller.
    //Access to the event buffers must be serialised
    EnterCriticalSection ( &m_criticalSection );
    if ( m_rxEvents.size() )
    {
        pMsg = m_rxEvents.front();
        m_rxEvents.pop_front();

        if ( 0 == m_rxEvents.size() )
        {
            //No events left, so clear the signal
            ResetEvent ( m_hRxNotify );
        }
    }

    LeaveCriticalSection ( &m_criticalSection );
    return ( pMsg );
}


/***************************************************************************
****																	****
****	W800RF32::LoadSecurityCodes										****
****																	****
***************************************************************************/

bool W800RF32::LoadSecurityCodes()
{
    HRESULT hr;
    IXMLDOMDocument* pXMLDoc = NULL;
    bool retVal = false;

    while ( 1 )
    {
        CoInitialize ( NULL );

        if ( S_OK != CoCreateInstance ( CLSID_DOMDocument30, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, ( void** ) &pXMLDoc ) )
        {
            break;
        }

        CComVariant filename;
        VARIANT_BOOL bSuccess;
        char path[MAX_PATH];
        ::GetCurrentDirectory ( MAX_PATH, path );
        strcat ( path, "\\security.xml" );
        filename = path;

        if ( S_OK != pXMLDoc->load ( filename, &bSuccess ) )
        {
            break;
        }

        IXMLDOMElement* pXMLElement;
        hr = pXMLDoc->get_documentElement ( &pXMLElement );

        IXMLDOMNode* pNode;
        pXMLElement->get_firstChild ( &pNode );

        while ( pNode )
        {
            string command;
            uint8 code = 0;
            string tamper;
            string lowBattery;
            string delay;

            // Read the entry
            IXMLDOMNode* pItem;
            if ( S_OK == pNode->selectSingleNode ( L"Code", &pItem ) )
            {
                CComBSTR text;
                pItem->get_text ( &text );
                code = ( uint8 ) wcstol ( text, NULL, 0 );
                pItem->Release();
            }

            if ( S_OK == pNode->selectSingleNode ( L"Command", &pItem ) )
            {
                CComBSTR text;
                pItem->get_text ( &text );
                int length = WideCharToMultiByte ( CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL );
                char* str = new char[length+1];
                WideCharToMultiByte ( CP_UTF8, 0, text, -1, str, length, NULL, NULL );
                command = str;
                delete [] str;
                pItem->Release();
            }

            if ( S_OK == pNode->selectSingleNode ( L"Tamper", &pItem ) )
            {
                CComBSTR text;
                pItem->get_text ( &text );
                if ( !_wcsicmp ( text, L"true" ) )
                {
                    tamper = "true";
                }
                if ( !_wcsicmp ( text, L"false" ) )
                {
                    tamper = "false";
                }
                pItem->Release();
            }

            if ( S_OK == pNode->selectSingleNode ( L"LowBattery", &pItem ) )
            {
                CComBSTR text;
                pItem->get_text ( &text );
                if ( !_wcsicmp ( text, L"true" ) )
                {
                    lowBattery = "true";
                }
                if ( !_wcsicmp ( text, L"false" ) )
                {
                    lowBattery = "false";
                }
                pItem->Release();
            }

            if ( S_OK == pNode->selectSingleNode ( L"Delay", &pItem ) )
            {
                CComBSTR text;
                pItem->get_text ( &text );
                if ( !_wcsicmp ( text, L"min" ) )
                {
                    delay = "min";
                }
                if ( !_wcsicmp ( text, L"max" ) )
                {
                    delay = "max";
                }
                pItem->Release();
            }

            m_securityMap[code] = new SecurityCode ( code, command, tamper, lowBattery, delay );

            // Move to the next entry
            IXMLDOMNode* pSibling;
            pNode->get_nextSibling ( &pSibling );
            pNode->Release();
            pNode = pSibling;
        }

        pXMLElement->Release();
        retVal = true;
        break;
    }

    // Cleanup
    if ( pXMLDoc )
    {
        pXMLDoc->Release();
    }

    CoUninitialize();

    return retVal;
}


/***************************************************************************
****																	****
****	W800RF32::LoadPCRemoteCodes										****
****																	****
***************************************************************************/

bool W800RF32::LoadPCRemoteCodes()
{
    HRESULT hr;
    IXMLDOMDocument* pXMLDoc = NULL;
    bool retVal = false;

    while ( 1 )
    {
        CoInitialize ( NULL );

        if ( S_OK != CoCreateInstance ( CLSID_DOMDocument30, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, ( void** ) &pXMLDoc ) )
        {
            break;
        }

        CComVariant filename;
        VARIANT_BOOL bSuccess;
        char path[MAX_PATH];
        ::GetCurrentDirectory ( MAX_PATH, path );
        strcat ( path, "\\pcremote.xml" );
        filename = path;

        if ( S_OK != pXMLDoc->load ( filename, &bSuccess ) )
        {
            break;
        }

        IXMLDOMElement* pXMLElement;
        hr = pXMLDoc->get_documentElement ( &pXMLElement );

        IXMLDOMNode* pNode;
        pXMLElement->get_firstChild ( &pNode );

        while ( pNode )
        {
            string command;
            uint8 code = 0;

            // Read the entry
            IXMLDOMNode* pItem;
            if ( S_OK == pNode->selectSingleNode ( L"Code", &pItem ) )
            {
                CComBSTR text;
                pItem->get_text ( &text );
                code = ( uint8 ) wcstol ( text, NULL, 0 );
                pItem->Release();
            }

            if ( S_OK == pNode->selectSingleNode ( L"Command", &pItem ) )
            {
                CComBSTR text;
                pItem->get_text ( &text );
                int length = WideCharToMultiByte ( CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL );
                char* str = new char[length+1];
                WideCharToMultiByte ( CP_UTF8, 0, text, -1, str, length, NULL, NULL );
                command = str;
                delete [] str;
                pItem->Release();
            }

            m_pcRemoteMap[code] = new PCRemoteCode ( code, command );

            // Move to the next entry
            IXMLDOMNode* pSibling;
            pNode->get_nextSibling ( &pSibling );
            pNode->Release();
            pNode = pSibling;
        }

        pXMLElement->Release();
        retVal = true;
        break;
    }

    // Cleanup
    if ( pXMLDoc )
    {
        pXMLDoc->Release();
    }

    CoUninitialize();

    return retVal;
}


/***************************************************************************
****																	****
****	W800RF32::ReadRFThreadProc										****
****																	****
***************************************************************************/

DWORD WINAPI W800RF32::ReadRFThreadProc
(
    void* _pArg
)
{
    W800RF32* pW800RF32 = ( W800RF32* ) _pArg;
    pW800RF32->ReadRF();
    return 0;
}


/***************************************************************************
****																	****
****	W800RF32::ReadRF												****
****																	****
***************************************************************************/

void W800RF32::ReadRF()
{
    // Create an event that will be signalled when data is available
    OVERLAPPED overlapped;
    memset ( &overlapped, 0, sizeof ( overlapped ) );
    overlapped.hEvent = CreateEvent ( NULL, FALSE, FALSE, NULL );

    // Create an array of event handles to wait on
    HANDLE handles[2];
    handles[0] = m_hExitEvent;
    handles[1] = overlapped.hEvent;

    //IO Loop
    while ( 1 )
    {
        DWORD dwEvtMask;
        if ( !WaitCommEvent ( m_hComPort, &dwEvtMask, &overlapped ) )
        {
            // Wait for either some data to arrive or the signal that this thread should exit.
            if ( WAIT_OBJECT_0 == WaitForMultipleObjects ( 2, handles, FALSE, INFINITE ) )
            {
                // We must exit - application is closing
                return;
            }
        }

        // Read the input data
        unsigned char rxBuffer[4];
        DWORD bytesRead;
        if ( !ReadFile ( m_hComPort, rxBuffer, 4, &bytesRead, &overlapped ) )
        {
            //Wait for the read to complete
            if ( ERROR_IO_PENDING == GetLastError() )
            {
                GetOverlappedResult ( m_hComPort, &overlapped, &bytesRead, TRUE );
            }
        }


        if ( 4 != bytesRead )
        {
            continue;
        }

        xplMsg* pMsg = NULL;
        do
        {
            if ( pMsg = ProcessSensor ( rxBuffer, m_msgSource ) )
            {
                break;
            }

            if ( pMsg = ProcessSecurity ( rxBuffer, m_msgSource ) )
            {
                break;
            }

            if ( pMsg = ProcessPCRemote ( rxBuffer, m_msgSource ) )
            {
                break;
            }

            if ( pMsg = ProcessX10 ( rxBuffer, m_msgSource ) )
            {
                break;
            }

        }
        while ( 0 );

        if ( NULL != pMsg )
        {
            // Queue the message
            // Note: Access to the buffer must be serialised
            EnterCriticalSection ( &m_criticalSection );
            m_rxEvents.push_back ( pMsg );
            SetEvent ( m_hRxNotify );
            LeaveCriticalSection ( &m_criticalSection );
        }
    }
}


/***************************************************************************
****																	****
****	W800RF32::ProcessSensor											****
****																	****
***************************************************************************/

xplMsg* W800RF32::ProcessSensor
(
    uint8* _rf,
    string const& _msgSource
)
{
    if ( SensorManager* pSensorManager = SensorManager::Get() )
    {
        return ( pSensorManager->ProcessRF ( _rf, _msgSource ) );
    }

    return NULL;
}


/***************************************************************************
****																	****
****	W800RF32::ProcessSecurity										****
****																	****
***************************************************************************/

xplMsg* W800RF32::ProcessSecurity
(
    uint8* _rf,
    string const& _msgSource
)
{
    if ( ( _rf[0] ^ _rf[1] ) != 0x0f )
    {
        // Not an X10 Security Message
        return NULL;
    }

    if ( ( _rf[2] ^ _rf[3] ) != 0xff )
    {
        // Data Invalid
        return NULL;
    }

    // Data Bytes need to be reversed
    uint8 ch1 = 0;
    uint8 ch2 = 0;
    uint8 rf1 = _rf[0];
    uint8 rf2 = _rf[2];

    for ( int i=0; i<8; ++i )
    {
        ch1 <<= 1;
        ch1 |= ( rf1 & 0x01 );
        rf1 >>= 1;

        ch2 <<= 1;
        ch2 |= ( rf2 & 0x01 );
        rf2 >>= 1;
    }

    // Create a security message
    // Note: for now we just use remote.basic
    xplMsg* pMsg = xplMsg::Create ( xplMsg::c_xplTrig, _msgSource , "*", "remote", "basic" );

    char str[8];
    sprintf ( str, "%d", ( int ) ch2 );
    pMsg->AddValue ( "keys", str );

    sprintf ( str, "%d", ( int ) ch1 );
    pMsg->AddValue ( "device", str );
    return pMsg;
}


/***************************************************************************
****																	****
****	W800RF32::ProcessPCRemote										****
****																	****
***************************************************************************/

xplMsg* W800RF32::ProcessPCRemote
(
    uint8* _rf,
    string const& _msgSource
)
{
    // byte 1 must be the complement of byte 0,
    // and byte 3 the complement of byte 2.
    if ( ( _rf[0] ^ _rf[1] ) != 0xff )
    {
        // Data Invalid
        return NULL;
    }

    if ( ( _rf[2] ^ _rf[3] ) != 0xff )
    {
        // Data Invalid
        return NULL;
    }

    if ( 0x77 != _rf[0] )
    {
        // Not a PC Remote message
        return NULL;
    }

    // Data Bytes need to be reversed
    uint8 ch = 0;
    uint8 rf2 = _rf[2];

    for ( int i=0; i<8; ++i )
    {
        ch <<= 1;
        ch |= ( rf2 & 0x01 );
        rf2 >>= 1;
    }

    // Send a remote.basic message
    xplMsg* pMsg = xplMsg::Create ( xplMsg::c_xplTrig, _msgSource , "*", "remote", "basic" );

    // Translate the keycode into a string
    string keys;
    map<uint8,PCRemoteCode*>::iterator iter = m_pcRemoteMap.find ( ch );
    if ( iter == m_pcRemoteMap.end() )
    {
        // No name found, so just send the code number
        char str[8];
        sprintf ( str, "%d", ( int ) ch );
        pMsg->AddValue ( "keys", str );
    }
    else
    {
        pMsg->AddValue ( "keys", iter->second->m_command );
    }

    pMsg->AddValue ( "device", "X10 PC Remote" );
    return pMsg;
}


/***************************************************************************
****																	****
****	W800RF32::ProcessX10											****
****																	****
***************************************************************************/

xplMsg* W800RF32::ProcessX10
(
    uint8* _rf,
    string const& _msgSource
)
{
    // byte 1 must be the complement of byte 0,
    // and byte 3 the complement of byte 2.
    if ( ( _rf[0] ^ _rf[1] ) != 0xff )
    {
        // Data Invalid
        return NULL;
    }

    if ( ( _rf[2] ^ _rf[3] ) != 0xff )
    {
        // Data Invalid
        return NULL;
    }

    // Data Bytes need to be reversed
    uint8 ch1 = 0;
    uint8 ch2 = 0;
    uint8 rf1 = _rf[0];
    uint8 rf2 = _rf[2];

    for ( int i=0; i<8; ++i )
    {
        ch1 <<= 1;
        ch1 |= ( rf1 & 0x01 );
        rf1 >>= 1;

        ch2 <<= 1;
        ch2 |= ( rf2 & 0x01 );
        rf2 >>= 1;
    }

    uint8 houseCode = ch1 & 0x0f;

    // Bit 5 of ch1 is bit 3 of the UnitCode
    unsigned char unitCode = ( ch1 & 0x20 ) >>2;

    // Bit 1 of ch2 is bit 2 of the UnitCode
    unitCode |= ( ( ch2 & 0x02 ) <<1 );

    // Bit 4 of ch2 is bit 1 of the UnitCode
    unitCode |= ( ( ch2 & 0x10 ) >>3 );

    // Bit 3 of ch2 is bit 0 of the UnitCode
    unitCode |= ( ( ch2 & 0x08 ) >>3 );

    ++unitCode;	// Unit codes are 1-16, not 0-15.

    // Create an event from the input data
    xplMsg* pMsg = xplMsg::Create ( "xpl-trig", _msgSource, "*", "x10", "basic" );

    if ( ch2 == 0x11 )
    {
        // Bright Command
        pMsg->AddValue ( "command", "bright" );
        pMsg->AddValue ( "house", GetHouseCodeString ( houseCode ) );
        pMsg->AddValue ( "data1", "1" );
    }
    else if ( ch2 == 0x19 )
    {
        // Dim Command
        pMsg->AddValue ( "command", "dim" );
        pMsg->AddValue ( "house", GetHouseCodeString ( houseCode ) );
        pMsg->AddValue ( "data1", "1" );
    }
    else if ( ch2 == 0x09 )
    {
        // All Lights on
        pMsg->AddValue ( "command", "all_lights_on" );
        pMsg->AddValue ( "house", GetHouseCodeString ( houseCode ) );
    }
    else if ( ch2 == 0x01 )
    {
        // All Lights off
        pMsg->AddValue ( "command", "all_lights_off" );
        pMsg->AddValue ( "house", GetHouseCodeString ( houseCode ) );
    }
    else
    {
        char unitCodeStr[8];
        sprintf ( unitCodeStr, "%d", ( int ) unitCode );

        // On or Off
        if ( ch2 & 0x04 )
        {
            // Off
            pMsg->AddValue ( "command", "off" );
        }
        else
        {
            // On
            pMsg->AddValue ( "command", "on" );
        }

        pMsg->AddValue ( "device", GetHouseCodeString ( houseCode ) + unitCodeStr );
    }

    return pMsg;
}


