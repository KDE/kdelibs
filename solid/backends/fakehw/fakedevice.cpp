/*  This file is part of the KDE project
    Copyright (C) 2006 Michaël Larouche <michael.larouche@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/
#include "fakedevice.h"

#include "fakecapability.h"
#include "fakeprocessor.h"
#include "fakeblock.h"
#include "fakestorage.h"
#include "fakecdrom.h"
#include "fakevolume.h"
#include "fakeopticaldisc.h"
#include "fakecamera.h"
#include "fakeportablemediaplayer.h"
#include "fakenetworkhw.h"
#include "fakeacadapter.h"
#include "fakebattery.h"
#include "fakebutton.h"
#include "fakedisplay.h"
#include "fakeaudiohw.h"

#include <QStringList>
#include <QDBusConnection>

using namespace Solid::Ifaces;

class FakeDevice::Private
{
public:
    QString udi;
    QMap<QString, QVariant> propertyMap;
    QStringList capabilityList;
    bool locked;
    QString lockReason;
    bool broken;
};

FakeDevice::FakeDevice(const QString &udi, const QMap<QString, QVariant> &propertyMap)
    : Solid::Ifaces::Device(), d(new Private)
{
    d->udi = udi;
    d->propertyMap = propertyMap;
    d->capabilityList = d->propertyMap["capability"].toString().simplified().split(',');
    d->locked = false;
    d->broken = false;

    QDBusConnection::sessionBus().registerObject( udi, this, QDBusConnection::ExportNonScriptableSlots );

    // Force instantiation of all the capabilities
    // this way they'll get exported on the bus
    // that means they'll be created twice, but that won't be
    // a problem for unit testing.
    foreach ( QString capability, d->capabilityList )
    {
        Capability::Type type = FakeCapability::fromString( capability );
        createCapability( type );
    }
}

FakeDevice::~FakeDevice()
{
    delete d;
}

QString FakeDevice::udi() const
{
    return d->udi;
}

QString FakeDevice::parentUdi() const
{
    return d->propertyMap["parent"].toString();
}

QString FakeDevice::vendor() const
{
    return d->propertyMap["vendor"].toString();
}

QString FakeDevice::product() const
{
    return d->propertyMap["name"].toString();
}

QVariant FakeDevice::property(const QString &key) const
{
    return d->propertyMap[key];
}

QMap<QString, QVariant> FakeDevice::allProperties() const
{
    return d->propertyMap;
}

bool FakeDevice::propertyExists(const QString &key) const
{
    return d->propertyMap.contains(key);
}

bool FakeDevice::setProperty(const QString &key, const QVariant &value)
{
    if ( d->broken ) return false;

    PropertyChange change_type = PropertyModified;

    if (!d->propertyMap.contains(key))
    {
        change_type = PropertyAdded;
    }

    d->propertyMap[key] = value;

    QMap<QString,int> change;
    change[key] = change_type;

    emit propertyChanged( change );

    return true;
}

bool FakeDevice::removeProperty(const QString &key)
{
    if ( d->broken || !d->propertyMap.contains( key ) ) return false;

    d->propertyMap.remove( key );

    QMap<QString,int> change;
    change[key] = PropertyRemoved;

    emit propertyChanged( change );

    return true;
}

void FakeDevice::setBroken( bool broken )
{
    d->broken = broken;
}

bool FakeDevice::isBroken()
{
    return d->broken;
}

bool FakeDevice::lock( const QString &reason )
{
    if ( d->broken || d->locked ) return false;

    d->locked = true;
    d->lockReason = reason;

    return true;
}

bool FakeDevice::unlock()
{
    if ( d->broken || !d->locked ) return false;

    d->locked = false;
    d->lockReason = QString();

    return true;
}

bool FakeDevice::isLocked() const
{
    return d->locked;
}

QString FakeDevice::lockReason() const
{
    return d->lockReason;
}

void FakeDevice::raiseCondition( const QString &condition, const QString &reason )
{
    emit conditionRaised( condition, reason );
}

bool FakeDevice::queryCapability(const Capability::Type &capability) const
{
    return d->capabilityList.contains( FakeCapability::toString(capability) );
}

QObject *FakeDevice::createCapability(const Capability::Type &capability)
{
    // Do not try to cast with a unsupported capability.
    if( !queryCapability(capability) )
        return 0;

    FakeCapability *iface = 0;

    switch(capability)
    {
        case Capability::Processor:
            iface = new FakeProcessor(this);
            break;
        case Capability::Block:
            iface = new FakeBlock(this);
            break;
        case Capability::Storage:
            iface = new FakeStorage(this);
            break;
        case Capability::Cdrom:
            iface = new FakeCdrom(this);
            break;
        case Capability::Volume:
            iface = new FakeVolume(this);
            break;
        case Capability::OpticalDisc:
            iface = new FakeOpticalDisc(this);
            break;
        case Capability::Camera:
            iface = new FakeCamera(this);
            break;
        case Capability::PortableMediaPlayer:
            iface = new FakePortableMediaPlayer(this);
            break;
        case Capability::NetworkHw:
            iface = new FakeNetworkHw(this);
            break;
        case Capability::AcAdapter:
            iface = new FakeAcAdapter(this);
            break;
        case Capability::Battery:
            iface = new FakeBattery(this);
            break;
        case Capability::Button:
            iface = new FakeButton(this);
            break;
        case Capability::Display:
            iface = new FakeDisplay(this);
            break;
        case Capability::AudioHw:
            iface = new FakeAudioHw(this);
            break;
        case Capability::Unknown:
            break;
    }

    if(iface)
    {
        QDBusConnection::sessionBus().registerObject( d->udi+'/'+FakeCapability::toString( capability ), iface,
                                                      QDBusConnection::ExportNonScriptableSlots );
    }

    return iface;
}

#include "fakedevice.moc"
