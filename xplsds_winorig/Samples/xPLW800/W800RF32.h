/***************************************************************************
****																	****
****	W800RF32.h														****
****																	****
****	X10 RF interface												****
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

#ifndef _W800RF32_H
#define _W800RF32_H

#include <windows.h>
#include <deque>
#include <map>
#include <xplCore.h>
#include <xplMsg.h>

using namespace xpl;

class W800RF32
{
public:
    // RFX Sensor Types
    enum
    {
        RFXSensor_Temperature = 0,
        RFXSensor_Humidity,
        RFXSensor_Pressure,
        RFXSensor_RFU3,
        RFXSensor_TemperaturePlusHalf,
        RFXSensor_RFU5,
        RFXSensor_RFU6,
        RFXSensor_RFU7
    };

    // Create a new W800RF32 object.
    // Returns NULL if any errors occurred during creation.
    static W800RF32* Create ( int32 _comPortIndex, HANDLE _hRxNofity, string const& _msgSource );

    // Clean up and delete a W800RF32 object
    static void Destroy ( W800RF32* _pW800RF32 )
    {
        delete _pW800RF32;
    }

    // Poll to see if any X10 events have been received
    // Returns NULL if no events are waiting to be processed.
    // The calling function is responsible for deleting any returned event.
    virtual xplMsg* Poll ( void );

    static unsigned char c_unitCodeTranslation[16];

    // Thread procedures that read data from the W800RF32 device
    static DWORD WINAPI W800RF32::ReadRFThreadProc ( void* _pArg );
    void W800RF32::ReadRF();

private:
    struct SecurityCode
    {
        SecurityCode ( uint8 const _code, string const& _command, string const& _tamper, string const& _lowBattery, string const& _delay ) :
            m_code ( _code ),
            m_command ( _command ),
            m_tamper ( _tamper ),
            m_lowBattery ( _lowBattery ),
            m_delay ( _delay )
        {
        }

        uint8	m_code;
        string	m_command;
        string	m_tamper;
        string	m_lowBattery;
        string	m_delay;
    };

    struct PCRemoteCode
    {
        PCRemoteCode ( uint8 const _code, string const& _command ) :
            m_code ( _code ),
            m_command ( _command )
        {
        }

        uint8	m_code;
        string	m_command;
    };

    // Constructor.  Only to be called from the static Create method.
    W800RF32 ( HANDLE _hComPort, HANDLE _hRxNotify, string const& _msgSource );


    // Destructor.  Only to be called from the static Destroy method.
    ~W800RF32();

    xplMsg* ProcessSensor ( uint8* _rf, string const& _msgSource );
    xplMsg* ProcessSecurity ( uint8* _rf, string const& _msgSource );
    xplMsg* ProcessPCRemote ( uint8* _rf, string const& _msgSource );
    xplMsg* ProcessX10 ( uint8* _rf, string const& _msgSource );


    // Load the security code mappings from the security.xml file.
    bool LoadSecurityCodes();

    // Load the PC Remote code mappings from the pcremote.xml file.
    bool LoadPCRemoteCodes();

    // Helpers for X10 codes
    string const& GetHouseCodeString ( uint8 const _houseCode ) const
    {
        return ( c_houseCodes[_houseCode] );
    }

    CRITICAL_SECTION			m_criticalSection;
    deque<xplMsg*>				m_rxEvents;
    HANDLE						m_hComPort;
    HANDLE						m_hThread;
    HANDLE						m_hExitEvent;
    HANDLE						m_hRxNotify;
    string						m_msgSource;
    map<uint8,SecurityCode*>	m_securityMap;
    map<uint8,PCRemoteCode*>	m_pcRemoteMap;

    static string const			c_houseCodes[16];

};


#endif //_W800RF32_H
