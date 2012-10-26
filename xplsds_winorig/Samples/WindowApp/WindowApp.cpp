/***************************************************************************
****																	****
****	WindowApp.cpp     									            ****
****																	****
****	Example code demonstrating how to build an xPL Windows App.     ****
****																	****
****	Copyright (c) 2007 Mal Lansell.									****
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

#include "xplDevice.h"
#include "xplUDP.h"
#include "xplConfigItem.h"
#include "xplMsg.h"
#include "EventLog.h"
#include "WindowApp.h"

using namespace xpl;

#define MAX_LOADSTRING 100

// Set the config version here.  Each time a new version of the application
// is to be released, increment the version number.  This ensures that the
// new version will ignore any config items saved in the registry by
// previous versions, and instead enters config mode.
string const c_version = "1.0.0";

// The app name must be exactly the same as the compiled executable
// (but without the .exe), or the event logging will not work.
string const c_appName = "WindowApp";

HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

void                HandleMessages ( xplDevice* _pDevice );
void                Configure ( xplDevice* _pDevice );
ATOM				MyRegisterClass ( HINSTANCE hInstance );
BOOL				InitInstance ( HINSTANCE, int );
LRESULT CALLBACK	WndProc ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK	About ( HWND, UINT, WPARAM, LPARAM );


/***************************************************************************
****																	****
****	WinMain															****
****																	****
***************************************************************************/

int APIENTRY WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
    // ************************
    // Init
    // ************************

    MSG msg;
    HACCEL hAccelTable;

    // Initialize global strings
    LoadString ( hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING );
    LoadString ( hInstance, IDC_WINDOWAPP, szWindowClass, MAX_LOADSTRING );
    MyRegisterClass ( hInstance );

    // Perform application initialization:
    if ( !InitInstance ( hInstance, nCmdShow ) )
    {
        goto exit;
    }

    hAccelTable = LoadAccelerators ( hInstance, ( LPCTSTR ) IDC_WINDOWAPP );

    // Create the EventLog
    xpl::EventLog::Create ( c_appName );

    // Create the xPL communications object
    xplComms* pComms = xplUDP::Create ( true );
    if ( NULL == pComms )
    {
        goto exit;
    }

    // Create the xPL Device
    // Change "vendor" to your own vendor ID.  Vendor IDs can be no more than 8 characters
    // in length.  Post a message to the xPL Yahoo Group (http://groups.yahoo.com/group/ukha_xpl)
    // to request a vendor ID.
    // Replace "device" with a suitable device name for your application - usually something
    // related to what is being xPL-enabled.  Device names can be no more than 8 characters.
    xplDevice* pDevice = xplDevice::Create ( "vendor", "device", c_version, true, true, pComms );
    if ( NULL == pDevice )
    {
        goto exit;
    }

    // Create any additional config items
    // As an example, the following code creates an item to hold the index
    // of a com port.  The value can be changed by the user in xPLHal.
    xplConfigItem* pItem = new xplConfigItem ( "comport", "reconf" );
    if ( NULL != pItem )
    {
        // Default the com port to COM1
        pItem->AddValue ( "1" );
        pDevice->AddConfigItem ( pItem );
    }

    // Get the message and config events
    // The xplMsgEvent is signalled when an xPL message is received by the xplDevice.
    // The configEvent is signalled when the config items have been changed.
    HANDLE xplMsgEvent = pDevice->GetMsgEvent();
    HANDLE configEvent = pDevice->GetConfigEvent();

    // Init the xplDevice
    // Note that all config items must have been set up before Init() is called.
    pDevice->Init();


    // ************************
    // Main loop
    // ************************

    while ( 1 )
    {
        if ( PeekMessage ( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
            if ( !GetMessage ( &msg, NULL, 0, 0 ) )
            {
                //WM_QUIT has been posted
                break;
            }

            TranslateMessage ( &msg );
            DispatchMessage ( &msg );
        }

        // Deal with any received xPL messages
        if ( WAIT_OBJECT_0 == WaitForSingleObject ( xplMsgEvent, 0 ) )
        {
            HandleMessages ( pDevice );
        }

        // Is configuration required?
        if ( WAIT_OBJECT_0 == WaitForSingleObject ( configEvent, 0 ) )
        {
            // Reset the configEvent before doing the configuration.
            // This ensures that if a config update is received by the
            // xPLDevice thread while we're in Configure(), it will
            // not be missed.
            ResetEvent ( configEvent );
            Configure ( pDevice );
        }

        // Deal with any App specific stuff here
        // If we were waiting on an event, double check it with a
        // call the WaitForSingleObject in the same way that we did
        // above for the xplMsgEvent and configEvent.
        //
        // if( WAIT_OBJECT_0 == WaitForSingleObject( m_hApp, 0 ) )
        // {
        // }
        //
    }

exit:
    // ************************
    // Clean up and exit
    // ************************

    // Destroy the xPL Device
    // This also deletes any config items we may have added
    if ( pDevice )
    {
        xplDevice::Destroy ( pDevice );
        pDevice = NULL;
    }

    // Destroy the comms object
    if ( pComms )
    {
        xplUDP::Destroy ( pComms );
        pComms = NULL;
    }

    // Destroy the event log
    if ( EventLog::Get() )
    {
        EventLog::Destroy();
    }

    return 0;
}


/***************************************************************************
****																	****
****	WinMain															****
****	MyRegisterClass()                                               ****
****	                                                                ****
****	Registers the window class.                                     ****
****	                                                                ****
****	This function and its usage are only necessary if you want this ****
****    code to be compatible with Win32 systems prior to the           ****
****    'RegisterClassEx' function that was added to Windows 95. It is  ****
****    important to call this function so that the application will    ****
****    get 'well formed' small icons associated with it.               ****
****																	****
***************************************************************************/

ATOM MyRegisterClass ( HINSTANCE hInstance )
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof ( WNDCLASSEX );

    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= ( WNDPROC ) WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInstance;
    wcex.hIcon			= LoadIcon ( hInstance, ( LPCTSTR ) IDI_WINDOWAPP );
    wcex.hCursor		= LoadCursor ( NULL, IDC_ARROW );
    wcex.hbrBackground	= ( HBRUSH ) ( COLOR_WINDOW+1 );
    wcex.lpszMenuName	= ( LPCTSTR ) IDC_WINDOWAPP;
    wcex.lpszClassName	= szWindowClass;
    wcex.hIconSm		= LoadIcon ( wcex.hInstance, ( LPCTSTR ) IDI_SMALL );

    return RegisterClassEx ( &wcex );
}


/***************************************************************************
****																	****
****	InitInstance                                                    ****
****                                                                    ****
****    Saves instance handle and creates main window                   ****
****                                                                    ****
****    In this function, we save the instance handle in a global       ****
****    variable and create and display the main program window.        ****
****																	****
***************************************************************************/

BOOL InitInstance ( HINSTANCE hInstance, int nCmdShow )
{
    HWND hWnd;

    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindow ( szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL );

    if ( !hWnd )
    {
        return FALSE;
    }

    ShowWindow ( hWnd, nCmdShow );
    UpdateWindow ( hWnd );

    return TRUE;
}


/***************************************************************************
****																	****
****    WndProc                                                         ****
****                                                                    ****
****    Processes messages for the main window.                         ****
****																	****
***************************************************************************/

LRESULT CALLBACK WndProc ( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch ( message )
    {
    case WM_COMMAND:
        wmId    = LOWORD ( wParam );
        wmEvent = HIWORD ( wParam );
        // Parse the menu selections:
        switch ( wmId )
        {
        case IDM_ABOUT:
            DialogBox ( hInst, ( LPCTSTR ) IDD_ABOUTBOX, hWnd, ( DLGPROC ) About );
            break;
        case IDM_EXIT:
            DestroyWindow ( hWnd );
            break;
        default:
            return DefWindowProc ( hWnd, message, wParam, lParam );
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint ( hWnd, &ps );
        // TODO: Add any drawing code here...
        EndPaint ( hWnd, &ps );
        break;
    case WM_DESTROY:
        PostQuitMessage ( 0 );
        break;
    default:
        return DefWindowProc ( hWnd, message, wParam, lParam );
    }
    return 0;
}


/***************************************************************************
****																	****
****    About                                                           ****
****                                                                    ****
****    Message handler for about box.                                  ****
****																	****
***************************************************************************/

LRESULT CALLBACK About ( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        if ( LOWORD ( wParam ) == IDOK || LOWORD ( wParam ) == IDCANCEL )
        {
            EndDialog ( hDlg, LOWORD ( wParam ) );
            return TRUE;
        }
        break;
    }
    return FALSE;
}


/***************************************************************************
****																	****
****	HandleMessages  							            		****
****																	****
***************************************************************************/

void HandleMessages ( xplDevice* _pDevice )
{
    xplMsg* pMsg = NULL;

    // Get each queued xPL message in turn
    while ( pMsg = _pDevice->GetMsg() )
    {
        // Process the xPL message here.
        // ...

        // Done with it.
        pMsg->Release();
    }
}


/***************************************************************************
****																	****
****	Configure   										            ****
****																	****
***************************************************************************/

void Configure ( xplDevice* _pDevice )
{
    // Examine each of our config items in turn, and if they have changed,
    // take the appropriate action.

    // As an example, the Com Port index.
    //
    //  xplConfigItem const* pItem = _pDevice->GetConfigItem( "comport" );
    //  string value = pItem->GetValue();
    //  if( !value.empty() )
    //  {
    //      int comPort = atol( value.c_str() );
    //
    //      Deal with any change to the Com port number
    //      ...
    //  }
    //
}
