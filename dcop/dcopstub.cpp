#include "dcopstub.h"

DCOPStub::DCOPStub( const QString& app, const QString& obj )
    : m_app( app ), m_obj( obj )
{
}

DCOPStub::~DCOPStub()
{
}

QString DCOPStub::app() const
{
    return m_app;
}

QString DCOPStub::obj() const
{
    return m_obj;
}
