/***************************************************************************
****																	****
****	xplDevice.cpp													****
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

// #include <Assert.h> - not on linux
// #include <tchar.h> - not on linux
#include "xplDevice.h"
#include "xplCore.h"
#include "xplStringUtils.h"
#include "xplComms.h"
#include "xplMsg.h"
#include "xplFilter.h"
#include "xplConfigItem.h"
#include <strings.h>

using namespace xpl;

string const xplDevice::c_xplGroup = "xpl-group";

uint32 const xplDevice::c_rapidHeartbeatFastInterval = 3;	// Three seconds for the first
uint32 const xplDevice::c_rapidHeartbeatTimeout = 120;		// two minutes, after which the rate drops to
uint32 const xplDevice::c_rapidHeartbeatSlowInterval = 30;	// once every thirty seconds.


/***************************************************************************
****																	****
****	xplDevice::Create												****
****																	****
***************************************************************************/

// xplDevice* xplDevice::Create
// (
// 	string const& _vendorId,
// 	string const& _deviceId,
// 	string const& _version,
// 	bool const _configInRegistry,
// 	bool const _bFilterMsgs,
// 	xplComms* _pComms
// )
// {
// 	if( NULL == _pComms )
// 	{
// 		assert(0);
// 		return NULL;
// 	}
//
// 	// Create a new xplDevice
// 	xplDevice* pDevice = new xplDevice( _vendorId, _deviceId, _version, _configInRegistry, _bFilterMsgs, _pComms );
//
// 	// Add the configurable items
// 	xplConfigItem* pItem;
//
// 	pItem = new xplConfigItem( "newconf", "reconf" );
// 	pItem->AddValue( "default" );
// 	pDevice->AddConfigItem( pItem );
//
// 	pItem = new xplConfigItem( "interval", "reconf" );
// 	pItem->AddValue( "5" );
// 	pDevice->AddConfigItem( pItem );
//
// 	pItem = new xplConfigItem( "group", "option", 16 );
// 	pDevice->AddConfigItem( pItem );
//
// 	pItem = new xplConfigItem( "filter", "option", 16 );
// 	pDevice->AddConfigItem( pItem );
//
// 	return( pDevice );
// }


/***************************************************************************
****																	****
****	xplDevice::Destroy												****
****																	****
***************************************************************************/
/*
void xplDevice::Destroy
(
	xplDevice* _pDevice
)
{
	if( NULL == _pDevice )
	{
    cout << "device already destroyed\n";
		assert(0);
		return;
	}

// 	cout << "deiniting device in thread " << Thread::currentTid() << " \n";
	_pDevice->Deinit();
//   cout << "deleting device in thread " << Thread::currentTid() << " \n";
	delete _pDevice;
//   cout << "deleted device in thread " << Thread::currentTid() << " \n";
}*/


/***************************************************************************
****																	****
****	xplDevice constructor											****
****																	****
***************************************************************************/

xplDevice::xplDevice
(
    string const& _vendorId,
    string const& _deviceId,
    string const& _version,
    bool const _bConfigInRegistry,
    bool const _bFilterMsgs,
    xplComms* _pComms
) :
    m_version ( _version ),
    m_bConfigInRegistry ( _bConfigInRegistry ),
    m_bFilterMsgs ( _bFilterMsgs ),
    m_pComms ( _pComms ),
    m_bConfigRequired ( true ),
    m_bInitialised ( false ),
    m_heartbeatInterval ( 5 ),
    m_nextHeartbeat ( 0 ),
    m_bExitThread ( false ),

    m_bWaitingForHub ( true ),
    m_rapidHeartbeatCounter ( c_rapidHeartbeatTimeout/c_rapidHeartbeatFastInterval ),
    devLog ( Logger::get ( "xplsdk.device" ) )

{
    assert ( m_pComms );

    m_vendorId = StringToLower ( _vendorId );
    m_deviceId = StringToLower ( _deviceId );
    m_instanceId = "default";

    devLog.setLevel (Message::PRIO_TRACE  );
    
    SetCompleteId();

    m_hRxInterrupt = new Poco::Event ( false );

}


/***************************************************************************
****																	****
****	xplDevice::~xplDevice											****
****																	****
***************************************************************************/

xplDevice::~xplDevice ( void )
{
//     cout << "destroying xpldevice\n";
    if ( m_bInitialised )
    {
        m_bExitThread = true;
        //cout << "trying to trigger exit of hbeat thread with m_hRxInterrupt: " << m_hRxInterrupt << "\n";
        m_hRxInterrupt->set();

        // Wait for the thread to exit
        m_hThread.join();
        //cout << "joined with hbeat thread\n";

        //Delete the filters
        uint32 i;
        for ( i=0; i<m_filters.size(); ++i )
        {
            delete m_filters[i];
        }

        m_bInitialised = false;
    }

    //Delete the config items
    for ( int i=0; i<m_configItems.size(); ++i )
    {
        delete m_configItems[i];
    }
    delete m_hRxInterrupt ;
}


/***************************************************************************
****																	****
****	xplDevice::Init													****
****																	****
***************************************************************************/

bool xplDevice::Init()
{
    if ( m_bInitialised )
    {
        // Already initialised
        assert ( 0 );
        return false;
    }
    // Restore any saved configuration
    m_bConfigRequired = true;
    LoadConfig();

    m_bInitialised = true;
    m_bExitThread = false;

    // Create the thread that will handle heartbeats
    m_hThread.start ( *this );

    //register to get all the rxed messages from the comms
    m_pComms->rxNotificationCenter.addObserver ( Observer<xplDevice, MessageRxNotification> ( *this,&xplDevice::HandleRx ) );

    return true;
}



/***************************************************************************
****																	****
****	xplDevice::LoadConfig											****
****																	****
***************************************************************************/

void xplDevice::LoadConfig()
{
// 	m_bConfigRequired = true;
//
// 	if( m_bConfigInRegistry )
// 	{
// 		// Read from the registry
// 		char subKey[256];
// #ifdef UNICODE
// 		UnicodeString vendor( GetVendorId().c_str() );
// 		UnicodeString device( GetDeviceId().c_str() );
// #else
// 		string vendor = GetVendorId();
// 		string device = GetDeviceId();
// #endif
// 		_stprintf( subKey, TEXT("SOFTWARE\\xPL\\%s\\%s"), vendor.c_str(), device.c_str() );
// 		HKEY hKey;
// 		LONG res = RegOpenKeyEx( HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey );
//
// 		if( ERROR_SUCCESS == res )
// 		{
// 			// Read the version number
// 			 bufferSize;
//     		if( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("Version"), NULL, NULL, NULL, &bufferSize ) )
// 			{
// 				// Read the registry data into a buffer
// 				int8* pBuffer = new int8[bufferSize];
// 				 type;
// 				if( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("Version"), NULL, &type, (unsigned char*)pBuffer, &bufferSize ) )
// 				{
// #ifdef UNICODE
// 					int8* pNewBuffer = new int8[bufferSize];
// 					WideCharToMultiByte( CP_ACP, 0, (WCHAR*)pBuffer, -1, pNewBuffer, bufferSize, NULL, NULL );
// 					delete [] pBuffer;
// 					pBuffer = pNewBuffer;
// #endif
// 					if( ( REG_SZ == type ) && ( !stricmp( pBuffer, m_version.c_str() ) ) )
// 					{
// 						// Version number matches, so we don't need to go into configuration mode
// 						m_bConfigRequired = false;
// 					}
// 				}
// 			}
//
// 			// Read the config values from the key
// 			for( uint32 i=0; i<m_configItems.size(); ++i )
// 			{
// 				xplConfigItem* pItem = m_configItems[i];
// 				pItem->RegistryLoad( hKey );
// 			}
//
// 			// Close the key
// 			RegCloseKey( hKey );
// 		}
// 	}
// 	else
// 	{
// 		Read from a file
// 		string configFilename = GetVendorId();
// 		configFilename += "_";
// 		configFilename += GetDeviceId();
// 		configFilename += ".cfg";
//
// #ifdef UNICODE
// 		UnicodeString unicodeFilename = UnicodeString( configFilename.c_str() );
// 		HANDLE hFile = CreateFile( unicodeFilename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0 );
// #else
// 		//HANDLE hFile = CreateFile( configFilename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0 );
//     Poco::File hFile = File(configFilename.c_str());
//     if (!hFile.exists()){
//         hFile.createFile();
//     }
// #endif
//
// 		//if( INVALID_HANDLE_VALUE != hFile )
// 		if (hFile.canWrite());
// 		{
// 			// Read the version number
// 			 int bufferSize = 0;
// 			 int bytesRead;
// 			//ReadFile( hFile, &bufferSize, sizeof(bufferSize), &bytesRead, NULL );
//        ifstream myhfile;
//        myhfile.open(hFile.path().c_str());
//        myhfile.read( (char*)(&bufferSize), sizeof(bufferSize));
//         //TODO check read size
//
// 			int8* pBuffer = new int8[bufferSize];
// 			//ReadFile( hFile, pBuffer, bufferSize, &bytesRead, NULL );
//       myhfile.read( pBuffer, bufferSize);
//
// 			//if( !stricmp( pBuffer, m_version.c_str() ) )
//       if( !strcasecmp( pBuffer, m_version.c_str() ) )
// 			{
// 				// Version number matches, so we don't need to go into configuration mode
// 				m_bConfigRequired = false;
// 			}
//
// 			// Read the config items
// 			for( uint32 i=0; i<m_configItems.size(); ++i )
// 			{
// 				xplConfigItem* pItem = m_configItems[i];
//         pItem->FileLoad( myhfile );
// 			}
//
// 			//CloseHandle( hFile );
//       myhfile.close();
// // 		}
// 	}
//
// 	// If the config data was read ok, then configure this device.
// 	if( !m_bConfigRequired )
// 	{
//         // Configure the xplDevice
//         Configure();
// 		m_bConfigRequired = false;
//
//         // Set the config event
// 		//SetEvent( m_hConfig );
//     m_hConfig.notifyAsync(NULL, m_hConfig);
// 	}
}


/***************************************************************************
****																	****
****	xplDevice::SaveConfig											****
****																	****
***************************************************************************/

void xplDevice::SaveConfig() const
{
// 	if( m_bConfigInRegistry )
// 	{
// 		// Attempt to open the registry key for this
// 		// device, or create it if it doesn't yet exist.
// 		_TCHAR subKey[256];
// #ifdef UNICODE
// 		UnicodeString vendor( GetVendorId().c_str() );
// 		UnicodeString device( GetDeviceId().c_str() );
// #else
// 		string vendor = GetVendorId();
// 		string device = GetDeviceId();
// #endif
// 		_stprintf( subKey, TEXT("SOFTWARE\\xPL\\%s\\%s"), vendor.c_str(), device.c_str() );
//
// 		HKEY hKey;
// 		 disposition;
// 		LONG res = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
// 								   subKey,
// 								   0,
// 								   NULL,
// 								   REG_OPTION_NON_VOLATILE,
// 								   KEY_WRITE,
// 								   NULL,
// 								   &hKey,
// 								   &disposition );
//
// 		if( ERROR_SUCCESS == res )
// 		{
// 			// Store the version number
// 			 length = ()(m_version.size() + 1);
// #ifdef UNICODE
// 			UnicodeString version( m_version.c_str() );
// 			length <<= 1;
// #else
// 			string version = m_version;
// #endif
// 			RegSetValueEx( hKey, TEXT("Version"), 0, REG_SZ, (uint8*)version.c_str(), length );
//
// 			// Store the config values in the key
// 			for( uint32 i=0; i<m_configItems.size(); ++i )
// 			{
// 				xplConfigItem* pItem = m_configItems[i];
// 				pItem->RegistrySave( hKey );
// 			}
//
// 			// Close the key
// 			RegCloseKey( hKey );
// 		}
// 	}
// 	else
// 	{
// 		// Save to a file
// 		string configFilename = GetVendorId();
// 		configFilename += "_";
// 		configFilename += GetDeviceId();
// 		configFilename += ".cfg";
//
// #ifdef UNICODE
// 		UnicodeString unicodeFilename = UnicodeString( configFilename.c_str() );
// 		HANDLE hFile = CreateFile( unicodeFilename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0 );
// #else
// 		HANDLE hFile = CreateFile( configFilename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0 );
// #endif
//
// 		if( INVALID_HANDLE_VALUE != hFile )
// 		{
// 			// Write the version number
// 			 bufferSize = ()(m_version.size() + 1);
// 			 bytesWritten;
// 			WriteFile( hFile, &bufferSize, sizeof(bufferSize), &bytesWritten, NULL );
// 			WriteFile( hFile, m_version.c_str(), bufferSize, &bytesWritten, NULL );
//
// 			for( uint32 i=0; i<m_configItems.size(); ++i )
// 			{
// 				xplConfigItem* pItem = m_configItems[i];
// 				pItem->FileSave( hFile );
// 			}
//
// 			CloseHandle( hFile );
// 		}
// 	}
}


/***************************************************************************
****																	****
****	xplDevice::Configure											****
****																	****
***************************************************************************/

void xplDevice::Configure()
{
    // Set the device instance
    xplConfigItem const* pItem;

    pItem = GetConfigItem ( "newconf" );
    if ( pItem )
    {
        m_instanceId = pItem->GetValue();
        SetCompleteId();
    }

    // Set the heartbeat interval.  It must be between 5
    // and 9, otherwise the closest valid interval is used instead.
    pItem = GetConfigItem ( "interval" );
    if ( pItem )
    {
        m_heartbeatInterval = atol ( pItem->GetValue().c_str() );
        if ( m_heartbeatInterval < 5 )
        {
            m_heartbeatInterval = 5;
        }
        else if ( m_heartbeatInterval > 30 )
        {
            m_heartbeatInterval = 30;
        }
    }

    // Note: Groups are simple strings, and are
    // read from the config items as needed

    // Remove any old filters
    uint32 i;
    for ( i=0; i<m_filters.size(); ++i )
    {
        delete m_filters[i];
    }
    m_filters.clear();

    // Create the new filters
    pItem = GetConfigItem ( "filter" );
    if ( pItem )
    {
        for ( i=0; i<pItem->GetNumValues(); ++i )
        {
            xplFilter* pFilter = new xplFilter ( pItem->GetValue ( i ) );
            m_filters.push_back ( pFilter );
        }
    }
}


/***************************************************************************
****																	****
****	xplDevice::AddConfigItem										****
****																	****
***************************************************************************/

bool xplDevice::AddConfigItem
(
    xplConfigItem* _pItem
)
{
    // Config items may only be added before xplDevice::Init() is called
    if ( m_bInitialised )
    {
        assert ( 0 );
        return false;
    }

    // Make sure the item does not already exist
    if ( NULL != GetConfigItem ( _pItem->GetName() ) )
    {
        // Item exists
        assert ( 0 );
        return false;
    }

    // Add the item to the list
    m_configItems.push_back ( _pItem );
    return true;
}


/***************************************************************************
****																	****
****	xplDevice::RemoveConfigItem										****
****																	****
***************************************************************************/

bool xplDevice::RemoveConfigItem
(
    string const& _name
)
{
    for ( vector<xplConfigItem*>::iterator iter = m_configItems.begin(); iter != m_configItems.end(); ++iter )
    {
        if ( ( *iter )->GetName() == _name )
        {
            // Found
            delete *iter;
            m_configItems.erase ( iter );
            return true;
        }
    }

    // Item not found
    return false;
}


/***************************************************************************
****																	****
****	xplDevice::GetConfigItem										****
****																	****
***************************************************************************/

xplConfigItem const* xplDevice::GetConfigItem
(
    string const& _name
) const
{
    for ( vector<xplConfigItem*>::const_iterator iter = m_configItems.begin(); iter != m_configItems.end(); ++iter )
    {
        if ( ( *iter )->GetName() == _name )
        {
            return ( *iter );
        }
    }

    // Item not found
    return ( NULL );
}


// void xplDevice::addRXObserver ( Observer(C& object, Callback method) arg1) {
//     rxTaskManager.addObserver(arg1);
// }

// void xplDevice::addDeviceConfigObserver ( Observer< typename tname,  typename notname >arg1 ) {
//     configTaskManager.addObserver(arg1);
// }



/***************************************************************************
****																	****
****	xplDevice::IsMsgForThisApp										****
****																	****
***************************************************************************/

bool xplDevice::IsMsgForThisApp
(
    xplMsg* _pMsg
)
{
    // Reject any messages that were originally broadcast by us
    if ( _pMsg->GetSource().toString() == m_completeId )
    {
        // If we're waiting for a hub, then receiving a
        // reflected message (which will be our heartbeat)
        // means it is up and running.
        if ( m_bWaitingForHub )
        {
            m_bWaitingForHub = false;
            SetNextHeartbeatTime();
        }

        return false;
    }

    // Check the target.
    string const& target = _pMsg->GetTarget().toString();

    // Is the message for all devices
    if ( target != string ( "*" ) )
    {
        // Is the message for this device
        if ( target != m_completeId )
        {
            // Is the message for a group?
            string targetVendorDevice;
            string group;
            StringSplit ( target, '.', &targetVendorDevice, &group );
            if ( targetVendorDevice != c_xplGroup )
            {
                // Target is not a group either, so stop now
                return false;
            }

            // Target is a group - but does this device belong to it?
            xplConfigItem const* pItem = GetConfigItem ( "group" );
            if ( NULL == pItem )
            {
                // No groups item
                return false;
            }

            uint32 i;
            for ( i=0; i<pItem->GetNumValues(); ++i )
            {
                if ( group == pItem->GetValue ( i ) )
                {
                    break;
                }
            }

            if ( i == pItem->GetNumValues() )
            {
                // Target did not match any of the groups
                return false;
            }
        }
    }

    // Apply the filters
    uint32 i;
    for ( i=0; i<m_filters.size(); ++i )
    {
        if ( m_filters[i]->Allow ( *_pMsg ) )
        {
            break;
        }
    }

    // If there are filters, and we haven't found one that passes the message, stop here.
    if ( i && ( i==m_filters.size() ) )
    {
        return false;
    }

    // Message passed validation
    return true;
}


/***************************************************************************
****																	****
****	xplDevice::SetNextHeartbeatTime									****
****																	****
***************************************************************************/

void xplDevice::SetNextHeartbeatTime()
{
    // Set the new heartbeat time
    int64_t currentTime;
    Poco::Timestamp tst;
    tst.update();
    currentTime = tst.epochMicroseconds();
    //GetSystemTimeAsFileTime( (FILETIME*)&currentTime );

    // If we're waiting for a hub, we have to send at more
    // rapid intervals - every 3 seconds for the first two
    // minutes, then once every 30 seconds after that.
    if ( m_bWaitingForHub )
    {
        if ( m_rapidHeartbeatCounter )
        {
            // This counter starts at 40 for 2 minutes of
            // heartbeats at 3 second intervals.
            --m_rapidHeartbeatCounter;


            m_nextHeartbeat = currentTime + ( ( int64_t ) c_rapidHeartbeatFastInterval * 1000l );
        }
        else
        {
            // one second
            m_nextHeartbeat = currentTime + ( ( int64_t ) c_rapidHeartbeatSlowInterval * 1000l );
        }
    }
    else
    {
        // It is time to send a heartbeat
        if ( m_bConfigRequired )
        {
            // one minute
            m_nextHeartbeat = currentTime + 60*1000l;
        }
        else
        {
            // 60000000 is one minute in the 100 nanosecond intervals
            // that the system time is measured in.
            m_nextHeartbeat = currentTime + ( ( int64_t ) m_heartbeatInterval * 60*1000l );
        }
    }
}


/***************************************************************************
****																	****
****	xplDevice::HandleMsgForUs											****
****																	****
***************************************************************************/

bool xplDevice::HandleMsgForUs
(
    xplMsg* _pMsg
)
{
    if ( _pMsg->GetType() == xplMsg::c_xplCmnd )
    {
        if ( "config" == _pMsg->GetSchemaClass() )
        {
            if ( "current" == _pMsg->GetSchemaType() )
            {
                // Config values request
                if ( "request" == StringToLower ( _pMsg->GetValue ( "command" ) ) )
                {
                    SendConfigCurrent();
                    return true;
                }
            }
            else if ( "list" == _pMsg->GetSchemaType() )
            {
                // Config list request
                if ( string ( "request" ) == StringToLower ( _pMsg->GetValue ( "command" ) ) )
                {
                    SendConfigList();
                    return true;
                }
            }
            else if ( "response" == _pMsg->GetSchemaType() )
            {
                uint32 i;
                for ( i=0; i<m_configItems.size(); ++i )
                {
                    // Clear the existing config item values
                    xplConfigItem* pItem = m_configItems[i];
                    pItem->ClearValues();

                    // Copy all the new values from the message into the config item
                    xplMsgItem const* pValues = _pMsg->GetMsgItem ( pItem->GetName() );
                    if ( NULL != pValues )
                    {
                        for ( uint32 j=0; j<pValues->GetNumValues(); ++j )
                        {
                            string value = pValues->GetValue ( j );
                            if ( !value.empty() )
                            {
                                pItem->AddValue ( value );
                            }
                        }
                    }
                }

                // Configure the xplDevice
                Configure();

                // Save configuration
                SaveConfig();
                m_bConfigRequired = false;

                // Set the config event
                //SetEvent( m_hConfig );
//         m_hConfig->set();
                //configTaskManager.start(new SampleTask("ConfigTask"));
                cout << "posted reconfig from thread " << Thread::currentTid()  << "\n";
                configNotificationCenter.postNotification ( new DeviceConfigNotification() );
                cout << "posted reconfig from thread " << Thread::currentTid()  << "\n";

                // Send a heartbeat so everyone gets our latest status
                m_pComms->SendHeartbeat ( m_completeId, m_heartbeatInterval, m_version );
                SetNextHeartbeatTime();
                return true;
            }
        }
        else if ( "hbeat" == _pMsg->GetSchemaClass() )
        {
            if ( "request" == _pMsg->GetSchemaType() )
            {
                // We've been asked to send a heartbeat
                if ( m_bConfigRequired )
                {
                    // Send a config heartbeat
                    m_pComms->SendConfigHeartbeat ( m_completeId, m_heartbeatInterval, m_version );
                }
                else
                {
                    // Send a heartbeat
                    m_pComms->SendHeartbeat ( m_completeId, m_heartbeatInterval, m_version );
                }

                // Calculate the time of the next heartbeat
                SetNextHeartbeatTime();
            }
        }
    }

    return false;
}


/***************************************************************************
****																	****
****	xplDevice::SendConfigList										****
****																	****
****																	****
****	xpl-stat														****
****	{																****
****	hop=1															****
****	source=[VENDOR]-[DEVICE].[INSTANCE]								****
****	target=*														****
****	}																****
****	config.list														****
****	{																****
****	reconf=newconf													****
****	option=interval													****
****	option=group[16]												****
****	option=filter[16]												****
****	}																****
****																	****
****																	****
***************************************************************************/

void xplDevice::SendConfigList() const
{
    AutoPtr<xplMsg> pMsg = new xplMsg ( xplMsg::c_xplStat, m_completeId, "*", "config", "list" );

    if ( pMsg )
    {
        for ( uint32 i=0; i<m_configItems.size(); ++i )
        {
            xplConfigItem* pItem = m_configItems[i];
            if ( pItem->GetMaxValues() > 1 )
            {
                // Use the array notation if there can be more than one value assigned
                int8 value[128];
                sprintf ( value, "%s[%d]", pItem->GetName().c_str(), pItem->GetMaxValues() );
                pMsg->AddValue ( pItem->GetType(), value );
            }
            else
            {
                pMsg->AddValue ( pItem->GetType(), pItem->GetName() );
            }
        }

        // Call xplComms::TxMsg directly, since we may be in config mode
        // and xplDevice::SendMessage would block it.
        m_pComms->TxMsg ( *pMsg );

    }
}


/***************************************************************************
****																	****
****	xplDevice::SendConfigCurrent									****
****																	****
****																	****
****	xpl-stat														****
****	{																****
****	hop=1															****
****	source=[VENDOR]-[DEVICE].[INSTANCE]								****
****	target=*														****
****	}																****
****	config.current													****
****	{																****
****	item1=value1													****
****	item2=value2													****
****	..																****
****	itemN=valueN													****
****	}																****
****																	****
****																	****
***************************************************************************/

void xplDevice::SendConfigCurrent() const
{
    AutoPtr<xplMsg> pMsg = new xplMsg ( xplMsg::c_xplStat, m_completeId, "*", "config", "current" );

    if ( pMsg )
    {
        for ( uint32 i=0; i<m_configItems.size(); ++i )
        {
            xplConfigItem* pItem = m_configItems[i];
            for ( uint32 j=0; j<pItem->GetNumValues(); ++j )
            {
                pMsg->AddValue ( pItem->GetName(), pItem->GetValue ( j ) );
            }
        }

        // Call xplComms::TxMsg directly, since we may be in config mode
        // and xplDevice::SendMessage would block it.
        m_pComms->TxMsg ( *pMsg );

    }
}


/***************************************************************************
****																	****
****	xplDevice::SendMsg												****
****																	****
***************************************************************************/

bool xplDevice::SendMsg
(
    xplMsg* _pMsg
)
{
    // Code outside of xplDevice cannot send messages until
    // the application has been configured.
    if ( m_bConfigRequired )
    {
        return false;
    }

    // Cannot send messages if we're paused
// 	if( m_bPaused )
// 	{
// 		return false;
// 	}

    // Queue the message.
    //EnterCriticalSection( &m_criticalSection );
// 	m_criticalSection.lock();
// 	_pMsg->AddRef();
// 	m_txBuffer.push_back( _pMsg );
// 	//SetEvent( m_hRxInterrupt );
//   m_hRxInterrupt->set();
// 	//LeaveCriticalSection( &m_criticalSection );
//   m_criticalSection.unlock();
    //I see no reason to wait to send it...
    m_pComms->TxMsg ( *_pMsg );

    return true;
}


// /***************************************************************************
// ****																	****
// ****	xplDevice::GetMsg												****
// ****																	****
// ***************************************************************************/
//
// xplMsg* xplDevice::GetMsg()
// {
// 	xplMsg* pMsg = NULL;
//
// 	// If there are any messages in the buffer, remove the first one
//     // and return it to the caller.
// 	// Access to the message buffer must be serialised
// 	//EnterCriticalSection( &m_criticalSection );
//   m_criticalSection.lock();
//
//     if( m_rxBuffer.size() )
// 	{
// 		pMsg = m_rxBuffer.front();
// 		m_rxBuffer.pop_front();
//
// 		if( 0 == m_rxBuffer.size() )
// 		{
// 			//No events left, so clear the signal
// 			//ResetEvent( m_hMsgRx );
//       m_hMsgRx->reset();
// 		}
// 	}
//
// 	//LeaveCriticalSection( &m_criticalSection );
// 	m_criticalSection.unlock();
// 	return( pMsg );
// }


/***************************************************************************
****																	****
****	xplDevice::SetCompleteId										****
****																	****
***************************************************************************/

void xplDevice::SetCompleteId()
{
    m_completeId = StringToLower ( m_vendorId + string ( "-" ) + m_deviceId + string ( "." ) + m_instanceId );
}


/***************************************************************************
****																	****
****	xplDevice::Run													****
****																	****
***************************************************************************/

void xplDevice::run ( void )
{
    if ( !m_bInitialised )
    {
        assert ( 0 );
        return;
    }

    while ( !m_bExitThread )
    {
        // Deal with heartbeats
        int64_t currentTime;
        Poco::Timestamp tst;
        tst.update();
        currentTime = tst.epochMicroseconds();
        //GetSystemTimeAsFileTime( (FILETIME*)&currentTime );
        
        if ( m_nextHeartbeat <= currentTime )
        {
            poco_debug( commsLog, "Sending heartbeat" );
            // It is time to send a heartbeat
            if ( m_bConfigRequired )
            {
                // Send a config heartbeat, then calculate the time of the next one
                m_pComms->SendConfigHeartbeat ( m_completeId, m_heartbeatInterval, m_version );
            }
            else
            {
                // Send a heartbeat, then calculate the time of the next one
                m_pComms->SendHeartbeat ( m_completeId, m_heartbeatInterval, m_version );
            }

            SetNextHeartbeatTime();
        }
        // Calculate the time (in milliseconds) until the next heartbeat
        int32 heartbeatTimeout = ( int32 ) ( ( m_nextHeartbeat - currentTime ) ); // Divide by 10000 to convert 100 nanosecond intervals to milliseconds.
        poco_debug ( devLog, "Sleeping " + NumberFormatter::format ( heartbeatTimeout/1000 ) + " seconds till next hbeat" );
        m_hRxInterrupt->tryWait ( heartbeatTimeout );
        //Thread::sleep();
        poco_debug ( devLog, "Woken up for hbeat or interrupt" );

    }
// 	cout << "exiting dev thread (ret)\n";
    return;
}


void xplDevice::HandleRx ( MessageRxNotification* mNot )
{
//         cout << "device: start handle RX in thread " << Thread::currentTid() <<"\n";
    AutoPtr<xplMsg> pMsg = mNot->message;
//    Process any xpl message received
    if ( NULL != pMsg )
    {
        if ( ( !m_bFilterMsgs ) || IsMsgForThisApp ( pMsg ) )
        {
            // Call our own handler
            HandleMsgForUs ( pMsg );

            //EnterCriticalSection( &m_criticalSection );
            m_criticalSection.lock();
            //m_rxBuffer.push_back( pMsg );
            //pMsg->AddRef();

//             cout << "device: posting message from thread " << Thread::currentTid() << "\n";

            //increase the ref count before handing it off
            mNot->duplicate();
            rxNotificationCenter.postNotification ( mNot );
//             cout << "device: posted message from thread " << Thread::currentTid()  << "\n";
            //LeaveCriticalSection( &m_criticalSection );
            m_criticalSection.unlock();
        }

//         pMsg->Release();
    }
    mNot->release();
//     cout << "device: stop handle RX in thread " << Thread::currentTid() <<"\n";
}



/***************************************************************************
****																	****
****	DeviceThread													****
****																	****
***************************************************************************/

bool xplDevice::DeviceThread
(
    void* _lpArg
)
{
    xplDevice* pDevice = ( xplDevice* ) _lpArg;
    pDevice->run();
    return ( 0 );
}


