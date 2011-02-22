/*
    Copyright 2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "halaudiointerface.h"

#include "haldevice.h"
#include <QtDBus/QDBusInterface>

using namespace Solid::Backends::Hal;

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
    QString capacity = m_device->prop("info.category").toString();

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

QVariant AudioInterface::driverHandle() const
{
    Solid::AudioInterface::AudioDriver d = driver();

    if (d == Solid::AudioInterface::Alsa)
    {
        QVariant card_id = m_device->prop("alsa.card");
        QVariant dev_id = m_device->prop("alsa.device");
        QVariant subdev_id;
        //TODO alsa.subdevice

        return QList<QVariant>() << card_id << dev_id << subdev_id;
    }
    else if (d == Solid::AudioInterface::OpenSoundSystem)
    {
        return m_device->prop("oss.device_file");
    }
    else
    {
        return QVariant();
    }
}

QString AudioInterface::name() const
{
    Solid::AudioInterface::AudioDriver d = driver();

    if (d == Solid::AudioInterface::Alsa)
    {
        QVariant card_id = m_device->prop("alsa.card_id");
        if (card_id.isValid())
        {
            return card_id.toString().trimmed() + QLatin1String(" (") + m_device->prop("alsa.device_id").toString().trimmed() + ')';
        }
        return m_device->prop("alsa.device_id").toString();
    }
    else if (d == Solid::AudioInterface::OpenSoundSystem)
    {
        QVariant card_id = m_device->prop("oss.card_id");
        if (card_id.isValid())
        {
            return card_id.toString().trimmed() + QLatin1String(" (") + m_device->prop("oss.device_id").toString().trimmed() + ')';
        }
        return m_device->prop("oss.device_id").toString();
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
        QString type = m_device->prop("alsa.type").toString();

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
        QString type = m_device->prop("oss.type").toString();

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

    QString parentUdi = m_device->parentUdi();
    if (!parentUdi.isEmpty()) {
        QDBusInterface parentIface(QLatin1String("org.freedesktop.Hal"), m_device->parentUdi(), "org.freedesktop.Hal.Device", QDBusConnection::systemBus());
        const QDBusMessage &reply = parentIface.call("GetProperty", QLatin1String("info.subsystem"));
        if ((reply.type() != QDBusMessage::ReplyMessage && reply.errorName() == "org.freedesktop.Hal.NoSuchProperty") ||
                (reply.type() == QDBusMessage::ReplyMessage && reply.arguments().at(0) == "sound")) {
            const QDBusMessage &reply2 = parentIface.call("GetProperty", QLatin1String("info.parent"));
            if (reply2.type() == QDBusMessage::ReplyMessage) {
                parentUdi = reply2.arguments().at(0).toString();
            }
        }

        HalDevice parentDevice(parentUdi);
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
            QString busName = parentDevice.prop("info.subsystem").toString();
            QString driverName = parentDevice.prop("info.linux.driver").toString();
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
