/***************************************************************************
****																	****
****	Service.cpp														****
****																	****
****	Implementation of a Windows Service								****
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

#include <assert.h>
#include "Service.h"

void WINAPI ServiceMain ( DWORD _argc, LPTSTR *_argv );
void WINAPI ServiceCtrlHandler ( DWORD _opcode );

Service* Service::s_pInstance = NULL;


/***************************************************************************
****																	****
****	ServiceMain - Main service function called by Windows			****
****																	****
***************************************************************************/

void WINAPI ServiceMain
(
    DWORD argc,
    LPTSTR *argv
)
{
    Service* pService = Service::Get();
    if ( NULL == pService )
    {
        return;
    }

    pService->ServiceMain();
    return;
}


/***************************************************************************
****																	****
****	ServiceCtrlHandler - Deals with control codes sent by Windows	****
****																	****
***************************************************************************/

void WINAPI ServiceCtrlHandler
(
    DWORD _opcode
)
{
    Service* pService = Service::Get();
    if ( NULL == pService )
    {
        return;
    }

    pService->ServiceCtrlHandler ( _opcode );
    return;
}


/***************************************************************************
****																	****
****	Service::Create													****
****																	****
***************************************************************************/

Service* Service::Create
(
    string const& _serviceName,
    string const& _serviceDescription,
    pfnAppProc _pAppProc,
    void* _pAppProcContext
)
{
    if ( NULL != s_pInstance )
    {
        assert ( 0 );
        return NULL;
    }

    if ( NULL == _pAppProc )
    {
        assert ( 0 );
        return NULL;
    }

    s_pInstance = new Service ( _serviceName, _serviceDescription, _pAppProc, _pAppProcContext );
    return ( s_pInstance );
}


/***************************************************************************
****																	****
****	Service::Destroy												****
****																	****
***************************************************************************/

void Service::Destroy()
{
    delete s_pInstance;
    s_pInstance = NULL;
}


/***************************************************************************
****																	****
****	Service Constructor												****
****																	****
***************************************************************************/

Service::Service
(
    string const& _serviceName,
    string const& _serviceDescription,
    pfnAppProc _pAppProc,
    void* _pAppProcContext
) :
    m_serviceName ( _serviceName ),
    m_serviceDescription ( _serviceDescription ),
    m_pAppProc ( _pAppProc ),
    m_pAppProcContext ( _pAppProcContext )
{
    assert ( NULL == s_pInstance );
    s_pInstance = this;

    // Create the events to control the application
    m_hActive = CreateEvent ( NULL, TRUE, TRUE, NULL );
    m_hExit = CreateEvent ( NULL, TRUE, FALSE, NULL );

    m_dispatchTable[0].lpServiceName = new char[m_serviceName.size() +1];
    strcpy ( m_dispatchTable[0].lpServiceName, m_serviceName.c_str() );
    m_dispatchTable[0].lpServiceProc = ::ServiceMain;
    m_dispatchTable[1].lpServiceName = NULL;
    m_dispatchTable[1].lpServiceProc = NULL;
    return;
}


/***************************************************************************
****																	****
****	Service Destructor												****
****																	****
***************************************************************************/

Service::~Service()
{
    CloseHandle ( m_hActive );
    CloseHandle ( m_hExit );
    s_pInstance = NULL;
}


/***************************************************************************
****																	****
****	Service::ProcessCommandLine										****
****																	****
****	Parses the command line and installs, uninstalls or runs the	****
****	service as appropriate											****
****																	****
****	8th August 2004													****
****																	****
***************************************************************************/

int Service::ProcessCommandLine
(
    int argc,
    char* argv[]
)
{
    if ( 1 == argc )
    {
        // No command line arguments, so we assume that
        // we're running as a console application.
        // This allows the program to run on non-NT versions of
        // Windows such as Win98, and is also handy for debugging.

        return ( CommonMain() );
    }
    else if ( 2 == argc )
    {
        if ( !stricmp ( argv[1], "/Install" ) )
        {
            return ( Install() );
        }

        if ( !stricmp ( argv[1], "/Uninstall" ) )
        {
            return ( Uninstall() );
        }
        if ( !stricmp ( argv[1], "/Run" ) )
        {
            if ( !StartServiceCtrlDispatcher ( GetDispatchTable() ) )
            {
                // Failed to start as a service, so run as an console app
                return ( CommonMain() );
            }
            return ( 0 );
        }
    }

    // Something was wrong
    printf ( "\n\nUsage:\n\nTo Install this as a service use %s /install\nTo Uninstall use %s /uninstall\n\nUse no command line arguments to run $s as a console application\n", argv[0], argv[0], argv[0] );
    return ( -1 );
}


/***************************************************************************
****																	****
****	Service::Install												****
****																	****
***************************************************************************/

int Service::Install()
{
    SC_HANDLE hSCManager = OpenSCManager ( NULL,NULL,SC_MANAGER_ALL_ACCESS );
    if ( hSCManager == NULL )
    {
        printf ( "\nUnable to access the Service Control Manager\n" );
        return ( -1 );
    }

    // Remove any existing service
    Remove ( hSCManager );

    // Build the exe path, including the /Run command
    // line argument so we know to run as a service.
    char exePath[MAX_PATH+1];
    GetModuleFileName ( NULL, exePath, MAX_PATH+1 );
    strcat ( exePath, " /Run" );

    SC_HANDLE hService = CreateService (
                             hSCManager,
                             GetServiceName().c_str(),
                             GetServiceName().c_str(),			// Service name to display
                             SERVICE_ALL_ACCESS,					// Desired access
                             SERVICE_WIN32_OWN_PROCESS,			// Service type
                             SERVICE_AUTO_START,					// Start type
                             SERVICE_ERROR_NORMAL,				// Error control type
                             exePath,			  				// Service's binary
                             NULL,								// No load ordering group
                             NULL,								// No tag identifier
                             NULL,								// No dependencies
                             NULL,								// LocalSystem account
                             NULL );								// No password


    if ( NULL == hService )
    {
        printf ( "\nFailed to install service %s\n", GetServiceName().c_str() );
        return ( 0 );
    }

    // Set the service description so it shows up on the management console.
    // This function only exists on W2k and up, so we have to go the LoadLibrary route.
    HMODULE hLib = ::LoadLibrary ( "ADVAPI32.DLL" );
    if ( hLib )
    {
        ChangeServiceConfigType pfnChangeServiceConfig = ( ChangeServiceConfigType ) ::GetProcAddress ( hLib, "ChangeServiceConfig2A" );
        if ( pfnChangeServiceConfig )
        {
            SERVICE_DESCRIPTION sd;
            sd.lpDescription = const_cast<char*> ( m_serviceDescription.c_str() );
            pfnChangeServiceConfig ( hService, SERVICE_CONFIG_DESCRIPTION, ( void* ) &sd );
        }
        ::FreeLibrary ( hLib );
    }

    // Start the service
    StartService ( hService, 0, NULL );

    CloseServiceHandle ( hService );
    printf ( "\nService %s installed\n", GetServiceName().c_str() );

    return ( 0 );
}


/***************************************************************************
****																	****
****	Service::Uninstall												****
****																	****
***************************************************************************/

int Service::Uninstall()
{
    SC_HANDLE hSCManager = OpenSCManager ( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if ( NULL != hSCManager )
    {
        if ( !Remove ( hSCManager ) )
        {
            return ( 0 );
        }
    }

    printf ( "\n\nFailed to uninstall service %s\n", GetServiceName().c_str() );
    return ( 0 );
}


/***************************************************************************
****																	****
****	Service::Remove													****
****																	****
***************************************************************************/

int Service::Remove
(
    SC_HANDLE _hSCManager
)
{
    // Stop and Remove the service
    SC_HANDLE hService = OpenService ( _hSCManager, GetServiceName().c_str(), SERVICE_ALL_ACCESS );
    if ( NULL != hService )
    {
        // Stop the service
        SERVICE_STATUS ServiceStatus;
        ControlService ( hService, SERVICE_CONTROL_STOP, &ServiceStatus );

        // Remove the service
        if ( DeleteService ( hService ) )
        {
            CloseServiceHandle ( hService );
            printf ( "\n\nService %s uninstalled\n", GetServiceName().c_str() );
            return ( 0 );
        }
    }

    return ( -1 );
}


/***************************************************************************
****																	****
****	Service::ServiceCtrlHandler										****
****																	****
***************************************************************************/

void Service::ServiceCtrlHandler
(
    DWORD _opcode
)
{
    switch ( _opcode )
    {
    case SERVICE_CONTROL_PAUSE:
    {
        m_serviceStatus.dwCurrentState = SERVICE_PAUSED;
        ResetEvent ( m_hActive );
        break;
    }

    case SERVICE_CONTROL_CONTINUE:
    {
        m_serviceStatus.dwCurrentState = SERVICE_RUNNING;
        SetEvent ( m_hActive );
        break;
    }

    case SERVICE_CONTROL_STOP:
    {
        m_serviceStatus.dwWin32ExitCode = 0;
        m_serviceStatus.dwCurrentState = SERVICE_STOPPED;
        m_serviceStatus.dwCheckPoint = 0;
        m_serviceStatus.dwWaitHint = 0;
        SetServiceStatus ( m_serviceStatusHandle, &m_serviceStatus );
        SetEvent ( m_hExit );
        break;
    }

    case SERVICE_CONTROL_INTERROGATE:
    {
        break;
    }
    }

    return;
}


/***************************************************************************
****																	****
****	Service::ServiceMain											****
****																	****
***************************************************************************/

void Service::ServiceMain()
{
    m_serviceStatus.dwServiceType = SERVICE_WIN32;
    m_serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    m_serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    m_serviceStatus.dwWin32ExitCode = 0;
    m_serviceStatus.dwServiceSpecificExitCode = 0;
    m_serviceStatus.dwCheckPoint = 0;
    m_serviceStatus.dwWaitHint = 0;

    m_serviceStatusHandle = RegisterServiceCtrlHandler ( m_serviceName.c_str(), ::ServiceCtrlHandler );
    if ( m_serviceStatusHandle == ( SERVICE_STATUS_HANDLE ) 0 )
    {
        return;
    }

    m_serviceStatus.dwCurrentState = SERVICE_RUNNING;
    m_serviceStatus.dwCheckPoint = 0;
    m_serviceStatus.dwWaitHint = 0;
    if ( !SetServiceStatus ( m_serviceStatusHandle, &m_serviceStatus ) )
    {
        return;
    }

    // Call the common main code - used by
    // both services and debug applications
    CommonMain();
    return;
}


/***************************************************************************
****																	****
****	Service::CommonMain												****
****																	****
***************************************************************************/

int Service::CommonMain()
{
    // Call the application procedure.  This should not
    // return until m_hExit is signalled.
    m_pAppProc ( m_hActive, m_hExit, m_pAppProcContext );
    return ( 0 );
}




