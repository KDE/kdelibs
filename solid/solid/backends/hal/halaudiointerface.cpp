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

#include "backends/hal/halaudiointerface.h"

#include "backends/hal/haldevice.h"

AudioInterface::AudioInterface(HalDevice *device)
    : DeviceInterface(device),
    m_soundcardType(Solid::AudioInterface::InternalSoundcard),
    m_soundcardTypeValid(false)
{

}

AudioInterface::~AudioInterface()
{

}


Solid::AudioInterface::AudioDriver AudioInterface::driver() const
{
    QString capacity = m_device->property("info.category").toString();

    if (capacity == "alsa")
    {
        return Solid::AudioInterface::Alsa;
    }
    else if (capacity == "oss")
    {
        return Solid::AudioInterface::OpenSoundSystem;
    }
    else
    {
        return Solid::AudioInterface::UnknownAudioDriver;
    }
}

QString AudioInterface::driverHandler() const
{
    Solid::AudioInterface::AudioDriver d = driver();

    if (d == Solid::AudioInterface::Alsa)
    {
        QVariant card_id = m_device->property("alsa.card");
        QVariant dev_id = m_device->property("alsa.device");

        if (card_id.isValid() && dev_id.isValid())
        {
            return QString("hw:%1,%2").arg(card_id.toInt()).arg(dev_id.toInt());
        }
        else if (card_id.isValid())
        {
            return QString("hw:%1").arg(card_id.toInt());
        }
        else
        {
            return QString();
        }
    }
    else if (d == Solid::AudioInterface::OpenSoundSystem)
    {
        return m_device->property("oss.device_file").toString();
    }
    else
    {
        return QString();
    }
}

QString AudioInterface::name() const
{
    Solid::AudioInterface::AudioDriver d = driver();

    if (d == Solid::AudioInterface::Alsa)
    {
        QVariant card_id = m_device->property("alsa.card_id");
        if (card_id.isValid())
        {
            return card_id.toString().trimmed() + QLatin1String(" (") + m_device->property("alsa.device_id").toString().trimmed() + ')';
        }
        return m_device->property("alsa.device_id").toString();
    }
    else if (d == Solid::AudioInterface::OpenSoundSystem)
    {
        QVariant card_id = m_device->property("oss.card_id");
        if (card_id.isValid())
        {
            return card_id.toString().trimmed() + QLatin1String(" (") + m_device->property("oss.device_id").toString().trimmed() + ')';
        }
        return m_device->property("oss.device_id").toString();
    }
    else
    {
        return QString();
    }
}

Solid::AudioInterface::AudioInterfaceTypes AudioInterface::deviceType() const
{
    Solid::AudioInterface::AudioDriver d = driver();

    if (d == Solid::AudioInterface::Alsa)
    {
        QString type = m_device->property("alsa.type").toString();

        if (type == "control")
        {
            return Solid::AudioInterface::AudioControl;
        }
        else if (type == "capture")
        {
            return Solid::AudioInterface::AudioInput;
        }
        else if (type == "playback")
        {
            return Solid::AudioInterface::AudioOutput;
        }
        else
        {
            return Solid::AudioInterface::UnknownAudioInterfaceType;
        }
    }
    else if (d == Solid::AudioInterface::OpenSoundSystem)
    {
        QString type = m_device->property("oss.type").toString();

        if (type == "mixer")
        {
            return Solid::AudioInterface::AudioControl;
        }
        else if (type == "pcm")
        {
            return Solid::AudioInterface::AudioInput|Solid::AudioInterface::AudioOutput;
        }
        else
        {
            return Solid::AudioInterface::UnknownAudioInterfaceType;
        }
    }
    else
    {
        return Solid::AudioInterface::UnknownAudioInterfaceType;
    }
}

Solid::AudioInterface::SoundcardType AudioInterface::soundcardType() const
{
    if (m_soundcardTypeValid)
    {
        return m_soundcardType;
    }

    if (! m_device->parentUdi().isEmpty())
    {
        HalDevice parentDevice(m_device->parentUdi());
        QString productName = parentDevice.product();
        QString deviceName = name();
        if (productName.contains("headset", Qt::CaseInsensitive) ||
                productName.contains("headphone", Qt::CaseInsensitive) ||
                deviceName.contains("headset", Qt::CaseInsensitive) ||
                deviceName.contains("headphone", Qt::CaseInsensitive))
        {
            m_soundcardType = Solid::AudioInterface::Headset;
        }
        else if (productName.contains("modem", Qt::CaseInsensitive) ||
                deviceName.contains("modem", Qt::CaseInsensitive))
        {
            m_soundcardType = Solid::AudioInterface::Modem;
        }
        else
        {
            QString busName = parentDevice.property("info.bus").toString();
            QString driverName = parentDevice.property("info.linux.driver").toString();
            if (busName == "ieee1394")
            {
                m_soundcardType = Solid::AudioInterface::FirewireSoundcard;
            }
            else if (busName == "usb" || busName == "usb_device" || driverName.contains("usb", Qt::CaseInsensitive))
            {
                m_soundcardType = Solid::AudioInterface::UsbSoundcard;
            }
            else
            {
                m_soundcardType = Solid::AudioInterface::InternalSoundcard;
            }
        }
        m_soundcardTypeValid = true;
    }
    return m_soundcardType;
}

#include "backends/hal/halaudiointerface.moc"
