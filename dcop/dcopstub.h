#ifndef _DCOPSTUB_H
#define _DCOPSTUB_H

#include <qstring.h>

class DCOPStub
{
public:
    DCOPStub( const QString& app, const QString& obj );
    virtual ~DCOPStub();
    
    QString app() const;
    QString obj() const;
    
private:
    QString m_app;
    QString m_obj;
};

#endif
