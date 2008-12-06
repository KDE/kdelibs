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

#include "fakeaudiointerface.h"

#include <QtCore/QStringList>


using namespace Solid::Backends::Fake;

FakeAudioInterface::FakeAudioInterface(FakeDevice *device)
    : FakeDeviceInterface(device)
{

}

FakeAudioInterface::~FakeAudioInterface()
{

}


Solid::AudioInterface::AudioDriver FakeAudioInterface::driver() const
{
    QString driver = fakeDevice()->property("driver").toString();

    if (driver == "alsa")
    {
        return Solid::AudioInterface::Alsa;
    }
    else if (driver == "oss")
    {
        return Solid::AudioInterface::OpenSoundSystem;
    }
    else
    {
        return Solid::AudioInterface::UnknownAudioDriver;
    }
}

QVariant FakeAudioInterface::driverHandle() const
{
    if (driver() == Solid::AudioInterface::Alsa) {
        const QStringList x = fakeDevice()->property("driverHandle").toString().split(',');
        QList<QVariant> ret;
        foreach (const QString &num, x) {
            ret << num.toInt();
        }
        return ret;
    }
    return fakeDevice()->property("driverHandle");
}

QString FakeAudioInterface::name() const
{
    return fakeDevice()->property("name").toString();
}

Solid::AudioInterface::AudioInterfaceTypes FakeAudioInterface::deviceType() const
{
    Solid::AudioInterface::AudioInterfaceTypes result;

    QStringList type_list = fakeDevice()->property("type").toString().split(',');

    foreach (const QString &type_str, type_list)
    {
        if (type_str == "control")
        {
            result|=Solid::AudioInterface::AudioControl;
        }
        else if (type_str == "input")
        {
            result|=Solid::AudioInterface::AudioInput;
        }
        else if (type_str == "output")
        {
            result|=Solid::AudioInterface::AudioOutput;
        }
    }

    return result;
}

Solid::AudioInterface::SoundcardType FakeAudioInterface::soundcardType() const
{
    QString type_str = fakeDevice()->property("soundcardType").toString();

    if (type_str == "internal")
    {
        return Solid::AudioInterface::InternalSoundcard;
    }
    else if (type_str == "usb")
    {
        return Solid::AudioInterface::UsbSoundcard;
    }
    else if (type_str == "firewire")
    {
        return Solid::AudioInterface::FirewireSoundcard;
    }
    else if (type_str == "headset")
    {
        return Solid::AudioInterface::Headset;
    }
    return Solid::AudioInterface::InternalSoundcard;
}

#include "backends/fakehw/fakeaudiointerface.moc"
