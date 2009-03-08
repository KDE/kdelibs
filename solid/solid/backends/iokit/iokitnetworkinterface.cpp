/*  This file is part of the KDE project
    Copyright (C) 2009 Harald Fernengel <harry@kdevelop.org>

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

#include "iokitnetworkinterface.h"

#include "iokitdevice.h"

#include <QtCore/qstringlist.h>
#include <QtCore/qdebug.h>

using namespace Solid::Backends::IOKit;

NetworkInterface::NetworkInterface(IOKitDevice *device)
    : DeviceInterface(device)
{
}

NetworkInterface::~NetworkInterface()
{
}

QString NetworkInterface::ifaceName() const
{
    return m_device->property(QLatin1String("BSD Name")).toString();
}

bool NetworkInterface::isWireless() const
{
    return m_device->propertyExists(QLatin1String("IO80211HardwareVersion"));
}

QString NetworkInterface::hwAddress() const
{
    // TODO - how to get the hw address? Return a string representation of the MAC addr for now
    const QByteArray ba = IOKitDevice(m_device->parentUdi()).property(QLatin1String("IOMACAddress")).toByteArray();
    qDebug() << ba.length();
    if (ba.length() != 6)
        return QString();

    QString result;
    for (int i = 0; i < 6; ++i) {
        // produce a hex number, 2 digits width, 0-padded followed by a colon
        result += QString::fromLatin1("%1:").arg(uchar(ba.at(i)), 2, 16, QLatin1Char('0'));
    }
    // remove tailing colon
    result.chop(1);

    return result;
}

qulonglong NetworkInterface::macAddress() const
{
    // IOMacAddress property returns the mac address encoded in 6 bytes
    const QByteArray ba = IOKitDevice(m_device->parentUdi()).property(QLatin1String("IOMACAddress")).toByteArray();
    if (ba.length() != 6)
        return 0;

    // need to bit-shift instead of memcpy because of alignment and endianess
    qulonglong result;
    result  = qulonglong((uchar)ba.at(0)) << 40;
    result += qulonglong((uchar)ba.at(1)) << 32;
    result += qulonglong((uchar)ba.at(2)) << 24;
    result += qulonglong((uchar)ba.at(3)) << 16;
    result += qulonglong((uchar)ba.at(4)) << 8;
    result += (uchar)ba.at(5);

    return result;
}

#include "backends/iokit/iokitnetworkinterface.moc"
