/***************************************************************************
****																	****
****	xplConfigItem.h													****
****																	****
****	Container for xpl device configuration data						****
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

#ifndef _XPLCONFIGITEM_H
#define _XPLCONFIGITEM_H

#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <string>
#include <vector>
#include "xplCore.h"
#include "xplDevice.h"
#include "xplMsgItem.h"

namespace xpl
{

/**
 * Represents a variable whose value can be edited in xPLHal.
 * A config item is used to reflect the value of a variable so that it may be
 * changed by the user in xPLHal.  A good example is a com port index.  It
 * is impossible for an application to reliably detect which com port a piece
 * of hardware is attached to, so it becomes necessary to let the user set
 * the value.  By exposing the com port index via an xplConfigItem, this
 * becomes possible.
 * <p>
 * Once an xplConfigItem has been created, its default value (or values - the
 * item can be an array) must be set through calls to AddValue.  The
 * xplConfigItem is then added to the xplDevice by calling xplDevice::AddConfigItem.
 * Once an xplConfigItem has been added to the xplDevice, it will automatically
 * be presented to the user in xPLHal.  Its value will also be saved to the
 * registry or config file (depending on how the xplDevice was defined), and it
 * will be restored when the application is next started.
 */
class xplConfigItem: public xplMsgItem
{
public:
    /**
     * Constructor.
     * Creates a new, empty xplConfigItem, containing no values.  These must be
     * added through calls to AddValue.
     * @param _name unique name identifying the config item.  This name will be
     * displayed in the xPLHal user interface.
     * @param _type type of config item.  Must be one of "config", "reconf" or
     * "option".  Config elements specify items that are mandatory for the device
     * to function, and that cannot be changed once a device is running.  Reconf
     * elements specify items which are mandatory for the device to operate, but
     * whose value can be changed at any time while the device is operating.
     * Option elements specify items that are not required for device operation,
     * where there is a suitable default that can be used.
     * @param _maxValues maximum number of values that can be associated with this
     * config item.  Defaults to one.  If the item is an array of values, the size
     * of that array is specified here, otherwise it should be set to one.
     * @see AddValue.
     */
    xplConfigItem ( string const& _name, string const& _type, unsigned int const _maxValues = 1 ) :
        xplMsgItem ( _name ),
        m_type ( _type ),
        m_maxValues ( _maxValues )
    {
    }

    /**
     * Destructor
     */
    ~xplConfigItem()
    {
    }

    /**
     * Adds a value to this config item.
     * The number of values cannot exceed that which was passed into
     * the constructor.
     * @param _value the value to add to this config item.
     * @return False if adding this value would put the number of
     * values stored in this item over the maximum specified in the
     * constructor.  Otherwise returns true.
     */
    virtual bool AddValue ( string const& _value );

    /**
     * Gets the configuration type for this item.
     * The configuration type is one of "config", "reconf" or "option".
     * Config elements specify items that are mandatory for the device
     * to function, and that cannot be changed once a device is running.  Reconf
     * elements specify items which are mandatory for the device to operate, but
     * whose value can be changed at any time while the device is operating.
     * Option elements specify items that are not required for device operation,
     * where there is a suitable default that can be used.
     * @return A string containing the config item type.
     */
    string const& GetType() const
    {
        return m_type;
    }

    /**
     * Gets the maximum number of values that can be stored in this item.
     * @return The maximum number of values that can be stored in this item.
     */
    unsigned int const GetMaxValues() const
    {
        return ( m_maxValues );
    }

private:
    friend class xplDevice;

    /**
     * Reads the config item's values from a file.
     * @param _hFile handle of the file containing the configuration data.
     * @return Always returns true.
     */
    bool FileLoad ( HANDLE& _hFile );

    /**
     * Writes the config item's values to a file.
     * @param _hFile handle of the file that will contain the configuration data.
     * @return Always returns true.
     */
    bool FileSave ( HANDLE& _hFile ) const;

    /**
     * Reads the config item's values from the registry.
     * @param _hKey registry key containing the config item's value.
     * @return True if successful, false if the config item cannot
     * be found.
     */
    bool RegistryLoad ( HKEY& _hKey );

    /**
     * Writes the config item's values to the registry.
     * @param _hKey registry key that will contain the config item's value.
     * @return Always returns true.
     */
    bool RegistrySave ( HKEY& _hKey ) const;

    string		m_type;			// "reconf", "config" or "option" - See xPL core docs.
    uint32		m_maxValues;
};

} // namespace xpl

#endif // _xplConfigItem_H

