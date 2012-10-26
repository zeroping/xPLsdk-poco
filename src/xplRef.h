/***************************************************************************
****																	****
****	xplRef.h														****
****																	****
****	Reference counting for various xPL objects						****
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

#ifndef _xplRef_H
#define _xplRef_H

#pragma once

#include "xplCore.h"

namespace xpl
{
/**
 * Provides reference counting for xPL objects.
 * Any class wishing to include reference counting should be derived from xplRef.
 * Derived classes must declare their destructor as protected virtual.
 * On construction, the reference count is set to one.  Calls to AddRef increment
 * the count.  Calls to Release decrement the count.  When the count reaches
 * zero, the object is deleted.
 */
class xplRef
{
public:
    /**
     * Initializes the RefCount to one.  The object
     * can only be deleted through a call to Release.
     * @see AddRef, Release
     */
    xplRef()
    {
        m_refs = 1;
    }

    /**
     * Increases the reference count of the object.
     * Every call to AddRef requires a matching call
     * to Release before the object will be deleted.
     * @see Release
     */
    void AddRef()
    {
        ++m_refs;
    }

    /**
     * Removes a reference to an object.
     * If this was the last reference to the message, the
     * object is deleted.
     * @see AddRef
     */
    void Release()
    {
        if ( 0 >= ( --m_refs ) )
        {
            delete this;
        }
    }

protected:
    virtual ~xplRef() {}

private:
    // Reference counting
    uint32	m_refs;

}; // class xplRef

} // namespace xpl

#endif // _xplRef_H

