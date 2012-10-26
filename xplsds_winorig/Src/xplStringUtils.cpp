/***************************************************************************
****																	****
****	StringUtils.cpp													****
****																	****
****	Utility functions to aid string manipulation					****
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

#include <Windows.h>
#include "xplCore.h"
#include "xplStringUtils.h"

using namespace xpl;

string const c_emptyString;


/***************************************************************************
****																	****
****	StringReadLine													****
****																	****
****	Extract a substring from the given start position to the next	****
****	line feed character (or end of string if that comes first)		****
****																	****
***************************************************************************/

uint32 xpl::StringReadLine
(
    string const& _str,
    uint32 const _start,
    string* _pLine
)
{
    // Look for the next linefeed or end of string
    uint32 pos = _start;

    while ( 1 )
    {
        char ch;
        while ( ch = _str[pos++] )
        {
            if ( ( ch == '\n' ) || ( ch == '\r' ) )
            {
                break;
            }
        }

        *_pLine = StringTrim ( _str.substr ( _start, pos- ( _start+1 ) ) );

        if ( !ch )
        {
            // We have reached the end of the data
            // Move the pointer back to the zero terminator.
            pos--;
            break;
        }

        if ( !_pLine->empty() )
        {
            // We have a string
            break;
        }

        // String was empty, so we carry on.
    }

    return ( pos );
}


/***************************************************************************
****																	****
****	StringSplit														****
****																	****
****	Split a string into two around the first occurance of one		****
****	of the character specified in _delim							****
****																	****
***************************************************************************/

bool xpl::StringSplit
(
    string const& _source,
    char const _delim,
    string* _pLeftStr,
    string* _pRightStr
)
{
    uint32 pos = ( uint32 ) _source.find_first_of ( _delim );
    if ( string::npos != pos )
    {
        // Character found
        *_pLeftStr = StringTrim ( _source.substr ( 0, pos ) );
        *_pRightStr = StringTrim ( _source.substr ( pos+1 ) );
        return true;
    }

    // Character not found in string
    return false;
}


/***************************************************************************
****																	****
****	StringTrim														****
****										  							****
****	Trim whitespace from around the string							****
****																	****
***************************************************************************/

string xpl::StringTrim
(
    string const& _str
)
{
    uint32 start = 0;
    uint32 end = ( uint32 ) _str.size();

    char ch;
    while ( ch = _str[start] )
    {
        if ( ( ch != ' ' ) && ( ch != '\t' ) && ( ch != '\n' ) && ( ch != '\r' ) )
        {
            break;
        }
        ++start;
    }

    while ( end-- )
    {
        ch = _str[end];
        if ( ( ch != ' ' ) && ( ch != '\t' ) && ( ch != '\n' ) && ( ch != '\r' ) )
        {
            break;
        }
    }

    if ( end >= start )
    {
        return ( _str.substr ( start, ( end-start ) +1 ) );
    }

    string empty;
    return ( empty );
}


/***************************************************************************
****																	****
****	StringToLower													****
****																	****
***************************************************************************/

string xpl::StringToLower
(
    string const& _str
)
{
    string outStr = _str;
    uint32 index = 0;
    char ch;
    while ( ch = outStr[index] )
    {
        if ( ( ch >= 'A' ) && ( ch <= 'Z' ) )
        {
            outStr[index] = ch + ( 'a'-'A' );
        }
        ++index;
    }

    return ( outStr );
}


/***************************************************************************
****																	****
****	StringToUpper													****
****																	****
***************************************************************************/

string xpl::StringToUpper
(
    string const& _str
)
{
    string outStr = _str;
    uint32 index = 0;
    char ch;
    while ( ch = outStr[index] )
    {
        if ( ( ch >= 'a' ) && ( ch <= 'z' ) )
        {
            outStr[index] = ch - ( 'a'-'A' );
        }
        ++index;
    }

    return ( outStr );
}


/***************************************************************************
****																	****
****	StringFromFloat													****
****																	****
***************************************************************************/

string	xpl::StringFromFloat
(
    float _f,
    uint32 _places,
    bool _bTrim /* = false */
)
{
    if ( _places > 10 )
    {
        _places = 10;
    }

    char format[8];
    sprintf ( format, "%%.%df", _places );

    char str[64];
    sprintf ( str, format, _f );

    if ( _places && _bTrim )
    {
        int pos = ( int ) strlen ( str );
        while ( pos-- )
        {
            if ( str[pos] == '0' )
            {
                str[pos] = 0;
            }
            else if ( str[pos] == '.' )
            {
                str[pos] = 0;
                break;
            }
            else
            {
                break;
            }
        }
    }

    return string ( str );
}


#ifdef UNICODE

/***************************************************************************
****																	****
****	UnicodeString Constructor										****
****																	****
***************************************************************************/

UnicodeString::UnicodeString
(
    char const* _asciiString
)
{
    uint32 length = MultiByteToWideChar ( CP_ACP, MB_PRECOMPOSED, _asciiString, -1, NULL, 0 );
    m_pBuffer = new uint16[length+1];
    MultiByteToWideChar ( CP_ACP, MB_PRECOMPOSED, _asciiString, -1, m_pBuffer, length );
}

#endif // #ifdef UNICODE


