#ifndef KXMLGUITEST_H
#define KXMLGUITEST_H

#include <kxmlguiclient.h>
#include <QtCore/QObject>

class Client : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    Client() {}

    void setXMLFile( const QString &f, bool merge = true, bool setXMLDoc = true ) { KXMLGUIClient::setXMLFile( f, merge, setXMLDoc ); }
    void setComponentData(const KComponentData &inst) { KXMLGUIClient::setComponentData(inst); }

public Q_SLOTS:
    void slotSec();
};
#endif
