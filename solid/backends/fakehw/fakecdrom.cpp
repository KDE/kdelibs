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

FakeCdrom::MediumTypes FakeCdrom::supportedMedia() const
{
    MediumTypes supported;

    QMap<MediumType, QString> map;
    map[Cdr] = "cdr";
    map[Cdrw] = "cdrw";
    map[Dvd] = "dvd";
    map[Dvdr] = "dvdr";
    map[Dvdrw] ="dvdrw";
    map[Dvdram] ="dvdram";
    map[Dvdplusr] ="dvdplusr";
    map[Dvdplusrw] ="dvdplusrw";
    map[Dvdplusdl] ="dvdplusrdl";

    QStringList supported_medialist = fakeDevice()->property("supportedMedia").toString().simplified().split(',');

    foreach( const MediumType type, map.keys() )
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

    foreach(const QString speed_str, speed_strlist)
    {
        speeds << speed_str.toInt();
    }

    return speeds;
}

#include "fakecdrom.moc"
