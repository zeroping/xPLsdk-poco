/***************************************************************************
****																	****
****	XMLUtils.h														****
****																	****
****	Utility functions for XML parsing.								****
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

#ifndef _XMLUTILS_H
#define _XMLUTILS_H

#include <atlbase.h>
#include <string>
#include <xplCore.h>
#include <xplMsg.h>

#include "msxml2.h"

namespace XMLUtils
{
using namespace xpl;

/**
 * Get the value of a single named attribute.
 * @param _pNode pointer to the XML node whose attribute is to be read.
 * @param _attributeName name of the attribute to read.
 * @return A string containing the attribute value.  The string will be empty if the attribute cannot be found.
 */
string GetAttributeValue ( IXMLDOMNode* _pNode, CComBSTR _attributeName );

/**
 * Set the value of a single named attribute.
 * @param _pNode pointer to the XML node whose attribute is to be set.
 * @param _attributeName name of the attribute to set.
 * @param _value value to be assigned to the attribute.
 * @return true if the attribute value was successfully changed.
 */
bool SetAttributeValue ( IXMLDOMNode* _pNode, CComBSTR _attributeName, CComBSTR _value );

/**
 * Get the text from an XML element.
 * @param _pRoot pointer to the XML node from where the path will start.
 * @param _path path to the element to be read.
 * @return A string containing the element text.  The string will be empty if the element cannot be found.
 */
string GetElementValue ( IXMLDOMNode* _pRoot, CComBSTR _path );

/**
 * Set the text of an XML element.
 * @param _pRoot pointer to the XML node from where the path will start.
 * @param _path path to the element to be set.
 * @param _value text to be assigned to the element
 * @return true if the element text was successfully changed.
 */
bool SetElementValue ( IXMLDOMNode* _pRoot, CComBSTR _path, CComBSTR _value );

/**
 * Add a new element to an XML document.  The new element will be created and added to the document.
 * @param _pXMLDoc XML document that the element will be added to.
 * @param _pParent XML node that the new element will be a child of.
 * @param _elementName name to be used for the new element.
 * @param _elementValue string to be used for the new element's text
 * @return pointer to the newly created element node.
 */
IXMLDOMNode* AddElement ( IXMLDOMDocument* _pXMLDoc, IXMLDOMNode* _pParent, CComBSTR _elementName, CComBSTR _elementValue );

/**
 * Add a new attribute an XML node.
 * @param _pXMLDoc XML document containing the node that the attribute will be added to.
 * @param _pNode XML node that the new eattribute will be added to.
 * @param _attributeName name to be used for the new attribute.  The name must be unique among the node's attributes.
 * @param _attributeValue text to be used for the new attribute's value.
 * @return true if the new attribute was successfully created.
 */
bool AddAttribute ( IXMLDOMDocument* _pXMLDoc, IXMLDOMNode* _pNode, CComBSTR _attributeName, CComBSTR _attributeValue );

/**
 * Helper function to convert an xPL message into an XML representation.
 * @param _pXMLDoc XML document that will contain the new XML fragment.
 * @param _pParent XML node under which the new XML fragment will be added.
 * @param _pMsg pointer to the xPL message to be converted to XML.
 * @return true if the xPL message was successfully converted to XML.
 * @see ReadMsg
 */
bool WriteMsg ( IXMLDOMDocument* _pXMLDoc, IXMLDOMNode* _pParent, xpl::xplMsg* _pMsg );

/**
 * Helper function to convert an XML fragment into an xPL message.
 * @param _pRoot the head of the XML fragment describing the xPL message.
 * @return pointer to an xPL messsage, or NULL if unsuccessful.  The caller is responsible for deleting any return object through a call to xplMsg::Release.
 * @see WriteMsg
 */
xpl::xplMsg* ReadMsg ( IXMLDOMNode* _pRoot );

} // namespace XMLUtils

#endif // _XMLUTILS_H
