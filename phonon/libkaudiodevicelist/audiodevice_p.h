/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_AUDIODEVICE_P_H
#define PHONON_AUDIODEVICE_P_H

#include "audiodevice.h"
#include <phonon/config-alsa.h>
#include <QtCore/QStringList>
#include <QtCore/QSharedData>


namespace Phonon
{
class AudioDevicePrivate : public QSharedData
{
    public:
        AudioDevicePrivate()
            : refCount(1),
            driver(Solid::AudioInterface::UnknownAudioDriver),
            index(-1),
            deviceNumber(-1),
            initialPreference(30),
            available(false),
            valid(false),
            captureDevice(false),
            playbackDevice(false),
            isAdvanced(false)
        {
        }

        void changeIndex(int newIndex, KSharedConfig::Ptr config);
        KConfigGroup configGroup(KSharedConfig::Ptr config);
        QString uniqueIdentifierFromDevice(const Solid::Device &);
        void applyHardwareDatabaseOverrides();
#ifdef HAVE_LIBASOUND2
        void deviceInfoFromPcmDevice(const QString &deviceName);
#endif // HAVE_LIBASOUND2

        int refCount;
        QString cardName;
        QStringList deviceIds;
        QString icon;
        QString uniqueId;
        Solid::AudioInterface::AudioDriver driver;
        struct AlsaId
        {
            AlsaId() : card(-1), device(-1), subdevice(-1) {}
            bool operator==(const AlsaId &x) const { return card == x.card && device == x.device && subdevice == x.subdevice; }
            int card;
            int device;
            int subdevice;
        } alsaId;
        int index;
        int deviceNumber;
        int initialPreference;
        bool available : 1;
        bool valid : 1;
        bool captureDevice : 1;
        bool playbackDevice : 1;
        bool isAdvanced : 1;
};
} // namespace Phonon

#endif // PHONON_AUDIODEVICE_P_H
// vim: sw=4 sts=4 et tw=100
