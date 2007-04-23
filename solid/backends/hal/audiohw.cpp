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

#include "haldevice.h"
#include <kdebug.h>

AudioHw::AudioHw(HalDevice *device)
    : DeviceInterface(device),
    m_soundcardType(Solid::AudioHw::InternalSoundcard),
    m_soundcardTypeValid(false)
{

}

AudioHw::~AudioHw()
{

}


Solid::AudioHw::AudioDriver AudioHw::driver() const
{
    QString capacity = m_device->property("info.category").toString();

    if (capacity == "alsa")
    {
        return Solid::AudioHw::Alsa;
    }
    else if (capacity == "oss")
    {
        return Solid::AudioHw::OpenSoundSystem;
    }
    else
    {
        return Solid::AudioHw::UnknownAudioDriver;
    }
}

QString AudioHw::driverHandler() const
{
    Solid::AudioHw::AudioDriver d = driver();

    if (d == Solid::AudioHw::Alsa)
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
    else if (d == Solid::AudioHw::OpenSoundSystem)
    {
        return m_device->property("oss.device_file").toString();
    }
    else
    {
        return QString();
    }
}

QString AudioHw::name() const
{
    Solid::AudioHw::AudioDriver d = driver();

    if (d == Solid::AudioHw::Alsa)
    {
        QVariant card_id = m_device->property("alsa.card_id");
        if (card_id.isValid())
        {
            return card_id.toString().trimmed() + QLatin1String(" (") + m_device->property("alsa.device_id").toString().trimmed() + ')';
        }
        return m_device->property("alsa.device_id").toString();
    }
    else if (d == Solid::AudioHw::OpenSoundSystem)
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

Solid::AudioHw::AudioHwTypes AudioHw::deviceType() const
{
    Solid::AudioHw::AudioDriver d = driver();

    if (d == Solid::AudioHw::Alsa)
    {
        QString type = m_device->property("alsa.type").toString();

        if (type == "control")
        {
            return Solid::AudioHw::AudioControl;
        }
        else if (type == "capture")
        {
            return Solid::AudioHw::AudioInput;
        }
        else if (type == "playback")
        {
            return Solid::AudioHw::AudioOutput;
        }
        else
        {
            return Solid::AudioHw::UnknownAudioHwType;
        }
    }
    else if (d == Solid::AudioHw::OpenSoundSystem)
    {
        QString type = m_device->property("oss.type").toString();

        if (type == "mixer")
        {
            return Solid::AudioHw::AudioControl;
        }
        else if (type == "pcm")
        {
            return Solid::AudioHw::AudioInput|Solid::AudioHw::AudioOutput;
        }
        else
        {
            return Solid::AudioHw::UnknownAudioHwType;
        }
    }
    else
    {
        return Solid::AudioHw::UnknownAudioHwType;
    }
}

Solid::AudioHw::SoundcardType AudioHw::soundcardType() const
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
        kDebug() << k_funcinfo << productName << ", " << deviceName << endl;
        if (productName.contains("headset", Qt::CaseInsensitive) ||
                productName.contains("headphone", Qt::CaseInsensitive) ||
                deviceName.contains("headset", Qt::CaseInsensitive) ||
                deviceName.contains("headphone", Qt::CaseInsensitive))
        {
            m_soundcardType = Solid::AudioHw::Headset;
        }
        else if (productName.contains("modem", Qt::CaseInsensitive) ||
                deviceName.contains("modem", Qt::CaseInsensitive))
        {
            m_soundcardType = Solid::AudioHw::Modem;
        }
        else
        {
            QString busName = parentDevice.property("info.bus").toString();
            QString driverName = parentDevice.property("info.linux.driver").toString();
            kDebug() << k_funcinfo << busName << ", " << driverName << endl;
            if (busName == "ieee1394")
            {
                m_soundcardType = Solid::AudioHw::FirewireSoundcard;
            }
            else if (busName == "usb" || busName == "usb_device" || driverName.contains("usb", Qt::CaseInsensitive))
            {
                m_soundcardType = Solid::AudioHw::UsbSoundcard;
            }
            else
            {
                m_soundcardType = Solid::AudioHw::InternalSoundcard;
            }
        }
        m_soundcardTypeValid = true;
    }
    return m_soundcardType;
}

#include "audiohw.moc"
