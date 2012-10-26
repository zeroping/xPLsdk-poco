/***************************************************************************
****																	****
****	xplMsg.h														****
****																	****
****	Classes for parsing and creating xpl Messages					****
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

#ifndef _xplMsg_H
#define _xplMsg_H

#pragma once

#include <string>
#include <vector>
#include "xplCore.h"
#include "xplRef.h"
#include "xplMsgItem.h"

namespace xpl
{

/**
 * Represents an xPL message.
 * The xplMsg class encapsulates all the parameters that make up an xPL message.
 * It provides a simple interface for creating and manipulating xPL messages,
 * freeing the user from the burden of parsing and formatting the raw message
 * data.
 * <p>
 * There are two ways to create an xplMsg object, both via static Create methods.
 * The first method creates a message from a buffer containing raw xPL message
 * data as it would be received from another xPL application over the network.
 * This approach will be most commonly used internally by the xplDevice class
 * when it receives a message, but it could also be used to build an xplMsg from
 * a string of data created by sprintf, for example.
 * <p>
 * The second method creates a skeleton xPL message containing a header and
 * schema but with no name=value pairs in the message body.  These values are
 * added individually through subsequent calls to xplMsg::AddValue.
 * <p>
 * Once created, sending the message is a simple matter of passing it to the
 * xplDevice::SendMessage method
 */
class xplMsg: public xplRef
{
public:
    /**
     * Creates an xplMsg.
     * Creates an xplMsg object from raw message data, exactly as
     * would be sent/received over the network.  It also allows the creation of
     * an xplMsg object from a char string. For example, sprintf could be used
     * to assemble the raw message, which would then be converted to an xplMsg
     * object by a call to this method.
     * @param _pBuffer pointer to the buffer containing the raw message data.
     * @return If the buffer contained valid xPL message data, this method returns
     * a pointer to a new xplMsg object, otherwise NULL is returned.  The caller
     * is responsible for deleting any return object through a call to xplMsg::Release.
     * @see Destroy, GetAsRawData
     */
    static xplMsg* Create ( int8 const* _pBuffer );

    /**
     * Creates an xplMsg.
     * Creates a skeleton xPL message.  Once created, the individual name=value pairs
     * that make up the message body are added through successive calls to
     * xplMsg::AddValue.
     * @param _type  message type.  Must be one of "xpl-cmnd", "xpl-trig" or "xpl-stat".
     * @param _source message sender.  Complete ID of the sending application
     * (in the form vendor-device.instance).
     * @param _target message destination.  Complete ID of the message destination
     * (in the form vendor-device.instance), or "*" to send to all.
     * @param _schemaClass message schema class.  First part of the xPL message
     * schema (for example, the "x10" in "x10.basic").
     * @param _schemaType message schema type.  Second part of the xPL message
     * schema (for example, the "basic" in "x10.basic").
     * @return A new xplMsg object, if the parameters are all valid.  Otherwise
     * the method returns NULL.  The caller is responsible for deleting any returned
     * object through a call to xplMsg::Release.
     * @see Destroy
     */
    static xplMsg* Create ( string const& _type, string const& _source, string const& _target, string const& _schemaClass, string const& _schemaType );

    /**
     * Adds a new name=value pair to the message body.
     * The body of an xPL message (inside the schema part) consists of a series
     * of name=value pairs of strings.  This method is used to add new pairs to
     * the message body.  Pairs appear in the final message in the same order that
     * they are added.  The exception to this rule occurs with pairs that share the
     * same name (It can be perfectly legitimate to have name=value pairs that
     * share the same name, and even the same value).  These items appear
     * immediately after the first item of that name in the message body.
     * @param _name name of the item that we wish to add.  For example, the
     * "command" in the "command=dim" pair that might appear in an x10.basic message.
     * @param _value value of that item.  For example the "dim" in the pair
     * "command=dim".
     * @param _delimiter Character that marks a point where the value string may
     * be broken to be split across multiple name=value lines.  This will only be
     * necessary if the length of the value exceeds the maximum 128 characters.
     * @see GetValue, SetValue, GetMsgItem.
     */
    void AddValue ( string const& _name, string const& _value, char const _delimiter = ',' );

    /**
     * Adds a new name=value pair to the message body.
     * The body of an xPL message (inside the schema part) consists of a series
     * of name=value pairs of strings.  This method is used to add new pairs to
     * the message body.  Pairs appear in the final message in the same order that
     * they are added.  The exception to this rule occurs with pairs that share the
     * same name (It can be perfectly legitimate to have name=value pairs that
     * share the same name, and even the same value).  These items appear
     * immediately after the first item of that name in the message body.
     * @param _name name of the item that we wish to add.  For example, the
     * "command" in the "command=dim" pair that might appear in an x10.basic message.
     * @param _value value of that item.  The value is an integer that will be converted
     * into a string when stored in the pair
     * @see GetValue, SetValue, GetMsgItem.
     */
    void AddValue ( string const& _name, int32 const _value );

    /**
     * Sets the value of an existing name-value pair in the message body.
     * Replaces the existing value with the new value.  If there is more
     * than one name=value pair with the specified name, the _index parameter
     * will be required to select the correct value to modify.  Because it is
     * valid to have more than one pair with the same name and the same value,
     * it is impossible to search for the  orrect pair even if supplied with
     * both the original name and the value.  For that reason, there is no
     * search method implemented that will return the index to use.  It is up
     * to the caller to track the order in which items are added.  The first
     * pair with a specific name will have an index of zero, the next pair
     * with that same name to be added will have an index of 1, and so on.
     * @param _name name of the item for which we wish to set the value.
     * @param _value new value for the pair.
     * @param _index the value index.  Optional parameter used when the name has
     * more than one value associated with it (as with filters and groups, for example).
     * Defaults to zero.
     * @return True if the name=value pair was added successfully.
     * @see AddValue, GetValue, GetMsgItem.
     */
    bool SetValue ( string const& _name, string const& _value, uint32 const _index = 0 );

    /**
     * Gets the value from a name-value pair in the message body.
     * @param _name name of the item for which we wish to obtain the value.
     * @param _index the value index.  Optional parameter used when the name has more
     * than one value associated with it (as with filters and groups, for example).
     * For a more detailed description of how the indexing works, @see xplMsg::SetValue.
     * Defaults to zero.
     * @return A string containing the value from the specified name=value pair.
     * @see AddValue, SetValue, GetMsgItem.
     */
    string const GetValue ( string const& _name, uint32 const _index = 0 ) const;

    /**
     * Gets the value from a name-value pair in the message body and converts it to an integer.
     * @param _name name of the item for which we wish to obtain the value.
     * @param _index the value index.  Optional parameter used when the name has more
     * than one value associated with it (as with filters and groups, for example).
     * For a more detailed description of how the indexing works, @see xplMsg::SetValue.
     * Defaults to zero.
     * @return An int containing the value from the specified name=value pair.  If the value does
     * not convert to an int, then -1 is returned.
     * @see AddValue, SetValue, GetMsgItem.
     */
    int const GetIntValue ( string const& _name, uint32 const _index = 0 ) const;

    /**
     * Gets the all the values from the name-value pairs with the specified name.
     * The values are concatenated into a single string, separated by the delimiter character.
     * @param _name name of the item for which we wish to obtain the values.
     * @param _delimiter Character used to separate the values when concatenated into
     * the single returned string.
     * @return A string containing all the values from the specified name=value pairs.
     * @see AddValue, SetValue, GetMsgItem.
     */
    string const GetCompleteValue ( string const& _name, char const _delimiter = ',' ) const;

    /**
     * Gets the number of xplMsgItems in the message.
     * @return The number of xplMsgItems contained in the message.
     * @see GetMsgItem.
     */
    uint32 GetNumMsgItems() const
    {
        return ( uint32 ) m_msgItems.size();
    }

    /**
     * Gets an xplMsgItem.
     * Gets a pointer to the xplMsgItem object that stores the array of values for the named item.
     * @param _name name of the item for which we wish to retrieve the value(s).
     * @return A pointer to the xplMsgItem object, or NULL if the named item does not exist.
     * @see AddValue, GetValue, SetValue.
     */
    xplMsgItem const* GetMsgItem ( string const& _name ) const;

    /**
     * Gets an xplMsgItem.
     * Gets a pointer to the xplMsgItem by index.
     * @param _index index of the item for which we wish to retrieve the value(s).
     * @return A pointer to the xplMsgItem object, or NULL if the index is out of range.
     * @see AddValue, GetValue, SetValue.
     */
    xplMsgItem const* GetMsgItem ( uint32 const _index ) const;

    /**
     * Gets the message in it's raw data form.
     * Fills a buffer with the message in the form of a string of characters
     * as would be transmitted over a network to another xPL application.
     * @param _pBuffer On return this will contain a pointer to the raw data.
     * @return The size of the message buffer in bytes.
     * @see Create
     */
    uint32 GetRawData ( int8** _pBuffer );

    /**
     * Gets the hop count of the message.
     * @return The hop count from the message header.
     * @see SetHop.
     */
    int32 GetHop() const
    {
        return m_hop;
    }

    /**
     * Gets the message type.  This can only be one of "xpl-cmnd",
     * "xpl-trig" or "xpl-stat"
     * @return A string containing the type of the message.
     * @see SetType.
     */
    string const& GetType() const
    {
        return m_type;
    }

    /**
     * Gets the message source.  The source is a string made
     * from the vendor, device and instance IDs of the message
     * sender, formatted as vendor-device.instance.
     * @return A string containing the message source.
     * @see SetSource.
     */
    string const& GetSource() const
    {
        return m_source;
    }

    /**
     * Gets the message target.  The target is a string made
     * from the vendor, device and instance IDs of the message
     * destination, formatted as vendor-device.instance.
     * The only exception is the special case string "*"
     * which is used to send the message to all.
     * @return A string containing the message target.
     * @see SetTarget.
     */
    string const& GetTarget() const
    {
        return m_target;
    }

    /**
     * Gets the schema class.  An xPL message schema name has two parts
     * separated by a period,  The schema class is the left hand part.
     * For example, the "x10" in "x10.basic".
     * @return A string containing the message schema class.
     * @see SetSchemaClass.
     */
    string const& GetSchemaClass() const
    {
        return m_schemaClass;
    }

    /**
     * Gets the schema type.  An xPL message schema name has two parts
     * separated by a period,  The schema type is the right hand part.
     * For example, the "basic" in "x10.basic".
     * @return A string containing the message schema type.
     * @see SetSchemaType.
     */
    string const& GetSchemaType() const
    {
        return m_schemaType;
    }

    /**
     * Sets the hop count of the message.  A new xPL message will
     * already have a hop count of one, which is the default value
     * assigned by the class constructor.
     * @param _hop the hop count.  This must be an integer between
     * one and nine.
     * @return True if the hop count was set.
     * @see GetHop.
     */
    bool SetHop ( uint32 _hop );

    /**
     * Sets the message type.  This can only be one of "xpl-cmnd",
     * "xpl-trig" or "xpl-stat"
     * @param _type the message type.
     * @return True if the message type was set.
     * @see GetType.
     */
    bool SetType ( string const& _type );

    /**
     * Sets the message source.  The source is a string made
     * from the vendor, device and instance IDs of the message
     * sender, formatted as vendor-device.instance.
     * @param _source the message source in the form vendor-device.instance.
     * @return True if the message source was set.
     * @see GetSource.
     */
    bool SetSource ( string const& _source );

    /**
     * Sets the message target.  The target is a string made
     * from the vendor, device and instance IDs of the message
     * destination, formatted as vendor-device.instance.
     * The only exception is the special case string "*"
     * which is used to send the message to all.
     * @param _target the message target.  Either a string formatted as
     * vendor-device.instance or "*".
     * @return True if the message target was set.
     * @see GetTarget.
     */
    bool SetTarget ( string const& _target );

    /**
     * Sets the schema class.  An xPL message schema name has two parts
     * separated by a period,  The schema class is the left hand part.
     * For example, the "x10" in "x10.basic".
     * @param _schemaClass the schema class.  This must be between one
     * and eight characters in length.
     * @return True if the schema class was set.
     * @see GetSchemaClass.
     */
    bool SetSchemaClass ( string const& _schemaClass );

    /**
     * Sets the schema type.  An xPL message schema name has two parts
     * separated by a period,  The schema type is the right hand part.
     * For example, the "basic" in "x10.basic".
     * @param _schemaType the schema type.  This must be between one
     * and eight characters in length.
     * @return True if the schema type was set.
     * @see GetSchemaType.
     */
    bool SetSchemaType ( string const& _schemaType );

    bool operator == ( xplMsg const& _rhs );

    // String constants for various pieces of an xPL message
    static string const c_xplCmnd;
    static string const c_xplTrig;
    static string const c_xplStat;
    static string const c_xplHop;
    static string const c_xplSource;
    static string const c_xplTarget;
    static string const c_xplOpenBrace;
    static string const c_xplCloseBrace;
    static string const c_xplTargetAll;

private:
    xplMsg();
    ~xplMsg();

    /**
     * Helper method for extracting a name=value pair from a string.
     * @param _str the string containing the xPL message in raw form.
     * @param _start the position in the string from which to start
     * reading.  This can contain whitespace (including newlines).
     * @param _pName pointer to a string that will be filled with
     * the name part of the name=value pair.
     * @param _pValue pointer to a string that will be filled with
     * the value part of the name=value pair.
     * @return The position in the string from which to continue reading.
     */
    static int32 ReadNameValuePair ( string const& _str, int32 const _start, string* _pName, string* _pValue );

    /**
     * Helper method for deleting the raw data buffer.
     */
    void InvalidateRawData();

    // Header elements
    int32						m_hop;
    string						m_type;
    string						m_source;
    string						m_target;

    // Body elements
    string						m_schemaClass;
    string						m_schemaType;
    vector<xplMsgItem*>			m_msgItems;

    // Raw data
    int8*						m_pBuffer;
    uint32						m_bufferSize;

    // Reference counting
    uint32						m_refCount;

}; // class xplMsg

} // namespace xpl

#endif // _xplMsg_H

