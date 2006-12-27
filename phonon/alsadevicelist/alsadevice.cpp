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

#include "alsadevice.h"
#include <QString>
#include "alsadevice_p.h"
#include <alsa/asoundlib.h>
#include "alsadeviceenumerator.h"
#include <kdebug.h>

namespace Phonon
{
AlsaDevice::AlsaDevice()
    : d(new AlsaDevicePrivate)
{
}

AlsaDevice::AlsaDevice(int card, int device)
    : d(new AlsaDevicePrivate)
{
    QString dmixName(QLatin1String("dmix:") + QString::number(card));
    QString hwName(QLatin1String("hw:") + QString::number(card));
    if (device != -1) {
        dmixName += ',';
        dmixName += QString::number(device);
        hwName += ',';
        hwName += QString::number(device);
    }
    kDebug(603) << k_funcinfo << dmixName << "/" << hwName << endl;

    // TODO only add the dmix device if it's actually usable with a standard PCM stream like 48kHz & 16bit
    /*{
        snd_pcm_t *pcm;
        if (0 == snd_pcm_open(&pcm, dmixName.toLatin1().constData(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) {
            snd_pcm_close(pcm);
        }
    }*/
    d->deviceIds << dmixName;
    d->deviceIds << hwName;
    d->deviceInfoFromControlDevice(hwName);
    d->deviceInfoFromPcmDevice(dmixName);
    d->deviceInfoFromPcmDevice(hwName);

    d->merge();
}

AlsaDevice::AlsaDevice(const QString &deviceName, AlsaControlOrPcm controlOrPcm)
    : d(new AlsaDevicePrivate)
{
    kDebug(603) << k_funcinfo << deviceName << (controlOrPcm & Control ? " ctl" : "") << (controlOrPcm & Pcm ? " pcm" : "") << endl;
    if (controlOrPcm & Control) {
        d->deviceInfoFromControlDevice(deviceName);
    }
    if (controlOrPcm & Pcm) {
        d->deviceInfoFromPcmDevice(deviceName);
    }

    d->merge();
}

void AlsaDevicePrivate::deviceInfoFromControlDevice(const QString &deviceName)
{
    snd_ctl_card_info_t *cardInfo;
    snd_ctl_card_info_malloc(&cardInfo);

    snd_ctl_t *ctl;
    if (0 == snd_ctl_open(&ctl, deviceName.toLatin1().constData(), 0 /*open mode: blocking, sync*/)) {
        if (0 == snd_ctl_card_info(ctl, cardInfo)) {
            //Get card identifier from a CTL card info.
            internalId = snd_ctl_card_info_get_id(cardInfo);
            kDebug(603) << k_funcinfo << internalId << endl;

            if (!deviceIds.contains(deviceName)) {
                deviceIds << deviceName;
            }

            //Get card name from a CTL card info.
            cardName = QString(snd_ctl_card_info_get_name(cardInfo)).trimmed();

            //Get card long name from a CTL card info.
            //kDebug(603) << snd_ctl_card_info_get_longname(cardInfo) << endl;

            //Get card mixer name from a CTL card info.
            mixerName = QString(snd_ctl_card_info_get_mixername(cardInfo)).trimmed();

            //Get card component list from a CTL card info.
            //kDebug(603) << snd_ctl_card_info_get_components(cardInfo) << endl;

            valid = true;

            //Get card driver name from a CTL card info.
            QString driver = snd_ctl_card_info_get_driver(cardInfo);
            if (driver.contains("usb", Qt::CaseInsensitive)) {
                // it's an external USB device
                if (cardName.contains("headset", Qt::CaseInsensitive)
                        || cardName.contains("headphone", Qt::CaseInsensitive)
                        || mixerName.contains("headset", Qt::CaseInsensitive)
                        || mixerName.contains("headphone", Qt::CaseInsensitive)) {
                    // it's a headset
                    icon = QLatin1String("headset");
                } else {
                    icon = QLatin1String("usb-soundcard");
                }
            } else {
                icon = QLatin1String("internal-soundcard");
            }
        }
        snd_ctl_close(ctl);
    }
    snd_ctl_card_info_free(cardInfo);
}

void AlsaDevicePrivate::merge()
{
    // check whether this device info has to be merged with another one
    AlsaDevice *altDevice = AlsaDeviceEnumerator::self()->deviceFor(internalId);
    if (altDevice) {
        Q_ASSERT(cardName == altDevice->d->cardName);
        Q_ASSERT(mixerName == altDevice->d->mixerName);
        Q_ASSERT(internalId == altDevice->d->internalId);
        Q_ASSERT(icon == altDevice->d->icon);
        foreach (QString deviceId, deviceIds) {
            altDevice->addDeviceId(deviceId);
            //altDevice->d->pcmName = pcmName;
        }
        valid = false;
    }
}

void AlsaDevicePrivate::deviceInfoFromPcmDevice(const QString &deviceName)
{
    snd_pcm_info_t *pcmInfo;
    snd_pcm_info_malloc(&pcmInfo);

    snd_pcm_t *pcm;
    if (0 == snd_pcm_open(&pcm, deviceName.toLatin1().constData(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK /*open mode: non-blocking, sync*/)) {
        if (0 == snd_pcm_info(pcm, pcmInfo)) {
            if (internalId.isNull()) {
                internalId = snd_pcm_info_get_id(pcmInfo);
            }
            //kDebug(603) << k_funcinfo << internalId << endl;
            if (!deviceIds.contains(deviceName)) {
                deviceIds << deviceName;
            }
            pcmName = QString(snd_pcm_info_get_name(pcmInfo)).trimmed();
            QString tmp = QString(deviceName).trimmed();
        }
        snd_pcm_close(pcm);
    }
    snd_pcm_info_free(pcmInfo);
}

void AlsaDevice::addDeviceId(const QString &deviceId)
{
    if (d->deviceIds.contains(deviceId)) {
        return;
    }
    d->deviceIds << deviceId;
}

bool AlsaDevice::isValid() const
{
    return d->valid;
}

AlsaDevice::AlsaDevice(const AlsaDevice& rhs)
    : d(rhs.d)
{
}

AlsaDevice::~AlsaDevice()
{
}

AlsaDevice &AlsaDevice::operator=(const AlsaDevice &rhs)
{
    d = rhs.d;
    return *this;
}

bool AlsaDevice::operator==(const AlsaDevice &rhs) const
{
    return (d->cardName == rhs.d->cardName && d->mixerName == rhs.d->mixerName && d->icon == rhs.d->icon);
}

QString AlsaDevice::cardName() const
{
    return d->cardName;
}

QString AlsaDevice::mixerName() const
{
    return d->mixerName;
}

QStringList AlsaDevice::deviceIds() const
{
    return d->deviceIds;
}

QString AlsaDevice::iconName() const
{
    return d->icon;
}

} // namespace Phonon

// vim: sw=4 sts=4 et tw=100
