#ifndef _DCOPSTUB_H
#define _DCOPSTUB_H

#include <qstring.h>

class DCOPStub
{
public:
    DCOPStub( const QCString& app, const QCString& obj );
    virtual ~DCOPStub();

    QCString app() const;
    QCString obj() const;

private:
    QCString m_app;
    QCString m_obj;
};

#endif
