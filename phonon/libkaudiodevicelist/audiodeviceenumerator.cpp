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
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/audiointerface.h>
#include <kconfiggroup.h>
#include <kio/kdirwatch.h>
#include <alsa/asoundlib.h>

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

/*
AudioDevice *AudioDeviceEnumerator::deviceFor(const QString &internalId)
{
    for (int i = 0; i < d->devicelist.size(); ++i) {
        if (d->devicelist[i].d->internalId == internalId) {
            return &d->devicelist[i];
        }
    }
    return 0;
}
*/

void AudioDeviceEnumeratorPrivate::findDevices()
{
    // ask Solid for the available audio hardware
    QSet<QString> alreadyFoundCards;

    QList<Solid::Device> devices = Solid::Device::listFromQuery("AudioInterface.deviceType  & 'AudioInput|AudioOutput'");
    foreach (Solid::Device device, devices) {
        AudioDevice dev(device, config);
        if (dev.isValid()) {
            if (dev.isCaptureDevice()) {
                capturedevicelist << dev;
                if (dev.isPlaybackDevice()) {
                    playbackdevicelist << dev;
                    alreadyFoundCards << QLatin1String("AudioIODevice_") + dev.cardName();
                } else {
                    alreadyFoundCards << QLatin1String("AudioCaptureDevice_") + dev.cardName();
                }
            } else {
                playbackdevicelist << dev;
                alreadyFoundCards << QLatin1String("AudioOutputDevice_") + dev.cardName();
            }
        }
    }

    // now look in the config file for disconnected devices
    QStringList groupList = config->groupList();
    foreach (QString groupName, groupList) {
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

    // now that we know about the hardware let's see what devices we can find in ~/.asoundrc and
    // /etc/asound.conf
//X     AudioDevice defaultDevice(QLatin1String("default"), config);
//X     if (defaultDevice.isValid()) {
//X         foreach (const AudioDevice &dev, capturedevicelist) {
//X             if (dev.d->alsaId == defaultCtlDevice.d->alsaId) {
//X                 AudioDevice foo = dev;
//X                 foo.d->deviceIds = defaultCtlDevice.d->deviceIds + foo.d->deviceIds;
//X                 break;
//X             }
//X         }
//X         foreach (const AudioDevice &dev, playbackdevicelist) {
//X             if (dev.d->alsaId == defaultCtlDevice.d->alsaId) {
//X                 AudioDevice foo = dev;
//X                 foo.d->deviceIds = defaultCtlDevice.d->deviceIds + foo.d->deviceIds;
//X                 break;
//X             }
//X         }
//X         if (defaultDevice.isPlaybackDevice()) {
//X             playbackdevicelist << defaultDevice;
//X         }
//X         if (defaultDevice.isCaptureDevice()) {
//X             capturedevicelist << defaultDevice;
//X         }
//X     }
    findVirtualDevices();
    //findAsoundrcDevices(QLatin1String("/etc/asound.conf"));
    //findAsoundrcDevices(QDir::homePath() + QLatin1String("/.asoundrc"));

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
    QList<DeviceHint> deviceHints;

    void **hints;
    //snd_config_update();
    if (snd_device_name_hint(-1, "pcm", &hints) < 0) {
        kDebug(600) << "snd_device_name_hint failed for 'pcm'" << endl;
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
    foreach (const DeviceHint &deviceHint, deviceHints) {
        AudioDevice dev(deviceHint.name, deviceHint.description, config);
        if (dev.isPlaybackDevice()) {
            playbackdevicelist << dev;
        }
        if (dev.isCaptureDevice()) {
            capturedevicelist << dev;
        } else {
            if (!dev.isPlaybackDevice()) {
                kDebug(600) << deviceHint.name << " doesn't work." << endl;
            }
        }
    }
}

void AudioDeviceEnumeratorPrivate::_k_asoundrcChanged(const QString &file)
{
    kDebug(600) << k_funcinfo << file << endl;
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
    //findAsoundrcDevices(QLatin1String("/etc/asound.conf"));
    //findAsoundrcDevices(QDir::homePath() + QLatin1String("/.asoundrc"));

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
}

void AudioDeviceEnumeratorPrivate::_k_deviceAdded(const QString &udi)
{
    kDebug(600) << k_funcinfo << udi << endl;
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
                        kDebug(600) << "removing from capturedevicelist: " << listedDev.cardName() << endl;
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
                        kDebug(600) << "removing from playbackdevicelist: " << listedDev.cardName() << endl;
                        playbackdevicelist.removeAll(listedDev);
                        break;
                    }
                }
                playbackdevicelist << dev;
            }
            kDebug(600) << "emit q.devicePlugged " << dev.cardName() << endl;
            emit q.devicePlugged(dev);
        }
    }
}

void AudioDeviceEnumeratorPrivate::_k_deviceRemoved(const QString &udi)
{
    kDebug(600) << k_funcinfo << udi << endl;
    AudioDevice dev;
    foreach (const AudioDevice &listedDev, capturedevicelist) {
        if (listedDev.udi() == udi && listedDev.isAvailable()) {
            // listedDev is the same devices as was removed
            kDebug(600) << "removing from capturedevicelist: " << listedDev.cardName() << endl;
            dev = listedDev;
            capturedevicelist.removeAll(listedDev);
            break;
        }
    }
    foreach (const AudioDevice &listedDev, playbackdevicelist) {
        if (listedDev.udi() == udi && listedDev.isAvailable()) {
            // listedDev is the same devices as was removed
            kDebug(600) << "removing from playbackdevicelist: " << listedDev.cardName() << endl;
            dev = listedDev;
            playbackdevicelist.removeAll(listedDev);
            break;
        }
    }

    if (dev.isValid()) {
        kDebug(600) << "emit q.deviceUnplugged " << dev.cardName() << endl;
        emit q.deviceUnplugged(dev);
    }
}

void AudioDeviceEnumeratorPrivate::findAsoundrcDevices(const QString &fileName)
{
    QFile asoundrcFile(fileName);
    asoundrcFile.open(QIODevice::ReadOnly);
    QTextStream asoundrcStream(&asoundrcFile);
    QString line;
    QStringList words;
    int depth = 0;
    while (!asoundrcStream.atEnd()) {
        line = asoundrcStream.readLine().simplified();
        //kDebug(600) << "'" << line << "'" << endl;
        if (line.startsWith('#')) {
            continue; //skip comment lines
        }
        if (line.contains('#')) { // truncate comments at the end of the line
            line = line.left(line.indexOf('#'));
            //kDebug(600) << "'" << line << "'" << endl;
        }
        words = line.split(' ', QString::SkipEmptyParts);
        foreach (QString word, words) {
            if (word == QLatin1String("{")) {
                ++depth;
            } else if (word == QLatin1String("}")) {
                --depth;
            } else if (depth == 0) {
                int index = word.indexOf('.');
                if (index != -1) {
                    QString type = word.left(index).toLower();
                    if (type == QLatin1String("pcm")) {
                        QString deviceName = word.right(word.size() - index - 1);
                        if (deviceName.startsWith('!')) {
                            deviceName = deviceName.right(deviceName.size() - 1);
                        }
                        AudioDevice dev(deviceName, config);
                        if (dev.isValid()) {
                            if (dev.isPlaybackDevice()) {
                                playbackdevicelist << dev;
                            }
                            if (dev.isCaptureDevice()) {
                                capturedevicelist << dev;
                            }
                        }
                    }
                }
            }
        }
    }
}

AudioDeviceEnumerator::~AudioDeviceEnumerator()
{
}

QList<AudioDevice> AudioDeviceEnumerator::availablePlaybackDevices()
{
    return audioDeviceEnumeratorPrivate->playbackdevicelist;
}

QList<AudioDevice> AudioDeviceEnumerator::availableCaptureDevices()
{
    return audioDeviceEnumeratorPrivate->capturedevicelist;
}

} // namespace Phonon
#include "audiodeviceenumerator.moc"

// vim: sw=4 sts=4 et tw=100
