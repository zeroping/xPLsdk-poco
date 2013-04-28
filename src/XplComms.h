/***************************************************************************
****																	****
****	XplComms.h														****
****																	****
****	Communications base class										****
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

#ifndef _XplComms_H
#define _XplComms_H

#define WIN32_LEAN_AND_MEAN

// #include <windows.h>
#include <string>
#include <Poco/Event.h>
#include <Poco/Notification.h>
#include <Poco/NotificationCenter.h>
#include "XplCore.h"
#include "XplMsg.h"

using namespace Poco;

namespace xpl
{

class XplMsg;

class MessageRxNotification: public Notification
{
public:
    MessageRxNotification ( AutoPtr<XplMsg> msgIn )
    {
        message = msgIn;
    }
    AutoPtr<XplMsg> message;
};

/**
 * Base class for communications objects
 */
class XplComms
{
public:

    static XplComms* instance();

    /**
     * Destructor.  Only to be called via the Destroy method.
     * @see Destroy
     */
    virtual ~XplComms();

    /**
     * Sends an xPL message.
     * @param _pMsg pointer to a completed XplMsg object describing the
     * message to be sent.
     * @param _pMsg the message to send.
     * @return True if the message was sent successfully.
     * @see XplMsg
     */
    virtual bool TxMsg ( XplMsg& _pMsg ) = 0;

    /**
     * Sends an xPL heartbeat message.
     * @param source the full vendor-device.instance name of this application.
     * @param interval the time between heartbeats in minutes.  The value
     * must be between 5 and 9 inclusive.
     * @param version version number of the application.
     */
    virtual void SendHeartbeat ( string const& source, uint32 const interval, string const& version ) = 0;

    /**
     * Sends an xPL config heartbeat message.
     * @param source the full vendor-device.instance name of this application.
     * @param interval the time between heartbeats in minutes.  The value
     * must be between 5 and 9 inclusive.  Note that this is the same interval
     * that would be passed to SendHeartbeat, and not the interval between
     * config heartbeats which is always one minute.
     * @param version version number of the application.
     */
    virtual void SendConfigHeartbeat ( string const& source, uint32 const interval, string const& version ) = 0;

    NotificationCenter rxNotificationCenter; // used to notify devices about incomming messages

protected:
    /**
     * Constructor.  Only to be called via the static Create method of
     * a derived class
     */
    XplComms();

    /**
     * Initialises the underlying communications objects
     * @return True if successful.
     * @see Disconnect
     */
    virtual bool Connect();

    /**
     * Deletes the underlying communications objects
     * @see Connect
     */
    virtual void Disconnect();

    /**
     * Tests whether communications have been initialised.
     * @return True if the Connect() method has been called successfully.
     * @see Connect, Disconnect
     */
    bool IsConnected() const
    {
        return connected_;
    }

private:
    bool	            	connected_;		// True if Connect() has been called successfully
};

} // namespace xpl

#endif // _XplComms_H

