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

FakePortableMediaPlayer::FakePortableMediaPlayer( FakeDevice *device )
    : FakeCapability( device )
{

}

FakePortableMediaPlayer::~FakePortableMediaPlayer()
{

}

FakePortableMediaPlayer::AccessType FakePortableMediaPlayer::accessMethod() const
{
    QString type = fakeDevice()->property("accessMethod").toString();

    if ( type == "MassStorage" )
    {
        return MassStorage;
    }
    else
    {
        return Proprietary;
    }
}

QStringList FakePortableMediaPlayer::outputFormats() const
{
    return fakeDevice()->property("outputFormats").toStringList();
}

QStringList FakePortableMediaPlayer::inputFormats() const
{
    return fakeDevice()->property("inputFormats").toStringList();
}

QStringList FakePortableMediaPlayer::playlistFormats() const
{
    return fakeDevice()->property("playlistFormats").toStringList();
}

#include "fakeportablemediaplayer.moc"
