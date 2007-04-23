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

#include "cdrom.h"

#include <QStringList>

Cdrom::Cdrom(HalDevice *device)
    : Storage(device)
{
    connect(device, SIGNAL(conditionRaised(const QString &, const QString &)),
             this, SLOT(slotCondition(const QString &, const QString &)));
}

Cdrom::~Cdrom()
{

}


Solid::Cdrom::MediumTypes Cdrom::supportedMedia() const
{
    Solid::Cdrom::MediumTypes supported;

    QMap<Solid::Cdrom::MediumType, QString> map;
    map[Solid::Cdrom::Cdr] = "storage.cdrom.cdr";
    map[Solid::Cdrom::Cdrw] = "storage.cdrom.cdrw";
    map[Solid::Cdrom::Dvd] = "storage.cdrom.dvd";
    map[Solid::Cdrom::Dvdr] = "storage.cdrom.dvdr";
    map[Solid::Cdrom::Dvdrw] ="storage.cdrom.dvdrw";
    map[Solid::Cdrom::Dvdram] ="storage.cdrom.dvdram";
    map[Solid::Cdrom::Dvdplusr] ="storage.cdrom.dvdplusr";
    map[Solid::Cdrom::Dvdplusrw] ="storage.cdrom.dvdplusrw";
    map[Solid::Cdrom::Dvdplusdl] ="storage.cdrom.dvdplusrdl";

    foreach (const Solid::Cdrom::MediumType type, map.keys())
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

    foreach (const QString speed_str, speed_strlist)
    {
        speeds << speed_str.toInt();
    }

    return speeds;
}

void Cdrom::slotCondition(const QString &name, const QString &/*reason */)
{
    if (name == "EjectPressed")
    {
        emit ejectPressed();
    }
}

#include "cdrom.moc"
