/***************************************************************************
****																	****
****	Service.h														****
****																	****
****	Class to ease the creation of Windows Services					****
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

#ifndef _SERVICE_H
#define _SERVICE_H

#include <Windows.h>
#include <Winsvc.h>
#include <string>

#pragma warning(disable:4996)   // Disable warning '... was declared deprecated' - required for .NET 2005

using namespace std;

/**
 * Helper class to encapsulate and simplify the process of creating and managing a Windows %Service
 */
class Service
{
public:
    /**
    * Typedef of the application's main procedure, which is called from
    * this service.  The function should only return when the _hExit
    * event is signalled.
    * @param _hActive handle to an event that is in the signalled state
    * when the application is running.  The application should pause
    * if this enters an unsignalled state.
    * @param _hExit handle to an event that is set to the signalled
    * state when the application should exit.
    * @param _pContext pointer to user defined data to provide a context
    * for the application procedure.
    */
    typedef void ( *pfnAppProc ) ( HANDLE _hActive, HANDLE _hExit, void* _pContext );

    /**
    * Creates the service singleton.
    * @param _serviceName name by which the service will be known.  This is displayed in the service management panel.
    * @param _serviceDescription description of the service's purpose.  This is displayed in the service management panel.
    * @param _pAppProc pointer to the function that will be called when the service is started.
    * @param _pAppProcContext pointer to user defined data to provide a context for the call to _pAppProc.  This will often
    * be a pointer to an instance of the class containing the _pAppProc method.
    * @return returns a pointer to the newly created service singleton object.
    */
    static Service* Create ( string const& _serviceName, string const& _serviceDescription, pfnAppProc _pAppProc, void* _pAppProcContext );

    /**
     * Obtain a pointer to the service singleton.  This method should not be called before Create.
     * @return returns a pointer to the service singleton object.
     * @see Create, Destroy
     */
    static Service* Get()
    {
        return s_pInstance;
    }

    /**
     * Destroys the service singleton object.
     * @see Create, Get
     */
    static void Destroy();

    /**
    * Handles the command line arguments passed to the application.
    * The arguments should be passed in unchanged.
    * The first command line argument is always the name of the executable.  If there are no other arguments,
    * the program will assume that it is running as a console application instead of as a service.  This allows
    * it program to run on non-NT versions of Windows such as Win98, and is also handy for debugging.
    * If there is one other argument, it must be either /Install, /Uninstall or /Run.
    * <p>/Install causes the service to be installed and started.
    * <p>/Uninstall causes the service to be stopped and removed.
    * <p>/Run causes the service to start normally.  If it is unable to start as a service, then it will run as a console application instead.
    * @param _argc the number of arguments being passed to the application.  This should be the same as the argc parameter passed into main().
    * @param _argv the array of command line arguments.  This must be the same as the argv parameter passed into main().
    * @return zero if the program exited without errors.
    */
    virtual int ProcessCommandLine ( int _argc, char* _argv[] );

private:
    typedef BOOL ( WINAPI *ChangeServiceConfigType ) ( SC_HANDLE, DWORD, LPCVOID );

    Service ( string const& _serviceName, string const& _serviceDescription, pfnAppProc _pAppProc, void* _pAppProcContext );
    virtual ~Service ( void );

    const string& GetServiceName ( void ) const
    {
        return m_serviceName;
    }
    const string& GetServiceDescription ( void ) const
    {
        return m_serviceDescription;
    }

    virtual int Install ( void );
    virtual int Uninstall ( void );

    int Remove ( SC_HANDLE _hSCManager );

    int CommonMain ( void );

    void ServiceMain ( void );
    void ServiceCtrlHandler ( DWORD _opcode );
    const SERVICE_TABLE_ENTRY* GetDispatchTable ( void ) const
    {
        return m_dispatchTable;
    }

    friend void WINAPI ServiceMain ( DWORD argc, LPTSTR *argv );
    friend void WINAPI ServiceCtrlHandler ( DWORD _opcode );

    SERVICE_STATUS			m_serviceStatus;
    SERVICE_STATUS_HANDLE	m_serviceStatusHandle;
    SERVICE_TABLE_ENTRY		m_dispatchTable[2];

    HANDLE					m_hActive;
    HANDLE					m_hExit;
    string					m_serviceName;
    string					m_serviceDescription;
    pfnAppProc				m_pAppProc;
    void*					m_pAppProcContext;

    static Service*			s_pInstance;
};

#endif // _SERVICE_H

