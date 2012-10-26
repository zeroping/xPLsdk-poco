/***************************************************************************
****																	****
****	xplComms.h														****
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

#ifndef _xplComms_H
#define _xplComms_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <string>
#include "xplCore.h"

namespace xpl
{

class xplMsg;

/**
 * Base class for communications objects
 */
class xplComms
{
public:
    /**
     * Destroys an xplComms object.
     * @param _pComms object to be deleted.
     */
    static void Destroy ( xplComms* _pComms );

    /**
     * Sends an xPL message.
     * @param _pMsg pointer to a completed xplMsg object describing the
     * message to be sent.
     * @param _pMsg the message to send.
     * @return True if the message was sent successfully.
     * @see xplMsg
     */
    virtual bool TxMsg ( xplMsg* _pMsg ) = 0;

    /**
     * Receive an xPL message.  This function will block until either a
     * message is received, the timeout expires, or the interrupt event
     * becomes signalled.
     * If a message is received before the timeout expires or the event
     * is signalled, it is processed into an xplMsg object which is then
     * returned. The caller is responsible for deleting the returned
     * object through a call to xplMsg::Release.
     * @param _hInterrupt handle to a Windows event that when signalled causes
     * the method to stop waiting and return.  Defaults to INVALID_HANDLE_VALUE.
     * @param _timeout number of milliseconds that the method should wait
     * before returning.  A value of zero will cause the function to return
     * immediately.  Passing INFINITE will disable the timeout (it will never
     * expire).  Defaults to zero.
     * @return A pointer to a new xplMsg object, or NULL if the timeout
     * expired or the wait was interrupted.
     * @see xplMsg
     */
    virtual xplMsg* RxMsg ( HANDLE _hInterrupt = INVALID_HANDLE_VALUE, uint32 _timeout = 0 ) = 0;

    /**
     * Sends an xPL heartbeat message.
     * @param _source the full vendor-device.instance name of this application.
     * @param _interval the time between heartbeats in minutes.  The value
     * must be between 5 and 9 inclusive.
     * @param _version version number of the application.
     */
    virtual void SendHeartbeat ( string const& _source, uint32 const _interval, string const& _version ) = 0;

    /**
     * Sends an xPL config heartbeat message.
     * @param _source the full vendor-device.instance name of this application.
     * @param _interval the time between heartbeats in minutes.  The value
     * must be between 5 and 9 inclusive.  Note that this is the same interval
     * that would be passed to SendHeartbeat, and not the interval between
     * config heartbeats which is always one minute.
     * @param _version version number of the application.
     */
    virtual void SendConfigHeartbeat ( string const& _source, uint32 const _interval, string const& _version ) = 0;

protected:
    /**
     * Constructor.  Only to be called via the static Create method of
     * a derived class
     */
    xplComms();

    /**
     * Destructor.  Only to be called via the Destroy method.
     * @see Destroy
     */
    virtual ~xplComms();

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
        return m_bConnected;
    }

    int8*	                m_pMsgBuffer;		// Buffer for temporary storage of incoming xPL message data
    static uint32 const     c_msgBufferSize;	// Size in bytes of temporary storage buffer

private:
    bool	            	m_bConnected;		// True if Connect() has been called successfully
};

} // namespace xpl

#endif // _xplComms_H

