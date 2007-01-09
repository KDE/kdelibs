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
#include <solid/audiohw.h>

namespace Phonon
{
AlsaDevice::AlsaDevice()
    : d(new AlsaDevicePrivate)
{
}

AlsaDevice::AlsaDevice(Solid::AudioHw *audioHw, KSharedConfig::Ptr config)
    : d(new AlsaDevicePrivate)
{
    kDebug(603) << k_funcinfo << audioHw->driverHandles() << endl;
    d->cardName = audioHw->name();
    d->deviceIds = audioHw->driverHandles();
    switch (audioHw->soundcardType()) {
        case Solid::AudioHw::InternalSoundcard:
            d->icon = QLatin1String("pci-card");
            break;
        case Solid::AudioHw::UsbSoundcard:
            d->icon = QLatin1String("usb-device");
            break;
        case Solid::AudioHw::FirewireSoundcard:
            d->icon = QLatin1String("firewire-device");
            break;
        case Solid::AudioHw::Headset:
            d->icon = QLatin1String("headset");
            break;
        case Solid::AudioHw::Modem:
            // should a modem be an invalid device so that it's not shown to the user?
            d->icon = QLatin1String("modem");
            break;
    }
    d->available = true;
    d->valid = true;

    QString groupName;
    if (audioHw->deviceType() == Solid::AudioHw::AudioInput) {
        d->captureDevice = true;
        groupName = QLatin1String("AudioCaptureDevice_");
    } else {
        groupName = QLatin1String("AudioOutputDevice_");
    }
    groupName += d->cardName;

    if (config->hasGroup(groupName)) {
        KConfigGroup deviceGroup(config.data(), groupName);
        d->index = deviceGroup.readEntry("index", -1);
    }
    if (d->index == -1) {
        KConfigGroup globalGroup(config.data(), "Globals");
        int nextIndex = globalGroup.readEntry("nextIndex", 0);
        d->index = nextIndex++;
        globalGroup.writeEntry("nextIndex", nextIndex);

        KConfigGroup deviceGroup(config.data(), groupName);
        deviceGroup.writeEntry("index", d->index);
        deviceGroup.writeEntry("cardName", d->cardName);
        deviceGroup.writeEntry("icon", d->icon);
        deviceGroup.writeEntry("captureDevice", d->captureDevice);
        config->sync();
    }
}

AlsaDevice::AlsaDevice(KConfigGroup &deviceGroup)
    : d(new AlsaDevicePrivate)
{
    d->index = deviceGroup.readEntry("index", d->index);
    d->cardName = deviceGroup.readEntry("cardName", d->cardName);
    d->icon = deviceGroup.readEntry("icon", d->icon);
    d->captureDevice = deviceGroup.readEntry("captureDevice", d->captureDevice);
    d->valid = true;
    d->available = false;
    // deviceIds stays empty because it's not available
}

#if 0
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

            valid = true;

            if (cardName.contains("headset", Qt::CaseInsensitive) ||
                    cardName.contains("headphone", Qt::CaseInsensitive)) {
                // it's a headset
                icon = QLatin1String("headset");
            } else {
                //Get card driver name from a CTL card info.
                QString driver = snd_ctl_card_info_get_driver(cardInfo);
                if (driver.contains("usb", Qt::CaseInsensitive)) {
                    // it's an external USB device
                    icon = QLatin1String("usb-device");
                } else {
                    icon = QLatin1String("pci-card");
                }
            }
        }
        snd_ctl_close(ctl);
    }
    snd_ctl_card_info_free(cardInfo);
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
            if (!deviceIds.contains(deviceName)) {
                deviceIds << deviceName;
            }
        }
        snd_pcm_close(pcm);
    }
    snd_pcm_info_free(pcmInfo);
}
#endif

int AlsaDevice::index() const
{
    return d->index;
}

bool AlsaDevice::available() const
{
    return d->available;
}

void AlsaDevice::ceaseToExist()
{
    if (d->available) {
        return; // you cannot remove devices that are plugged in
    }
    d->valid = false;
    KSharedConfig::Ptr config = KSharedConfig::openConfig("phonondevicesrc", false, false);
    QString groupName;
    if (d->captureDevice) {
        groupName = QLatin1String("AudioCaptureDevice_");
    } else {
        groupName = QLatin1String("AudioOutputDevice_");
    }
    groupName += d->cardName;
    config->deleteGroup(groupName);
    config->sync();
}

bool AlsaDevice::isValid() const
{
    return d->valid;
}

bool AlsaDevice::isCaptureDevice() const
{
    return d->captureDevice;
}

bool AlsaDevice::isPlaybackDevice() const
{
    return !d->captureDevice;
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
    return (d->cardName == rhs.d->cardName && d->icon == rhs.d->icon);
}

QString AlsaDevice::cardName() const
{
    return d->cardName;
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
