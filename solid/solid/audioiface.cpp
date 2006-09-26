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

#include "audioiface.h"

#include "soliddefs_p.h"
#include <solid/ifaces/audioiface.h>


Solid::AudioIface::AudioIface( QObject *backendObject )
    : Capability( backendObject )
{
}

Solid::AudioIface::~AudioIface()
{
}


Solid::AudioIface::AudioDriver Solid::AudioIface::driver()
{
    return_SOLID_CALL( Ifaces::AudioIface*, backendObject(), UnknownAudioDriver, driver() );
}

QString Solid::AudioIface::driverHandler()
{
    return_SOLID_CALL( Ifaces::AudioIface*, backendObject(), QString(), driverHandler() );
}

QString Solid::AudioIface::name()
{
    return_SOLID_CALL( Ifaces::AudioIface*, backendObject(), QString(), name() );
}

Solid::AudioIface::AudioIfaceTypes Solid::AudioIface::type()
{
    return_SOLID_CALL( Ifaces::AudioIface*, backendObject(), UnknownAudioIfaceType, type() );
}

#include "audioiface.moc"
