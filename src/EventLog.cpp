/***************************************************************************
****																	****
****	EventLog.cpp													****
****																	****
****	Provides support for writing to the Windows event log			****
****																	****
****	Copyright (c) 2007 Mal Lansell.									****
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

#include "EventLog.h"
#include "Poco/Logger.h"

using namespace xpl;

#ifndef EVENT_ELOG
 #define EVENT_ELOG 0x000003E8L
#endif

EventLog* EventLog::s_pInstance = NULL;


/***************************************************************************
****																	****
****	EventLog constructor											****
****																	****
***************************************************************************/

EventLog::EventLog
(
	string const& _appName,
	bool _bRegisterApp
)
{
	m_hLog = RegisterEventSource( NULL, _appName.c_str() );
 
	if( _bRegisterApp )
    {
		HKEY hk; 
        
		// Add your source name as a subkey under the Application 
        // key in the EventLog registry key. 
        string appKey = string( "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" ) + _appName;
        
		if( RegCreateKey( HKEY_LOCAL_MACHINE, appKey.c_str(), &hk ) == ERROR_SUCCESS )
        {
			char exePath[MAX_PATH];
			GetModuleFileName( GetModuleHandle(NULL), exePath, MAX_PATH );
               
            // Add the name to the EventMessageFile subkey. 
            if( ERROR_SUCCESS == RegSetValueEx( hk, "EventMessageFile", 0, REG_EXPAND_SZ, (LPBYTE)exePath, ()(strlen( exePath )+1) ) )
            {
				// Set the supported event types in the TypesSupported subkey. 
                 dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE | EVENTLOG_AUDIT_FAILURE;
                RegSetValueEx( hk, "TypesSupported", 0, REG_, (LPBYTE) &dwData, sizeof() ); 
            }
			
			RegCloseKey( hk ); 
		}
	}
}


/***************************************************************************
****																	****
****	EventLog destructor												****
****																	****
***************************************************************************/

EventLog::~EventLog()
{
     if( m_hLog )
	 {
		 DeregisterEventSource( m_hLog );
	 }
}



/***************************************************************************
****																	****
****	EventLog::ReportFailure											****
****																	****
***************************************************************************/
 
void EventLog::ReportFailure( string _msg )
{
    return poco_fatal(logger, _msg);
}

/***************************************************************************
****																	****
****	EventLog::ReportError											****
****																	****
***************************************************************************/

void EventLog::ReportError( string _msg )
{
    return poco_error(logger, _msg);
}


/***************************************************************************
****																	****
****	EventLog::ReportWarning											****
****																	****
***************************************************************************/

void EventLog::ReportWarning( string _msg )
{
    return poco_warning(logger, _msg);
}


/***************************************************************************
****																	****
****	EventLog::ReportInformation										****
****																	****
***************************************************************************/

void EventLog::ReportInformation( string _msg )
{
    return poco_information(logger, _msg);
}

