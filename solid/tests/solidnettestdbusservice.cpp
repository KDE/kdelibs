#include <QCoreApplication>
#include <QtDBus>
#include <QTimer>

#include <solid/networking.h>
#include "solidnettestdbusservice.h"
#include "networkingadaptor.h"

QString statusToString( uint status )
{
    QString asString;
    switch ( status )
    {
        case Solid::Networking::Unknown:
            asString = "Unknown";
            break;
        case Solid::Networking::Unconnected:
            asString = "Unconnected";
            break;
        case Solid::Networking::Connecting:
            asString ="Connecting";
            break;
        case Solid::Networking::Connected:
            asString = "Connected";
            break;
        case Solid::Networking::Disconnecting:
            asString ="Disconnecting";
            break;
        default:
            asString = "Crap passed to statusAsString()!";
    }
    return asString;
}

Behaviour::Behaviour( TestNetworkingService * service ) : mService( service )
{
    connect ( mService, SIGNAL( statusChanged( uint ) ), this, SLOT( serviceStatusChanged( uint ) ) );
}

GoOnlineOnRequest::GoOnlineOnRequest( TestNetworkingService * service ) : Behaviour( service )
{
}

void GoOnlineOnRequest::go()
{
    // do nothing this only reacts to events
}

void GoOnlineOnRequest::serviceStatusChanged( uint status )
{
    qDebug( "GoOnlineOnRequest::serviceStatusChanged()" );
    switch ( status )
    {
        case Solid::Networking::Connecting:
            qDebug( "  connecting..." );
            QTimer::singleShot( 5000, this, SLOT( doDelayedConnect() ) );
            break;
        case Solid::Networking::Disconnecting:
            qDebug( "  disconnecting..." );
            QTimer::singleShot( 5000, this, SLOT( doDelayedDisconnect() ) );
            break;
        default:
            ;
    }
}

void GoOnlineOnRequest::doDelayedConnect()
{
    mService->setStatus( ( uint )Solid::Networking::Connected );
}

void GoOnlineOnRequest::doDelayedDisconnect()
{
    mService->setStatus( ( uint )Solid::Networking::Unconnected );
}

TestNetworkingService::TestNetworkingService( const QString & ) : mStatus( Solid::Networking::Unconnected )
{
    new NetworkingAdaptor( this );
    mBehaviour = new GoOnlineOnRequest( this );
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService( "org.kde.Solid.Networking" );
    dbus.registerObject( "/status", this );
}

TestNetworkingService::~TestNetworkingService()
{

}

uint TestNetworkingService::requestConnection()
{
    qDebug( "TestNetworkingService::requestConnection()" );
    if ( mStatus == Solid::Networking::Unconnected )
    {
        setStatus( Solid::Networking::Connecting );
        return Solid::Networking::Accepted;
    }
    return Solid::Networking::AlreadyConnected;
}

void TestNetworkingService::releaseConnection()
{
    qDebug( "TestNetworkingService::releaseConnection()" );
    if ( mStatus == Solid::Networking::Connected )
    {
        setStatus( Solid::Networking::Disconnecting );
    }
}

uint TestNetworkingService::status() const
{
    return mStatus;
}

void TestNetworkingService::setStatus( uint status )
{
    qDebug( "Setting status to %s", qPrintable( statusToString( status ) ) );
    mStatus = status;
    emit statusChanged( mStatus );
}

int main( int argc, char** argv )
{
    QCoreApplication app( argc, argv );
    TestNetworkingService serv( "" );
    return app.exec();
}

// add a ctor arg to TestNetworkingService that sets its behaviour
// behaviour types - request, delay, statuschanged
//                 - deny requests
//                 - request, delay, online, offline
//                 - offline, delay, unrequested online
//                 - intermittent on/offline 
//                 - request, delay, online until released
//                 

#include "solidnettestdbusservice.moc"


