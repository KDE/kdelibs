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

#include <QStringList>

#include "fakeportablemediaplayer.h"

FakePortableMediaPlayer::FakePortableMediaPlayer(FakeDevice *device)
    : FakeDeviceInterface(device)
{

}

FakePortableMediaPlayer::~FakePortableMediaPlayer()
{

}

Solid::PortableMediaPlayer::AccessType FakePortableMediaPlayer::accessMethod() const
{
    QString type = fakeDevice()->property("accessMethod").toString();

    if (type == "MassStorage")
    {
        return Solid::PortableMediaPlayer::MassStorage;
    }
    else
    {
        return Solid::PortableMediaPlayer::Proprietary;
    }
}

QStringList FakePortableMediaPlayer::outputFormats() const
{
    return fakeDevice()->property("outputFormats").toString().simplified().split(',');
}

QStringList FakePortableMediaPlayer::inputFormats() const
{
    return fakeDevice()->property("inputFormats").toString().simplified().split(',');
}

QStringList FakePortableMediaPlayer::playlistFormats() const
{
    return fakeDevice()->property("playlistFormats").toString().simplified().split(',');
}

#include "fakeportablemediaplayer.moc"
