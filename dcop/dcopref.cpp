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

#define STR( s ) ( s.data() ? s.data() : "" )

bool DCOPReply::typeCheck( const char* t )
{
    if ( type == t )
	return TRUE;
    qWarning( "DCOPReply<%s>: cast to '%s' error",
	     STR( type ), t );
    return FALSE;
}

DCOPReply DCOPRef::callInternal( const QCString& fun, const QCString& args, const QByteArray& data )
{
    DCOPReply reply;
    if ( isNull() ) {
	qWarning( "DCOPRef: call '%s' on null reference error",
		  STR( fun ) );
	return reply;
    }
    QCString sig = fun;
    if ( fun.find('(') == -1 ) {
	sig += args;
	if( args.find( "<unknown" ) != -1 )
	    qWarning("DCOPRef: unknown type error "
		     "<\"%s\",\"%s\">::call(\"%s\",%s",
		     STR(m_app), STR(m_obj), STR(fun), args.data()+1 );
    }
    DCOPClient* dc = dcopClient();
    if ( !dc || !dc->isAttached() ) {
	qWarning( "DCOPRef::call():  no DCOP client or client not attached error" );
	return reply;
    }
    dc->call( m_app, m_obj, sig, data, reply.type, reply.data );
    return reply;
}

bool DCOPRef::sendInternal( const QCString& fun, const QCString& args, const QByteArray& data )
{
    if ( isNull() ) {
	qWarning( "DCOPRef: send '%s' on null reference error",
		  STR( fun ) );
	return FALSE;
    }
    Q_UNUSED( data );
    QCString sig = fun;
    if ( fun.find('(') == -1 ) {
	sig += args;
	if( args.find( "<unknown" ) != -1 )
	    qWarning("DCOPRef: unknown type error "
		     "<\"%s\",\"%s\">::send(\"%s\",%s",
		     STR(m_app), STR(m_obj), STR(fun), args.data()+1 );
    }
    DCOPClient* dc = dcopClient();
    if ( !dc || !dc->isAttached() ) {
	qWarning( "DCOPRef::send(): no DCOP client or client not attached error" );
	return false;
    }
    return dc->send( m_app, m_obj, sig, data );
}

DCOPRef::DCOPRef()
    :d(0)
{
}

DCOPRef::DCOPRef( const DCOPRef& ref )
    :d( ref.d )
{
    m_app = ref.app();
    m_obj = ref.obj();
    m_type = ref.type();
}

DCOPRef::DCOPRef( DCOPObject *o )
    : m_app( DCOPClient::mainClient() ? DCOPClient::mainClient()->appId() : QCString() ),
    m_obj( o->objId() ), m_type( o->interfaces().last() ), d(0)

{
}

DCOPRef::DCOPRef( const QCString& _app, const QCString& obj )
    : m_app( _app ), m_obj( obj ), d(0)
{
}

DCOPRef::DCOPRef( const QCString& _app, const QCString& _obj, const QCString& _type )
    : m_app( _app ), m_obj( _obj ), m_type( _type ), d(0)
{
}

bool DCOPRef::isNull() const
{
    return ( m_app.isNull() || m_obj.isNull() );
}

QCString DCOPRef::app() const
{
    return m_app;
}

QCString DCOPRef::obj() const
{
    return m_obj;
}

QCString DCOPRef::object() const
{
    return m_obj;
}


QCString DCOPRef::type() const
{
    return m_type;
}

void DCOPRef::setDCOPClient( DCOPClient* dc )
{
    d = (DCOPRefPrivate*) dc;
}

DCOPClient* DCOPRef::dcopClient() const
{
    return d ? (DCOPClient*)d : DCOPClient::mainClient();
}

DCOPRef& DCOPRef::operator=( const DCOPRef& ref )
{
    d = ref.d;
    m_app = ref.app();
    m_obj = ref.obj();
    m_type = ref.type();
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
    str << ref.obj();
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
