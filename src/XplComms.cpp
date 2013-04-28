/***************************************************************************
****																	****
****	XplComms.cpp													****
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

#include "XplCore.h"
#include "XplComms.h"
#include <iostream>
#include <Poco/Thread.h>


using namespace xpl;

/***************************************************************************
****																	****
****	XplComms::Destroy												****
****																	****
***************************************************************************/
/*
void XplComms::Destroy
(
	XplComms* _pComms
)
{
//     cout << "XplComms::Destroy in thread " << Thread::currentTid() << " \n";
	if( NULL == _pComms )
	{
//     cout << "pcomms already released\n";
		assert( 0 );
		return;
	}
// 	cout << "calling disconnect in thread " << Thread::currentTid() << " \n";
 	_pComms->Disconnect();
//   cout << "deleting pcomms in thread " << Thread::currentTid() << " \n";
	delete _pComms;
//   cout << "deleted pcomms in thread " << Thread::currentTid() << " \n";
}*/


/***************************************************************************
****																	****
****	XplComms constructor											****
****																	****
***************************************************************************/

XplComms::XplComms() :
    connected_ ( false )
{

}


/***************************************************************************
****																	****
****	XplComms destructor												****
****																	****
***************************************************************************/

XplComms::~XplComms()
{

}


/***************************************************************************
****																	****
****	XplComms::Connect												****
****																	****
***************************************************************************/

bool XplComms::Connect()
{
    connected_ = true;
    return true;
}


/***************************************************************************
****																	****
****	XplComms::Disconnect											****
****																	****
***************************************************************************/

void XplComms::Disconnect()
{
    connected_ = false;
}


