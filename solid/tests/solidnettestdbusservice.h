#ifndef SOLID_NETWORKING_TESTSERVICE_H
#define SOLID_NETWORKING_TESTSERVICE_H

//#include <solid/networking.h>
#include <QtCore/QObject>

class QTimer;

class TestNetworkingService;

class Behaviour : public QObject
{
    Q_OBJECT
    public:
        Behaviour( TestNetworkingService * );
    public Q_SLOTS:
        virtual void go() = 0;
        virtual void serviceStatusChanged( uint ) = 0;
    protected:
        TestNetworkingService * mService;
};

class GoOnlineOnRequest : public Behaviour
{
Q_OBJECT
public:
    GoOnlineOnRequest( TestNetworkingService * );
public Q_SLOTS:
    void go();
    void serviceStatusChanged( uint );
private Q_SLOTS:
    void doDelayedConnect();
    void doDelayedDisconnect();
};

class TestNetworkingService : public QObject
{
Q_OBJECT

    Q_PROPERTY( uint Status  READ status )
public:
    TestNetworkingService( const QString & behaviour );
    ~TestNetworkingService();
    void setStatus( uint );
public Q_SLOTS:
    uint requestConnection(); /*Result*/
    void releaseConnection();
    uint status() const;
Q_SIGNALS:
    void statusChanged( uint );
private:
    uint mStatus;
    Behaviour * mBehaviour;
};

#endif
