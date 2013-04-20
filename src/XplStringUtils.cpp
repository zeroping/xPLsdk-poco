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

//#include <Windows.h>
#include "XplCore.h"
#include "XplStringUtils.h"
#include <Poco/String.h>

using Poco::trim;
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

        *_pLine = trim ( _str.substr ( _start, pos- ( _start+1 ) ) );

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
        *_pLeftStr = trim ( _source.substr ( 0, pos ) );
        *_pRightStr = trim ( _source.substr ( pos+1 ) );
        return true;
    }

    // Character not found in string
    return false;
}




