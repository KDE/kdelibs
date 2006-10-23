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

#include <QStringList>

#include "fakeaudiohw.h"


FakeAudioHw::FakeAudioHw( FakeDevice *device )
    : FakeCapability( device )
{

}

FakeAudioHw::~FakeAudioHw()
{

}


Solid::AudioHw::AudioDriver FakeAudioHw::driver()
{
    QString driver = fakeDevice()->property( "driver" ).toString();

    if ( driver == "alsa" )
    {
        return Solid::AudioHw::Alsa;
    }
    else if ( driver == "oss" )
    {
        return Solid::AudioHw::OpenSoundSystem;
    }
    else
    {
        return Solid::AudioHw::UnknownAudioDriver;
    }
}

QString FakeAudioHw::driverHandler()
{
    return fakeDevice()->property( "driverHandler" ).toString();
}

QString FakeAudioHw::name()
{
    return fakeDevice()->property( "name" ).toString();
}

Solid::AudioHw::AudioHwTypes FakeAudioHw::type()
{
    Solid::AudioHw::AudioHwTypes result;

    QStringList type_list = fakeDevice()->property( "type" ).toString().split( "," );

    foreach( QString type_str, type_list )
    {
        if ( type_str == "control" )
        {
            result|=Solid::AudioHw::AudioControl;
        }
        else if ( type_str == "input" )
        {
            result|=Solid::AudioHw::AudioInput;
        }
        else if ( type_str == "output" )
        {
            result|=Solid::AudioHw::AudioOutput;
        }
    }

    return result;
}

#include "fakeaudiohw.moc"
