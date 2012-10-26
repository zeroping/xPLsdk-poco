/***************************************************************************
****																	****
****	RegUtils.cpp													****
****																	****
****	Utility functions to aid registry access.						****
****																	****
****	Copyright (c) 2007 Mal Lansell.									****
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

#include <Windows.h>
#include "xplCore.h"
#include "RegUtils.h"
#include "EventLog.h"

using namespace xpl;


/***************************************************************************
****																	****
****	RegOpen															****
****																	****
***************************************************************************/

bool xpl::RegOpen
(
    HKEY _root,
    string const& _path,
    HKEY* _pKey
)
{
    HKEY hKey;
    if ( ERROR_SUCCESS != RegOpenKeyEx ( _root, TEXT ( _path.c_str() ), 0, KEY_READ, &hKey ) )
    {
        if ( EventLog* pEventLog = EventLog::Get() )
        {
            char msg[MAX_PATH];
            sprintf ( msg, "Failed to open registry key: %s", _path.c_str() );
            pEventLog->ReportError ( msg );
        }
        return false;
    }

    if ( _pKey )
    {
        *_pKey = hKey;
    }

    return true;
}


/***************************************************************************
****																	****
****	RegClose														****
****																	****
***************************************************************************/

bool xpl::RegClose
(
    HKEY _key
)
{
    RegCloseKey ( _key );
    return true;
}


/***************************************************************************
****																	****
****	RegRead (uint32)												****
****																	****
***************************************************************************/

bool xpl::RegRead
(
    HKEY _key,
    string const& _name,
    uint32* _val
)
{
    DWORD type;
    DWORD size;
    DWORD val;

    size = sizeof ( DWORD );
    if ( ( ERROR_SUCCESS != RegQueryValueEx ( _key, TEXT ( _name.c_str() ), NULL, &type, ( BYTE* ) &val, &size ) ) || ( REG_DWORD != type ) )
    {
        if ( EventLog* pEventLog = EventLog::Get() )
        {
            char msg[MAX_PATH];
            sprintf ( msg, "Failed to read registry value: %s", _name.c_str() );
            pEventLog->ReportError ( msg );
        }
        return false;
    }

    if ( _val )
    {
        *_val = val;
        return true;
    }

    return false;
}


/***************************************************************************
****																	****
****	RegRead (bool)													****
****																	****
***************************************************************************/

bool xpl::RegRead
(
    HKEY _key,
    string const& _name,
    bool* _bState
)
{
    uint32 val;
    if ( RegRead ( _key, _name, &val ) )
    {
        if ( _bState )
        {
            *_bState = ( val!=0 );
            return true;
        }
        assert ( 0 );
    }

    return false;
}


/***************************************************************************
****																	****
****	RegRead (string)												****
****																	****
***************************************************************************/

bool xpl::RegRead
(
    HKEY _key,
    string const& _name,
    string* _str
)
{
    bool bRes = false;
    char* pBuffer = NULL;
    do
    {
        DWORD bufferSize;
        if ( ERROR_SUCCESS != RegQueryValueEx ( _key, TEXT ( _name.c_str() ), NULL, NULL, NULL, &bufferSize ) )
        {
            break;
        }

        char* pBuffer = new char[bufferSize];
        DWORD type;
        if ( ERROR_SUCCESS != RegQueryValueEx ( _key, TEXT ( _name.c_str() ), NULL, &type, ( unsigned char* ) pBuffer, &bufferSize ) )
        {
            break;
        }

        if ( REG_SZ != type )
        {
            break;
        }

        if ( _str )
        {
            *_str = pBuffer;
            bRes = true;
        }

    }
    while ( 0 );

    delete [] pBuffer;
    return bRes;
}

