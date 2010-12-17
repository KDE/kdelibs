/*
    Copyright 2010 Alex Fiestas <alex@eyeos.org>
    Copyright 2010 UFO Coders <info@ufocoders.com>

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

#include "udevnetworkinterface.h"
#include "udevdevice.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/if_arp.h>
#include <linux/wireless.h>

#include <QFile>
#include <QFileInfo>
#include <QtCore/QStringList>
#include <QDebug>
using namespace Solid::Backends::UDev;

NetworkInterface::NetworkInterface(UDevDevice *device)
    : DeviceInterface(device)
{
    
}

NetworkInterface::~NetworkInterface()
{

}

QString NetworkInterface::ifaceName() const
{
    return m_device->property("INTERFACE").toString();
}

bool NetworkInterface::isWireless() const
{
    QFile typeFile(m_device->deviceName() + "/type");
    if (!typeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "UdevNetworkInterface: typeFile can't be opened";
        return false;
    }
    int mediaType = typeFile.readAll().trimmed().toInt();
    if (mediaType == ARPHRD_ETHER) {
        struct iwreq iwr;

        int ioctl_fd = socket (PF_INET, SOCK_DGRAM, 0);
        strncpy (iwr.ifr_ifrn.ifrn_name, ifaceName().toAscii().constData(), IFNAMSIZ);

        QFileInfo phyDir(m_device->deviceName() + "/phy80211");
        
        if ((ioctl (ioctl_fd, SIOCGIWNAME, &iwr) == 0) || phyDir.isDir()) {
            return true;
        }
    }
    return false;
}

QString NetworkInterface::hwAddress() const
{
    bool addr_len = false;
    QFile lenFile(m_device->deviceName() + "/addr_len");

    if (lenFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        lenFile.readAll().trimmed().toInt(&addr_len);
        if (addr_len) {
            QFile addrFile(m_device->deviceName() + "/address");
            if (addrFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QByteArray addr = addrFile.readAll().trimmed();
                if (!addr.isEmpty()) {
                    return QString::fromAscii(addr);
                }
            }
        }
    }

    return QString(QLatin1String("00:00:00:00:00:00"));
}

qulonglong NetworkInterface::macAddress() const
{
    QString hwAddr = hwAddress();
    qulonglong mac_address = 0;
    if (hwAddr != QLatin1String("00:00:00:00:00:00")) {
        unsigned int a5, a4, a3, a2, a1, a0;

        if (sscanf (hwAddr.toAscii().constData(), "%x:%x:%x:%x:%x:%x",
                &a5, &a4, &a3, &a2, &a1, &a0) == 6) {
            mac_address =
                ((qulonglong)a5<<40) |
                ((qulonglong)a4<<32) |
                ((qulonglong)a3<<24) |
                ((qulonglong)a2<<16) |
                ((qulonglong)a1<< 8) |
                ((qulonglong)a0<< 0);
        }
    }
    return mac_address;
}

#include "backends/udev/udevnetworkinterface.moc"
