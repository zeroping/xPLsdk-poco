/***************************************************************************
****																	****
****	xplCore.h														****
****																	****
****	Basic types and preprocessor directives							****
****																	****
****	Copyright (c) 2005 Mal Lansell.									****
****    Email: xpl@lansell.org                                          ****
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

#ifndef XPLCORE_H
#define XPLCORE_H

#include <assert.h>
#include <string>
#ifdef _MSC_VER
#pragma once

// Disable certain warnings (mostly caused by STL)
#pragma warning(disable:4786)   // Disable warning about symbols longer than 255 characters
#pragma warning(disable:4503)   // Disable warning 'decorated name length exceeded, name was truncated'
#pragma warning(disable:4018)   // Disable warning 'signed/unsigned mismatch'
#pragma warning(disable:4100)   // Disable warning 'unreferenced formal parameter'
#pragma warning(disable:4201)   // Disable warning 'nonstandard extension used : nameless struct/union'

#pragma warning(disable:4996)   // Disable warning '... was declared deprecated' - required for .NET 2005


#endif // MSC_VER


#ifdef NULL
#undef NULL
#endif
#define NULL 0

#define HANDLE void*
#define true true
#define false false
#include <inttypes.h>

// Basic types
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
//typedef unsigned __int64    uint64; - now in inttypes.h

typedef char				int8;
typedef short				int16;
typedef int					int32;
//typedef __int64				int64; - now in inttypes.h

typedef float               float32;
typedef double              float64;


// Declare the xpl namespace
namespace std {}
namespace xpl
{
using namespace std;
}
using namespace std;

class XPLAddress
{
public:
    bool bcast;
    string vendor, device, instance;
    XPLAddress()
    {
        vendor="*";
        device="*";
        instance="*";
        bcast = false;
    }
    string toString() const
    {
        if ( bcast )
        {
            return "*";
        }
        return vendor + "-" + device + "." + instance;
    }
    /**
     * @brief Tests to see if the other address matches against our own (which may have *'s)
     *
     * @param other to test against
     **/
    bool match(XPLAddress other){
        return (bcast  || (
        (vendor=="*" || vendor.compare ( other.vendor ) == 0) 
        && (device=="*" || device.compare ( other.device ) == 0)
        && (instance=="*" || instance.compare ( other.instance ) == 0)));
    }
};

class XPLSchema
{
public:
    string schema, type;
};



// Fix for namespace-related compiler bug
#ifdef _MSC_VER
namespace xpl
{
}
#endif


#endif // XPLCORE_H
