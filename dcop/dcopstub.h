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
class DCOPStubPrivate;

#include <stdlib.h>

#include <qstring.h>

/**
* Abstract base class for dcop stubs as created by the
* dcopidl2cpp compiler
*
*/

class DCOPStub
{
public:
    /**
       Creates a DCOPStub for application @p app and object @p obj
     */
    DCOPStub( const QCString& app, const QCString& obj );

    /** 
      Creates a DCOPStub for application @p app and object @p obj
       that operates on the DCOPClient @p client
     */
    DCOPStub( DCOPClient* client, const QCString& app, const QCString& obj );
    virtual ~DCOPStub();

    /**
       Return the application id.
     */
    QCString app() const;
    /**
       Return the object  id.
     */
    QCString obj() const;

    enum Status{ CallSucceeded, CallFailed };
    /**
     * Return the status of the last call, either @p CallSucceeded or
     * @p CallFailed
     *
     *See @ref ok();
     */
    Status status() const;


    /**
     *Return whether no error occured
     *
     * See @ref status();
     */
    bool ok()  const;

protected:

    /**
       Sets the status to status. Possible values are 'CallSucceeded' and 'CallFailed'
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
    */
    DCOPClient* dcopClient();

    // Never use. Used only for default ctors of inherited stubs,
    // because of the way ctors of virtual base classes work.
    // Otherwise dcopidl2cpp would have to call ctors of all, even
    // indirect, bases.
    enum never_use_t { never_use };
    DCOPStub( never_use_t ) { abort(); }
    
private:
    QCString m_app;
    QCString m_obj;
    Status m_status;

protected:
    virtual	void virtual_hook( int id, void* data );
private:
    DCOPStubPrivate *d;
};

#endif
