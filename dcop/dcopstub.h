/*
Copyright (c) 1999 Preston Brown <pbrown@kde.org>
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _DCOPSTUB_H
#define _DCOPSTUB_H

class DCOPClient;
class DCOPRef;
class DCOPStubPrivate;

#include <stdlib.h>

#include <qstring.h>
#include <dcoptypes.h>
#include "kdelibs_export.h"

/**
* Abstract base class for dcop stubs as created by the
* dcopidl2cpp compiler.
*
*/

class DCOP_EXPORT DCOPStub
{
public:
    /**
       Creates a DCOPStub for application @p app and object @p obj
       @param app the application id
       @param obj the object id
     */
    DCOPStub( const DCOPCString& app, const DCOPCString& obj );

    /** 
      Creates a DCOPStub for application @p app and object @p obj
       that operates on the DCOPClient @p client
       @param client the DCOPClient
       @param app the application id
       @param obj the object id
     */
    DCOPStub( DCOPClient* client, const DCOPCString& app, const DCOPCString& obj );
    
    /**
       Creates a DCOPStub for application ref.app() and object ref.obj()
       @param ref the DCOP reference
     */
    explicit DCOPStub( const DCOPRef& ref );
    virtual ~DCOPStub();

    /**
       Return the application id.
       @return the application id
     */
    DCOPCString app() const;
    /**
       Return the object  id.
       @return the object id
     */
    DCOPCString obj() const;

    /** Status enum. @see status() */
    enum Status{ CallSucceeded, CallFailed };
    /**
     * Return the status of the last call, either @p CallSucceeded or
     * @p CallFailed.
     *
     * @return the status of the last call
     * @see ok();
     */
    Status status() const;


    /**
     * Return whether no error occurred,
     *
     * @return true if the last call was successful, false otherwise
     * @see status();
     */
    bool ok()  const;

protected:

    /**
       Sets the status to status. Possible values are 'CallSucceeded' and 'CallFailed'
       @param _status the new status
       @see status()
     */
    void setStatus( Status _status );

    /**
      Invoked whenever a call fails.  The default implementation
      sets the status to CallFailed.
     */
    virtual void callFailed();
    
    /** 
      The dcopClient this stub operates on. Either the specific one
       specified in the constructor or DCOPClient::mainClient.
       @return the stub's DCOPClient
    */
    DCOPClient* dcopClient();

    /**
     * Never use. Used only for default ctors of inherited stubs,
     * because of the way ctors of virtual base classes work.
     * Otherwise dcopidl2cpp would have to call ctors of all, even
     * indirect, bases.
     * @since 3.1
     * @internal
     */
    enum never_use_t { never_use };
    /**
     * @since 3.1
     * @internal
     */
    DCOPStub( never_use_t ) { abort(); }
    
private:
    DCOPCString m_app;
    DCOPCString m_obj;
    Status m_status;

protected:
    /** Standard hack for adding virtuals later. @internal */
    virtual	void virtual_hook( int id, void* data );
private:
    DCOPStubPrivate *d;
};

#endif
