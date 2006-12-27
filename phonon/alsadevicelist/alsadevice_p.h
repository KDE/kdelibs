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

#ifndef PHONON_ALSADEVICE_P_H
#define PHONON_ALSADEVICE_P_H

#include "alsadevice.h"
#include <QSharedData>
#include <QStringList>
#include <QString>

namespace Phonon
{
class AlsaDevicePrivate : public QSharedData
{
    public:
        AlsaDevicePrivate() : card(-1), device(-1), valid(false) {}
        void deviceInfoFromControlDevice(const QString &deviceName);
        void deviceInfoFromPcmDevice(const QString &deviceName);
        void merge();

        int card;
        int device;
        QString cardName;
        QString mixerName;
        QString pcmName;
        QStringList deviceIds;
        QString internalId;
        QString icon;
        bool valid;
};
} // namespace Phonon

#endif // PHONON_ALSADEVICE_P_H
// vim: sw=4 sts=4 et tw=100
