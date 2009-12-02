/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davide.bettio@kdemail.net>
    Copyright (C) 2007 Jeff Mitchell <kde-dev@emailgoeshere.com>

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

#include "fakeportablemediaplayer.h"

#include <QtCore/QStringList>

using namespace Solid::Backends::Fake;

FakePortableMediaPlayer::FakePortableMediaPlayer(FakeDevice *device)
    : FakeDeviceInterface(device)
{

}

FakePortableMediaPlayer::~FakePortableMediaPlayer()
{

}

QStringList FakePortableMediaPlayer::supportedProtocols() const
{
    return fakeDevice()->property("supportedProtocols").toString().simplified().split(',');
}

QStringList FakePortableMediaPlayer::supportedDrivers(QString protocol) const
{
    Q_UNUSED(protocol);
    return fakeDevice()->property("supportedDrivers").toString().simplified().split(',');
}

QVariant Solid::Backends::Fake::FakePortableMediaPlayer::driverHandle(const QString &driver) const
{
    Q_UNUSED(driver);
    return QVariant();
}

#include "backends/fakehw/fakeportablemediaplayer.moc"
