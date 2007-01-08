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

#include "audiohw.h"

#include "soliddefs_p.h"
#include <solid/ifaces/audiohw.h>
#include <QStringList>


Solid::AudioHw::AudioHw( QObject *backendObject )
    : Capability( backendObject )
{
}

Solid::AudioHw::~AudioHw()
{
}


Solid::AudioHw::AudioDriver Solid::AudioHw::driver()
{
    return_SOLID_CALL( Ifaces::AudioHw*, backendObject(), UnknownAudioDriver, driver() );
}

QStringList Solid::AudioHw::driverHandles()
{
    Ifaces::AudioHw *iface = qobject_cast<Ifaces::AudioHw*>( backendObject() );
    if ( iface )
    {
        QString handle = iface->driverHandler();
        if ( iface->driver() == Alsa )
        {
            // TODO add logic from phonon/alsadevicelist/alsadevice.cpp
            //QStringList handles;
            return QStringList( handle );
        }
        else
        {
            return QStringList( handle );
        }
    }
    return QStringList();
}

QString Solid::AudioHw::name()
{
    return_SOLID_CALL( Ifaces::AudioHw*, backendObject(), QString(), name() );
}

Solid::AudioHw::AudioHwTypes Solid::AudioHw::deviceType()
{
    return_SOLID_CALL( Ifaces::AudioHw*, backendObject(), UnknownAudioHwType, deviceType() );
}

Solid::AudioHw::SoundcardType Solid::AudioHw::soundcardType()
{
    return_SOLID_CALL( Ifaces::AudioHw*, backendObject(), InternalSoundcard, soundcardType() );
}

#include "audiohw.moc"
