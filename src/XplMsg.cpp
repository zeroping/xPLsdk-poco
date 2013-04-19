/***************************************************************************
****																	****
****	xplMsg.cpp														****
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

#include "xplCore.h"
#include "xplStringUtils.h"
#include "xplMsg.h"
#include <iostream>
#include <sstream>

using namespace xpl;

string const xplMsg::c_xplCmnd = "xpl-cmnd";
string const xplMsg::c_xplTrig = "xpl-trig";
string const xplMsg::c_xplStat = "xpl-stat";
string const xplMsg::c_xplHop = "hop";
string const xplMsg::c_xplSource = "source";
string const xplMsg::c_xplTarget = "target";
string const xplMsg::c_xplOpenBrace = "{";
string const xplMsg::c_xplCloseBrace = "}";
string const xplMsg::c_xplTargetAll = "*";


/***************************************************************************
****																	****
****	xplMsg Constructor												****
****																	****
***************************************************************************/

xplMsg::xplMsg() :
    m_hop ( 1 ),
    m_refCount ( 1 )
{
}

xplMsg::xplMsg (
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

xplMsg::xplMsg ( string str )
{

    ParseFromString ( str );

}




/***************************************************************************
****																	****
****	xplMsg Destructor												****
****																	****
***************************************************************************/

xplMsg::~xplMsg()
{
    vector<xplMsgItem*>::iterator iter = m_msgItems.begin();
    while ( iter != m_msgItems.end() )
    {
        delete ( *iter );
        iter = m_msgItems.erase ( iter );
    }

}


/***************************************************************************
****																	****
****	xplMsg::Create													****
****																	****
***************************************************************************/

void xplMsg::ParseFromString ( string str )
{
    // Read the message type
    string line;
    int32 pos = StringReadLine ( str, 0, &line );
    SetType ( line );

    // Skip the opening brace
    pos = StringReadLine ( str, pos, &line );
    if ( c_xplOpenBrace != line )
    {
        // Opening brace not found
        goto xplMsgCreateFailed;
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
            // Invalid header item
            goto xplMsgCreateFailed;
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
        // Opening brace not found
        goto xplMsgCreateFailed;
    }

    // Read the name-value pairs
    while ( 1 )
    {
        string name;
        string value;

        pos = ReadNameValuePair ( str, pos, &name, &value );

        if ( name.empty() )
        {
            // Reached the end of the data without hitting a
            // closing brace.  The message is malformed.
            goto xplMsgCreateFailed;
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

    // Error occurred reading data
xplMsgCreateFailed:
    cout << "ERROR: failed to parse message!\n";

}




/***************************************************************************
****																	****
****	xplMsg::GetAsRawData											****
****																	****
***************************************************************************/

string xplMsg::GetRawData( )
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


// 		sprintf( m_pBuffer,
// 				"%s\n{\nhop=%d\nsource=%s\ntarget=%s\n}\n%s.%s\n{\n",
// 				m_type.c_str(),
// 				m_hop,
// 				m_source.c_str(),
// 				m_target.c_str(),
// 				m_schemaClass.c_str(),
// 				m_schemaType.c_str()
// 				);

        for ( std::vector< xplMsgItem* >::iterator iter = m_msgItems.begin(); iter != m_msgItems.end(); ++iter )
        {
            xplMsgItem const* pItem = *iter;
            for ( int i=0; i<pItem->GetNumValues(); ++i )
            {
                //sprintf( &m_pBuffer[strlen(m_pBuffer)], "%s=%s\n", pItem->GetName().c_str(), pItem->GetValue( i ).c_str() );
                msgstream << pItem->GetName() << "=" << pItem->GetValue ( i ) << "\n";
            }
        }

        // 		strcat( m_pBuffer, "}\n" );
        msgstream <<"}\n";
        m_raw = msgstream.str();;
    }

    return m_raw;
}


/***************************************************************************
****																	****
****	xplMsg::AddValue												****
****																	****
***************************************************************************/

void xplMsg::AddValue
(
    string const& _name,
    string const& _value,
    char const _delimiter /* = ',' */
)
{
    InvalidateRawData();

    xplMsgItem* pItem;

    // See if there is already an existing xplMsgItem with this name
    string lowerName = StringToLower ( _name );
    vector<xplMsgItem*>::iterator iter;
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
        // Create a new xplMsgItem
        pItem = new xplMsgItem ( lowerName );
        m_msgItems.push_back ( pItem );
    }

    pItem->AddValue ( _value, _delimiter );
}


/***************************************************************************
****																	****
****	xplMsg::AddValue												****
****																	****
***************************************************************************/

void xplMsg::AddValue
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
****	xplMsg::SetValue												****
****																	****
***************************************************************************/

bool xplMsg::SetValue
(
    string const& _name,
    string const& _value,
    uint32 const _index
)
{
    InvalidateRawData();

    // Find the xplMsgItem with this name
    for ( vector<xplMsgItem*>::iterator iter = m_msgItems.begin();  iter != m_msgItems.end(); ++iter )
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
****	xplMsg::GetMsgItem												****
****																	****
***************************************************************************/

xplMsgItem const* xplMsg::GetMsgItem
(
    string const& _name
) const
{
    string lowerName = StringToLower ( _name );
    for ( vector<xplMsgItem*>::const_iterator iter = m_msgItems.begin();  iter != m_msgItems.end(); ++iter )
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
****	xplMsg::GetMsgItem												****
****																	****
***************************************************************************/

xplMsgItem const* xplMsg::GetMsgItem
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
****	xplMsg::GetValue												****
****																	****
***************************************************************************/

string const xplMsg::GetValue
(
    string const& _name,
    uint32 const _index /*=0*/
) const
{
    if ( xplMsgItem const* pMsgItem = GetMsgItem ( _name ) )
    {
        // Get the value from the  xplMsgItem object
        return ( pMsgItem->GetValue ( _index ) );
    }

    // No entry found for this name
    return ( string ( "" ) );
}


/***************************************************************************
****																	****
****	xplMsg::GetIntValue												****
****																	****
***************************************************************************/

int const xplMsg::GetIntValue
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
****	xplMsg::GetCompleteValue										****
****																	****
***************************************************************************/

string const xplMsg::GetCompleteValue
(
    string const& _name,
    char const _delimiter /* = ',' */
) const
{
    string str;
    if ( xplMsgItem const* pMsgItem = GetMsgItem ( _name ) )
    {
        // Get the values from the xplMsgItem object
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
****	xplMsg::SetHop													****
****																	****
***************************************************************************/

bool xplMsg::SetHop
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
****	xplMsg::SetType													****
****																	****
***************************************************************************/

bool xplMsg::SetType
(
    string const& _type
)
{
    InvalidateRawData();

    string lowerType = StringToLower ( _type );

    //in case we've forgotten it...
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
****	xplMsg::SetSource												****
****																	****
***************************************************************************/

bool xplMsg::SetSource
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
        cout << "failing on source: " << _source << "\n";
        assert ( 0 );
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

    m_source.device = device;
    m_source.instance = instance;
    //TODO lowercaser
    return true;
}

bool xplMsg::SetSource
(
    XPLAddress const& _source
)
{
    InvalidateRawData();
    m_source = _source;

}


/***************************************************************************
****																	****
****	xplMsg::SetTarget												****
****																	****
***************************************************************************/

bool xplMsg::SetTarget
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
        cout << "Vendor: " << vendor << " is out of spec\n";
    }
    m_target.vendor = vendor;

    string device;
    string instance;
    StringSplit ( deviceInstance, '.', &device, &instance );

    if ( ( device.size() > 8 ) || device.empty() || ( instance.size() > 16 ) || instance.empty() )
    {
        assert ( 0 );
        return false;
    }
    m_target.device = device;
    m_target.instance = instance;
    return true;
    //TODO lowercaser
}

bool xplMsg::SetTarget
(
    XPLAddress const& _target
)
{
    InvalidateRawData();
    m_target = _target;

}


/***************************************************************************
****																	****
****	xplMsg::SetSchemaClass											****
****																	****
***************************************************************************/

bool xplMsg::SetSchemaClass
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

    m_schemaClass = StringToLower ( _schemaClass );
    return true;
}


/***************************************************************************
****																	****
****	xplMsg::SetSchemaType											****
****																	****
***************************************************************************/

bool xplMsg::SetSchemaType
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

    m_schemaType = StringToLower ( _schemaType );
    return true;
}


/***************************************************************************
****																	****
****	xplMsg::ReadNameValuePair										****
****																	****
***************************************************************************/

int32 xplMsg::ReadNameValuePair
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
****	xplMsg::InvalidateRawData										****
****																	****
***************************************************************************/

void xplMsg::InvalidateRawData()
{
    m_raw = "";
}


/***************************************************************************
****																	****
****	xplMsg::operator ==												****
****																	****
***************************************************************************/

bool xplMsg::operator ==
(
    xplMsg const& _rhs
)
{
    string pMsgDataLHS = GetRawData();

    // I know this is pure evil, but all we are doing is filling the
    // raw data cache, and we do not modify the actual xPL message.

    xplMsg* pNonConstRHS = const_cast<xplMsg*> ( &_rhs );

    string pMsgDataRHS = pNonConstRHS->GetRawData( );

    return ( pMsgDataLHS == pMsgDataRHS );
}





