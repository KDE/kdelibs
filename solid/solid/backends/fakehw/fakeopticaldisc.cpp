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

#include "fakeopticaldisc.h"

#include <QtCore/QStringList>

using namespace Solid::Backends::Fake;

FakeOpticalDisc::FakeOpticalDisc(FakeDevice *device)
    : FakeVolume(device)
{

}

FakeOpticalDisc::~FakeOpticalDisc()
{

}

Solid::OpticalDisc::ContentTypes FakeOpticalDisc::availableContent() const
{
    Solid::OpticalDisc::ContentTypes content;

    QMap<Solid::OpticalDisc::ContentType, QString> map;
    map[Solid::OpticalDisc::Audio] = "audio";
    map[Solid::OpticalDisc::Data] = "data";
    map[Solid::OpticalDisc::VideoCd] = "vcd";
    map[Solid::OpticalDisc::SuperVideoCd] = "svcd";
    map[Solid::OpticalDisc::VideoDvd] ="videodvd";

    QStringList content_typelist = fakeDevice()->property("availableContent").toString().split(',');

    foreach (const Solid::OpticalDisc::ContentType type, map.keys())
    {
        if (content_typelist.indexOf(map[type]) != -1)
        {
            content|= type;
        }
    }

    return content;
}

Solid::OpticalDisc::DiscType FakeOpticalDisc::discType() const
{
    QString type = fakeDevice()->property("discType").toString();

    if (type == "cd_rom")
    {
        return Solid::OpticalDisc::CdRom;
    }
    else if (type == "cd_r")
    {
        return Solid::OpticalDisc::CdRecordable;
    }
    else if (type == "cd_rw")
    {
        return Solid::OpticalDisc::CdRewritable;
    }
    else if (type == "dvd_rom")
    {
        return Solid::OpticalDisc::DvdRom;
    }
    else if (type == "dvd_ram")
    {
        return Solid::OpticalDisc::DvdRam;
    }
    else if (type == "dvd_r")
    {
        return Solid::OpticalDisc::DvdRecordable;
    }
    else if (type == "dvd_rw")
    {
        return Solid::OpticalDisc::DvdRewritable;
    }
    else if (type == "dvd_plus_r")
    {
        return Solid::OpticalDisc::DvdPlusRecordable;
    }
    else if (type == "dvd_plus_rw")
    {
        return Solid::OpticalDisc::DvdPlusRewritable;
    }
    else if (type == "dvd_plus_r_dl")
    {
        return Solid::OpticalDisc::DvdPlusRecordableDuallayer;
    }
    else if (type == "dvd_plus_rw_dl")
    {
        return Solid::OpticalDisc::DvdPlusRewritableDuallayer;
    }
    else if (type == "bd_rom")
    {
        return Solid::OpticalDisc::BluRayRom;
    }
    else if (type == "bd_r")
    {
        return Solid::OpticalDisc::BluRayRecordable;
    }
    else if (type == "bd_re")
    {
        return Solid::OpticalDisc::BluRayRewritable;
    }
    else if (type == "hddvd_rom")
    {
        return Solid::OpticalDisc::HdDvdRom;
    }
    else if (type == "hddvd_r")
    {
        return Solid::OpticalDisc::HdDvdRecordable;
    }
    else if (type == "hddvd_rw")
    {
        return Solid::OpticalDisc::HdDvdRewritable;
    }
    else
    {
        return Solid::OpticalDisc::UnknownDiscType;
    }
}

bool FakeOpticalDisc::isAppendable() const
{
    return fakeDevice()->property("isAppendable").toBool();
}

bool FakeOpticalDisc::isBlank() const
{
    return fakeDevice()->property("isBlank").toBool();
}

bool FakeOpticalDisc::isRewritable() const
{
    return fakeDevice()->property("isRewritable").toBool();
}

qulonglong FakeOpticalDisc::capacity() const
{
    return fakeDevice()->property("capacity").toULongLong();
}

#include "backends/fakehw/fakeopticaldisc.moc"
