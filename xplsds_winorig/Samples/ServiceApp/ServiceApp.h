/***************************************************************************
****																	****
****	ServiceApp.h      									            ****
****																	****
****	Example code demonstrating how to build an xPL Windows Service. ****
****																	****
****	Copyright (c) 2005 Mal Lansell.									****
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

#ifndef _SERVICEAPP_H
#define _SERVICEAPP_H

using namespace xpl;

class ServiceApp
{
public:
    ServiceApp();
    virtual ~ServiceApp();

    static void MainProc ( HANDLE _hActive, HANDLE _hExit, void* pContext );

private:
    void Run ( HANDLE _hActive, HANDLE _hExit );

    // Try to handle any received messages.
    void HandleMessages ( void );

    // Set the device parameters according to the contents of the config items
    void Configure();

    xplDevice*			m_pDevice;
    xplComms*			m_pComms;
};

#endif //_SERVICEAPP_H

