/*****************************************************************

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
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "dcopstub.h"
#include "dcopclient.h"

class DCOPStubPrivate
{
public:
    DCOPStubPrivate():dcopClient(0){}
    DCOPClient* dcopClient;
};

DCOPStub::DCOPStub( const QCString& app, const QCString& obj )
    : m_app( app ), m_obj( obj ), m_status( CallSucceeded ),d(0)
{
}

DCOPStub::DCOPStub( DCOPClient* client, const QCString& app, const QCString& obj )
    : m_app( app ), m_obj( obj ), m_status( CallSucceeded ),d(0)
{
    if ( client ) {
	d = new DCOPStubPrivate;
	d->dcopClient = client;
    }
}

DCOPStub::~DCOPStub()
{
    delete d;
}

DCOPClient* DCOPStub::dcopClient()
{
    if ( d )
	return d->dcopClient;
    return DCOPClient::mainClient();
}

DCOPStub::Status DCOPStub::status() const
{
    return m_status;
}

void DCOPStub::setStatus( Status _status )
{
    m_status = _status;
}

bool DCOPStub::ok() const
{
    return m_status != CallFailed;
}

void DCOPStub::callFailed()
{
    setStatus( CallFailed );
}

QCString DCOPStub::app() const
{
    return m_app;
}

QCString DCOPStub::obj() const
{
    return m_obj;
}

void DCOPStub::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }
