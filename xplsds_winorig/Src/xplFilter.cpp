/***************************************************************************
****																	****
****	xplFilter.cpp													****
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

#include "xplCore.h"
#include "xplStringUtils.h"
#include "xplFilter.h"
#include "xplMsg.h"

using namespace xpl;


/***************************************************************************
****																	****
****	xplFilter::xplFilter											****
****																	****
***************************************************************************/

xplFilter::xplFilter ( string const& _filterStr )
{
    // Parse the string an break it into its elements
    m_filterElementMask = 0;
    string remainder = _filterStr;

    StringSplit ( remainder, '.', &m_msgType, &remainder );
    if ( m_msgType != string ( "*" ) )
    {
        m_filterElementMask |= FilterElement_MsgType;
    }

    StringSplit ( remainder, '.', &m_vendor, &remainder );
    if ( m_vendor != string ( "*" ) )
    {
        m_filterElementMask |= FilterElement_Vendor;
    }

    StringSplit ( remainder, '.', &m_device, &remainder );
    if ( m_device != string ( "*" ) )
    {
        m_filterElementMask |= FilterElement_Device;
    }

    StringSplit ( remainder, '.', &m_instance, &remainder );
    if ( m_instance != string ( "*" ) )
    {
        m_filterElementMask |= FilterElement_Instance;
    }

    StringSplit ( remainder, '.', &m_class, &remainder );
    if ( m_class != string ( "*" ) )
    {
        m_filterElementMask |= FilterElement_Class;
    }

    StringSplit ( remainder, '.', &m_type, &remainder );
    if ( m_type != string ( "*" ) )
    {
        m_filterElementMask |= FilterElement_Type;
    }
}


/***************************************************************************
****																	****
****	xplFilter::Allow												****
****																	****
***************************************************************************/

bool xplFilter::Allow ( xplMsg const& _msg ) const
{
    // Check the message type
    if ( ( m_filterElementMask & FilterElement_MsgType )
            && ( _msg.GetType() != m_msgType ) )
    {
        return false;
    }

    // Check the schema class
    if ( ( m_filterElementMask & FilterElement_Class )
            && ( _msg.GetSchemaClass() != m_class ) )
    {
        return false;
    }

    // Check the schema type
    if ( ( m_filterElementMask & FilterElement_Type )
            && ( _msg.GetSchemaType() != m_type ) )
    {
        return false;
    }

    // Check the message source
    if ( m_filterElementMask & ( FilterElement_Vendor|FilterElement_Device|FilterElement_Instance ) )
    {
        // Extract the vendor from the message source
        string vendor;
        string deviceInstance;
        StringSplit ( _msg.GetSource(), '-', &vendor, &deviceInstance );

        // Check the message source vendor
        if ( ( m_filterElementMask & FilterElement_Vendor )
                && ( vendor != m_vendor ) )
        {
            return false;
        }

        if ( m_filterElementMask & ( FilterElement_Device|FilterElement_Instance ) )
        {
            // Extract the device and instance from the message source
            string device;
            string instance;
            StringSplit ( deviceInstance, '.', &device, &instance );

            // Check the message source device
            if ( ( m_filterElementMask & FilterElement_Device )
                    && ( device != m_device ) )
            {
                return false;
            }

            if ( ( m_filterElementMask & FilterElement_Instance )
                    && ( instance != m_instance ) )
            {
                return false;
            }
        }
    }

    return true;
}

