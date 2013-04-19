/***************************************************************************
****																	****
****	xplMsgItem.cpp													****
****																	****
****	Container for xpl device configuration data		 				****
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
#include "xplMsgItem.h"

using namespace xpl;


/***************************************************************************
****																	****
****	xplMsgItem::xplMsgItem											****
****																	****
***************************************************************************/

xplMsgItem::xplMsgItem ( string const& _name )
{
    m_name = StringToLower ( _name );
}


/***************************************************************************
****																	****
****	xplMsgItem::AddValue											****
****																	****
***************************************************************************/

bool xplMsgItem::AddValue
(
    string const& _value,
    char const _delimiter
)
{
    // Add the value, ensuring that it does not exceed the 128 character
    // maximum length.  If the string is longer, it must be broken down
    // and multiple name=value pairs created.
    while ( 1 )
    {
        string remainder = _value;
        if ( remainder.size() <= 128 )
        {
            m_values.push_back ( remainder );
            break;
        }

        // remainder needs to be split into multiple shorter strings
        int breakpos = ( int ) remainder.rfind ( _delimiter, 128 );
        if ( string::npos == breakpos )
        {
            // Failed to break the string down.
            return false;
        }

        m_values.push_back ( remainder.substr ( 0, breakpos ) );
        remainder = remainder.substr ( breakpos+1 );
    }

    return true;
}


/***************************************************************************
****																	****
****	xplMsgItem::SetValue											****
****																	****
***************************************************************************/

bool xplMsgItem::SetValue ( string const& _value, uint32 const _index /* = 0 */ )
{
    // Make sure there is a value at this index
    if ( _index >= GetNumValues() )
    {
        assert ( 0 );
        return false;
    }

    m_values[_index] = _value;
    return true;
}


/***************************************************************************
****																	****
****	xplMsgItem::GetValue											****
****																	****
***************************************************************************/

string const xplMsgItem::GetValue
(
    uint32 const _index /* =0 */
) const
{
    if ( _index >= m_values.size() )
    {
        return ( string ( "" ) );
    }

    return ( m_values[_index] );
}

