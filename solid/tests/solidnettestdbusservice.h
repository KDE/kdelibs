#ifndef SOLID_NETWORKING_TESTSERVICE_H
#define SOLID_NETWORKING_TESTSERVICE_H

//#include <solid/networking.h>
#include <QtCore/QObject>

class QTimer;

class TestNetworkingService : public QObject
{
Q_OBJECT
    Q_PROPERTY( uint Status  READ status )
public:
    TestNetworkingService();
    ~TestNetworkingService();
public Q_SLOTS:
    uint requestConnection(); /*Result*/
    void releaseConnection();
    uint status() const;
Q_SIGNALS:
    void statusChanged( uint );
private:
};

#endif
