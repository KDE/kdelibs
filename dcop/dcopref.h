#ifndef DCOPREF_H
#define DCOPREF_H

#include <qcstring.h>

class QDataStream;
class DCOPObject;

class DCOPRef
{
public:
    DCOPRef();
    DCOPRef( const DCOPRef& ref );
    DCOPRef( DCOPObject* );
    DCOPRef( const QCString& app, const QCString& obj );

    bool isNull() const;

    QCString app() const;
    QCString object() const;

    DCOPRef& operator=( const DCOPRef& );

    void setRef( const QCString& app, const QCString& obj );

private:
    QCString m_app;
    QCString m_obj;
};

QDataStream& operator<<( QDataStream&, const DCOPRef& ref );
QDataStream& operator>>( QDataStream&, DCOPRef& ref );

#endif
