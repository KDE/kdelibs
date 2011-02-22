/*
    Copyright 2010 Michael Zanetti <mzanetti@kde.org>
    Copyright 2010 Lukas Tinkl <ltinkl@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "upower.h"
#include "upowerdevice.h"
#include "upowerdeviceinterface.h"
#include "upowergenericinterface.h"
#include "upoweracadapter.h"
#include "upowerbattery.h"

#include <solid/genericinterface.h>
#include <solid/device.h>

#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <QtDBus/QDBusPendingReply>

using namespace Solid::Backends::UPower;

UPowerDevice::UPowerDevice(const QString &udi)
    : Solid::Ifaces::Device()
    , m_device(UP_DBUS_SERVICE,
               udi,
               UP_DBUS_INTERFACE_DEVICE,
               QDBusConnection::systemBus())
    , m_udi(udi)
{
    if (m_device.isValid())
        connect(&m_device, SIGNAL(Changed()), this, SLOT(slotChanged()));
}

UPowerDevice::~UPowerDevice()
{
}

QObject* UPowerDevice::createDeviceInterface(const Solid::DeviceInterface::Type& type)
{
    if (!queryDeviceInterface(type)) {
        return 0;
    }

    DeviceInterface *iface = 0;
    switch (type)
    {
    case Solid::DeviceInterface::GenericInterface:
        iface = new GenericInterface(this);
        break;
    case Solid::DeviceInterface::AcAdapter:
        iface = new AcAdapter(this);
        break;
    case Solid::DeviceInterface::Battery:
        iface = new Battery(this);
        break;
    default:
        break;
    }
    return iface;
}

bool UPowerDevice::queryDeviceInterface(const Solid::DeviceInterface::Type& type) const
{
    const uint uptype = prop("Type").toUInt();
    switch (type)
    {
        case Solid::DeviceInterface::GenericInterface:
            return true;
        case Solid::DeviceInterface::Battery:
            return (uptype == 2 || uptype == 3);
        case Solid::DeviceInterface::AcAdapter:
            return (uptype == 1);
        default:
            return false;
    }
}

QStringList UPowerDevice::emblems() const
{
    return QStringList();
}

QString UPowerDevice::description() const
{
    if (queryDeviceInterface(Solid::DeviceInterface::AcAdapter))
        return QObject::tr("A/C Adapter");
    else if (queryDeviceInterface(Solid::DeviceInterface::Battery))
        return QObject::tr("%1 Battery", "%1 is battery technology").arg(batteryTechnology());
    else
        return product();
}

QString UPowerDevice::batteryTechnology() const
{
    const uint tech = prop("Technology").toUInt();
    switch (tech)
    {
    case 1:
        return QObject::tr("Lithium Ion", "battery technology");
    case 2:
        return QObject::tr("Lithium Polymer", "battery technology");
    case 3:
        return QObject::tr("Lithium Iron Phosphate", "battery technology");
    case 4:
        return QObject::tr("Lead Acid", "battery technology");
    case 5:
        return QObject::tr("Nickel Cadmium", "battery technology");
    case 6:
        return QObject::tr("Nickel Metal Hydride", "battery technology");
    default:
        return QObject::tr("Unknown", "battery technology");
    }
}

QString UPowerDevice::icon() const
{
    if (queryDeviceInterface(Solid::DeviceInterface::AcAdapter)) {
        return "preferences-system-power-management";

    } else if (queryDeviceInterface(Solid::DeviceInterface::Battery)) {
        return "battery";

    } else {
        return QString();
    }
}

QString UPowerDevice::product() const
{
    QString result = prop("Model").toString();

    if (result.isEmpty()) {
        result = description();
    }

    return result;
}

QString UPowerDevice::vendor() const
{
    return prop("Vendor").toString();
}

QString UPowerDevice::udi() const
{
    return m_udi;
}

QString UPowerDevice::parentUdi() const
{
    return UP_UDI_PREFIX;
}

void UPowerDevice::checkCache(const QString &key) const
{
    if (m_cache.isEmpty()) // recreate the cache
        allProperties();

    if (m_cache.contains(key))
        return;

    QVariant reply = m_device.property(key.toUtf8());

    if (reply.isValid()) {
        m_cache[key] = reply;
    } else {
        m_cache[key] = QVariant();
    }
}

QVariant UPowerDevice::prop(const QString &key) const
{
    checkCache(key);
    return m_cache.value(key);
}

bool UPowerDevice::propertyExists(const QString &key) const
{
    checkCache(key);
    return m_cache.contains(key);
}

QMap<QString, QVariant> UPowerDevice::allProperties() const
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_device.service(), m_device.path(),
                                                       "org.freedesktop.DBus.Properties", "GetAll");
    QDBusPendingReply< QVariantMap > reply = QDBusConnection::systemBus().asyncCall(call);
    reply.waitForFinished();

    if (reply.isValid())
        m_cache = reply.value();
    else
        m_cache.clear();

    return m_cache;
}

void UPowerDevice::slotChanged()
{
    // given we cannot know which property/ies changed, clear the cache
    m_cache.clear();
    emit changed();
}
