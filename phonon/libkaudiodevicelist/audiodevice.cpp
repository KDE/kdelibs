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
#include "hardwaredatabase_p.h"

#include <solid/device.h>
#include <solid/audiointerface.h>
#include <solid/genericinterface.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kdebug.h>

#ifdef HAVE_LIBASOUND2
#include <alsa/asoundlib.h>
#endif // HAVE_LIBASOUND2

namespace Phonon
{
#if 0
QStringList AudioDevice::addSoftVolumeMixerControl(const AudioDevice &device, const QStringList &mixerControlNames)
{
    if (device.driver() != Solid::AudioInterface::Alsa) {
        return QStringList();
    }

    QStringList ids = device.deviceIds();
    foreach (const QString &mixerControlName, mixerControlNames) {
        QStringList tmp;
        foreach (QString id, ids) {
            id.replace(QLatin1Char('"'), QLatin1String("\\\""));
            tmp << QString("phonon_softvol:CARD=\"%1\",NAME=\"%2\",SLAVE=\"%3\"")
                .arg(0)
                .arg(mixerControlName)
                .arg(id);
        }
        ids = tmp;
    }
    return ids;
}
#endif

AudioDevice::AudioDevice()
    : d(new AudioDevicePrivate)
{
}

KConfigGroup AudioDevicePrivate::configGroup(KSharedConfig::Ptr config)
{
    QString groupName;
    if (captureDevice) {
        if (playbackDevice) {
            groupName = QLatin1String("AudioIODevice_");
        } else {
            groupName = QLatin1String("AudioCaptureDevice_");
        }
    } else {
        Q_ASSERT(playbackDevice);
        groupName = QLatin1String("AudioOutputDevice_");
    }
    groupName += udi;
    return KConfigGroup(config, groupName);
}

QString AudioDevicePrivate::uniqueIdentifierFromDevice(const Solid::Device &device)
{
    const Solid::GenericInterface *genericIface = device.as<Solid::GenericInterface>();
    Q_ASSERT(genericIface);
    const QString subsystem = genericIface->property(QLatin1String("info.subsystem")).toString();
    if (subsystem == "pci") {
        const QVariant vendor_id = genericIface->property("pci.vendor_id");
        if (vendor_id.isValid()) {
            const QVariant product_id = genericIface->property("pci.product_id");
            if (product_id.isValid()) {
                const QVariant subsys_vendor_id = genericIface->property("pci.subsys_vendor_id");
                if (subsys_vendor_id.isValid()) {
                    const QVariant subsys_product_id = genericIface->property("pci.subsys_product_id");
                    if (subsys_product_id.isValid()) {
                        return QString("pci:%1:%2:%3:%4")
                            .arg(vendor_id.toInt(), 4, 16, QLatin1Char('0'))
                            .arg(product_id.toInt(), 4, 16, QLatin1Char('0'))
                            .arg(subsys_vendor_id.toInt(), 4, 16, QLatin1Char('0'))
                            .arg(subsys_product_id.toInt(), 4, 16, QLatin1Char('0'));
                    }
                }
            }
        }
    } else if (subsystem == "usb" || subsystem == "usb_device") {
        const QVariant vendor_id = genericIface->property("usb.vendor_id");
        if (vendor_id.isValid()) {
            const QVariant product_id = genericIface->property("usb.product_id");
            if (product_id.isValid()) {
                return QString("usb:%1:%2")
                    .arg(vendor_id.toInt(), 4, 16, QLatin1Char('0'))
                    .arg(product_id.toInt(), 4, 16, QLatin1Char('0'));
            }
        }
    /*} else if (subsystem == "platform") {
    } else if (subsystem == "pnp") {
    } else if (subsystem == "serio") {
    } else if (subsystem == "scsi") {*/
    }
    return QString();
}

AudioDevice::AudioDevice(Solid::Device audioDevice, KSharedConfig::Ptr config)
    : d(new AudioDevicePrivate)
{
    Solid::AudioInterface *audioHw = audioDevice.as<Solid::AudioInterface>();
    //kDebug(603) << audioHw->driverHandle();
    d->udi = d->uniqueIdentifierFromDevice(audioDevice);
    d->driver = audioHw->driver();
    const QVariant handle = audioHw->driverHandle();
    if (d->udi.isEmpty()) {
        Solid::Device parent = audioDevice.parent();
        if (parent.isValid()) {
            d->udi = d->uniqueIdentifierFromDevice(parent);
            if (!d->udi.isEmpty()) {
                switch (audioHw->deviceType()) {
                case Solid::AudioInterface::AudioInput:
                    d->udi += QLatin1String(":capture");
                    break;
                case Solid::AudioInterface::AudioOutput:
                    d->udi += QLatin1String(":playback");
                    break;
                case 6: //Solid::AudioInterface::AudioInput | Solid::AudioInterface::AudioOutput:
                    d->udi += QLatin1String(":both");
                    break;
                default:
                    break;
                }
                switch (d->driver) {
                case Solid::AudioInterface::Alsa:
                    d->udi += QLatin1String(":alsa");
                    if (handle.type() == QVariant::List) {
                        const QList<QVariant> handles = handle.toList();
                        if (handles.size() > 1 && handles.at(1).isValid()) {
                            d->udi += QLatin1Char(':') + handles.at(1).toString();
                        }
                    }
                    break;
                case Solid::AudioInterface::OpenSoundSystem:
                    d->udi += QLatin1String(":oss");
                    break;
                default:
                    break;
                }
            }
        }
        if (d->udi.isEmpty()) {
            d->udi = audioHw->name();
            d->udi += audioDevice.vendor();
            d->udi += audioDevice.product();
            if (parent.isValid()) {
                d->udi += parent.vendor();
                d->udi += parent.product();
            }
            d->udi += QString::number(audioHw->driver());
            d->udi += QLatin1Char('_');
            d->udi += QString::number(audioHw->deviceType());
        }
    }
    d->cardName = audioHw->name();

    // prefer devices Solid tells us about
    d->initialPreference += 5;

    switch (d->driver) {
    case Solid::AudioInterface::UnknownAudioDriver:
        d->valid = false;
        return;
    case Solid::AudioInterface::OpenSoundSystem:
        if (handle.type() != QVariant::String) {
            d->valid = false;
            return;
        }
        d->deviceIds << handle.toString();
        break;
    case Solid::AudioInterface::Alsa:
        if (handle.type() != QVariant::List) {
            d->valid = false;
            return;
        }
        const QList<QVariant> handles = handle.toList();
        if (handles.size() < 1) {
            d->valid = false;
            return;
        }
        QString x_phononId = QLatin1String("x-phonon:CARD=") + handles.first().toString(); // the first is either an int (card number) or a QString (card id)
        QString fallbackId = QLatin1String("plughw:CARD=")   + handles.first().toString(); // the first is either an int (card number) or a QString (card id)
        if (handles.size() > 1 && handles.at(1).isValid()) {
            if (handles.at(1).toInt() == 0) {
                // prefer DEV=0 devices over DEV>0
                d->initialPreference += 1;
            }
            x_phononId += ",DEV=" + handles.at(1).toString();
            fallbackId += ",DEV=" + handles.at(1).toString();
            if (handles.size() > 2 && handles.at(2).isValid()) {
                x_phononId += ",SUBDEV=" + handles.at(2).toString();
                fallbackId += ",SUBDEV=" + handles.at(2).toString();
            }
        }
        d->deviceIds << x_phononId << fallbackId;
        break;
    }
    d->icon = audioDevice.icon();
    if (d->icon.isEmpty()) {
        switch (audioHw->soundcardType()) {
        case Solid::AudioInterface::InternalSoundcard:
            d->icon = QLatin1String("audio-card");
            break;
        case Solid::AudioInterface::UsbSoundcard:
            d->icon = QLatin1String("audio-card-usb");
            d->initialPreference -= 10;
            break;
        case Solid::AudioInterface::FirewireSoundcard:
            d->icon = QLatin1String("audio-card-firewire");
            d->initialPreference -= 10;
            break;
        case Solid::AudioInterface::Headset:
            if (audioDevice.udi().contains("usb", Qt::CaseInsensitive) ||
                    d->cardName.contains("usb", Qt::CaseInsensitive)) {
                d->icon = QLatin1String("audio-headset-usb");
            } else {
                d->icon = QLatin1String("audio-headset");
            }
            d->initialPreference -= 10;
            break;
        case Solid::AudioInterface::Modem:
            d->icon = QLatin1String("modem");
            // should a modem be a valid device so that it's shown to the user?
            d->valid = false;
            return;
        }
    }
    d->available = true;
    d->valid = true;

    Solid::AudioInterface::AudioInterfaceTypes deviceType = audioHw->deviceType();
    if (deviceType == Solid::AudioInterface::AudioInput) {
        d->captureDevice = true;
    } else {
        if (deviceType == Solid::AudioInterface::AudioOutput) {
            d->playbackDevice = true;
        } else {
            Q_ASSERT(deviceType == (Solid::AudioInterface::AudioOutput | Solid::AudioInterface::AudioInput));
            d->captureDevice = true;
            d->playbackDevice = true;
        }
    }

    KConfigGroup deviceGroup = d->configGroup(config);
    if (deviceGroup.exists()) {
        d->index = deviceGroup.readEntry("index", -1);
    }
    if (d->index == -1) {
        KConfigGroup globalGroup(config, "Globals");
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
        deviceGroup.writeEntry("initialPreference", d->initialPreference);
        config->sync();
    } else {
        if (!deviceGroup.hasKey("udi") || !deviceGroup.hasKey("initialPreference")) {
            deviceGroup.writeEntry("initialPreference", d->initialPreference);
            deviceGroup.writeEntry("udi", d->udi);
            config->sync();
        }
        deviceGroup.writeEntry("icon", d->icon);
    }
    //kDebug(603) << deviceGroup.readEntry("udi", d->udi) << " == " << d->udi;

    d->applyHardwareDatabaseOverrides();
}

void AudioDevicePrivate::changeIndex(int newIndex, KSharedConfig::Ptr config)
{
    index = newIndex;
    KConfigGroup deviceGroup = configGroup(config);
    deviceGroup.writeEntry("index", index);
}

AudioDevice::AudioDevice(KConfigGroup &deviceGroup)
    : d(new AudioDevicePrivate)
{
    d->index = deviceGroup.readEntry("index", d->index);
    d->udi = deviceGroup.readEntry("udi", d->udi);
    if (d->udi.startsWith("/org/freedesktop/Hal/devices/")) {
        // old invalid group
        d->valid = false;
        return;
    }
    d->cardName = deviceGroup.readEntry("cardName", d->cardName);
    d->icon = deviceGroup.readEntry("icon", d->icon);
    d->driver = static_cast<Solid::AudioInterface::AudioDriver>(deviceGroup.readEntry("driver", static_cast<int>(d->driver)));
    d->captureDevice = deviceGroup.readEntry("captureDevice", d->captureDevice);
    d->playbackDevice = deviceGroup.readEntry("playbackDevice", d->playbackDevice);
    d->valid = true;
    d->available = false;
    d->initialPreference = deviceGroup.readEntry("initialPreference", 0);
    // deviceIds stays empty because it's not available

    d->applyHardwareDatabaseOverrides();
}

AudioDevice::AudioDevice(const QString &alsaDeviceName, KSharedConfig::Ptr config)
    : d(new AudioDevicePrivate)
{
#ifdef HAVE_LIBASOUND2
    d->driver = Solid::AudioInterface::Alsa;
    d->deviceIds << alsaDeviceName;
    d->cardName = alsaDeviceName;
    d->udi = alsaDeviceName;
    d->deviceInfoFromPcmDevice(alsaDeviceName);

    KConfigGroup deviceGroup(config, alsaDeviceName);
    if (config->hasGroup(alsaDeviceName)) {
        d->index = deviceGroup.readEntry("index", -1);
    }
    if (d->index == -1) {
        KConfigGroup globalGroup(config, "Globals");
        int nextIndex = globalGroup.readEntry("nextIndex", 0);
        d->index = nextIndex++;
        globalGroup.writeEntry("nextIndex", nextIndex);

        deviceGroup.writeEntry("index", d->index);
        deviceGroup.writeEntry("cardName", d->cardName);
        deviceGroup.writeEntry("icon", d->icon);
        deviceGroup.writeEntry("driver", static_cast<int>(d->driver));
        deviceGroup.writeEntry("captureDevice", d->captureDevice);
        deviceGroup.writeEntry("playbackDevice", d->playbackDevice);
        deviceGroup.writeEntry("initialPreference", d->initialPreference);
    } else {
        if (!deviceGroup.hasKey("initialPreference")) {
            deviceGroup.writeEntry("initialPreference", d->initialPreference);
        }
        if (d->captureDevice) { // only "promote" devices
            deviceGroup.writeEntry("captureDevice", d->captureDevice);
        }
        if (d->playbackDevice) { // only "promote" devices
            deviceGroup.writeEntry("playbackDevice", d->playbackDevice);
        }
        deviceGroup.writeEntry("icon", d->icon);
    }
    config->sync();

    d->applyHardwareDatabaseOverrides();
#endif // HAVE_LIBASOUND2
}

AudioDevice::AudioDevice(const QString &alsaDeviceName, const QString &description, KSharedConfig::Ptr config)
    : d(new AudioDevicePrivate)
{
#ifdef HAVE_LIBASOUND2
    d->driver = Solid::AudioInterface::Alsa;
    d->deviceIds << alsaDeviceName;
    QStringList lines = description.split("\n");
    d->cardName = lines.first();
    if (lines.size() > 1) {
        d->cardName = i18n("%1 (%2)", d->cardName, lines[1]);
    }

    snd_pcm_t *pcm;
    const QByteArray deviceNameEnc = alsaDeviceName.toUtf8();
    if (0 == snd_pcm_open(&pcm, deviceNameEnc.constData(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK /*open mode: non-blocking, sync */)) {
        d->available = true;
        d->playbackDevice = true;
        d->valid = true;
        snd_pcm_close(pcm);
    }
    if (0 == snd_pcm_open(&pcm, deviceNameEnc.constData(), SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK /*open mode: non-blocking, sync */)) {
        d->available = true;
        d->captureDevice = true;
        d->valid = true;
        snd_pcm_close(pcm);
    }

    if (description.contains("headset", Qt::CaseInsensitive) ||
            description.contains("headphone", Qt::CaseInsensitive)) {
        // it's a headset
        if (description.contains("usb", Qt::CaseInsensitive)) {
            d->icon = QLatin1String("audio-headset-usb");
            d->initialPreference -= 10;
        } else {
            d->icon = QLatin1String("audio-headset");
            d->initialPreference -= 10;
        }
    } else {
        //Get card driver name from a CTL card info.
        if (description.contains("usb", Qt::CaseInsensitive)) {
            // it's an external USB device
            d->icon = QLatin1String("audio-card-usb");
            d->initialPreference -= 10;
        } else {
            d->icon = QLatin1String("audio-card");
        }
    }

    KConfigGroup deviceGroup(config, alsaDeviceName);
    if (config->hasGroup(alsaDeviceName)) {
        d->index = deviceGroup.readEntry("index", -1);
    }
    if (d->index == -1) {
        KConfigGroup globalGroup(config, "Globals");
        int nextIndex = globalGroup.readEntry("nextIndex", 0);
        d->index = nextIndex++;
        globalGroup.writeEntry("nextIndex", nextIndex);

        deviceGroup.writeEntry("index", d->index);
        deviceGroup.writeEntry("cardName", d->cardName);
        deviceGroup.writeEntry("icon", d->icon);
        deviceGroup.writeEntry("driver", static_cast<int>(d->driver));
        deviceGroup.writeEntry("captureDevice", d->captureDevice);
        deviceGroup.writeEntry("playbackDevice", d->playbackDevice);
        deviceGroup.writeEntry("initialPreference", d->initialPreference);
    } else {
        if (!deviceGroup.hasKey("initialPreference")) {
            deviceGroup.writeEntry("initialPreference", d->initialPreference);
        }
        if (d->captureDevice) { // only "promote" devices
            deviceGroup.writeEntry("captureDevice", d->captureDevice);
        }
        if (d->playbackDevice) { // only "promote" devices
            deviceGroup.writeEntry("playbackDevice", d->playbackDevice);
        }
        deviceGroup.writeEntry("icon", d->icon);
    }
    config->sync();

    d->applyHardwareDatabaseOverrides();
#endif // HAVE_LIBASOUND2
}

#ifdef HAVE_LIBASOUND2
void AudioDevicePrivate::deviceInfoFromPcmDevice(const QString &deviceName)
{
    //kDebug(603) << deviceName;
    snd_pcm_info_t *pcmInfo;
    snd_pcm_info_malloc(&pcmInfo);

    snd_pcm_t *pcm;
    const QByteArray deviceNameEnc = deviceName.toUtf8();
    if (0 == snd_pcm_open(&pcm, deviceNameEnc.constData(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK /*open mode: non-blocking, sync */)) {
        if (0 == snd_pcm_info(pcm, pcmInfo)) {
            available = true;
            playbackDevice = true;
            alsaId.card = snd_pcm_info_get_card(pcmInfo);
            alsaId.device = snd_pcm_info_get_device(pcmInfo);
            alsaId.subdevice = snd_pcm_info_get_subdevice(pcmInfo);
            valid = true;
        }
        snd_pcm_close(pcm);
    }
    if (0 == snd_pcm_open(&pcm, deviceNameEnc.constData(), SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK /*open mode: non-blocking, sync */)) {
        captureDevice = true;
        snd_pcm_close(pcm);
    }
    if (valid) {
        QByteArray hwDeviceNameEnc;
        if (alsaId.card < 0) {
            Q_ASSERT(false);
            hwDeviceNameEnc = snd_pcm_info_get_id(pcmInfo);
            if (hwDeviceNameEnc.startsWith("dmix:")) {
                hwDeviceNameEnc = "hw:" + hwDeviceNameEnc.right(hwDeviceNameEnc.size() - 5);
            }
//X             if (0 == snd_pcm_open(&pcm, hwDeviceNameEnc.constData(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK /*open mode: non-blocking, sync */)) {
//X                 if (0 == snd_pcm_info(pcm, pcmInfo)) {
//X                     alsaId.card = snd_pcm_info_get_card(pcmInfo);
//X                     alsaId.device = snd_pcm_info_get_device(pcmInfo);
//X                     alsaId.subdevice = snd_pcm_info_get_subdevice(pcmInfo);
//X                 }
//X                 snd_pcm_close(pcm);
//X             }
        } else {
            hwDeviceNameEnc = "hw:" + QByteArray::number(alsaId.card) + ','
                + QByteArray::number(alsaId.device) + ',' + QByteArray::number(alsaId.subdevice);
        }

        QString probablyTheCardName;
        QString driver;
        snd_ctl_card_info_t *cardInfo;
        snd_ctl_card_info_malloc(&cardInfo);
        snd_ctl_t *ctl;
        if (0 == snd_ctl_open(&ctl, hwDeviceNameEnc.constData(), 0 /*open mode: blocking, sync */)) {
            if (0 == snd_ctl_card_info(ctl, cardInfo)) {
                //Get card identifier from a CTL card info.
                //internalId = snd_ctl_card_info_get_id(cardInfo);

                //Get card name from a CTL card info.
                probablyTheCardName = QString(snd_ctl_card_info_get_name(cardInfo)).trimmed();
                driver = snd_ctl_card_info_get_driver(cardInfo);
            }
            snd_ctl_close(ctl);
        }
        snd_ctl_card_info_free(cardInfo);

        if (deviceNameEnc.startsWith("iec958:")) {
            initialPreference -= 20;
        } else if (cardName.contains("IEC958", Qt::CaseInsensitive) ||
                    cardName.contains("s/pdif", Qt::CaseInsensitive) ||
                    cardName.contains("spdif", Qt::CaseInsensitive) ||
                    probablyTheCardName.contains("IEC958", Qt::CaseInsensitive) ||
                    probablyTheCardName.contains("s/pdif", Qt::CaseInsensitive) ||
                    probablyTheCardName.contains("spdif", Qt::CaseInsensitive)) {
            initialPreference -= 20;
        }
        if (cardName.contains("headset", Qt::CaseInsensitive) ||
                cardName.contains("headphone", Qt::CaseInsensitive) ||
                probablyTheCardName.contains("headset", Qt::CaseInsensitive) ||
                probablyTheCardName.contains("headphone", Qt::CaseInsensitive)) {
            // it's a headset
            if (driver.contains("usb", Qt::CaseInsensitive)) {
                icon = QLatin1String("audio-headset-usb");
                initialPreference -= 10;
            } else {
                icon = QLatin1String("audio-headset");
                initialPreference -= 10;
            }
        } else {
            //Get card driver name from a CTL card info.
            if (driver.contains("usb", Qt::CaseInsensitive)) {
                // it's an external USB device
                icon = QLatin1String("audio-card-usb");
                initialPreference -= 10;
            } else {
                icon = QLatin1String("audio-card");
            }
        }
    }
    snd_pcm_info_free(pcmInfo);
}
#endif // HAVE_LIBASOUND2

QString AudioDevice::udi() const
{
    return d->udi;
}

int AudioDevice::index() const
{
    return d->index;
}

int AudioDevice::initialPreference() const
{
    return d->initialPreference;
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
    groupName += d->udi;
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
}

AudioDevice::~AudioDevice()
{
}

AudioDevice &AudioDevice::operator=(const AudioDevice &rhs)
{
    d = rhs.d;
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

Solid::AudioInterface::AudioDriver AudioDevice::driver() const
{
    return d->driver;
}

void AudioDevicePrivate::applyHardwareDatabaseOverrides()
{
    // now let's take a look at the hardware database whether we have to override something
    if (HardwareDatabase::contains(udi)) {
        HardwareDatabase::Entry e = HardwareDatabase::entryFor(udi);
        if (!e.name.isEmpty()) {
            cardName = e.name;
        }
        if (!e.iconName.isEmpty()) {
            icon = e.iconName;
        }
        initialPreference = e.initialPreference;
    }
}

} // namespace Phonon

// vim: sw=4 sts=4 et tw=100
