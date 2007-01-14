/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>

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

#include "fakecdrom.h"

#include <QStringList>

FakeCdrom::FakeCdrom( FakeDevice *device )
    : FakeStorage( device )
{

}

FakeCdrom::~FakeCdrom()
{

}

Solid::Cdrom::MediumTypes FakeCdrom::supportedMedia() const
{
    Solid::Cdrom::MediumTypes supported;

    QMap<Solid::Cdrom::MediumType, QString> map;
    map[Solid::Cdrom::Cdr] = "cdr";
    map[Solid::Cdrom::Cdrw] = "cdrw";
    map[Solid::Cdrom::Dvd] = "dvd";
    map[Solid::Cdrom::Dvdr] = "dvdr";
    map[Solid::Cdrom::Dvdrw] ="dvdrw";
    map[Solid::Cdrom::Dvdram] ="dvdram";
    map[Solid::Cdrom::Dvdplusr] ="dvdplusr";
    map[Solid::Cdrom::Dvdplusrw] ="dvdplusrw";
    map[Solid::Cdrom::Dvdplusdl] ="dvdplusrdl";

    QStringList supported_medialist = fakeDevice()->property("supportedMedia").toString().simplified().split(',');

    foreach( const Solid::Cdrom::MediumType type, map.keys() )
    {
        if ( supported_medialist.indexOf(map[type]) != -1 )
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

    foreach(const QString &speed_str, speed_strlist)
    {
        speeds << speed_str.toInt();
    }

    return speeds;
}

#include "fakecdrom.moc"
