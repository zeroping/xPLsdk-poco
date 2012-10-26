/***************************************************************************
****																	****
****	RegUtils.h														****
****																	****
****	Utility functions for Registry access							****
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

#pragma once

#ifndef _REGUTILS_H
#define _REGUTILS_H

#include <string>
//#include <tchar.h>
#include "xplCore.h"

namespace xpl
{

/**
 * Open a registry key.
 * Opens the specified registry key.
 * @param _root Handle to a predefined top level key, such
 * as HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER.
 * @param _path string containing the path to the key to be opened.
 * @param _pKey pointer to a handle that will be filled with the opened registry key.
 * @return true if the key is successfully opened.
 */
bool RegOpen ( HKEY _root, string const& _path, HKEY* _pKey );

/**
 * Closes a registry key
 * Closes the registry key opened by a call to RegOpen.
 * @param _key Handle to the open key that is to be closed.
 * @return true if the key is successfully closed.
 */
bool RegClose ( HKEY _key );

/**
 * Reads a boolean from the registry.
 * Gets the value of a  from the registry and converts it to a bool.
 * @param _key Handle to an open registry key containing the value to be read.
 * @param _name string containing the name of the item to read.
 * @param _bState pointer to a bool that will be filled with the item's value.
 * @return true if the value is successfully read.
 */
bool RegRead ( HKEY _key, string const& _name, bool* _bState );

/**
 * Reads a uint32 from the registry.
 * Gets the value of a REG_ type registry entry.
 * @param _key Handle to an open registry key containing the value to be read.
 * @param _name string containing the name of the item to read.
 * @param _val pointer to a uint32 that will be filled with the item's value.
 * @return true if the value is successfully read.
 */
bool RegRead ( HKEY _key, string const& _name, uint32* _val );

/**
 * Reads a string from the registry.
 * @param _key Handle to an open registry key containing the value to be read.
 * @param _name string containing the name of the item to read.
 * @param _str pointer to a string that will be filled with the item's value.
 * @return true if the string is successfully read.
 */
bool RegRead ( HKEY _key, string const& _name, string* _str );

} // namespace xpl

#endif // _REGUTILS_H
