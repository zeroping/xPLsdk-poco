/***************************************************************************
****																	****
****	StringUtils.h													****
****																	****
****	Utility functions for string manipulation						****
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

#ifndef _STRINGUTILS_H
#define _STRINGUTILS_H

#include <string>
//#include <tchar.h> - not in linux
#include <stdio.h>
#include <string.h>
#include "XplCore.h"

namespace xpl
{

/**
 * Reads a line of text from a string.
 * Extracts a line of text (i.e everything from the start position up to the
 * next newline or end of string), and removes any whitespace from both ends.
 * This function also skips empty lines.
 * @param _str string containing all the text.
 * @param _start the index into the string where the line of text starts.
 * @param _pLine pointer to a string that will be filled with the line of text.
 * @return A new start position to use in subsequent calls.
 */
uint32 StringReadLine ( string const& _str, uint32 const _start, string* _pLine );

/**
 * Splits a string into two pieces.
 * Finds the first instance of the specified character and splits the string
 * into the two pieces to the left and right of this character.  The character
 * itself is not included in either string.
 * @param _source string containing the text to be split.
 * @param _delim the delimiting character marking the point where the string
 * should be split.
 * @param _pLeftStr pointer to a string that will be filled with the text to the
 * left of the delimiting character.
 * @param _pRightStr pointer to a string that will be filled with the text to the
 * right of the delimiting character.
 * @return True if the delimiting character was found and the string split in two.
 * Otherwise returns false.
 */
bool StringSplit ( string const& _source, char const _delim, string* _pLeftStr, string* _pRightStr );


} // namespace xpl

#endif // _STRINGUTILS_H
