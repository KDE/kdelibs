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

#include "dcopref.h"
#include "dcopclient.h"
#include "dcopobject.h"

#include <qdatastream.h>

DCOPRef::DCOPRef()
{
}

DCOPRef::DCOPRef( const DCOPRef& ref )
{
    m_app = ref.app();
    m_obj = ref.object();
    m_type = ref.type();
}

DCOPRef::DCOPRef( DCOPObject *o )
    : m_app( DCOPClient::mainClient() ? DCOPClient::mainClient()->appId() : QCString() ), 
      m_obj( o->objId() ), m_type( o->interfaces().last() )
{
}

DCOPRef::DCOPRef( const QCString& _app, const QCString& obj )
    : m_app( _app ), m_obj( obj )
{
}

DCOPRef::DCOPRef( const QCString& _app, const QCString& _obj, const QCString& _type )
    : m_app( _app ), m_obj( _obj ), m_type( _type )
{
}

bool DCOPRef::isNull() const
{
    return ( m_app.isEmpty() || m_obj.isEmpty() );
}

QCString DCOPRef::app() const
{
    return m_app;
}

QCString DCOPRef::object() const
{
    return m_obj;
}

QCString DCOPRef::type() const
{
    return m_type;
}


DCOPRef& DCOPRef::operator=( const DCOPRef& ref )
{
    m_app = ref.app();
    m_obj = ref.object();

    return *this;
}

void DCOPRef::setRef( const QCString& _app, const QCString& _obj )
{
    m_app = _app;
    m_obj = _obj;
    m_type = 0;
}

void DCOPRef::setRef( const QCString& _app, const QCString& _obj, const QCString& _type )
{
    m_app = _app;
    m_obj = _obj;
    m_type = _type;
}

void DCOPRef::clear()
{
    m_app = 0;
    m_obj = 0;
    m_type = 0;
}

QDataStream& operator<<( QDataStream& str, const DCOPRef& ref )
{
    str << ref.app();
    str << ref.object();
    str << ref.type();

    return str;
}

QDataStream& operator>>( QDataStream& str, DCOPRef& ref )
{
    QCString a, o, t;
    str >> a >> o >> t;

    ref.setRef( a, o, t );

    return str;
}
