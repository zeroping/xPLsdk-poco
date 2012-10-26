/***************************************************************************
****																	****
****	W800.h															****
****																	****
****	xpl-enables the W800RF32 X10 rf receiver						****
****																	****
****	Copyright (c) 2005 Mal Lansell.									****
****																	****
****	SOFTWARE NOTICE AND LICENSE										****
****																	****
****	This work (including software, documents, or other related		****
****	items) is being provided by the copyright holders under the		****
****	following license. By obtaining, using and/or copying this		****
****	work, you (the licensee) agree that you have read, understood,	****
****	and will comply with the following terms and conditions:		****
****																	****
****	Permission to use, copy, and distribute this software and its	****
****	documentation, without modification, for any purpose and		****
****	without fee or royalty is hereby granted, provided that you		****
****	include the full text of this NOTICE on ALL copies of the		****
****	software and documentation or portions thereof.					****
****																	****
****	THIS SOFTWARE AND DOCUMENTATION IS PROVIDED "AS IS," AND		****
****	COPYRIGHT HOLDERS MAKE NO REPRESENTATIONS OR WARRANTIES,		****
****	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO, WARRANTIES OF	****
****	MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT	****
****	THE USE OF THE SOFTWARE OR DOCUMENTATION WILL NOT INFRINGE ANY	****
****	THIRD PARTY PATENTS, COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS.	****
****																	****
****	COPYRIGHT HOLDERS WILL NOT BE LIABLE FOR ANY DIRECT, INDIRECT,	****
****	SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF ANY USE OF THE	****
****	SOFTWARE OR DOCUMENTATION.										****
****																	****
****	The name and trademarks of copyright holders may NOT be used in	****
****	advertising or publicity pertaining to the software without		****
****	specific, written prior permission.  Title to copyright in this ****
****	software and any associated documentation will at all times		****
****	remain with copyright holders.									****
****																	****
***************************************************************************/

#pragma once

#ifndef _W800_H
#define _W800_H

#include "W800RF32.h"

using namespace xpl;

class W800
{
public:
    W800();
    virtual ~W800();

    static void MainProc ( HANDLE _hActive, HANDLE _hExit, void* pContext );

private:
    void Run ( HANDLE _hActive, HANDLE _hExit );

    // Try to handle any received messages.
    void HandleMessages ( void );

    // Set the device parameters according to the contents of the config items
    void Configure();

    xplDevice*			m_pDevice;
    xplComms*			m_pComms;
    W800RF32*			m_pW800;
    xplMsg*				m_pMsg;
    bool				m_bBlockDuplicates;
    int64				m_nextMessageTime;
    int32				m_comPort;
    uint32				m_timeout;
    HANDLE				m_hRxNotify;
    static string const	c_version;
};

#endif //_W800_H

