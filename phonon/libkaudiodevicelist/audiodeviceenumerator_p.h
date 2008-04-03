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

#ifndef PHONON_AUDIODEVICEENUMERATOR_P_H
#define PHONON_AUDIODEVICEENUMERATOR_P_H

#include "audiodeviceenumerator.h"
#include <QtCore/QList>
#include "audiodevice.h"
#include <ksharedconfig.h>

namespace Phonon
{
class AudioDeviceEnumeratorPrivate
{
    public:
        AudioDeviceEnumeratorPrivate();

        void _k_deviceAdded(const QString &);
        void _k_deviceRemoved(const QString &);
        void _k_asoundrcChanged(const QString &file);
        void findVirtualDevices();

        QList<AudioDevice> playbackdevicelist;
        QList<AudioDevice> capturedevicelist;
        void findDevices();
        void renameDevices(QList<AudioDevice> *devicelist);

        KSharedConfig::Ptr config;

        AudioDeviceEnumerator q;
};
} // namespace Phonon

#endif // PHONON_AUDIODEVICEENUMERATOR_P_H
// vim: sw=4 sts=4 et tw=100
