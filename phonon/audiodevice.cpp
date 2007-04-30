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

#include "audiodevice.h"

#include "audiodevice_p.h"
#include "audiodeviceenumerator.h"
#include "phononnamespace_p.h"
#include <solid/device.h>
#include <solid/audiohw.h>
#include <kconfiggroup.h>

namespace Phonon
{
AudioDevice::AudioDevice()
    : d(new AudioDevicePrivate)
{
}

AudioDevice::AudioDevice(Solid::Device audioDevice, KSharedConfig::Ptr config)
    : d(new AudioDevicePrivate)
{
    Solid::AudioHw *audioHw = audioDevice.as<Solid::AudioHw>();
    pDebug() << Q_FUNC_INFO << audioHw->driverHandles();
    d->udi = audioDevice.udi();
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
        d->icon = QLatin1String("modem");
        // should a modem be a valid device so that it's shown to the user?
        d->valid = false;
        return;
    }
    d->driver = audioHw->driver();
    d->available = true;
    d->valid = true;

    QString groupName;
    Solid::AudioHw::AudioHwTypes deviceType = audioHw->deviceType();
    if (deviceType == Solid::AudioHw::AudioInput) {
        d->captureDevice = true;
        groupName = QLatin1String("AudioCaptureDevice_");
    } else {
        if (deviceType == Solid::AudioHw::AudioOutput) {
            d->playbackDevice = true;
            groupName = QLatin1String("AudioOutputDevice_");
        } else {
            Q_ASSERT(deviceType == (Solid::AudioHw::AudioOutput | Solid::AudioHw::AudioInput));
            d->captureDevice = true;
            d->playbackDevice = true;
            groupName = QLatin1String("AudioIODevice_");
        }
    }
    groupName += d->cardName;

    KConfigGroup deviceGroup(config.data(), groupName);
    if (config->hasGroup(groupName)) {
        d->index = deviceGroup.readEntry("index", -1);
    }
    if (d->index == -1) {
        KConfigGroup globalGroup(config.data(), "Globals");
        int nextIndex = globalGroup.readEntry("nextIndex", 0);
        d->index = nextIndex++;
        globalGroup.writeEntry("nextIndex", nextIndex);

        deviceGroup.writeEntry("index", d->index);
        deviceGroup.writeEntry("cardName", d->cardName);
        deviceGroup.writeEntry("icon", d->icon);
        deviceGroup.writeEntry("driver", static_cast<int>(d->driver));
        deviceGroup.writeEntry("captureDevice", d->captureDevice);
        deviceGroup.writeEntry("playbackDevice", d->playbackDevice);
        deviceGroup.writeEntry("udi", d->udi);
        config->sync();
    } else if (!deviceGroup.hasKey("udi")) {
        deviceGroup.writeEntry("udi", d->udi);
        config->sync();
    }
    pDebug() << deviceGroup.readEntry("udi", d->udi) << " == " << d->udi;
    //Q_ASSERT(deviceGroup.readEntry("udi", d->udi) == d->udi);
}

AudioDevice::AudioDevice(KConfigGroup &deviceGroup)
    : d(new AudioDevicePrivate)
{
    d->index = deviceGroup.readEntry("index", d->index);
    d->cardName = deviceGroup.readEntry("cardName", d->cardName);
    d->icon = deviceGroup.readEntry("icon", d->icon);
    d->driver = static_cast<Solid::AudioHw::AudioDriver>(deviceGroup.readEntry("driver", static_cast<int>(d->driver)));
    d->captureDevice = deviceGroup.readEntry("captureDevice", d->captureDevice);
    d->playbackDevice = deviceGroup.readEntry("playbackDevice", d->playbackDevice);
    d->udi = deviceGroup.readEntry("udi", d->udi);
    d->valid = true;
    d->available = false;
    // deviceIds stays empty because it's not available
}

#if 0
void AudioDevicePrivate::deviceInfoFromControlDevice(const QString &deviceName)
{
    snd_ctl_card_info_t *cardInfo;
    snd_ctl_card_info_malloc(&cardInfo);

    snd_ctl_t *ctl;
    if (0 == snd_ctl_open(&ctl, deviceName.toLatin1().constData(), 0 /*open mode: blocking, sync */)) {
        if (0 == snd_ctl_card_info(ctl, cardInfo)) {
            //Get card identifier from a CTL card info.
            internalId = snd_ctl_card_info_get_id(cardInfo);
            pDebug() << Q_FUNC_INFO << internalId;

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

void AudioDevicePrivate::deviceInfoFromPcmDevice(const QString &deviceName)
{
    snd_pcm_info_t *pcmInfo;
    snd_pcm_info_malloc(&pcmInfo);

    snd_pcm_t *pcm;
    if (0 == snd_pcm_open(&pcm, deviceName.toLatin1().constData(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK /*open mode: non-blocking, sync */)) {
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

const QString &AudioDevice::udi() const
{
    return d->udi;
}

int AudioDevice::index() const
{
    return d->index;
}

bool AudioDevice::isAvailable() const
{
    return d->available;
}

bool AudioDevice::ceaseToExist()
{
    if (d->available) {
        return false; // you cannot remove devices that are plugged in
    }
    d->valid = false;
    KSharedConfig::Ptr config = KSharedConfig::openConfig("phonondevicesrc", KConfig::NoGlobals);
    QString groupName;
    if (d->captureDevice) {
        if (d->playbackDevice) {
            groupName = QLatin1String("AudioIODevice_");
        } else {
            groupName = QLatin1String("AudioCaptureDevice_");
        }
    } else {
        groupName = QLatin1String("AudioOutputDevice_");
    }
    groupName += d->cardName;
    config->deleteGroup(groupName);
    config->sync();
    return true;
}

bool AudioDevice::isValid() const
{
    return d->valid;
}

bool AudioDevice::isCaptureDevice() const
{
    return d->captureDevice;
}

bool AudioDevice::isPlaybackDevice() const
{
    return d->playbackDevice;
}

AudioDevice::AudioDevice(const AudioDevice &rhs)
    : d(rhs.d)
{
    ++d->refCount;
}

AudioDevice::~AudioDevice()
{
    --d->refCount;
    if (d->refCount == 0) {
        delete d;
        d = 0;
    }
}

AudioDevice &AudioDevice::operator=(const AudioDevice &rhs)
{
    --d->refCount;
    if (d->refCount == 0) {
        delete d;
        d = 0;
    }

    d = rhs.d;
    ++d->refCount;
    return *this;
}

bool AudioDevice::operator==(const AudioDevice &rhs) const
{
    return d->udi == rhs.d->udi;
    /*
    return (d->cardName == rhs.d->cardName &&
            d->icon == rhs.d->icon &&
            d->deviceIds == rhs.d->deviceIds &&
            d->captureDevice == rhs.d->captureDevice &&
            d->playbackDevice == rhs.d->playbackDevice);
            */
}

QString AudioDevice::cardName() const
{
    return d->cardName;
}

QStringList AudioDevice::deviceIds() const
{
    return d->deviceIds;
}

QString AudioDevice::iconName() const
{
    return d->icon;
}

Solid::AudioHw::AudioDriver AudioDevice::driver() const
{
    return d->driver;
}

} // namespace Phonon

// vim: sw=4 sts=4 et tw=100
