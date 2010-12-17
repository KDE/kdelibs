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


#ifndef AUDIOINTERFACEPRIVATE_H
#define AUDIOINTERFACEPRIVATE_H

#include <solid/ifaces/audiointerface.h>

namespace Solid
{
namespace Backends
{
namespace UDev
{
class UDevDevice;
class UdevAudioInterfacePrivate
{
public:
    UdevAudioInterfacePrivate(UDevDevice *device);

    Solid::AudioInterface::SoundcardType soundcardType();

public:
    int                                         m_cardnum;
    int                                         m_devicenum;
    QString                                     m_name;
    QString                                     m_deviceFile;
    Solid::AudioInterface::AudioDriver          m_driver;
    Solid::AudioInterface::AudioInterfaceTypes  m_type;

private:
    QString cardNumberToName();
    QString deviceName(char type);
    QByteArray grepHelper(const QString &path, const QByteArray &grepValue);

    bool isHardware(const char *lastElement);
    bool isAlsaControl(const char *lastElement);
    bool isAlsaPcm(const char *lastElement);
    bool isAlsaHw(const char *lastElement);
    bool isAlsaMidi(const char *lastElement);
    bool isAlsaTimer(const char *lastElement);
    bool isAlsaSequencer(const char *lastElement);
    bool isOSSSequencer(const QByteArray &lastElement);
    bool isOSSDevice(const QByteArray &lastElement, const char *lastElementAscii);

    Solid::AudioInterface::SoundcardType        m_soundcardType;

private:
  UDevDevice       *m_device;
};
}
}
}

#endif // AUDIOINTERFACEPRIVATE_H
