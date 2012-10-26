/***************************************************************************
****																	****
****	ConsoleApp.cpp  									            ****
****																	****
****	Example code demonstrating how to build an xPL Windows Service. ****
****																	****
****	Copyright (c) 2007 Mal Lansell.									****
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

#include "xplDevice.h"
#include "xplUDP.h"
#include "xplConfigItem.h"
#include "xplMsg.h"
#include "EventLog.h"

using namespace xpl;

// Set the config version here.  Each time a new version of the application
// is to be released, increment the version number.  This ensures that the
// new version will ignore any config items saved in the registry by
// previous versions, and instead enters config mode.
string const c_version = "1.1.0";

// The app name must be exactly the same as the compiled executable
// (but without the .exe), or the event logging will not work.
string const c_appName = "ConsoleApp";

void HandleMessages ( xplDevice* _pDevice );
void Configure ( xplDevice* _pDevice );


/***************************************************************************
****																	****
****	main															****
****																	****
***************************************************************************/

int main ( int argc, char* argv[] )
{
    // ************************
    // Init
    // ************************

    // Create the EventLog
    xpl::EventLog::Create ( c_appName );

    // Create the xPL communications object
    xplComms* pComms = xplUDP::Create ( true );
    if ( NULL == pComms )
    {
        goto exit;
    }

    // Create the xPL Device
    // Change "vendor" to your own vendor ID.  Vendor IDs can be no more than 8 characters
    // in length.  Post a message to the xPL Yahoo Group (http://groups.yahoo.com/group/ukha_xpl)
    // to request a vendor ID.
    // Replace "device" with a suitable device name for your application - usually something
    // related to what is being xPL-enabled.  Device names can be no more than 8 characters.
    xplDevice* pDevice = xplDevice::Create ( "vendor", "device", c_version, true, true, pComms );
    if ( NULL == pDevice )
    {
        goto exit;
    }

    // Create any additional config items
    // As an example, the following code creates an item to hold the index
    // of a com port.  The value can be changed by the user in xPLHal.
    xplConfigItem* pItem = new xplConfigItem ( "comport", "reconf" );
    if ( NULL != pItem )
    {
        // Default the com port to COM1
        pItem->AddValue ( "1" );
        pDevice->AddConfigItem ( pItem );
    }

    // Get the message and config events
    // The xplMsgEvent is signalled when an xPL message is received by the xplDevice.
    // The configEvent is signalled when the config items have been changed.
    HANDLE xplMsgEvent = pDevice->GetMsgEvent();
    HANDLE configEvent = pDevice->GetConfigEvent();

    // Init the xplDevice
    // Note that all config items must have been set up before Init() is called.
    pDevice->Init();


    // ************************
    // Main loop
    // ************************

    while ( 1 )
    {
        // Wait for an event to occur.
        // If possible, design your application so that an event is signalled when
        // something needs to be taken care of.  That way, the application can
        // sit here not taking any CPU time, until there is work to do.
        // Add your own events to the handles array.  Don't forget to change the
        // number of events in the WaitForMultipleObjects call.
        HANDLE handles[2];
        handles[0] = xplMsgEvent;
        handles[1] = configEvent;

        DWORD res = WaitForMultipleObjects ( 2, handles, FALSE, INFINITE );

        // Deal with any received xPL messages
        if ( WAIT_OBJECT_0 == res )
        {
            HandleMessages ( pDevice );
        }

        // Is configuration required?
        if ( ( WAIT_OBJECT_0 + 1 ) == res )
        {
            // Reset the configEvent before doing the configuration.
            // This ensures that if a config update is received by the
            // xPLDevice thread while we're in Configure(), it will
            // not be missed.
            ResetEvent ( configEvent );
            Configure ( pDevice );
        }

        // Deal with any App specific stuff here
        // If we were waiting on another event, check it in the same way
        // that we did for the xplMsgEvent and configEvent above.
        //
        // if( ( WAIT_OBJECT_0 + 2 ) == res )
        // {
        // }
        //
    }

exit:
    // ************************
    // Clean up and exit
    // ************************

    // Destroy the xPL Device
    // This also deletes any config items we may have added
    if ( pDevice )
    {
        xplDevice::Destroy ( pDevice );
        pDevice = NULL;
    }

    // Destroy the comms object
    if ( pComms )
    {
        xplUDP::Destroy ( pComms );
        pComms = NULL;
    }

    // Destroy the event log
    if ( EventLog::Get() )
    {
        EventLog::Destroy();
    }

    return 0;
}


/***************************************************************************
****																	****
****	HandleMessages  							            		****
****																	****
***************************************************************************/

void HandleMessages ( xplDevice* _pDevice )
{
    xplMsg* pMsg = NULL;

    // Get each queued message in turn
    while ( pMsg = _pDevice->GetMsg() )
    {
        // Process the message here.
        // ...

        // Done with it.
        pMsg->Release();
    }
}


/***************************************************************************
****																	****
****	Configure   										            ****
****																	****
***************************************************************************/

void Configure ( xplDevice* _pDevice )
{
    // Examine each of our config items in turn, and if they have changed,
    // take the appropriate action.

    // As an example, the Com Port index.
    //
    //  xplConfigItem const* pItem = _pDevice->GetConfigItem( "comport" );
    //  string value = pItem->GetValue();
    //  if( !value.empty() )
    //  {
    //      int comPort = atol( value.c_str() );
    //
    //      Deal with any change to the Com port number
    //      ...
    //  }
    //
}


