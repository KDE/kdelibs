/*
    Copyright 2010 Alex Fiestas <alex@eyeos.org>
    Copyright 2010 UFO Coders <info@ufocoders.com>

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

#include <QString>
#include <QFile>
#include <QDebug>

#include "udevdevice.h"
#include "udevaudiointerface.h"
#include "udevaudiointerface_p.h"

using namespace Solid::Backends::UDev;

UdevAudioInterfacePrivate::UdevAudioInterfacePrivate(UDevDevice *device) : m_device(device)
{
    m_cardnum = -1;
    m_devicenum = -1;
    m_soundcardType = Solid::AudioInterface::InternalSoundcard;
    m_driver = Solid::AudioInterface::UnknownAudioDriver;
    m_type = Solid::AudioInterface::UnknownAudioInterfaceType;

    QString path = m_device->deviceName();

    int lastSlash = path.length() - path.lastIndexOf(QLatin1String("/")) -1;
    QByteArray lastElement = path.right(lastSlash).toAscii();
    
    const char *lastElementAscii = lastElement.constData();

    if (isHardware(lastElementAscii)) {
        return;
    }

    if (isAlsaControl(lastElementAscii)) {
        return;
    }
    if (isAlsaPcm(lastElementAscii)) {
        return;
    }

    if (isAlsaHw(lastElementAscii)) {
        return;
    }
    
    if (isAlsaMidi(lastElementAscii)) {
        return;
    }

    if (isAlsaTimer(lastElementAscii)) {
        return;
    }

    if (isAlsaSequencer(lastElementAscii)) {
        return;
    }

    if (isOSSSequencer(lastElement)) {
        return;
    }

    if (isOSSDevice(lastElement, lastElementAscii)) {
        return;
    }
}

Solid::AudioInterface::SoundcardType UdevAudioInterfacePrivate::soundcardType()
{
    UdevQt::Device device = m_device->udevDevice();
    UdevQt::Device parentDevice = device.parent();

    if (parentDevice.isValid()) {

        QString productName = parentDevice.sysfsProperty("product").toString();
        QString deviceName = m_name;
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
            QString busName = parentDevice.subsystem();
            QString driverName = parentDevice.driver();
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
    }

    return m_soundcardType;
}


bool UdevAudioInterfacePrivate::isHardware(const char* lastElement)
{
    //Root devices like /sys/devices/pci0000:00/0000:00:1b.0/sound/card0 only have sound capability
    //in hal, so ATM just report it as unknown
    if (sscanf(lastElement, "card%d", &m_cardnum) == 1) {
        m_driver = Solid::AudioInterface::UnknownAudioDriver;
        m_name = m_device->property("ID_MODEL_FROM_DATABASE").toString();
        m_type = Solid::AudioInterface::UnknownAudioInterfaceType;
        return true;
    }
    return false;
}

bool UdevAudioInterfacePrivate::isAlsaControl(const char* lastElement)
{
    if (sscanf (lastElement, "controlC%d", &m_cardnum) == 1) {
        m_deviceFile = m_device->property("DEVNAME").toString();
        m_name = cardNumberToName();
        m_driver = Solid::AudioInterface::Alsa;
        m_type = Solid::AudioInterface::AudioControl;
        return true;
    }
    return false;
}

bool UdevAudioInterfacePrivate::isAlsaPcm(const char* lastElement)
{
    char type;
    if (sscanf (lastElement, "pcmC%dD%d%c", &m_cardnum, &m_devicenum, &type) == 3) {
        m_driver = Solid::AudioInterface::Alsa;
        m_name = cardNumberToName();
        QString name = deviceName(type);
        if (!name.isEmpty()) {
            m_name.append(QLatin1String(" (") + name + ')');
        }
        if (type == 'p') {
            m_type = Solid::AudioInterface::AudioOutput;
        } else if(type == 'c') {
            m_type = Solid::AudioInterface::AudioInput;
        } else {
            m_type = Solid::AudioInterface::UnknownAudioInterfaceType;
        }
        return true;
    }
    return false;
}

bool UdevAudioInterfacePrivate::isAlsaHw(const char* lastElement)
{
    if (sscanf(lastElement, "hwC%dD%d", &m_cardnum, &m_devicenum) == 2) {
        m_driver = Solid::AudioInterface::Alsa;
        m_name = cardNumberToName();
        m_name.append(QLatin1String("(HDA Intel ALSA hardware specific Device)"));
        return true;
    }
    return false;
}

bool UdevAudioInterfacePrivate::isAlsaMidi(const char* lastElement)
{
    if (sscanf(lastElement, "midiC%dD%d", &m_cardnum, &m_devicenum) == 2) {
        m_driver = Solid::AudioInterface::Alsa;
        m_name = cardNumberToName();
        m_name.append(QLatin1String("(ALSA MIDI Device)"));
        return true;
    }
    return false;
}

bool UdevAudioInterfacePrivate::isAlsaTimer(const char* lastElement)
{
    if(lastElement == QLatin1String("timer")) {
        /* ALSA Global timer device */
        m_driver = Solid::AudioInterface::Alsa;
        m_name = QLatin1String("ALSA Timer Device");
        m_deviceFile = m_device->property("DEVNAME").toString();
        return true;
    }
    return false;
}

bool UdevAudioInterfacePrivate::isAlsaSequencer(const char* lastElement)
{
    if (lastElement == QLatin1String("seq")) {
        /* ALSA global sequencer devices */
        m_driver = Solid::AudioInterface::Alsa;
        m_name = QLatin1String("ALSA Sequencer Device");
        m_deviceFile = m_device->property("DEVNAME").toString();
        return true;
    }
    return false;
}

bool UdevAudioInterfacePrivate::isOSSSequencer(const QByteArray& lastElement)
{
    if (lastElement.startsWith("sequencer")) {
        /* OSS global sequencer devices */
        m_driver = Solid::AudioInterface::OpenSoundSystem;
        m_name = QLatin1String("OSS Sequencer Device");
        m_deviceFile = m_device->property("DEVNAME").toString();
        return true;
    }
    return false;
}

bool UdevAudioInterfacePrivate::isOSSDevice(const QByteArray& lastElement, const char* lastElementAscii)
{
    m_driver = Solid::AudioInterface::UnknownAudioDriver;
    m_type = Solid::AudioInterface::UnknownAudioInterfaceType;
    m_cardnum = 0;

    m_deviceFile = m_device->property("DEVNAME").toString();

    if (lastElement.startsWith("dsp")) {
        m_driver = Solid::AudioInterface::OpenSoundSystem;
        sscanf (lastElementAscii, "dsp%d", &m_cardnum);
    }

    if (lastElement.startsWith("adsp")) {
        m_driver = Solid::AudioInterface::OpenSoundSystem;
        sscanf (lastElementAscii, "adsp%d", &m_cardnum);
    }

    if (lastElement.startsWith("midi")) {
        m_driver = Solid::AudioInterface::OpenSoundSystem;
        sscanf (lastElementAscii, "midi%d", &m_cardnum);
    }

    if (lastElement.startsWith("amidi")) {
        m_driver = Solid::AudioInterface::OpenSoundSystem;
        sscanf (lastElementAscii, "amidi%d", &m_cardnum);
    }

    if (lastElement.startsWith("audio")) {
        m_driver = Solid::AudioInterface::OpenSoundSystem;
        sscanf (lastElementAscii, "audio%d", &m_cardnum);
    }

    if (lastElement.startsWith("mixer")) {
        m_driver = Solid::AudioInterface::OpenSoundSystem;
        m_type = Solid::AudioInterface::AudioControl;
        sscanf (lastElementAscii, "mixer%d", &m_cardnum);
    }

    if (m_driver != Solid::AudioInterface::UnknownAudioDriver) {
        m_name = cardNumberToName();
        QString path;
        path.sprintf("/proc/asound/card%d/pcm0p/info", m_cardnum);
        QByteArray cardName = grepHelper(path, "name: ");
        if (!cardName.isEmpty()) {
            m_name.append(QLatin1String(" (") + cardName + ')');
        } else {
            m_name.append(QLatin1String(" (OSS Device)"));
        }
        return true;
    }
    return false;
}

QString UdevAudioInterfacePrivate::cardNumberToName()
{
    QString toFind;
    toFind.sprintf("%2d [", m_cardnum);

    QByteArray line = grepHelper(QLatin1String("/proc/asound/cards"), toFind.toAscii());

    int cut =  line.length() - line.lastIndexOf(" - ") - 3;
    QString name = line.right(cut);
    if (!name.isEmpty()) {
        return name;
    }

    return QString();
}

QString UdevAudioInterfacePrivate::deviceName(char type)
{
    QString path;
    path.sprintf("/proc/asound/card%d/pcm%d%c/info",m_cardnum, m_devicenum, type);
    return grepHelper(path, "name: ");
}

QByteArray UdevAudioInterfacePrivate::grepHelper(const QString& path, const QByteArray& grepValue)
{
    QFile file(path);

    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray line = file.readLine();
            while(!line.isNull()) {
                if (line.startsWith(grepValue)) {
                    line.remove(0, grepValue.length());
                    return line.trimmed();
                }
                line = file.readLine();
            }
        } else {
            qDebug() << "grepHelper: Cannot open file: " << path;
        }
    } else {
        qDebug() << "grepHelper: File does not exists: " << path;
    }

    return QByteArray();
}