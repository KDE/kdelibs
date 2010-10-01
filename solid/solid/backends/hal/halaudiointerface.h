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

#ifndef SOLID_BACKENDS_HAL_HALAUDIOINTERFACE_H
#define SOLID_BACKENDS_HAL_HALAUDIOINTERFACE_H

#include <solid/ifaces/audiointerface.h>
#include "haldeviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace Hal
{
class HalDevice;

class AudioInterface : public DeviceInterface, virtual public Solid::Ifaces::AudioInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::AudioInterface)

public:
    AudioInterface(HalDevice *device);
    virtual ~AudioInterface();

    virtual Solid::AudioInterface::AudioDriver driver() const;
    virtual QVariant driverHandle() const;

    virtual QString name() const;
    virtual Solid::AudioInterface::AudioInterfaceTypes deviceType() const;
    virtual Solid::AudioInterface::SoundcardType soundcardType() const;

private:
    mutable Solid::AudioInterface::SoundcardType m_soundcardType;
    mutable bool m_soundcardTypeValid;
};
}
}
}

#endif // SOLID_BACKENDS_HAL_HALAUDIOINTERFACE_H
