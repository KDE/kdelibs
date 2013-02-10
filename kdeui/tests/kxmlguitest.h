#ifndef KXMLGUITEST_H
#define KXMLGUITEST_H

#include <kxmlguiclient.h>
#include <QtCore/QObject>

class Client : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    Client() {}

    using KXMLGUIClient::setXMLFile;
    using KXMLGUIClient::setComponentName;

public Q_SLOTS:
    void slotSec();
};
#endif
