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

#include "portablemediaplayer.h"

PortableMediaPlayer::PortableMediaPlayer(HalDevice *device)
    : DeviceInterface(device)
{

}

PortableMediaPlayer::~PortableMediaPlayer()
{

}

Solid::PortableMediaPlayer::AccessType PortableMediaPlayer::accessMethod() const
{
    QString type = m_device->property("portable_audio_player.access_method").toString();

    if (type == "storage")
    {
        return Solid::PortableMediaPlayer::MassStorage;
    }
    else
    {
        return Solid::PortableMediaPlayer::Proprietary;
    }
}

QStringList PortableMediaPlayer::outputFormats() const
{
    return m_device->property("portable_audio_player.output_formats").toStringList();
}

QStringList PortableMediaPlayer::inputFormats() const
{
    return m_device->property("portable_audio_player.input_formats").toStringList();
}

QStringList PortableMediaPlayer::playlistFormats() const
{
    return m_device->property("portable_audio_player.playlist_format").toStringList();
}

#include "portablemediaplayer.moc"
