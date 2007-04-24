#include <QCoreApplication>

#include "solidnettestdbusservice.h"
#include "networkingadaptor.h"

TestNetworkingService::TestNetworkingService( )
{
    new NetworkingAdaptor( this );
}

TestNetworkingService::~TestNetworkingService()
{

}

uint TestNetworkingService::requestConnection()
{
    return 0;
}

void TestNetworkingService::releaseConnection()
{

}

uint TestNetworkingService::status() const
{
    return 0;
}

#include "solidnettestdbusservice.moc"

int main( int argc, char** argv )
{
    QCoreApplication app( argc, argv );
    TestNetworkingService serv;
    return app.exec();
}
