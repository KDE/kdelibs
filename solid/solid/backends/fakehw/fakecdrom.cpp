/*
    Copyright 2006 Davide Bettio <davide.bettio@kdemail.net>

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

#include "fakecdrom.h"

#include <QtCore/QStringList>

using namespace Solid::Backends::Fake;

FakeCdrom::FakeCdrom(FakeDevice *device)
    : FakeStorage(device)
{

}

FakeCdrom::~FakeCdrom()
{

}

Solid::OpticalDrive::MediumTypes FakeCdrom::supportedMedia() const
{
    Solid::OpticalDrive::MediumTypes supported;

    QMap<Solid::OpticalDrive::MediumType, QString> map;
    map[Solid::OpticalDrive::Cdr] = "cdr";
    map[Solid::OpticalDrive::Cdrw] = "cdrw";
    map[Solid::OpticalDrive::Dvd] = "dvd";
    map[Solid::OpticalDrive::Dvdr] = "dvdr";
    map[Solid::OpticalDrive::Dvdrw] ="dvdrw";
    map[Solid::OpticalDrive::Dvdram] ="dvdram";
    map[Solid::OpticalDrive::Dvdplusr] ="dvdplusr";
    map[Solid::OpticalDrive::Dvdplusrw] ="dvdplusrw";
    map[Solid::OpticalDrive::Dvdplusdl] ="dvdplusrdl";
    map[Solid::OpticalDrive::Dvdplusdlrw] ="dvdplusrwdl";
    map[Solid::OpticalDrive::Bd] ="bd";
    map[Solid::OpticalDrive::Bdr] ="bdr";
    map[Solid::OpticalDrive::Bdre] ="bdre";
    map[Solid::OpticalDrive::HdDvd] ="hddvd";
    map[Solid::OpticalDrive::HdDvdr] ="hddvdr";
    map[Solid::OpticalDrive::HdDvdrw] ="hddvdrw";

    QStringList supported_medialist = fakeDevice()->property("supportedMedia").toString().simplified().split(',');

    foreach (const Solid::OpticalDrive::MediumType type, map.keys())
    {
        if (supported_medialist.indexOf(map[type]) != -1)
        {
            supported|= type;
        }
    }

    return supported;
}

int FakeCdrom::readSpeed() const
{
    return fakeDevice()->property("readSpeed").toInt();
}

int FakeCdrom::writeSpeed() const
{
    return fakeDevice()->property("writeSpeed").toInt();
}

QList<int> FakeCdrom::writeSpeeds() const
{
    QList<int> speeds;
    QStringList speed_strlist = fakeDevice()->property("writeSpeeds").toString().simplified().split(',');

    foreach (const QString &speed_str, speed_strlist)
    {
        speeds << speed_str.toInt();
    }

    return speeds;
}

bool FakeCdrom::eject()
{
    return false;
}

#include "backends/fakehw/fakecdrom.moc"
