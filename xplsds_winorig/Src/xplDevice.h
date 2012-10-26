/***************************************************************************
****																	****
****	xplDevice.h														****
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

#pragma once

#ifndef _xPLDevice_H
#define _xPLDevice_H

#include <winsock2.h>
#include <string>
#include <vector>
#include <deque>
#include "xplcore.h"

namespace xpl
{

class xplComms;
class xplMsg;
class xplFilter;
class xplConfigItem;

/**
 * Implements the core xPL functionality.
 * The xplDevice class provides all the functionality required to create
 * a basic xPL-enabled application.  It handles the sending and receiving
 * of xPL messages, sends heartbeats and supports comfiguration via xPLHal
 * All the main application has to do is to watch a pair of events.
 * The first can be obtained by calling xplDevice::GetMsgEvent, and is
 * signalled whenever an xPL message has been received, and is not a message
 * that the xPL Device deals with automatically.  The message can be obtained
 * by calling xplDevice::GetMsg.  The second event is used to indicate that
 * the config items have been changed, and is obtained by calling
 * xplDevice::GetConfigEvent.
 * <p>
 * The procedure for creating and initialising an xplDevice object is as
 * follows:
 * <p>
 * 1) Create a communications object for the xplDevice to use.  For LAN or
 * Internet communications, create an xplUDP object.
 * <p>
 * 2) Call xplDevice::Create to generate a new xplDevice object, passing in the
 * communications object created in step 1.
 * <p>
 * 3) Create any config items that your application requires, and use the
 * AddConfigItem method to add them to the xplDevice.
 * <p>
 * 4) Finish with a call to xplDevice::Init.  This will load any existing
 * configuration and signal the config event so your application can
 * do any initialisation that is based on its config item values.
 * <p>
 * That is all there is to it.  When you are finished, simply call
 * xplDevice::Destroy to delete the xplDevice object.
 * <p>
 * Note: xPL messages not related to configuration will be ignored until
 * the application leaves config mode (i.e. when the user has configured
 * it in xPLHal).  During this time your application will also be unable
 * to send any xPL Messages, and should hold off performing its main functions.
 * The method xplDevice::IsInConfigMode can be used to determine whether your
 * processing should go ahead.
 */
class xplDevice
{
public:
    /**
     * Create an xplDevice.
     * The xplDevice provides the all of the core xPL functionality for an application.
     * @param _vendorId your vendor ID.  A string containing the vendor name of the application author.
     * Vendor names are assigned on a first-come, first-served basis by the xPL project organsisers.
     * Vendor IDs are a maximum of 8 characters in length.  To request a vendor name, post a message to
     * the xPL Yahoo group http://groups.yahoo.com/groups/ukha_xpl.
     * @param _deviceId your device ID.  A string containing a name related to the device that
     * this application supports, usually the same as the application name.
     * Device IDs are a maximum of 8 characters in length.
     * @param _version version number of the application.  This should be in the form major.minor.revision,
     * for example - 4.1.12, and should match the version number used in the installer properties.
     * If the version does not match the value stored in the registry or config file, the application
     * will enter config mode, requiring the user to review any new values that have been added or
     * changed since the last revision.
     * @param _bConfigInRegistry where to store the application configuration.  If true, config items are
     * read from the registry.  If false, those values are taken from a configuration file instead.
     * @param _bFilterMsgs whether to apply filters and to examine the message target before adding it to
     * the received message queue.  This should normally be true unless the application needs to look at
     * messages intended for other applications.
     * @param _pComms communications object to use when sending/receiving xPL messages.  For normal
     * ethernet communications, create an xplUDP object and pass it in here.
     * @return A pointer to a new xplDevice.  If an error occured during creation, the method returns
     * NULL instead.
     * @see Destroy, xplComms, xplUDP
     */
    static xplDevice* Create ( string const& _vendorId, string const& _deviceId, string const& _version, bool const _bConfigInRegistry, bool const _bFilterMsgs, xplComms* _pComms );

    /**
     * Deletes the xplDevice and cleans up any associated objects.
     * @param _pDevice pointer to the xplDevice object to be destroyed.
     * @see Create
     */
    static void Destroy ( xplDevice* _pDevice );

    /**
     * Initialises the xplDevice.  Loads any existing configuration
     * data from the registry or a file, which will trigger the
     * registered config callback functions if successful.  It then
     * starts the thread that will handle all xPL Msg traffic.
     * <p>
     * All config items and callbacks must be set up before calling
     * this method.
     * @return False if the xplDevice has already been intialised,
     * without having since been deinitialised.  Otherwise it
     * returns true.
     * @see Deinit
     */
    bool Init();

    /**
     * Deinitialises the xplDevice
     * @return False if the xplDevice has not been initialised,
     * otherwise it returns true.
     * @see Init
     */
    bool Deinit();

    /**
     * Pauses the xplDevice.  This method is provided to support the
     * ability to suspend and resume operations when running as a service.
     */
    void Pause();

    /**
     * Unpauses the xplDevice.  This method is provided to support the
     * ability to suspend and resume operations when running as a service.
     */
    void Resume();

    /**
     * Tests whether the application is waiting for the hub to appear.
     * Without a hub, xPL communications cannot take place.  This often
     * happens at start-up, when some services may start before the hub.
     * During this time, the heatbeats are sent out at 10 second
     * intervals instead of the usual rate, otherwise a delay of several
     * minutes might occur before the hub receives the next heartbeat
     * and starts to send xPL traffic to our application.
     * @return True if the application is waiting for a hub to appear,
     * false otherwise.
     */
    bool IsWaitingForHub() const
    {
        return m_bWaitingForHub;
    }

    /**
     * Tests whether the application is in config mode.
     * In config mode, the application needs to be configured via xPLHal before
     * normal operation can begin.  While in this mode, attempts to call
     * SendMsg will fail, and the registered message callbacks will not
     * be triggered.  This method is provided so that the application can
     * check the config status to avoid sending messages in vain.
     * @return True if the application is in config mode, false otherwise.
     */
    bool IsInConfigMode() const
    {
        return m_bConfigRequired;
    }

    /**
     * Sends an xPL message.
     * This method will fail if the application is in config mode (that
     * is, waiting to be set up in xPLHal).  The message is added to a
     * queue, and will be sent as soon as the device thread is able.
     * @param _pMsg xplMsg object containing the xPL message to be sent
     * @return True if the message was queued successfully.
     * @see xplMsg
     */
    bool SendMsg ( xplMsg* _pMsg );

    /**
     * Gets the next xPL message from the queue of received messages.
     * This method should be called when the event returned by GetMsgEvent
     * is in a signalled state.  The event will only become unsignalled
     * when no more messages are waiting.
     * <p>
     * When you are finished with the message, it should be destroyed by
     * calling xplMsg::Release.
     * @return A pointer to an xPL message, or NULL if no messages were
     * waiting.
     */
    xplMsg* GetMsg();

    /**
     * Gets an event that when signalled, indicates that an xPL message
     * has been received.  The message(s) can be obtained through a call(s)
     * to GetMsg.
     * <p>
     * When the application is finished using this event, it
     * must be destroyed with a call to CloseHandle.
     * @return A handle to an event that is signalled when at least one
     * message has been received.
     */
    HANDLE GetMsgEvent()
    {
        return m_hMsgRx;
    }

    /**
     * Gets an event that when signalled, indicates that the config
     * items have changed.
     * @return A handle to an event that is signalled when the config items have changed
     */
    HANDLE GetConfigEvent()
    {
        return m_hConfig;
    }

    /**
     * Adds a config item to the device.  Each item represents a variable
     * that the user can modify in xPLHal.
     * <p>
     * This function must only be called before Init(), i.e. before the
     * config item values are read from the registry or configuration file.
     * @param _pItem config item to be added to the xplDevice
     * @return False if a config item with the same name has already been
     * added, otherwise it returns true.
     * @see GetConfigItem, xplConfigItem
     */
    bool AddConfigItem ( xplConfigItem* _pItem );

    /**
     * Removes a config item to the device.  A config item represents a variable
     * that the user can modify in xPLHal.
     * <p>
     * @param _name name of the config item to remove.
     * @return False if a config item with this name cannot be found, otherwise it returns true.
     * @see AddConfigItem, GetConfigItem, xplConfigItem
     */
    bool RemoveConfigItem ( string const& _name );

    /**
     * Gets the config item with the specified name.
     * Searches the xplDevice's list of config items for one that matches
     * the provided name.
     * @param _name name of the config item to get.
     * @return the config item, or NULL if no item by that name can be
     * found.
     * @see AddConfigItem, xplConfigItem
     */
    xplConfigItem const* GetConfigItem ( string const& _name ) const;

    /**
     * Gets the vendor ID string.
     * The vendor ID is a string containing the vendor name of the application
     * author.  Vendor names are assigned on a first-come, first-served basis
     * by the xPL project organsisers.  Vendor IDs are a maximum of 8 characters
     * in length.  To request a vendor name, post a Msg to the xPL Yahoo
     * group http://groups.yahoo.com/groups/ukha_xpl.
     * @return A string containing the vendor ID.
     */
    string const& GetVendorId() const
    {
        return m_vendorId;
    }

    /**
     * Gets the device ID string.
     * The device ID is a string containing a name related to the device that
     * this application supports, usually the same as the application name.
     * Device IDs are a maximum of 8 characters in length.
     * @return A string containing the device ID.
     */
    string const& GetDeviceId() const
    {
        return m_deviceId;
    }

    /**
     * Gets the instance ID string.
     * The instance ID is a string containing a unique name that identifies
     * a particular instance of an application.  Instance IDs are a maximum
     * of 16 characters in length.  The instance ID is initialised to the
     * value "default", but this must be changed in xPLHal before the
     * application can leave config mode.
     * @return A string containing the instance ID.
     */
    string const& GetInstanceId() const
    {
        return m_instanceId;
    }

    /**
     * Gets a complete ID string.
     * The complete ID is the vendor, device and instance IDs combined into the
     * form vendor-device.instance as they would appear in an xpl Msg
     * source or target element.
     * @return A string containing the complete ID.
     */
    string const& GetCompleteId() const
    {
        return m_completeId;
    }

    /**
     * Sets the instance id.
     * The instance ID is a string containing a unique name that identifies
     * a particular instance of an application.  Instance IDs are a maximum
     * of 16 characters in length.  The instance ID is usually initialised
     * to the value "default", but this can be changed in xPLHal or by
     * this method.
     * @see GetCompleteId, GetVendorId, GetDeviceId, GetInstanceId
     */
    void SetInstanceId ( string const& _instanceId )
    {
        m_instanceId = _instanceId;
        SetCompleteId();
    }

private:
    /**
     * Handles the xPL configuration messages.
     * Registered as a message callback during Init.
     * @param _pMsg message to be handled
     * @param _pContext callback context.  A pointer to the xplDevice
     * that registered this callback.
     * @return True if the message has been handled.
     * @see HandleMsg, RegisterMsgCallback, pfnMsgCallback
     */
    static bool MsgCallback ( xplMsg* _pMsg, void* _pContext );

    /**
     * Constructor.  Only to be called via the static Create method.
     * Parameters are as described for Create.
     * @see Create, Destroy
     */
    xplDevice ( string const& _vendorId, string const& _deviceId, string const& _version, bool const _configInRegistry, bool const _bFilterMsgs, xplComms* _pComms );

    /**
     * Destructor.  Only to be called via the static Destroy method.
     * @see Destroy, Create
     */
    ~xplDevice();

    /**
     * Tests whether the received message should be processed by this application
     * @param _pMsg the message to test.
     * @return True if the message is intended for this application
     */
    bool IsMsgForThisApp ( xplMsg* _pMsg );

    /**
     * Tries to handle a received message.
     * This method should only be called by MsgCallback.
     * @param _pMsg the received message.
     * @return True if the message was dealt with.
     * @see MsgCallback
     */
    bool HandleMsg ( xplMsg* _pMsg );

    /**
     * Configures the xplDevice.  Sets the device parameters according to
     * the values stored in the config items.  This method should only be
     * called by ConfigCallback.
     * @see ConfigCallback
     */
    void Configure();

    /**
     * Loads the config items.  Values for the config items are read from
     * the registry or config file.  Where the values come from depends
     * on m_bConfigInRegistry, whcih is set during Create
     * @see SaveConfig, Create
     */
    void LoadConfig();

    /**
     * Saves the config items.  Values for the config items are written
     * to the registry or config file.  Where the values are stored
     * depends on m_bConfigInRegistry, whcih is set during Create
     * @see LoadConfig, Create
     */
    void SaveConfig() const;

    /**
     * Sends a config.list xPL message.
     */
    void SendConfigList() const;

    /**
     * Sends a config.current xPL message.
     */
    void SendConfigCurrent() const;

    /**
     * Sets the time when the next heartbeat should be sent.
     */
    void SetNextHeartbeatTime();

    /**
     * Builds the complete ID string.
     * The complete ID is the vendor, device and instance IDs combined into the
     * form vendor-device.instance as they would appear in an xpl message
     * source or target element.
     * @see GetCompleteId, GetVendorId, GetDeviceId, GetInstanceId
     */
    void SetCompleteId();

    /**
     * Thread procedure that handles all the xplDevice message traffic
     * @param _lpArg thread procedure argument.  Points to the xplDevice object.
     * @return The exit code of the thread.
     */
    static DWORD WINAPI DeviceThread ( void* _lpArg );

    /**
     * Handles all the xplDevice message traffic.
     * This method is called from DeviceThread and does not
     * exit until the thread is finished.  It handles heartbeats,
     * triggers callbacks when messages are received, and sends
     * messages queued by calls to SendMsg.
     * @return False if the xplDevice has not been initialised,
     * otherwise it returns true.
     */
    bool Run();

    string					m_vendorId;					// Application vendor name
    string					m_deviceId;					// Application device name
    string					m_instanceId;				// Application instance name
    string					m_completeId;				// Complete ID string of the form "vendor-device.instance"
    string					m_version;					// Version number of the application.  This should match the version number used in the installer properties.

    int64					m_nextHeartbeat;			// Time of next heartbeat message
    uint32					m_heartbeatInterval;		// Interval in minutes between heartbeats.  Must be between 5 and 9 inclusive.
    uint32					m_rapidHeartbeatCounter;	// Counts down to zero to stop the rapid heatbeats after two minutes.
    bool					m_bWaitingForHub;			// True if we haven't yet detected the presence of the hub

    HANDLE					m_hThread;					// Handle to the xplDevice thread
    HANDLE					m_hRxInterrupt;				// Event that is signalled to interrupt the device thread waiting for messages.
    HANDLE					m_hActive;					// Manual-reset event that is set to not-signalled when the device thread should be paused.
    HANDLE                  m_hMsgRx;					// Event that is signalled when there is a message available.
    HANDLE                  m_hConfig;					// Event that is signalled when the config items have been updated.
    CRITICAL_SECTION		m_criticalSection;			// Used to serialise access to m_txBuffer.
    vector<xplMsg*>			m_txBuffer;					// Stores messages until they can be sent by the device thread.
    deque<xplMsg*>          m_rxBuffer;					// Store messages that have been received.
    bool					m_bPaused;					// Set to true when device activity is paused.
    bool					m_bExitThread;				// If true, the device thread should exit.

    bool					m_bConfigRequired;			// True if configuration via xPLHal is required
    bool					m_bConfigInRegistry;		// Config values to be loaded/saved in the registry
    vector<xplConfigItem*>	m_configItems;				// List of config items
    vector<xplFilter*>		m_filters;					// List of message filters
    bool					m_bFilterMsgs;				// If false, all messages received by the app are queued - regardless of the message target or any filters that have been set.
    bool					m_bInitialised;				// True if Init() has been called
    xplComms*				m_pComms;					// Communications object to use for sending/receiving  messages

    static string const		c_xplGroup;						// Constant containing the string for a group message target
    static uint32 const		c_rapidHeartbeatFastInterval;	// Three seconds for the first
    static uint32 const		c_rapidHeartbeatTimeout;		// two minutes, after which the rate drops to
    static uint32 const		c_rapidHeartbeatSlowInterval;	// once every thirty seconds.
};

} // namespace xpl

#endif // _xPLDevice_H

