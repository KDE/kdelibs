#include "dcopstub.h"

DCOPStub::DCOPStub( const QCString& app, const QCString& obj )
    : m_app( app ), m_obj( obj )
{
}

DCOPStub::~DCOPStub()
{
}

QCString DCOPStub::app() const
{
    return m_app;
}

QCString DCOPStub::obj() const
{
    return m_obj;
}
