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

#include "fakeopticaldisc.h"

#include <QStringList>

FakeOpticalDisc::FakeOpticalDisc( FakeDevice *device )
    : FakeVolume( device )
{

}

FakeOpticalDisc::~FakeOpticalDisc()
{

}

FakeOpticalDisc::ContentTypes FakeOpticalDisc::availableContent() const
{
    ContentTypes content;

    QMap<ContentType, QString> map;
    map[Audio] = "audio";
    map[Data] = "data";
    map[VideoCd] = "vcd";
    map[SuperVideoCd] = "svcd";
    map[VideoDvd] ="videodvd";

    QStringList content_typelist = fakeDevice()->property("availableContent").toString().split(',');

    foreach( const ContentType type, map.keys() )
    {
        if ( content_typelist.indexOf(map[type]) != -1 )
        {
            content|= type;
        }
    }

    return content;
}

FakeOpticalDisc::DiscType FakeOpticalDisc::discType() const
{
    QString type = fakeDevice()->property("discType").toString();

    if ( type == "cd_rom" )
    {
        return CdRom;
    }
    else if ( type == "cd_r" )
    {
        return CdRecordable;
    }
    else if ( type == "cd_rw" )
    {
        return CdRewritable;
    }
    else if ( type == "dvd_rom" )
    {
        return DvdRom;
    }
    else if ( type == "dvd_ram" )
    {
        return DvdRam;
    }
    else if ( type == "dvd_r" )
    {
        return DvdRecordable;
    }
    else if ( type == "dvd_rw" )
    {
        return DvdRewritable;
    }
    else if ( type == "dvd_plus_r" )
    {
        return DvdPlusRecordable;
    }
    else if ( type == "dvd_plus_rw" )
    {
        return DvdPlusRewritable;
    }
    else
    {
        return UnknownDiscType;
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

#include "fakeopticaldisc.moc"
