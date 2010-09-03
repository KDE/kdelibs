/*  Copyright 2010  Michael Zanetti <mzanetti@kde.org>
              2010  Lukas Tinkl <ltinkl@redhat.com>

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
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.

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
#include <QtDBus/QDBusReply>
#include <QtCore/QDebug>

using namespace Solid::Backends::UPower;

UPowerDevice::UPowerDevice(const QString &udi)
    : m_device(UP_DBUS_SERVICE,
               udi,
               UP_DBUS_INTERFACE_DEVICE,
               QDBusConnection::systemBus()),
    m_udi(udi)
{
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
    const uint uptype = property("Type").toUInt();
    switch (type)
    {
        case Solid::DeviceInterface::GenericInterface:
            return true;
        case Solid::DeviceInterface::Battery:
            return (uptype == 2);
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
        return QObject::tr("AC adapter");
    else if (queryDeviceInterface(Solid::DeviceInterface::Battery))
        return QObject::tr("%1 battery", "%1 is battery technology").arg(batteryTechnology());
    else
        return product();
}

QString UPowerDevice::batteryTechnology() const
{
    const uint tech = property("Technology").toUInt();
    switch (tech)
    {
    case 1:
        return QObject::tr("Lithium ion", "battery technology");
    case 2:
        return QObject::tr("Lithium polymer", "battery technology");
    case 3:
        return QObject::tr("Lithium iron phosphate", "battery technology");
    case 4:
        return QObject::tr("Lead acid", "battery technology");
    case 5:
        return QObject::tr("Nickel cadmium", "battery technology");
    case 6:
        return QObject::tr("Nickel metal hydride", "battery technology");
    default:
        return QObject::tr("Unknown", "battery technology");
    }
}

QString UPowerDevice::icon() const
{
    if (queryDeviceInterface(Solid::DeviceInterface::Battery))
        return "battery";

    return QString();
}

QString UPowerDevice::product() const
{
    return property("Model").toString();
}

QString UPowerDevice::vendor() const
{
    return property("Vendor").toString();
}

QString UPowerDevice::udi() const
{
    return m_udi;
}

QString UPowerDevice::parentUdi() const
{
    return QString();
}

void UPowerDevice::checkCache(const QString &key) const
{
    if (m_cache.contains(key) && !m_invalidKeys.contains(key)) {
        return;
    }
    
    QVariant reply = m_device.property(key.toUtf8());

    if (reply.isValid()) {
        m_cache[key] = reply;
    } else {
        m_cache[key] = QVariant();
    }

    m_invalidKeys.remove(key);
}

QVariant UPowerDevice::property(const QString &key) const
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
    QDBusMessage message = QDBusMessage::createMethodCall(m_device.service(), m_device.path(), QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("GetAll"));
    QList<QVariant> arguments;
    arguments << m_device.interface();
    message.setArguments(arguments);

    QDBusMessage reply = m_device.connection().call(message);

    if (reply.type() != QDBusMessage::ReplyMessage)
    {
        qWarning() << Q_FUNC_INFO << "error:" << reply;
        return QVariantMap();
    }

    m_cache = QDBusReply<QVariantMap>(reply).value();

    return m_cache;
}

void UPowerDevice::slotChanged()
{
    // given we cannot know which property/ies changed, clear the cache
    m_cache.clear();
    emit changed();
}
