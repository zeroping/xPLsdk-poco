/***************************************************************************
****																	****
****	xplMsgItem.h	 												****
****																	****
****	Represents a name=value pair from an xPL Message				****
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

#ifndef _xplMsgItem_H
#define _xplMsgItem_H

#include <string>
#include <vector>
#include "xplCore.h"

namespace xpl
{

/**
 * Represents name=value pairs in an xPL message body.
 * An xplMsg object will contain one xplMsgItem for each unique name in its
 * list of name=value pairs.  Where there are multiple name=value pairs
 * using the same name, the xplMsgItem will contain all the values.
 */
class xplMsgItem
{
public:
    /**
     * Constructor.
     * @param _name name of the name=value pairs that will be stored
     * in this item.
     */
    xplMsgItem ( string const& _name );

    /**
     * Destructor.
     */
    ~xplMsgItem() {}

    /**
     * Gets the name of this item.
     * @return The item name
     * @see AddValue, GetValue, GetNumValues
     */
    string const& GetName() const
    {
        return m_name;
    }

    /**
     * Gets the number of values stored in this item
     * @return The number of values stored in this item.
     * @see GetValue
     */
    uint32 const GetNumValues() const
    {
        return ( ( uint32 ) m_values.size() );
    }

    /**
     * Gets specific value.
     * Values are stored in the order that they are added.
     * Because duplicate name=value pairs are allowed, no
     * 'find' methods are provided, so it is left up to the
     * application to keep track of value indices.
     * @param _index The index of the value to retrieve.  Defaults
     * to zero.
     * @return A string containing the value, or an empty string
     * if the index was out of range.
     * @see GetNumValues, AddValue, GetName
     */
    string const GetValue ( const uint32 _index = 0 ) const;

    /**
     * Adds a value to this item.
     * @param _value the value to add to this item.
     * @param _delimiter Character that marks a point where the value string may
     * be broken to be split across multiple name=value lines.  This will only be
     * necessary if the length of the value exceeds the maximum 128 characters.
     * @return Returns true unless the value cannot be broken into strings of
     * less than 128 characters.
     */
    virtual bool AddValue ( string const& _value, char const _delimiter = ',' );

    /**
     * Changes a specific value.  Replaces an existing indexed value
     * string with another string.
     * @param _value the new value to replace the old one.
     * @param _index index of the value to be replaced.  Defaults
     * to zero.
     * @return True if the value is successfully changed, false if
     * _index was out of range.
     * @see AddValue, GetValue, GetNumValues
     */
    bool SetValue ( string const& _value, const uint32 _index = 0 );

    /**
     * Removes all the values stored in this item
     */
    void ClearValues()
    {
        m_values.clear();
    }

private:
    string				m_name;
    vector<string>		m_values;

}; // class xplMsgItem

} // namespace xpl

#endif // _xplMsgItem_H

