#include "dcopref.h"
#include "dcopclient.h"
#include "dcopobject.h"

#include <qdatastream.h>

#include <kapp.h>

DCOPRef::DCOPRef()
{
}

DCOPRef::DCOPRef( const DCOPRef& ref )
{
    m_app = ref.app();
    m_obj = ref.object();
}

DCOPRef::DCOPRef( DCOPObject* object )
{
    m_app = kapp->dcopClient()->appId();
    m_obj = object->objId();
}

DCOPRef::DCOPRef( const QCString& app, const QCString& obj )
    : m_app( app ), m_obj( obj )
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

DCOPRef& DCOPRef::operator=( const DCOPRef& ref )
{
    m_app = ref.app();
    m_obj = ref.object();

    return *this;
}

void DCOPRef::setRef( const QCString& app, const QCString& obj )
{
    m_app = app;
    m_obj = obj;
}

QDataStream& operator<<( QDataStream& str, const DCOPRef& ref )
{
    str << ref.app();
    str << ref.object();

    return str;
}

QDataStream& operator>>( QDataStream& str, DCOPRef& ref )
{
    QCString a, o;
    str >> a >> o;

    ref.setRef( a, o );

    return str;
}
