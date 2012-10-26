/***************************************************************************
****																	****
****	SensorManager.cpp												****
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

#include <assert.h>
#include <atlbase.h>
#include <comutil.h>
#include <msxml2.h>
#include "SensorManager.h"
#include "xplMsg.h"

SensorManager* SensorManager::s_pInstance = NULL;


/***************************************************************************
****																	****
****	Sensor class													****
****																	****
***************************************************************************/

class Sensor
{
public:
    enum
    {
        Type_Temperature = 0,
        Type_Humidity,
        Type_Pressure,
        Type_RFU011,
        Type_TemperaturePlusHalf,
        Type_RFU101,
        Type_RFU110,
        Type_RFU111,
        Type_Generic
    };

    Sensor ( uint8 const _unitCode, string const& _name, uint8 const _sensorType, float const _value ) :
        m_unitCode ( _unitCode ),
        m_name ( _name ),
        m_type ( _sensorType ),
        m_current ( _value ),
        m_high ( _value ),
        m_low ( _value )
    {
        // If no name is provided, create one
        if ( m_name.empty() )
        {
            char str[16];
            sprintf ( str, "Unit_%d", ( int ) m_unitCode );
            m_name = str;
        }
    }

    ~Sensor() {}

    void SetCurrent ( float const _current )
    {
        m_current = _current;
        if ( _current > m_high )
        {
            m_high = _current;
        }
        if ( _current < m_low )
        {
            m_low = _current;
        }
    }

    uint8 GetUnitCode() const
    {
        return m_unitCode;
    }
    uint8 GetType() const
    {
        return m_type;
    }
    string GetName() const
    {
        return m_name;
    }
    float GetCurrent() const
    {
        return m_current;
    }
    float GetHigh() const
    {
        return m_high;
    }
    float GetLow() const
    {
        return m_low;
    }

private:
    string	m_name;
    uint8	m_unitCode;
    uint8	m_type;
    float	m_current;
    float	m_high;
    float	m_low;
};


/***************************************************************************
****																	****
****	SensorManager Constructor										****
****																	****
***************************************************************************/

SensorManager::SensorManager()
{
    LoadSensors();
}


/***************************************************************************
****																	****
****	SensorManager Destructor										****
****																	****
***************************************************************************/

SensorManager::~SensorManager()
{
    // Save the sensor map
    SaveSensors();

    // Clear the sensor map
    map<uint8,Sensor*>::iterator iter = m_sensorMap.begin();
    while ( iter != m_sensorMap.end() )
    {
        delete iter->second;
        iter = m_sensorMap.erase ( iter );
    }
}


/***************************************************************************
****																	****
****	SensorManager::ProcessRF										****
****																	****
***************************************************************************/

xplMsg* SensorManager::ProcessRF
(
    uint8* _rf,
    string const& _msgSource
)
{
    if ( ( _rf[0] ^ _rf[1] ) != 0xf0 )
    {
        // Not a sensor message
        return NULL;
    }

    uint8 parity = ( ( _rf[0]>>4 )
                     + ( _rf[0]&0x0f )
                     + ( _rf[1]>>4 )
                     + ( _rf[1]&0x0f )
                     + ( _rf[2]>>4 )
                     + ( _rf[2]&0x0f )
                     + ( _rf[3]>>4 ) ) & 0x0f;

    if ( parity != ( ( ~_rf[3] ) & 0x0f ) )
    {
        // Parity error
        return NULL;
    }

    if ( _rf[3] & 0x10 )
    {
        // Message is an info or error code
        // Construct a log.basic message
        xplMsg* pMsg = xplMsg::Create ( xplMsg::c_xplTrig, _msgSource , "*", "log", "basic" );
        string type = "err";
        string code = "";

        string errorStr = "RF Sensor ";
        map<uint8,Sensor*>::iterator iter = m_sensorMap.find ( _rf[0] );
        if ( iter == m_sensorMap.end() )
        {
            char str[16];
            sprintf ( str, "Unit_%d", ( int ) _rf[0] );
            errorStr += str;
        }
        else
        {
            Sensor* pSensor = iter->second;
            errorStr += pSensor->GetName();
        }

        switch ( _rf[2] )
        {
        case 0x01:
        {
            errorStr += ": Sensor addresses incremented";
            code = "1";
            type = "wrn";
            break;
        }
        case 0x02:
        {
            errorStr += ": Low voltage detected";
            code = "2";
            type = "wrn";
            break;
        }
        case 0x81:
        {
            errorStr += ": No 1-Wire device connected";
            code = "129";
            break;
        }
        case 0x82:
        {
            errorStr += ": 1-Wire ROM CRC error";
            code = "130";
            break;
        }
        case 0x83:
        {
            errorStr += ": 1-Wire device connected is not a DS1820";
            code = "131";
            break;
        }
        case 0x84:
        {
            errorStr += ": No end of read signal received from 1-Wire device";
            code = "132";
            break;
        }
        case 0x85:
        {
            errorStr += ": 1-Wire scratchpad CRC error";
            code = "133";
            break;
        }
        default:
        {
            errorStr += ": Undefined error";
            char str[16];
            sprintf ( str, "%d", ( int ) _rf[2] );
            code = str;
            break;
        }
        }

        pMsg->AddValue ( "type", type );
        pMsg->AddValue ( "text", errorStr );
        pMsg->AddValue ( "code", code );
        return ( pMsg );
    }


    // Message is a measurement
    string typeStr = "generic";
    float fCurrent = ( float ) _rf[2];
    uint8 sensorType = _rf[3] >> 5;
    if ( Sensor::Type_Temperature == sensorType )
    {
        typeStr = "temp";
    }
    if ( Sensor::Type_TemperaturePlusHalf == sensorType )
    {
        fCurrent += 0.5f;
        sensorType = Sensor::Type_Temperature;
        typeStr = "temp";
    }
    else if ( Sensor::Type_Humidity == sensorType )
    {
        typeStr = "humidity";
    }
    else if ( Sensor::Type_Pressure == sensorType )
    {
        typeStr = "pressure";
    }

    // Get the sensor details
    Sensor* pSensor = NULL;
    map<uint8,Sensor*>::iterator iter = m_sensorMap.find ( _rf[0] );
    if ( iter == m_sensorMap.end() )
    {
        // No entry for this sensor yet, so create a new one
        pSensor = new Sensor ( _rf[0], "", sensorType, fCurrent );
        m_sensorMap[_rf[0]] = pSensor;
    }
    else
    {
        pSensor = iter->second;
        pSensor->SetCurrent ( fCurrent );
    }

    // Send sensor.basic
    xplMsg* pMsg = xplMsg::Create ( xplMsg::c_xplTrig, _msgSource , "*", "sensor", "basic" );
    pMsg->AddValue ( "device", pSensor->GetName() );
    pMsg->AddValue ( "type", typeStr );

    char valueStr[32];
    sprintf ( valueStr, "%.1f", pSensor->GetCurrent() );
    pMsg->AddValue ( "current", valueStr );

    sprintf ( valueStr, "%.1f", pSensor->GetLow() );
    pMsg->AddValue ( "lowest", valueStr );

    sprintf ( valueStr, "%.1f", pSensor->GetHigh() );
    pMsg->AddValue ( "highest", valueStr );

    return pMsg;
}


/***************************************************************************
****																	****
****	SensorManager::LoadSensors										****
****																	****
***************************************************************************/

bool SensorManager::LoadSensors()
{
    HRESULT hr;
    IXMLDOMDocument* pXMLDoc = NULL;
    bool retVal = false;

    while ( 1 )
    {
        CoInitialize ( NULL );

        if ( S_OK != CoCreateInstance ( CLSID_DOMDocument30, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, ( void** ) &pXMLDoc ) )
        {
            break;
        }

        CComVariant filename;
        VARIANT_BOOL bSuccess;
        char path[MAX_PATH];
        ::GetCurrentDirectory ( MAX_PATH, path );
        strcat ( path, "\\sensors.xml" );
        filename = path;

        if ( S_OK != pXMLDoc->load ( filename, &bSuccess ) )
        {
            break;
        }

        IXMLDOMElement* pXMLElement;
        hr = pXMLDoc->get_documentElement ( &pXMLElement );

        IXMLDOMNode* pNode;
        pXMLElement->get_firstChild ( &pNode );

        while ( pNode )
        {
            string name;
            uint8 unitCode = 0;
            uint8 sensorType = Sensor::Type_Generic;
            float low = 0.0f;
            float high = 0.0f;

            // Read the entry
            IXMLDOMNode* pItem;
            if ( S_OK == pNode->selectSingleNode ( L"UnitCode", &pItem ) )
            {
                CComBSTR text;
                pItem->get_text ( &text );
                unitCode = ( uint8 ) wcstol ( text, NULL, 0 );
                pItem->Release();
            }

            if ( S_OK == pNode->selectSingleNode ( L"Name", &pItem ) )
            {
                CComBSTR text;
                pItem->get_text ( &text );
                int length = WideCharToMultiByte ( CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL );
                char* str = new char[length+1];
                WideCharToMultiByte ( CP_UTF8, 0, text, -1, str, length, NULL, NULL );
                name = str;
                delete [] str;
                pItem->Release();
            }

            if ( S_OK == pNode->selectSingleNode ( L"Type", &pItem ) )
            {
                CComBSTR text;
                pItem->get_text ( &text );

                if ( text == L"Temperature" )
                {
                    sensorType = Sensor::Type_Temperature;
                }
                else if ( text == L"Humidity" )
                {
                    sensorType = Sensor::Type_Humidity;
                }
                else if ( text == L"Pressure" )
                {
                    sensorType = Sensor::Type_Pressure;
                }
                pItem->Release();
            }

            if ( S_OK == pNode->selectSingleNode ( L"Low", &pItem ) )
            {
                CComBSTR text;
                pItem->get_text ( &text );
                low = ( float ) wcstod ( text, NULL );
                pItem->Release();
            }

            if ( S_OK == pNode->selectSingleNode ( L"High", &pItem ) )
            {
                CComBSTR text;
                pItem->get_text ( &text );
                high = ( float ) wcstod ( text, NULL );
                pItem->Release();
            }

            Sensor* pSensor = new Sensor ( unitCode, name, sensorType, low );
            pSensor->SetCurrent ( high );
            m_sensorMap[unitCode] = pSensor;

            // Move to the next entry
            IXMLDOMNode* pSibling;
            pNode->get_nextSibling ( &pSibling );
            pNode->Release();
            pNode = pSibling;
        }

        pXMLElement->Release();
        retVal = true;
        break;
    }

    // Cleanup
    if ( pXMLDoc )
    {
        pXMLDoc->Release();
    }

    CoUninitialize();

    return retVal;
}


/***************************************************************************
****																	****
****	SensorManager::SaveSensors										****
****																	****
***************************************************************************/

bool SensorManager::SaveSensors()
{
    char filename[MAX_PATH];
    ::GetCurrentDirectory ( MAX_PATH, filename );
    strcat ( filename, "\\sensors.xml" );

    FILE* fp = fopen ( filename, "w" );
    if ( NULL == fp )
    {
        return false;
    }

    fprintf ( fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" );
    fprintf ( fp, "<SensorList>\n" );

    map<uint8,Sensor*>::iterator iter = m_sensorMap.begin();
    while ( iter != m_sensorMap.end() )
    {
        Sensor* pSensor = iter->second;
        ++iter;

        fprintf ( fp, "<Sensor>\n" );
        fprintf ( fp, "<UnitCode>%d</UnitCode>\n", pSensor->GetUnitCode() );
        fprintf ( fp, "<Name>%s</Name>\n", pSensor->GetName().c_str() );

        switch ( pSensor->GetType() )
        {
        case Sensor::Type_Temperature:
        {
            fprintf ( fp, "<Type>Generic</Type>\n" );
            break;
        }
        case Sensor::Type_Humidity:
        {
            fprintf ( fp, "<Type>Humidity</Type>\n" );
            break;
        }
        case Sensor::Type_Pressure:
        {
            fprintf ( fp, "<Type>Pressure</Type>\n" );
            break;
        }
        default:
        {
            fprintf ( fp, "<Type>Generic</Type>\n" );
        }
        }

        fprintf ( fp, "<Low>%.1f</Low>\n", pSensor->GetLow() );
        fprintf ( fp, "<High>%.1f</High>\n", pSensor->GetHigh() );
        fprintf ( fp, "</Sensor>\n" );
    }

    fprintf ( fp, "</SensorList>\n" );
    fclose ( fp );
    return true;
}



