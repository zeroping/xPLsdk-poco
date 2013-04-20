/***************************************************************************
****																	****
****	XplMsg.cpp														****
****																	****
****	Handle the parsing of xpl messages								****
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

#include "XplCore.h"
#include "XplStringUtils.h"
#include "XplMsg.h"
#include <iostream>
#include <sstream>
#include <Poco/Logger.h>
#include <Poco/String.h>

using namespace xpl;
using Poco::toLower;

string const XplMsg::c_xplCmnd = "xpl-cmnd";
string const XplMsg::c_xplTrig = "xpl-trig";
string const XplMsg::c_xplStat = "xpl-stat";
string const XplMsg::c_xplHop = "hop";
string const XplMsg::c_xplSource = "source";
string const XplMsg::c_xplTarget = "target";
string const XplMsg::c_xplOpenBrace = "{";
string const XplMsg::c_xplCloseBrace = "}";
string const XplMsg::c_xplTargetAll = "*";


/***************************************************************************
****																	****
****	XplMsg Constructor												****
****																	****
***************************************************************************/

XplMsg::XplMsg() :
    m_hop ( 1 ),
    m_refCount ( 1 )
{
}

XplMsg::XplMsg (
    string const& _type,
    string const& _source,
    string const& _target,
    string const& _schemaClass,
    string const& _schemaType
) :
    m_hop ( 1 ),
    m_refCount ( 1 )
{
    SetType ( _type );
    SetSource ( _source );
    SetTarget ( _target );
    SetSchemaClass ( _schemaClass );
    SetSchemaType ( _schemaType );
}

XplMsg::XplMsg ( string str )
{
    ParseFromString ( str );
}




/***************************************************************************
****																	****
****	XplMsg Destructor												****
****																	****
***************************************************************************/

XplMsg::~XplMsg()
{
    vector<AutoPtr<XplMsgItem> >::iterator iter = m_msgItems.begin();
    while ( iter != m_msgItems.end() )
    {
//         delete ( *iter );
        iter = m_msgItems.erase ( iter );
    }

}


/***************************************************************************
****																	****
****	XplMsg::Create													****
****																	****
***************************************************************************/

void XplMsg::ParseFromString ( string str )
{
    // Read the message type
    string line;
    int32 pos = StringReadLine ( str, 0, &line );
    SetType ( line );

    // Skip the opening brace
    pos = StringReadLine ( str, pos, &line );
    if ( c_xplOpenBrace != line )
    {
        throw XplMsgParseException("Opening brace not found");
    }

    // Read the name-value pairs  from the header
    while ( 1 )
    {
        string name;
        string value;

        pos = ReadNameValuePair ( str, pos, &name, &value );

        if ( c_xplCloseBrace == name )
        {
            // Closing brace found
            break;
        }

        if ( c_xplHop == name )
        {
            SetHop ( atol ( value.c_str() ) );
        }
        else if ( c_xplSource == name )
        {
            SetSource ( value );
        }
        else if ( c_xplTarget == name )
        {
            SetTarget ( value );
        }
        else
        {
            throw XplMsgParseException("Invalid header item");
        }
    }

    // Read the schema class and type
    {
        string schemaClass;
        string schemaType;
        pos = StringReadLine ( str, pos, &line );
        StringSplit ( line, '.', &schemaClass, &schemaType );
        SetSchemaClass ( schemaClass );
        SetSchemaType ( schemaType );
    }

    // Read the message body
    // Skip the opening brace
    pos = StringReadLine ( str, pos, &line );
    if ( c_xplOpenBrace != line )
    {
        throw XplMsgParseException("Opening brace not found");
    }

    // Read the name-value pairs
    while ( 1 )
    {
        string name;
        string value;

        pos = ReadNameValuePair ( str, pos, &name, &value );

        if ( name.empty() )
        {
            throw XplMsgParseException("Reached the end of the data without hitting a closing brace.  The message is malformed.");
        }

        if ( c_xplCloseBrace == name )
        {
            // Closing brace found
            break;
        }

        // Add the name=value pair to the message
        AddValue ( name, value );
    }

    // Copy the original raw data
    m_raw = str;

    // Message successfully read from buffer
    return;
    
}




/***************************************************************************
****																	****
****	XplMsg::GetAsRawData											****
****																	****
***************************************************************************/

string XplMsg::GetRawData( )
{
    if ( m_raw.size() ==0 )
    {
        ostringstream msgstream;
        msgstream << m_type << "\n";
        msgstream << "{\n";
        msgstream << "hop=" << m_hop << "\n";
        msgstream << "source=" << m_source.toString() << "\n";
        msgstream << "target=" << m_target.toString() << "\n";
        msgstream << "}\n";
        msgstream << m_schemaClass<<"." << m_schemaType << "\n";
        msgstream << "{\n";

        for ( std::vector< AutoPtr<XplMsgItem>  >::iterator iter = m_msgItems.begin(); iter != m_msgItems.end(); ++iter )
        {
            AutoPtr<XplMsgItem>  pItem = *iter;
            for ( int i=0; i<pItem->GetNumValues(); ++i )
            {
                msgstream << pItem->GetName() << "=" << pItem->GetValue ( i ) << "\n";
            }
        }
        msgstream <<"}\n";
        m_raw = msgstream.str();;
    }

    return m_raw;
}


/***************************************************************************
****																	****
****	XplMsg::AddValue												****
****																	****
***************************************************************************/

void XplMsg::AddValue
(
    string const& _name,
    string const& _value,
    char const _delimiter /* = ',' */
)
{
    InvalidateRawData();

    XplMsgItem* pItem;

    // See if there is already an existing XplMsgItem with this name
    string lowerName = toLower ( _name );
    vector<AutoPtr<XplMsgItem> >::iterator iter;
    for ( iter = m_msgItems.begin(); iter != m_msgItems.end(); ++iter )
    {
        if ( _name == ( *iter )->GetName() )
        {
            // Found a match
            break;
        }
    }

    if ( iter != m_msgItems.end() )
    {
        // Item found
        pItem = *iter;
    }
    else
    {
        // Create a new XplMsgItem
        pItem = new XplMsgItem ( lowerName );
        m_msgItems.push_back ( pItem );
    }

    pItem->AddValue ( _value, _delimiter );
}


/***************************************************************************
****																	****
****	XplMsg::AddValue												****
****																	****
***************************************************************************/

void XplMsg::AddValue
(
    string const& _name,
    int32 const _value
)
{
    char str[16];
    sprintf ( str, "%d", _value );
    AddValue ( _name, str );
}


/***************************************************************************
****																	****
****	XplMsg::SetValue												****
****																	****
***************************************************************************/

bool XplMsg::SetValue
(
    string const& _name,
    string const& _value,
    uint32 const _index
)
{
    InvalidateRawData();

    // Find the XplMsgItem with this name
    for ( vector<AutoPtr<XplMsgItem> >::iterator iter = m_msgItems.begin();  iter != m_msgItems.end(); ++iter )
    {
        if ( _name == ( *iter )->GetName() )
        {
            return ( ( *iter )->SetValue ( _value, _index ) );
        }
    }

    // No entry found for this name
    return ( false );
}


/***************************************************************************
****																	****
****	XplMsg::GetMsgItem												****
****																	****
***************************************************************************/

AutoPtr<XplMsgItem>  XplMsg::GetMsgItem
(
    string const& _name
) const
{
    string lowerName = toLower ( _name );
    for ( vector<AutoPtr<XplMsgItem> >::const_iterator iter = m_msgItems.begin();  iter != m_msgItems.end(); ++iter )
    {
        if ( lowerName == ( *iter )->GetName() )
        {
            // Found a matching item
            return ( *iter );
        }
    }

    // No entry found for this name
    return ( NULL );
}


/***************************************************************************
****																	****
****	XplMsg::GetMsgItem												****
****																	****
***************************************************************************/

AutoPtr<XplMsgItem>  XplMsg::GetMsgItem
(
    uint32 const _index
) const
{
    if ( _index >= m_msgItems.size() )
    {
        // Index out of range
        assert ( 0 );
        return NULL;
    }

    return ( m_msgItems[_index] );
}


/***************************************************************************
****																	****
****	XplMsg::GetValue												****
****																	****
***************************************************************************/

string const XplMsg::GetValue
(
    string const& _name,
    uint32 const _index /*=0*/
) const
{
    if ( XplMsgItem const* pMsgItem = GetMsgItem ( _name ) )
    {
        // Get the value from the  XplMsgItem object
        return ( pMsgItem->GetValue ( _index ) );
    }

    // No entry found for this name
    return ( string ( "" ) );
}


/***************************************************************************
****																	****
****	XplMsg::GetIntValue												****
****																	****
***************************************************************************/

int const XplMsg::GetIntValue
(
    string const& _name,
    uint32 const _index /*=0*/
) const
{
    string value = GetValue ( _name, _index );
    if ( value.empty() )
    {
        return 0;
    }

    int intVal = atoi ( value.c_str() );
    return intVal;
}


/***************************************************************************
****																	****
****	XplMsg::GetCompleteValue										****
****																	****
***************************************************************************/

string const XplMsg::GetCompleteValue
(
    string const& _name,
    char const _delimiter /* = ',' */
) const
{
    string str;
    if ( XplMsgItem const* pMsgItem = GetMsgItem ( _name ) )
    {
        // Get the values from the XplMsgItem object
        for ( int i=0; i<pMsgItem->GetNumValues(); ++i )
        {
            if ( i )
            {
                str += _delimiter;
            }
            str += pMsgItem->GetValue ( i );
        }
    }

    return str;
}


/***************************************************************************
****																	****
****	XplMsg::SetHop													****
****																	****
***************************************************************************/

bool XplMsg::SetHop
(
    uint32 _hop
)
{
    InvalidateRawData();

    if ( _hop > 9 )
    {
        // Hop cannot exceed 9
        assert ( 0 );
        return ( false );
    }

    m_hop = _hop;
    return true;
}


/***************************************************************************
****																	****
****	XplMsg::SetType													****
****																	****
***************************************************************************/

bool XplMsg::SetType
(
    string const& _type
)
{
    InvalidateRawData();

    string lowerType = toLower ( _type );

    //in case we've forgotten the leading bit...
    if ( lowerType.size() ==4 )
    {
        lowerType = "xpl-" + lowerType;
    }

    if ( ( "xpl-trig" != lowerType )
            && ( "xpl-cmnd" != lowerType )
            && ( "xpl-stat" != lowerType ) )
    {
        // Invalid message type
        assert ( 0 );
        return ( false );
    }

    m_type = lowerType;
    return true;
}


/***************************************************************************
****																	****
****	XplMsg::SetSource												****
****																	****
***************************************************************************/

bool XplMsg::SetSource
(
    string const& _source
)
{
    // Source must consist of vendor ID (max 8 chars), device ID
    // (max 8 chars) and instance ID (max 16 chars) in the form
    // vendor-device.instance
    string vendor;
    string deviceInstance;
    StringSplit ( _source, '-', &vendor, &deviceInstance );

    if ( ( vendor.size() > 8 ) || vendor.empty() || deviceInstance.empty() )
    {
        Logger::get ( "xplsdk.comms" ).warning("Invalid xPl source: " + _source);
        return false;
    }
    m_source.vendor = vendor;

    string device;
    string instance;
    StringSplit ( deviceInstance, '.', &device, &instance );

    if ( ( device.size() > 8 ) || device.empty() || ( instance.size() > 16 ) || instance.empty() )
    {
        assert ( 0 );
        return false;
    }

    m_source.device = toLower(device);
    m_source.instance = toLower(instance);
    
    return true;
}

bool XplMsg::SetSource
(
    XPLAddress const& _source
)
{
    InvalidateRawData();
    m_source = _source;

}


/***************************************************************************
****																	****
****	XplMsg::SetTarget												****
****																	****
***************************************************************************/

bool XplMsg::SetTarget
(
    string const& _target
)
{

    InvalidateRawData();

    // For broadcasts, the target can be "*"
    if ( "*" == _target )
    {
        m_target.bcast = true;
        return ( true );
    }

    // If not "*", the target must consist of vendor ID (max 8 chars),
    // device ID (max 8 chars) and instance ID (max 16 chars) in the
    // form vendor-device.instance
    string vendor;
    string deviceInstance;
    StringSplit ( _target, '-', &vendor, &deviceInstance );


    //if( ( vendor.size() > 8 ) || vendor.empty() || deviceInstance.empty() )
    if ( vendor.empty() || deviceInstance.empty() )
    {
        assert ( 0 );
        return false;
    }
    if ( vendor.size() > 8 )
    {
        Logger::get ( "xplsdk.comms" ).warning("Invalid xPl vendor: " + vendor);
    }
    m_target.vendor = vendor;

    string device;
    string instance;
    StringSplit ( deviceInstance, '.', &device, &instance );

    if ( ( device.size() > 8 ) || device.empty() || ( instance.size() > 16 ) || instance.empty() )
    {
        Logger::get ( "xplsdk.comms" ).warning("Invalid xPl dest: " + device);
        assert ( 0 );
        return false;
    }
    m_target.device = toLower(device);
    m_target.instance = toLower(instance);
    return true;
}

bool XplMsg::SetTarget
(
    XPLAddress const& _target
)
{
    InvalidateRawData();
    m_target = _target;

}


/***************************************************************************
****																	****
****	XplMsg::SetSchemaClass											****
****																	****
***************************************************************************/

bool XplMsg::SetSchemaClass
(
    string const& _schemaClass
)
{
    InvalidateRawData();

    // Schema class cannot exceed 8 characters
    if ( _schemaClass.empty() || ( _schemaClass.size() > 8 ) )
    {
        assert ( 0 );
        return false;
    }

    m_schemaClass = toLower ( _schemaClass );
    return true;
}


/***************************************************************************
****																	****
****	XplMsg::SetSchemaType											****
****																	****
***************************************************************************/

bool XplMsg::SetSchemaType
(
    string const& _schemaType
)
{
    InvalidateRawData();

    // Schema type cannot exceed 8 characters
    if ( _schemaType.empty() || ( _schemaType.size() > 8 ) )
    {
        assert ( 0 );
        return false;
    }

    m_schemaType = toLower ( _schemaType );
    return true;
}


/***************************************************************************
****																	****
****	XplMsg::ReadNameValuePair										****
****																	****
***************************************************************************/

int32 XplMsg::ReadNameValuePair
(
    string const& _str,
    int32 const _start,
    string* _name,
    string* _value
)
{
    // Scan the string, noting the positions of the = and \n characters
    string line;
    int32 pos = StringReadLine ( _str, _start, &line );

    if ( !StringSplit ( line, '=', _name, _value ) )
    {
        // We didn't find an = sign, so we return the whole line in name.
        // The line is probably a closing brace - by returning it
        // we allow the caller to see if that is the case.
        *_name = line;
    }

    return ( pos );
}


/***************************************************************************
****																	****
****	XplMsg::InvalidateRawData										****
****																	****
***************************************************************************/

void XplMsg::InvalidateRawData()
{
    m_raw = "";
}


/***************************************************************************
****																	****
****	XplMsg::operator ==												****
****																	****
***************************************************************************/

bool XplMsg::operator ==
(
    XplMsg const& _rhs
)
{
    string pMsgDataLHS = GetRawData();

    // I know this is pure evil, but all we are doing is filling the
    // raw data cache, and we do not modify the actual xPL message.

    XplMsg* pNonConstRHS = const_cast<XplMsg*> ( &_rhs );

    string pMsgDataRHS = pNonConstRHS->GetRawData( );

    return ( pMsgDataLHS == pMsgDataRHS );
}





