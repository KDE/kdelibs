#ifndef KXMLGUITEST_H
#define KXMLGUITEST_H

#include <kxmlguiclient.h>
#include <qobject.h>

class Client : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    Client() {}

    void setXMLFile( const QString &f, bool merge = true ) { KXMLGUIClient::setXMLFile( f, merge ); }
    void setInstance( KInstance *inst ) { KXMLGUIClient::setInstance( inst ); }

public Q_SLOTS:
    void slotSec();
};
#endif
