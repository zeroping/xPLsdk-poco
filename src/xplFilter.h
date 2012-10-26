/***************************************************************************
****																	****
****	xplFilter.h														****
****																	****
****	xPL Message Filters												****
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

#ifndef _XPLFILTER_H
#define _XPLFILTER_H

#include <string>
#include "xplCore.h"

namespace xpl
{

class xplDevice;
class xplMsg;

/**
 * Implements filtering of xPL messages.
 * Note: This class has an entirely private interface, with only the xplDevice
 * class being granted access.
 * <p>
 * Filters are one of the config items defined by the xplDevice, and are set
 * up by the user in xPLHal.  They take the form of a string defined as
 * follows:
 * <p>
 *     [msgtype].[vendor].[device].[instance].[class].[type]
 * <p>
 * All elements must be present, but may be wildcarded with a '*'.  If any
 * filters have been defined, only messages that pass at least one of them
 * will be acted upon.
 */
class xplFilter
{
private:
    friend class xplDevice;

    /**
     * Constructor.
     * @param _filterStr A string in the form
     * [msgtype].[vendor].[device].[instance].[class].[type]
     * All elements must be present, but may be wildcarded with a '*'.
     */
    xplFilter ( string const& _filterStr );

    /**
     * Destructor.
     */
    ~xplFilter() {}

    /**
     * Filters a message.  Compares the message elements to the filters.
     * If any filter matches the message, it is allowed to pass.
     * @param _msg the message to be tested.
     * @return True if the message passes the filters, false if it
     * should be ignored.
     */
    bool Allow ( xplMsg const& _msg ) const;

    enum
    {
        FilterElement_MsgType	= 0x00000001,
        FilterElement_Vendor	= 0x00000002,
        FilterElement_Device	= 0x00000004,
        FilterElement_Instance	= 0x00000008,
        FilterElement_Class		= 0x00000010,
        FilterElement_Type		= 0x00000020
    };

    string	m_msgType;
    string	m_vendor;
    string	m_device;
    string	m_instance;
    string	m_class;
    string	m_type;
    uint32	m_filterElementMask;	//Bit cleared if element is wildcard

}; // class xplFilter

} // namespace xpl

#endif //_XPLFILTER_H

