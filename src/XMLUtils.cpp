/***************************************************************************
****																	****
****	XMLUtils.cpp													****
****																	****
****	Utility functions to aid XML parsing.							****
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

#include "XMLUtils.h"

using namespace XMLUtils;


/***************************************************************************
****																	****
****	XMLUtils::GetElementValue										****
****																	****
***************************************************************************/

string XMLUtils::GetElementValue
(
    IXMLDOMNode* _pRoot,
    CComBSTR _path
)
{
    USES_CONVERSION;
    IXMLDOMNode* pElement = NULL;

    do
    {
        if ( S_OK != _pRoot->selectSingleNode ( _path, &pElement ) )
        {
            break;
        }

        CComBSTR text;
        if ( S_OK != pElement->get_text ( &text ) )
        {
            break;
        }

        string value = W2A ( text );
        pElement->Release();
        return value;
    }
    while ( 0 );

    // failed
    if ( pElement )
    {
        pElement->Release();
    }
    return string ( "" );
}


/***************************************************************************
****																	****
****	XMLUtils::SetElementValue										****
****																	****
***************************************************************************/

bool XMLUtils::SetElementValue
(
    IXMLDOMNode* _pRoot,
    CComBSTR _path,
    CComBSTR _value
)
{
    USES_CONVERSION;
    IXMLDOMNode* pElement = NULL;

    do
    {
        if ( S_OK != _pRoot->selectSingleNode ( _path, &pElement ) )
        {
            break;
        }

        if ( S_OK != pElement->put_text ( _value ) )
        {
            break;
        }

        return true;
    }
    while ( 0 );

    // failed
    if ( pElement )
    {
        pElement->Release();
    }
    return false;
}


/***************************************************************************
****																	****
****	XMLUtils::GetAttributeValue										****
****																	****
***************************************************************************/

string XMLUtils::GetAttributeValue
(
    IXMLDOMNode* _pNode,
    CComBSTR _attributeName
)
{
    USES_CONVERSION;

    IXMLDOMNamedNodeMap *pAttributes = NULL;
    IXMLDOMNode* pAttribute = NULL;

    do
    {
        if ( S_OK != _pNode->get_attributes ( &pAttributes ) )
        {
            break;
        }

        if ( S_OK != pAttributes->getNamedItem ( _attributeName, &pAttribute ) )
        {
            break;
        }

        CComBSTR text;
        if ( S_OK != pAttribute->get_text ( &text ) )
        {
            break;
        }

        string value = W2A ( text );
        pAttribute->Release();
        pAttributes->Release();
        return value;
    }
    while ( 0 );

    // failed
    if ( pAttribute )
    {
        pAttribute->Release();
    }
    if ( pAttributes )
    {
        pAttributes->Release();
    }

    return string ( "" );
}


/***************************************************************************
****																	****
****	XMLUtils::SetAttributeValue										****
****																	****
***************************************************************************/

bool XMLUtils::SetAttributeValue
(
    IXMLDOMNode* _pNode,
    CComBSTR _attributeName,
    CComBSTR _value
)
{
    bool bRetVal = false;
    IXMLDOMNamedNodeMap *pAttributes = NULL;
    IXMLDOMNode* pAttribute = NULL;

    do
    {
        if ( S_OK != _pNode->get_attributes ( &pAttributes ) )
        {
            break;
        }

        if ( S_OK != pAttributes->getNamedItem ( _attributeName, &pAttribute ) )
        {
            break;
        }

        if ( S_OK != pAttribute->put_text ( _value ) )
        {
            break;
        }
        bRetVal = true;
    }
    while ( 0 );

    // failed
    if ( pAttribute )
    {
        pAttribute->Release();
    }
    if ( pAttributes )
    {
        pAttributes->Release();
    }

    return bRetVal;
}


/***************************************************************************
****																	****
****	XMLUtils::AddElement											****
****																	****
***************************************************************************/

IXMLDOMNode* XMLUtils::AddElement
(
    IXMLDOMDocument* _pXMLDoc,
    IXMLDOMNode* _pParent,
    CComBSTR _elementName,
    CComBSTR _elementValue
)
{
    IXMLDOMElement* pNode = NULL;
    _pXMLDoc->createElement ( _elementName, &pNode );
    if ( pNode )
    {
        pNode->put_text ( _elementValue );
        IXMLDOMNode* pNewNode = NULL;
        _pParent->appendChild ( pNode, &pNewNode );
        pNewNode->Release();
        return pNode;
    }

    return NULL;
}


/***************************************************************************
****																	****
****	XMLUtils::AddAttribute											****
****																	****
***************************************************************************/

bool XMLUtils::AddAttribute
(
    IXMLDOMDocument* _pXMLDoc,
    IXMLDOMNode* _pNode,
    CComBSTR _attributeName,
    CComBSTR _attributeValue
)
{
    IXMLDOMNamedNodeMap *pAttributes = NULL;
    if ( S_OK != _pNode->get_attributes ( &pAttributes ) )
    {
        return false;
    }

    IXMLDOMAttribute* pNode = NULL;
    _pXMLDoc->createAttribute ( _attributeName, &pNode );
    if ( pNode )
    {

        IXMLDOMNode* pNewNode;
        pAttributes->setNamedItem ( pNode, &pNewNode );
        if ( pNewNode )
        {
            pNewNode->put_text ( _attributeValue );
            return true;
        }
    }

    return false;
}


/***************************************************************************
****																	****
****	XMLUtils::ReadMsg												****
****																	****
***************************************************************************/

xpl::xplMsg* XMLUtils::ReadMsg
(
    IXMLDOMNode* _pRoot
)
{
    // Read the message type
    string type = XMLUtils::GetElementValue ( _pRoot, L"Type" );
    string target = XMLUtils::GetElementValue ( _pRoot, L"Target" );
    string schemaClass = XMLUtils::GetElementValue ( _pRoot, L"SchemaClass" );
    string schemaType = XMLUtils::GetElementValue ( _pRoot, L"SchemaType" );

    // Create the basic message
    xpl::xplMsg* pMsg = xpl::xplMsg::Create ( type, "x-x.x", target, schemaClass, schemaType );
    if ( !pMsg )
    {
        return NULL;
    }

    // Read the body
    IXMLDOMNode* pNode;
    CComBSTR nodeName;

    _pRoot->get_firstChild ( &pNode );
    while ( pNode )
    {
        pNode->get_nodeName ( &nodeName );

        if ( nodeName == L"Item" )
        {
            string name = XMLUtils::GetAttributeValue ( pNode, L"name" );
            string value = XMLUtils::GetAttributeValue ( pNode, L"value" );

            pMsg->AddValue ( name, value );
        }

        // Move to the next type
        IXMLDOMNode* pNextNode;
        pNode->get_nextSibling ( &pNextNode );
        pNode->Release();
        pNode = pNextNode;
    }

    return pMsg;
}


/***************************************************************************
****																	****
****	XMLUtils::WriteMsg												****
****																	****
***************************************************************************/

bool XMLUtils::WriteMsg
(
    IXMLDOMDocument* _pXMLDoc,
    IXMLDOMNode* _pParent,
    xpl::xplMsg* _pMsg
)
{
    IXMLDOMNode* pMsgNode = XMLUtils::AddElement ( _pXMLDoc, _pParent, L"xPLMsg", L"" );
    if ( !pMsgNode )
    {
        return false;
    }

    XMLUtils::AddElement ( _pXMLDoc, pMsgNode, L"Type", _pMsg->GetType().c_str() );
    XMLUtils::AddElement ( _pXMLDoc, pMsgNode, L"Target", _pMsg->GetTarget().c_str() );
    XMLUtils::AddElement ( _pXMLDoc, pMsgNode, L"SchemaClass", _pMsg->GetSchemaClass().c_str() );
    XMLUtils::AddElement ( _pXMLDoc, pMsgNode, L"SchemaType", _pMsg->GetSchemaType().c_str() );

    // Write the body
    for ( int i=0; i<_pMsg->GetNumMsgItems(); ++i )
    {
        xpl::xplMsgItem const* pMsgItem = _pMsg->GetMsgItem ( i );
        string name = pMsgItem->GetName();
        for ( int j=0; j<pMsgItem->GetNumValues(); ++j )
        {
            string value = pMsgItem->GetValue ( j );

            IXMLDOMNode* pItemNode = XMLUtils::AddElement ( _pXMLDoc, pMsgNode, L"Item", "" );
            if ( pItemNode )
            {
                XMLUtils::AddAttribute ( _pXMLDoc, pItemNode, L"name", name.c_str() );
                XMLUtils::AddAttribute ( _pXMLDoc, pItemNode, L"value", value.c_str() );
            }
        }
    }

    return true;
}


