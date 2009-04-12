/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "wmicdrom.h"

#include <QtCore/QStringList>

using namespace Solid::Backends::Wmi;

Cdrom::Cdrom(WmiDevice *device)
    : Storage(device), m_ejectInProgress(false)
{
    connect(device, SIGNAL(conditionRaised(const QString &, const QString &)),
             this, SLOT(slotCondition(const QString &, const QString &)));
}

Cdrom::~Cdrom()
{

}


Solid::OpticalDrive::MediumTypes Cdrom::supportedMedia() const
{
    Solid::OpticalDrive::MediumTypes supported;

    QMap<Solid::OpticalDrive::MediumType, QString> map;
    map[Solid::OpticalDrive::Cdr] = "storage.cdrom.cdr";
    map[Solid::OpticalDrive::Cdrw] = "storage.cdrom.cdrw";
    map[Solid::OpticalDrive::Dvd] = "storage.cdrom.dvd";
    map[Solid::OpticalDrive::Dvdr] = "storage.cdrom.dvdr";
    map[Solid::OpticalDrive::Dvdrw] ="storage.cdrom.dvdrw";
    map[Solid::OpticalDrive::Dvdram] ="storage.cdrom.dvdram";
    map[Solid::OpticalDrive::Dvdplusr] ="storage.cdrom.dvdplusr";
    map[Solid::OpticalDrive::Dvdplusrw] ="storage.cdrom.dvdplusrw";
    map[Solid::OpticalDrive::Dvdplusdl] ="storage.cdrom.dvdplusrdl";
    map[Solid::OpticalDrive::Dvdplusdlrw] ="storage.cdrom.dvdplusrwdl";
    map[Solid::OpticalDrive::Bd] ="storage.cdrom.bd";
    map[Solid::OpticalDrive::Bdr] ="storage.cdrom.bdr";
    map[Solid::OpticalDrive::Bdre] ="storage.cdrom.bdre";
    map[Solid::OpticalDrive::HdDvd] ="storage.cdrom.hddvd";
    map[Solid::OpticalDrive::HdDvdr] ="storage.cdrom.hddvdr";
    map[Solid::OpticalDrive::HdDvdrw] ="storage.cdrom.hddvdrw";

    foreach (const Solid::OpticalDrive::MediumType type, map.keys())
    {
        if (m_device->property(map[type]).toBool())
        {
            supported|= type;
        }
    }

    return supported;
}

int Cdrom::readSpeed() const
{
    return m_device->property("storage.cdrom.read_speed").toInt();
}

int Cdrom::writeSpeed() const
{
    return m_device->property("storage.cdrom.write_speed").toInt();
}

QList<int> Cdrom::writeSpeeds() const
{
    QList<int> speeds;
    QStringList speed_strlist = m_device->property("storage.cdrom.write_speeds").toStringList();

    foreach (const QString &speed_str, speed_strlist)
    {
        speeds << speed_str.toInt();
    }

    return speeds;
}

void Cdrom::slotCondition(const QString &name, const QString &/*reason */)
{
    if (name == "EjectPressed")
    {
        emit ejectPressed(m_device->udi());
    }
}

bool Cdrom::eject()
{
    if (m_ejectInProgress) {
        return false;
    }
    m_ejectInProgress = true;

    return callWmiDriveEject();
}

bool Cdrom::callWmiDriveEject()
{
    QString udi = m_device->udi();
    QString interface = "org.freedesktop.Wmi.Device.Storage";

    // HACK: Eject doesn't work on cdrom drives when there's a mounted disc,
    // let's try to workaround this by calling a child volume...
    // if (m_device->property("storage.removable.media_available").toBool()) {
        // QDBusInterface manager("org.freedesktop.Wmi",
                               // "/org/freedesktop/Wmi/Manager",
                               // "org.freedesktop.Wmi.Manager",
                               // QDBusConnection::systemBus());

        // QDBusReply<QStringList> reply = manager.call("FindDeviceStringMatch", "info.parent", udi);

        // if (reply.isValid())
        // {
            // QStringList udis = reply;
            // if (!udis.isEmpty()) {
                // udi = udis[0];
                // interface = "org.freedesktop.Wmi.Device.Volume";
            // }
        // }
    // }

    // QDBusConnection c = QDBusConnection::systemBus();
    // QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Wmi", udi,
                                                      // interface, "Eject");

    // msg << QStringList();


    // return c.callWithCallback(msg, this,
                              // SLOT(slotDBusReply(const QDBusMessage &)),
                              // SLOT(slotDBusError(const QDBusError &)));
    return false;
}

void Solid::Backends::Wmi::Cdrom::slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    if (m_ejectInProgress) {
        m_ejectInProgress = false;

        if (exitCode==0) {
            emit ejectDone(Solid::NoError, QVariant(), m_device->udi());
        } else {
            emit ejectDone(Solid::UnauthorizedOperation,
                           m_process->readAllStandardError(),
                           m_device->udi());
        }
    }

    delete m_process;
}

#include "backends/wmi/wmicdrom.moc"
