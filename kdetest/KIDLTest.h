#ifndef KIDLTEST_H
#define KIDLTEST_H

#include <dcopobject.h>

class KIDLTest : virtual public DCOPObject
{
    K_DCOP
public:
    KIDLTest( const QString& id );

    virtual QString hello( const QString& name );
};

#endif
