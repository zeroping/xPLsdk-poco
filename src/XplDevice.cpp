/***************************************************************************
****																	****
****	XplDevice.cpp													****
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
#include "XplDevice.h"
#include "XplCore.h"
#include "XplStringUtils.h"
#include "XplComms.h"
#include "XplMsg.h"
#include "xplFilter.h"
#include "XplConfigItem.h"
#include <../../src/heeks/skeleton/prim.h>

#include <strings.h>
#include <Poco/String.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/Path.h>
#include <Poco/SAX/SAXException.h>

using namespace xpl;
using namespace Poco;
using Poco::Util::PropertyFileConfiguration;
using Poco::Util::AbstractConfiguration;
using Poco::toLower;

string const XplDevice::c_xplGroup = "xpl-group";

uint32 const XplDevice::c_rapidHeartbeatFastInterval = 3;	// Three seconds for the first
uint32 const XplDevice::c_rapidHeartbeatTimeout = 120;		// two minutes, after which the rate drops to
uint32 const XplDevice::c_rapidHeartbeatSlowInterval = 30;	// once every thirty seconds.



/***************************************************************************
****																	****
****	XplDevice constructor											****
****																	****
***************************************************************************/

XplDevice::XplDevice
(
    string const& _vendorId,
    string const& _deviceId,
    string const& _version,
    bool const _bConfigInRegistry,
    bool const _bFilterMsgs,
    XplComms* _pComms
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
    devLog.setLevel ("trace");
//     Logger::setLevel("xplsdk.device", Message::PRIO_TRACE  );
    assert ( devLog.trace() );

    m_vendorId = toLower ( _vendorId );
    m_deviceId = toLower ( _deviceId );
    m_instanceId = "default";
    
    SetCompleteId();

    m_hRxInterrupt = new Poco::Event ( false );

}


/***************************************************************************
****																	****
****	XplDevice::~XplDevice											****
****																	****
***************************************************************************/

XplDevice::~XplDevice ( void )
{
     cout << "destroying XplDevice\n";
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
//     for ( int i=0; i<m_configItems.size(); ++i )
//     {
//         delete m_configItems[i];
//     }
    delete m_hRxInterrupt ;
}


/***************************************************************************
****																	****
****	XplDevice::Init													****
****																	****
***************************************************************************/

bool XplDevice::Init()
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
    m_pComms->rxNotificationCenter.addObserver ( Observer<XplDevice, MessageRxNotification> ( *this,&XplDevice::HandleRx ) );

    return true;
}


Poco::Path XplDevice::GetConfigFileLocation() {
    Poco::Path p ( Poco::Path::home() );
    p.pushDirectory ( ".xPL" );
    File test = File(p);
    if (!test.exists()){
        poco_debug ( devLog, "dir doesn't exist:  " + p.toString() );
        test.createDirectory();
    }
    p.pushDirectory ( "xPLSDK_configs" );
    test = File(p);
    if (!test.exists()){
        poco_debug ( devLog, "dir doesn't exist:  " + p.toString() );
        test.createDirectory();
    }
    p.setFileName ( GetCompleteId() + ".conf" );
    test = File(p);
    if (!test.exists()){
        poco_debug ( devLog, "file doesn't exist:  " + p.toString() );
        test.createFile();
    }
    return p;
}


/***************************************************************************
****																	****
****	XplDevice::LoadConfig											****
****																	****
***************************************************************************/

void XplDevice::LoadConfig()
{

    poco_trace ( devLog, "loading config for  "  + GetCompleteId() );
    
    Poco::Path p = GetConfigFileLocation();

    PropertyFileConfiguration* cfgp;
    try{
        cfgp =  new PropertyFileConfiguration(p.toString());
    } catch (Poco::FileException e) {
        poco_debug ( devLog, "Failed to parse  " + p.toString() );
        cfgp = (new PropertyFileConfiguration());
    }
    m_configStore = cfgp;
        
    m_bConfigRequired = true;
  
    
    AbstractConfiguration::Keys itemKeys;
    m_configStore->keys(itemKeys);
    for ( AbstractConfiguration::Keys::iterator iter = itemKeys.begin(); iter != itemKeys.end(); ++iter )
    {
        poco_debug ( devLog, " item: " + *iter );
    }
    
    
    
    if ( m_configStore->hasProperty ( "instanceId" ) )
    {
        //looks like we really have a config
        poco_debug ( devLog, "found instance ID" );
        m_bConfigRequired = false;
        SetInstanceId ( m_configStore->getString ( "instanceId" ) );
        
        if ( m_configStore->hasProperty ( "configItems" )) {
            AbstractConfiguration::Keys confItemKeys;
            m_configStore->keys("configItems", confItemKeys);
            poco_debug ( devLog, "found " + NumberFormatter::format(confItemKeys.size()) + "keys" );
            for ( AbstractConfiguration::Keys::iterator iter = confItemKeys.begin(); iter != confItemKeys.end(); ++iter )
            {
                if(m_configStore->hasProperty ( "configItems."+(*iter)+".numValues" )){
                    // we have a config item entry with the required parts, but should only restore it if it matches a programatically-added configitem.
                    AutoPtr<XplConfigItem> cfgItem = GetConfigItem(*iter);
                    if (cfgItem.get()== NULL){
                        poco_warning ( devLog, "Found a config item for name " + *iter + ", but no programatically-created config item exists with that name" );
                        continue;
                    }
                    poco_trace ( devLog, "Config item: " + *iter );
                    int numValues = m_configStore->getInt ( "configItems."+(*iter)+".numValues");
                    
                    for (int i= 0; i<numValues; i++) {
                        poco_trace( devLog, "Value " );
                        string valname = "configItems."+(*iter)+".value" + NumberFormatter::format(i);
                        if(m_configStore->hasProperty (valname) ){
                            cfgItem->AddValue(m_configStore->getString ( valname ));
                        }
                    }
                }
            }
            
        }
    }
  
// 	// If the config data was read ok, then configure this device.
	if( !m_bConfigRequired )
	{
    // Configure the XplDevice
    Configure();
    m_bConfigRequired = false;

        // Set the config event
		//SetEvent( m_hConfig );
//     m_hConfig.notifyAsync(NULL, m_hConfig);
	}
}


/***************************************************************************
****																	****
****	XplDevice::SaveConfig											****
****																	****
***************************************************************************/

void XplDevice::SaveConfig()
{
    
    
    Poco::Path p = GetConfigFileLocation();
    poco_debug ( devLog, "saving config for  " + GetCompleteId() + " to " + p.toString());

    m_configStore->setString("vendorId", GetVendorId());
    m_configStore->setString("deviceId", GetDeviceId());
    m_configStore->setString("instanceId", GetInstanceId());

    
    if(m_configItems.size()) {
        m_configStore->setInt("configItems",m_configItems.size());
        for ( vector<AutoPtr<XplConfigItem> >::iterator iter = m_configItems.begin(); iter != m_configItems.end(); ++iter )
        {
            poco_debug ( devLog, "saving config item  " + (*iter)->GetName());
            m_configStore->setString("configItems." + (*iter)->GetName(), ""  );
            m_configStore->setString("configItems." + (*iter)->GetName() + ".numValues" , NumberFormatter::format((*iter)->GetNumValues())  );
            
            for (int vindex=0; vindex<(*iter)->GetNumValues(); vindex++) {
                m_configStore->setString("configItems." + (*iter)->GetName() + ".value" + NumberFormatter::format(vindex) , (*iter)->GetValue(vindex)  );
            }
        }
    }
    
    
    m_configStore->save(p.toString());
    poco_debug ( devLog, "saved to " + p.toString());
    
}


/***************************************************************************
****																	****
****	XplDevice::Configure											****
****																	****
***************************************************************************/

void XplDevice::Configure()
{
    poco_debug ( devLog, "Configuring  " + GetCompleteId() );
    // Set the device instance
    XplConfigItem const* pItem;

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
****	XplDevice::AddConfigItem										****
****																	****
***************************************************************************/

bool XplDevice::AddConfigItem
(
    AutoPtr<XplConfigItem> _pItem
)
{
    poco_debug ( devLog, "Adding config item to "  + GetCompleteId() + ": " + _pItem->GetName() + " = " + _pItem->GetValue());
    
    // Config items may only be added before XplDevice::Init() is called
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
****	XplDevice::RemoveConfigItem										****
****																	****
***************************************************************************/

bool XplDevice::RemoveConfigItem
(
    string const& _name
)
{
    poco_debug ( devLog, "removing config item for "  + GetCompleteId() + ": " + _name );
    for ( vector< AutoPtr<XplConfigItem> >::iterator iter = m_configItems.begin(); iter != m_configItems.end(); ++iter )
    {
        if ( ( *iter )->GetName() == _name )
        {
            // Found
            m_configItems.erase ( iter );
            return true;
        }
    }
    // Item not found
    return false;
}


/***************************************************************************
****																	****
****	XplDevice::GetConfigItem										****
****																	****
***************************************************************************/

AutoPtr<XplConfigItem> XplDevice::GetConfigItem
(
    string const& _name
) const
{
    poco_debug ( devLog, "get config item for "  + GetCompleteId() + ": " + _name );
    for ( vector< AutoPtr<XplConfigItem> >::const_iterator iter = m_configItems.begin(); iter != m_configItems.end(); ++iter )
    {
        if ( ( *iter )->GetName() == _name )
        {
            return ( *iter );
        }
    }

    // Item not found
    return ( NULL );
}


// void XplDevice::addRXObserver ( Observer(C& object, Callback method) arg1) {
//     rxTaskManager.addObserver(arg1);
// }

// void XplDevice::addDeviceConfigObserver ( Observer< typename tname,  typename notname >arg1 ) {
//     configTaskManager.addObserver(arg1);
// }



/***************************************************************************
****																	****
****	XplDevice::IsMsgForThisApp										****
****																	****
***************************************************************************/

bool XplDevice::IsMsgForThisApp
(
    XplMsg* _pMsg
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
//         poco_debug ( devLog, "target compare " + target + " " + m_completeId );
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
            XplConfigItem const* pItem = GetConfigItem ( "group" );
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
****	XplDevice::SetNextHeartbeatTime									****
****																	****
***************************************************************************/

void XplDevice::SetNextHeartbeatTime()
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
****	XplDevice::HandleMsgForUs											****
****																	****
***************************************************************************/

bool XplDevice::HandleMsgForUs
(
    XplMsg* _pMsg
)
{
    if ( _pMsg->GetType() == XplMsg::c_xplCmnd )
    {
        if ( "config" == _pMsg->GetSchemaClass() )
        {
            poco_debug ( devLog, "config message");
            if ( "current" == _pMsg->GetSchemaType() )
            {
                // Config values request
                if ( "request" == toLower ( _pMsg->GetValue ( "command" ) ) )
                {
                    SendConfigCurrent();
                    return true;
                }
            }
            else if ( "list" == _pMsg->GetSchemaType() )
            {
                // Config list request
                if ( string ( "request" ) == toLower ( _pMsg->GetValue ( "command" ) ) )
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
                    XplConfigItem* pItem = m_configItems[i];
                    pItem->ClearValues();

                    // Copy all the new values from the message into the config item
                    XplMsgItem const* pValues = _pMsg->GetMsgItem ( pItem->GetName() );
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

                // Configure the XplDevice
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
****	XplDevice::SendConfigList										****
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

void XplDevice::SendConfigList() const
{
    AutoPtr<XplMsg> pMsg = new XplMsg ( XplMsg::c_xplStat, m_completeId, "*", "config", "list" );

    if ( pMsg )
    {
        for ( uint32 i=0; i<m_configItems.size(); ++i )
        {
            AutoPtr<XplConfigItem> pItem = m_configItems[i];
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

        // Call XplComms::TxMsg directly, since we may be in config mode
        // and XplDevice::SendMessage would block it.
        m_pComms->TxMsg ( *pMsg );

    }
}


/***************************************************************************
****																	****
****	XplDevice::SendConfigCurrent									****
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

void XplDevice::SendConfigCurrent() const
{
    AutoPtr<XplMsg> pMsg = new XplMsg ( XplMsg::c_xplStat, m_completeId, "*", "config", "current" );

    if ( pMsg )
    {
        for ( uint32 i=0; i<m_configItems.size(); ++i )
        {
            AutoPtr<XplConfigItem> pItem = m_configItems[i];
            for ( uint32 j=0; j<pItem->GetNumValues(); ++j )
            {
                pMsg->AddValue ( pItem->GetName(), pItem->GetValue ( j ) );
            }
        }

        // Call XplComms::TxMsg directly, since we may be in config mode
        // and XplDevice::SendMessage would block it.
        poco_debug ( devLog, "sending config list" );
        m_pComms->TxMsg ( *pMsg );

    }
}


/***************************************************************************
****																	****
****	XplDevice::SendMsg												****
****																	****
***************************************************************************/

bool XplDevice::SendMsg
(
    XplMsg* _pMsg
)
{
    // Code outside of XplDevice cannot send messages until
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
// ****	XplDevice::GetMsg												****
// ****																	****
// ***************************************************************************/
//
// XplMsg* XplDevice::GetMsg()
// {
// 	XplMsg* pMsg = NULL;
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
****	XplDevice::SetCompleteId										****
****																	****
***************************************************************************/

void XplDevice::SetCompleteId()
{
    m_completeId = toLower ( m_vendorId + string ( "-" ) + m_deviceId + string ( "." ) + m_instanceId );
}


/***************************************************************************
****																	****
****	XplDevice::Run													****
****																	****
***************************************************************************/

void XplDevice::run ( void )
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
            poco_debug( devLog, "Sending heartbeat" );
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


void XplDevice::HandleRx ( MessageRxNotification* mNot )
{
//         cout << "device: start handle RX in thread " << Thread::currentTid() <<"\n";
//     poco_trace ( devLog, "packet rx" );
    AutoPtr<XplMsg> pMsg = mNot->message;
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

bool XplDevice::DeviceThread
(
    void* _lpArg
)
{
    XplDevice* pDevice = ( XplDevice* ) _lpArg;
    pDevice->run();
    return ( 0 );
}


