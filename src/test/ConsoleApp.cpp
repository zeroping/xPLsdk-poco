/***************************************************************************
****																	****
****	ConsoleApp.cpp  									            ****
****																	****
****	Example code demonstrating how to build an xPL Windows Service. ****
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
#include "Poco/Thread.h"
#include "Poco/RunnableAdapter.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
using Poco::RunnableAdapter;
using Poco::Thread;

using namespace xpl;

// Set the config version here.  Each time a new version of the application
// is to be released, increment the version number.  This ensures that the
// new version will ignore any config items saved in the registry by
// previous versions, and instead enters config mode.
string const c_version = "1.1.0";

// The app name must be exactly the same as the compiled executable
// (but without the .exe), or the event logging will not work.
string const c_appName = "ConsoleApp";

// void HandleMessages( xplDevice* _pDevice );
// void Configure( xplDevice* _pDevice );

Poco::Event* configEvent;
/***************************************************************************
****																	****
****	main															****
****																	****
***************************************************************************/
void handleConfigEvents()
{
    //Poco::Event* configEvent
}

class TestApp
{
public:
    TestApp();
    ~TestApp();
    void run();
    void HandleMessages ( MessageRxNotification* );
    void Configure ( DeviceConfigNotification* );
    bool testRunning();

    xplComms* pComms;
    xplDevice* pDevice;
    Thread listenThread;
    Poco::Event* xplMsgEvent;
    Mutex runningMutex;
    bool running;
};


TestApp::TestApp()
{
    // ************************
    // Init
    // ************************

    // Create the EventLog
    //xpl::EventLog::Create( c_appName );

    // Create the xPL communications object
    pComms = xplUDP::Create ( true );
    if ( NULL == pComms )
    {
        cout << "error\n\n";
        return;
    }

    // Create the xPL Device
    // Change "vendor" to your own vendor ID.  Vendor IDs can be no more than 8 characters
    // in length.  Post a message to the xPL Yahoo Group (http://groups.yahoo.com/group/ukha_xpl)
    // to request a vendor ID.
    // Replace "device" with a suitable device name for your application - usually something
    // related to what is being xPL-enabled.  Device names can be no more than 8 characters.
    cout << "app: xplUDP created\n";
    pDevice = xplDevice::Create ( "vendor", "device", c_version, true, true, pComms );
    if ( NULL == pDevice )
    {
        cout << "error\n\n";
        return;
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


    //register to observe config and RX messages

    //pDevice->rxTaskManager.addObserver(Observer<TestApp, MessageRxNotification>(*this,&TestApp::HandleMessages));
    pDevice->rxNotificationCenter.addObserver ( Observer<TestApp, MessageRxNotification> ( *this,&TestApp::HandleMessages ) );
    //pDevice->configTaskManager.addObserver(Observer<TestApp, DeviceConfigNotification>(*this,&TestApp::Configure));
    pDevice->configNotificationCenter.addObserver ( Observer<TestApp, DeviceConfigNotification> ( *this,&TestApp::Configure ) );

    // Init the xplDevice
    // Note that all config items must have been set up before Init() is called.
    pDevice->Init();

}


TestApp::~TestApp()
{


    // ************************
    // Clean up and exit
    // ************************

    runningMutex.lock();
    running = false;
    runningMutex.unlock();


    // Destroy the xPL Device
    // This also deletes any config items we may have added
    if ( pDevice )
    {
        cout << "app: destroying device\n";
        xplDevice::Destroy ( pDevice );
        pDevice = NULL;
    }


    // Destroy the comms object
    if ( pComms )
    {
        cout << "app: destroying pComms\n";
        xplUDP::Destroy ( pComms );
        pComms = NULL;
    }



}

bool TestApp::testRunning()
{
    bool ret;
    runningMutex.lock();
    ret = running;
    runningMutex.unlock();
    return ret;
}


void TestApp::run()
{
    // ************************
    // Main loop
    // ************************
    running = true;
    while ( testRunning() )
    {
        Thread::sleep ( 100 );
        //cout << "test app sleeping\n";
    }
    cout << "exiting app thread\n";

}


void TestApp::HandleMessages ( MessageRxNotification* mNot )
{
    cout<<"app: RX event fired in thread " << Thread::currentTid()  << "\n";
    xplMsg* pMsg = NULL;


    pMsg = mNot->message;
    cout << "app: start handling the message: " << pMsg->GetSchemaClass() << "." << pMsg->GetSchemaType() << "\n";
    int vnum;
    for ( vnum = 0; vnum < pMsg->GetNumMsgItems(); vnum++ )
    {
        cout << "\t" << pMsg->GetMsgItem ( vnum )->GetName() << " = " << pMsg->GetMsgItem ( vnum )->GetValue() << "\n";
    }

    mNot->release();
    cout << "app: stop handling the message " << Thread::currentTid()  << "\n";
}
/*
void TestApp::configThread() {
    cout << "config thread started\n";
    while( testRunning() )
    {
        try {
            configEvent->wait(1000);
        } catch (TimeoutException& e) {
            continue;
        }
        cout << "configEvent happened\n";
        Configure();
        configEvent->reset();

    }
    cout << "config stopped\n";

}*/

void TestApp::Configure ( DeviceConfigNotification* confNot )
{
    // Examine each of our config items in turn, and if they have changed,
    // take the appropriate action.

    // As an example, the Com Port index.
    //
    cout << "app: start handling rconfigure" << Thread::currentTid()  << "\n";
    xplConfigItem const* pItem = pDevice->GetConfigItem ( "comport" );
    string value = pItem->GetValue();
    if ( !value.empty() )
    {
        int comPort = atol ( value.c_str() );

//              Deal with any change to the Com port number
//              ...
        cout << "comport changed to: " << comPort << "\n";
        cout << "our name is: " << pDevice->GetVendorId() << "-" << pDevice->GetDeviceId() << "." <<pDevice->GetInstanceId() << "\n";
    }
    cout << "app: stop handling rconfigure" << Thread::currentTid()  << "\n";
    confNot->release();
}


TestApp* t;

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void
signal_callback_handler ( int signum )
{
    cout << "Caught signal " <<  signum << "\n";
    // Cleanup and close up stuff here

    delete t;

    // Terminate program
    exit ( signum );
}



int main ( int argc, char* argv[] )
{
    signal ( SIGINT, signal_callback_handler );

    t = new TestApp();

    t->run();


}


/***************************************************************************
****																	****
****	HandleMessages  							            		****
****																	****
***************************************************************************/



