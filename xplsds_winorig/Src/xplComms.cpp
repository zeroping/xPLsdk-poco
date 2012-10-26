/***************************************************************************
****																	****
****	xplComms.cpp													****
****																	****
****	xPL Communications												****
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
#include "xplComms.h"

using namespace xpl;

uint32 const xplComms::c_msgBufferSize = 2048;


/***************************************************************************
****																	****
****	xplComms::Destroy												****
****																	****
***************************************************************************/

void xplComms::Destroy
(
    xplComms* _pComms
)
{
    if ( NULL == _pComms )
    {
        assert ( 0 );
        return;
    }

    _pComms->Disconnect();
    delete _pComms;
}


/***************************************************************************
****																	****
****	xplComms constructor											****
****																	****
***************************************************************************/

xplComms::xplComms() :
    m_bConnected ( false )
{
    // Create a buffer for temporary storage of received messages
    m_pMsgBuffer = new int8[c_msgBufferSize];
}


/***************************************************************************
****																	****
****	xplComms destructor												****
****																	****
***************************************************************************/

xplComms::~xplComms()
{
    // Delete the message buffer
    delete [] m_pMsgBuffer;
}


/***************************************************************************
****																	****
****	xplComms::Connect												****
****																	****
***************************************************************************/

bool xplComms::Connect()
{
    m_bConnected = true;
    return true;
}


/***************************************************************************
****																	****
****	xplComms::Disconnect											****
****																	****
***************************************************************************/

void xplComms::Disconnect()
{
    m_bConnected = false;
}


