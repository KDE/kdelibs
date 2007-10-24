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

#include "audiodeviceenumerator.h"
#include "audiodeviceenumerator_p.h"
#include "audiodevice_p.h"
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QSet>
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/audiointerface.h>
#include <kconfiggroup.h>
#include <kio/kdirwatch.h>
#include <kglobal.h>
#include <kdebug.h>
#include <phonon/config-alsa.h>

#ifdef HAVE_LIBASOUND2
#include <alsa/asoundlib.h>
#endif // HAVE_LIBASOUND2

namespace Phonon
{

K_GLOBAL_STATIC(AudioDeviceEnumeratorPrivate, audioDeviceEnumeratorPrivate)

AudioDeviceEnumerator::AudioDeviceEnumerator(AudioDeviceEnumeratorPrivate *dd)
    : d(dd)
{
}

AudioDeviceEnumeratorPrivate::AudioDeviceEnumeratorPrivate()
    : q(this)
{
    config = KSharedConfig::openConfig("phonondevicesrc", KConfig::NoGlobals);
    findDevices();
    QObject::connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(const QString &)), &q, SLOT(_k_deviceAdded(const QString &)));
    QObject::connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(const QString &)), &q, SLOT(_k_deviceRemoved(const QString &)));
}

AudioDeviceEnumerator *AudioDeviceEnumerator::self()
{
    return &audioDeviceEnumeratorPrivate->q;
}

void AudioDeviceEnumeratorPrivate::findDevices()
{
    QSet<QString> alreadyFoundCards;

    // ask Solid for the available audio hardware
    const QList<Solid::Device> devices = Solid::Device::listFromQuery("AudioInterface.deviceType  & 'AudioInput|AudioOutput'");
    foreach (const Solid::Device &device, devices) {
        AudioDevice dev(device, config);
        if (dev.isValid()) {
            if (dev.isCaptureDevice()) {
                capturedevicelist << dev;
                if (dev.isPlaybackDevice()) {
                    playbackdevicelist << dev;
                    alreadyFoundCards << QLatin1String("AudioIODevice_") + dev.udi();
                } else {
                    alreadyFoundCards << QLatin1String("AudioCaptureDevice_") + dev.udi();
                }
            } else {
                playbackdevicelist << dev;
                alreadyFoundCards << QLatin1String("AudioOutputDevice_") + dev.udi();
            }
        }
    }

    // now look in the config file for disconnected devices
    QStringList groupList = config->groupList();
    foreach (const QString &groupName, groupList) {
        if (alreadyFoundCards.contains(groupName) || !groupName.startsWith(QLatin1String("Audio"))) {
            continue;
        }

        KConfigGroup configGroup(config.data(), groupName);
        AudioDevice dev(configGroup);
        if (dev.isValid()) {
            if (dev.isCaptureDevice()) {
                capturedevicelist << dev;
                if (dev.isPlaybackDevice()) {
                    playbackdevicelist << dev;
                }
            } else {
                playbackdevicelist << dev;
            }
            alreadyFoundCards << groupName;
        }
    }

    // now that we know about the hardware let's see what virtual devices we can find in
    // ~/.asoundrc and /etc/asound.conf
    findVirtualDevices();

//X     QFileSystemWatcher *watcher = new QFileSystemWatcher(QCoreApplication::instance());
//X     watcher->addPath(QDir::homePath() + QLatin1String("/.asoundrc"));
//X     watcher->addPath(QLatin1String("/etc/asound.conf"));
//X     q.connect(watcher, SIGNAL(fileChanged(const QString &)), &q, SLOT(_k_asoundrcChanged(const QString &)));
    KDirWatch *dirWatch = KDirWatch::self();
    dirWatch->addFile(QDir::homePath() + QLatin1String("/.asoundrc"));
    dirWatch->addFile(QLatin1String("/etc/asound.conf"));
    q.connect(dirWatch, SIGNAL(dirty(const QString &)), &q, SLOT(_k_asoundrcChanged(const QString &)));
}

struct DeviceHint
{
    QString name;
    QString description;
};

void AudioDeviceEnumeratorPrivate::findVirtualDevices()
{
#ifdef HAS_LIBASOUND_DEVICE_NAME_HINT
    QList<DeviceHint> deviceHints;

    void **hints;
    //snd_config_update();
    if (snd_device_name_hint(-1, "pcm", &hints) < 0) {
        kDebug(603) << "snd_device_name_hint failed for 'pcm'";
    }

    for (void **cStrings = hints; *cStrings; ++cStrings) {
        DeviceHint nextHint;
        char *x = snd_device_name_get_hint(*cStrings, "NAME");
        nextHint.name = QString::fromUtf8(x);
        free(x);

        if (nextHint.name.startsWith("front:") ||
                nextHint.name.startsWith("surround40:") ||
                nextHint.name.startsWith("surround41:") ||
                nextHint.name.startsWith("surround50:") ||
                nextHint.name.startsWith("surround51:") ||
                nextHint.name.startsWith("surround71:") ||
                nextHint.name.startsWith("default:") ||
                nextHint.name == "null"
                ) {
            continue;
        }

        x = snd_device_name_get_hint(*cStrings, "DESC");
        nextHint.description = QString::fromUtf8(x);
        free(x);

        deviceHints << nextHint;
    }
    snd_device_name_free_hint(hints);

    snd_config_update_free_global();
    snd_config_update();
    Q_ASSERT(snd_config);
    // after recreating the global configuration we can go and install custom configuration
    {
        // x-phonon: device
        QFile phononDefinition(":/phonon/phonondevice.alsa");
        phononDefinition.open(QIODevice::ReadOnly);
        const QByteArray phononDefinitionData = phononDefinition.readAll();

        snd_input_t *sndInput = 0;
        if (0 == snd_input_buffer_open(&sndInput, phononDefinitionData.constData(), phononDefinitionData.size())) {
            Q_ASSERT(sndInput);
            snd_config_load(snd_config, sndInput);
            snd_input_close(sndInput);
        }

#if 0
        // phonon_softvol: device
        QFile softvolDefinition(":/phonon/softvol.alsa");
        softvolDefinition.open(QIODevice::ReadOnly);
        const QByteArray softvolDefinitionData = softvolDefinition.readAll();

        sndInput = 0;
        if (0 == snd_input_buffer_open(&sndInput, softvolDefinitionData.constData(), softvolDefinitionData.size())) {
            Q_ASSERT(sndInput);
            snd_config_load(snd_config, sndInput);
            snd_input_close(sndInput);
        }
#endif
    }
    foreach (const DeviceHint &deviceHint, deviceHints) {
        AudioDevice dev(deviceHint.name, deviceHint.description, config);
        if (dev.isPlaybackDevice()) {
            playbackdevicelist << dev;
        }
        if (dev.isCaptureDevice()) {
            capturedevicelist << dev;
        } else {
            if (!dev.isPlaybackDevice()) {
                kDebug(603) << deviceHint.name << " doesn't work.";
            }
        }
    }
#elif defined(HAVE_LIBASOUND2)
#warning "please update your libasound! this code is not supported"
    snd_config_update();
    Q_ASSERT(snd_config);
    // after recreating the global configuration we can go and install custom configuration
    {
        // x-phonon: device
        QFile phononDefinition(":/phonon/phonondevice.alsa");
        phononDefinition.open(QIODevice::ReadOnly);
        const QByteArray phononDefinitionData = phononDefinition.readAll();

        snd_input_t *sndInput = 0;
        if (0 == snd_input_buffer_open(&sndInput, phononDefinitionData.constData(), phononDefinitionData.size())) {
            Q_ASSERT(sndInput);
            snd_config_load(snd_config, sndInput);
            snd_input_close(sndInput);
        }

#if 0
        // phonon_softvol: device
        QFile softvolDefinition(":/phonon/softvol.alsa");
        softvolDefinition.open(QIODevice::ReadOnly);
        const QByteArray softvolDefinitionData = softvolDefinition.readAll();

        sndInput = 0;
        if (0 == snd_input_buffer_open(&sndInput, softvolDefinitionData.constData(), softvolDefinitionData.size())) {
            Q_ASSERT(sndInput);
            snd_config_load(snd_config, sndInput);
            snd_input_close(sndInput);
        }
#endif
    }
#endif //HAS_LIBASOUND_DEVICE_NAME_HINT / HAVE_LIBASOUND2
}

void AudioDeviceEnumeratorPrivate::_k_asoundrcChanged(const QString &file)
{
#ifdef HAVE_LIBASOUND2
    kDebug(603) << file;
    QFileInfo changedFile(file);
    QFileInfo asoundrc(QDir::homePath() + QLatin1String("/.asoundrc"));
    if (changedFile != asoundrc) {
        asoundrc.setFile("/etc/asound.conf");
        if (changedFile != asoundrc) {
            return;
        }
    }
    QList<AudioDevice> oldPlaybackdevicelist = playbackdevicelist;
    QList<AudioDevice> oldCapturedevicelist = capturedevicelist;

    QList<AudioDevice>::Iterator it = playbackdevicelist.begin();
    while (it != playbackdevicelist.end()) {
        if (it->d->driver == Solid::AudioInterface::Alsa &&
                it->d->deviceIds.size() == 1 &&
                it->d->deviceIds.first() == it->cardName()) {
            it = playbackdevicelist.erase(it);
        } else {
            ++it;
        }
    }
    it = capturedevicelist.begin();
    while (it != capturedevicelist.end()) {
        if (it->d->driver == Solid::AudioInterface::Alsa &&
                it->d->deviceIds.size() == 1 &&
                it->d->deviceIds.first() == it->cardName()) {
            it = capturedevicelist.erase(it);
        } else {
            ++it;
        }
    }
    snd_config_update_free_global();
    snd_config_update();
    findVirtualDevices();

    foreach (const AudioDevice &dev, oldCapturedevicelist) {
        if (!capturedevicelist.contains(dev)) {
            emit q.deviceUnplugged(dev);
        }
    }
    foreach (const AudioDevice &dev, oldPlaybackdevicelist) {
        if (!playbackdevicelist.contains(dev)) {
            emit q.deviceUnplugged(dev);
        }
    }
    foreach (const AudioDevice &dev, playbackdevicelist) {
        if (!oldPlaybackdevicelist.contains(dev)) {
            emit q.devicePlugged(dev);
        }
    }
    foreach (const AudioDevice &dev, capturedevicelist) {
        if (!oldCapturedevicelist.contains(dev)) {
            emit q.devicePlugged(dev);
        }
    }
#endif // HAVE_LIBASOUND2
}

void AudioDeviceEnumeratorPrivate::_k_deviceAdded(const QString &udi)
{
    kDebug(603) << udi;
    Solid::Device _device(udi);
    Solid::AudioInterface *audiohw = _device.as<Solid::AudioInterface>();
    if (audiohw && (audiohw->deviceType()  & (Solid::AudioInterface::AudioInput |
                    Solid::AudioInterface::AudioOutput))) {
        // an audio i/o device was plugged in
        AudioDevice dev(_device, config);
        if (dev.isValid()) {
            if (dev.isCaptureDevice()) {
                foreach (const AudioDevice &listedDev, capturedevicelist) {
                    if (listedDev == dev && !listedDev.isAvailable()) {
                        // listedDev is the same devices as dev but shown as unavailable
                        kDebug(603) << "removing from capturedevicelist: " << listedDev.cardName();
                        capturedevicelist.removeAll(listedDev);
                        break;
                    }
                }
                capturedevicelist << dev;
            }
            if (dev.isPlaybackDevice()) {
                foreach (const AudioDevice &listedDev, playbackdevicelist) {
                    if (listedDev == dev && !listedDev.isAvailable()) {
                        // listedDev is the same devices as dev but shown as unavailable
                        kDebug(603) << "removing from playbackdevicelist: " << listedDev.cardName();
                        playbackdevicelist.removeAll(listedDev);
                        break;
                    }
                }
                playbackdevicelist << dev;
            }
            kDebug(603) << "emit q.devicePlugged " << dev.cardName();
            emit q.devicePlugged(dev);
        }
    }
}

void AudioDeviceEnumeratorPrivate::_k_deviceRemoved(const QString &udi)
{
    kDebug(603) << udi;
    AudioDevice dev;
    foreach (const AudioDevice &listedDev, capturedevicelist) {
        if (listedDev.udi() == udi && listedDev.isAvailable()) {
            // listedDev is the same devices as was removed
            kDebug(603) << "removing from capturedevicelist: " << listedDev.cardName();
            dev = listedDev;
            capturedevicelist.removeAll(listedDev);
            break;
        }
    }
    foreach (const AudioDevice &listedDev, playbackdevicelist) {
        if (listedDev.udi() == udi && listedDev.isAvailable()) {
            // listedDev is the same devices as was removed
            kDebug(603) << "removing from playbackdevicelist: " << listedDev.cardName();
            dev = listedDev;
            playbackdevicelist.removeAll(listedDev);
            break;
        }
    }

    if (dev.isValid()) {
        kDebug(603) << "emit q.deviceUnplugged " << dev.cardName();
        emit q.deviceUnplugged(dev);
    }
}

AudioDeviceEnumerator::~AudioDeviceEnumerator()
{
}

QDebug operator<<(QDebug &s, const Solid::AudioInterface::AudioDriver &driver)
{
    switch (driver) {
    case Solid::AudioInterface::Alsa:
        s.nospace() << "ALSA";
        break;
    case Solid::AudioInterface::OpenSoundSystem:
        s.nospace() << "OSS";
        break;
    case Solid::AudioInterface::UnknownAudioDriver:
        s.nospace() << "unknown driver";
        break;
    }
    return s.space();
}
QDebug operator<<(QDebug &s, const AudioDevice &dev)
{
    s.space() << "\n-" << dev.cardName() << dev.driver() << dev.deviceIds() << "index:" << dev.index() << "preference:" << dev.initialPreference() << "avail:" << dev.isAvailable();
    return s.space();
}

QList<AudioDevice> AudioDeviceEnumerator::availablePlaybackDevices()
{
    kDebug(603) << audioDeviceEnumeratorPrivate->playbackdevicelist;
    return audioDeviceEnumeratorPrivate->playbackdevicelist;
}

QList<AudioDevice> AudioDeviceEnumerator::availableCaptureDevices()
{
    return audioDeviceEnumeratorPrivate->capturedevicelist;
}

} // namespace Phonon
#include "audiodeviceenumerator.moc"

// vim: sw=4 sts=4 et tw=100
