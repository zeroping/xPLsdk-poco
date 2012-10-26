/***************************************************************************
****																	****
****	SensorManager.h													****
****																	****
****	Copyright (c) 2006 Mal Lansell.									****
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

#ifndef _SENSORMANAGER_H
#define _SENSORMANAGER_H

#include <string>
#include <map>
#include <xplCore.h>
#include <xplMsg.h>

using namespace xpl;

class Sensor;

class SensorManager
{
public:
    static SensorManager* Create()
    {
        assert ( NULL == s_pInstance );
        s_pInstance = new SensorManager();
        return s_pInstance;
    }

    static SensorManager* Get()
    {
        assert ( NULL != s_pInstance );
        return s_pInstance;
    }

    static void Destroy()
    {
        assert ( NULL != s_pInstance );
        delete s_pInstance;
        s_pInstance = NULL;
    }

    xplMsg* ProcessRF ( uint8* _rf, string const& _msgSource );

private:
    SensorManager();
    ~SensorManager();

    bool LoadSensors();
    bool SaveSensors();

    map<uint8,Sensor*>		m_sensorMap;

    static SensorManager*	s_pInstance;
};

#endif
