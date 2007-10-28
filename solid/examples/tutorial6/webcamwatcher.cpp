#include "webcamwatcher.h"

#include <solid/device.h>
#include <solid/devicenotifier.h>
#include <solid/deviceinterface.h>
#include <solid/video.h>

#include <klocale.h>
#include <kdebug.h>

WebcamWatcher::WebcamWatcher( QObject * parent ) : QObject( parent )
{
    // get a list of all the webcams in the system
    int found = 0;
    foreach (Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Video, QString()))
    {
        m_videoDevices << device.udi();
        getDetails( device );
        found++;
    }
    if ( found == 0 )
    {
        kDebug() << "No video devices found";
    }
    // on deviceAdded, check to see if the device was added
    connect( Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(const QString&)), SLOT(deviceAdded(const QString &)) );
    // likewise if removed
    connect( Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(const QString&)), SLOT(deviceRemoved(const QString &)) );
}

WebcamWatcher::~WebcamWatcher()
{

}

void WebcamWatcher::deviceAdded( const QString & udi )
{
    Solid::Device dev( udi );
    if ( dev.is<Solid::Video>() )
    {
        m_videoDevices << udi;
        getDetails( dev );
    }
}

void WebcamWatcher::getDetails( const Solid::Device & dev )
{
    kDebug() << "New video device at " << dev.udi();
    const Solid::Device * vendorDevice = &dev;
    while ( vendorDevice->isValid() && vendorDevice->vendor().isEmpty() )
    {
        vendorDevice = new Solid::Device( vendorDevice->parentUdi() );
    }
    if ( vendorDevice->isValid() )
    {
        kDebug() << "vendor: " << vendorDevice->vendor() << ", product: " << vendorDevice->product();
    }
    QStringList protocols = dev.as<Solid::Video>()->supportedProtocols();
    if ( protocols.contains( "video4linux" ) )
    {
        QStringList drivers = dev.as<Solid::Video>()->supportedDrivers( "video4linux" );
        if ( drivers.contains( "video4linux" ) )
        {
            kDebug() << "V4L device path is" << dev.as<Solid::Video>()->driverHandle( "video4linux" ).toString();
        }
    }
}

void WebcamWatcher::deviceRemoved( const QString & udi )
{
    Solid::Device dev = Solid::Device( udi );
    int i;
    if ( ( i = m_videoDevices.indexOf( udi ) ) != - 1 ) {
        kDebug() << udi;
        m_videoDevices.removeAt( i );
    }
}
