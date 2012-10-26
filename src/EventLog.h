/***************************************************************************
****																	****
****	EventLog.h														****
****																	****
****	Class to ease the process of writing to the Windows event log	****
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

#pragma once

#ifndef EventLog_H
#define EventLog_H

#include <string>
#include "xplCore.h"
//#include "Windows.h"

namespace xpl
{

/**
 * Helper class to simplify writing to the Windows Event Log
 */
class EventLog
{
public:
// 	/**
// 	 * Create the EventLog singleton.
// 	 * @param _appName the name of the executable (without the .exe).  If this is not right, logging will not work correctly.
// 	 * @param _bRegisterApp whether the application should be added to the event log entries in the regsistry.  Defaults to true.
// 	 * @see Get, Destroy
// 	 */
// 	static void Create( string const& _appName, bool _bRegisterApp = true )
// 	{
// 		assert( !s_pInstance );
// 		if( !s_pInstance )
// 		{
// 			s_pInstance = new EventLog( _appName, _bRegisterApp );
// 		}
// 	}
//
// 	/**
// 	 * Obtain a pointer to the EventLog singleton.  This method should not be called before Create.
// 	 * @return returns a pointer to the EventLog singleton object.
// 	 * @see Create, Destroy
// 	 */
// 	static EventLog* Get()
// 	{
// 		assert( s_pInstance );
// 		return s_pInstance;
// 	}
//
// 	/**
// 	 * Destroys the EventLog singleton object.
// 	 * @see Create, Get
// 	 */
// 	static void Destroy()
// 	{
// 		assert( s_pInstance );
// 		delete s_pInstance;
// 		s_pInstance = NULL;
// 	}

    /**
     * Add an information message to the event log.
     * @param _msg string containing the message text to be added to the log.
     * @return true if the message was successfully recorded in the log.
     * @see ReportWarning, ReportError, ReportFailure
     */
    void ReportInformation ( std::string _msg );

    /**
     * Add a warning message to the event log.
     * @param _msg string containing the message text to be added to the log.
     * @return true if the message was successfully recorded in the log.
     * @see ReportInformation, ReportError, ReportFailure
     */
    bool ReportWarning ( string _msg );

    /**
     * Add an error message to the event log.
     * @param _msg string containing the message text to be added to the log.
     * @return true if the message was successfully recorded in the log.
     * @see ReportInformation, ReportWarning, ReportFailure
     */
    void ReportError ( std::string _msg );

    /**
     * Add a failure message to the event log.
     * @param _msg string containing the message text to be added to the log.
     * @return true if the message was successfully recorded in the log.
     * @see ReportInformation, ReportWarning, ReportError
     */
    void ReportFailure ( std::string _msg );

private:
    EventLog ( string const& _appName, bool _bRegisterApp );

    ~EventLog();



    HANDLE				m_hLog;
    static EventLog*	s_pInstance;
};


} //Namespace xPL

#endif // EventLog_H
