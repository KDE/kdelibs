#ifndef __browserinterface_h__
#define __browserinterface_h__

#include <qobject.h>
#include <qvariant.h>

namespace KParts
{

class BrowserInterface : public QObject
{
    Q_OBJECT
public:
    BrowserInterface( QObject *parent, const char *name = 0 );
    virtual ~BrowserInterface();

    void callMethod( const char *name, const QVariant &argument );
};

};

#endif
