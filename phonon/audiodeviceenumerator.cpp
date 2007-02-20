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
#include <kdebug.h>
#include <QDir>
#include <solid/devicemanager.h>
#include <solid/device.h>
#include <solid/audiohw.h>
#include <kconfiggroup.h>

namespace Phonon
{

K_GLOBAL_STATIC(AudioDeviceEnumeratorPrivate, audioDeviceEnumeratorPrivate)

AudioDeviceEnumeratorPrivate::AudioDeviceEnumeratorPrivate()
{
    q.d = this;
    config = KSharedConfig::openConfig("phonondevicesrc", KConfig::NoGlobals);
    findDevices();
    QObject::connect(&Solid::DeviceManager::self(), SIGNAL(deviceAdded(const QString &)), &q, SLOT(_k_deviceAdded(const QString &)));
    QObject::connect(&Solid::DeviceManager::self(), SIGNAL(deviceRemoved(const QString &)), &q, SLOT(_k_deviceRemoved(const QString &)));
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

AudioDeviceEnumerator::AudioDeviceEnumerator(QObject *parent)
    : QObject(parent)
{
}

void AudioDeviceEnumeratorPrivate::findDevices()
{
    /*
    // first check the 'default' device and the devices defined in ~/.asoundrc and /etc/asound.conf
    AudioDevice defaultCtlDevice(QLatin1String("default"), AudioDevice::ControlAndPcm);
    if (defaultCtlDevice.isValid()) {
        devicelist << defaultCtlDevice;
    }
    findAsoundrcDevices(QDir::homePath() + "/.asoundrc");
    findAsoundrcDevices("/etc/asound.conf");
    */

    // ask Solid for the available audio hardware
    Solid::DeviceManager &manager = Solid::DeviceManager::self();

    QSet<QString> alreadyFoundCards;

    Solid::DeviceList devices = manager.findDevicesFromQuery("AudioHw.deviceType & 'AudioInput|AudioOutput'");
    foreach (Solid::Device device, devices) {
        Solid::AudioHw *audiohw = device.as<Solid::AudioHw>();
        Q_ASSERT(audiohw);
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
}

void AudioDeviceEnumeratorPrivate::_k_deviceAdded(const QString &udi)
{
    kDebug(603) << k_funcinfo << udi << endl;
    Solid::DeviceManager &manager = Solid::DeviceManager::self();
    Solid::Device _device = manager.findDevice(udi);
    Solid::AudioHw *audiohw = _device.as<Solid::AudioHw>();
    if (audiohw && (audiohw->deviceType() & (Solid::AudioHw::AudioInput | Solid::AudioHw::AudioOutput))) {
        // an audio i/o device was plugged in
        AudioDevice dev(_device, config);
        if (dev.isValid()) {
            if (dev.isCaptureDevice()) {
                foreach (const AudioDevice &listedDev, capturedevicelist) {
                    if (listedDev == dev && !listedDev.isAvailable()) {
                        // listedDev is the same devices as dev but shown as unavailable
                        kDebug(603) << "removing from capturedevicelist: " << listedDev.cardName() << endl;
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
                        kDebug(603) << "removing from playbackdevicelist: " << listedDev.cardName() << endl;
                        playbackdevicelist.removeAll(listedDev);
                        break;
                    }
                }
                playbackdevicelist << dev;
            }
            kDebug(603) << "emit q.devicePlugged " << dev.cardName() << endl;
            emit q.devicePlugged(dev);
        }
    }
}

void AudioDeviceEnumeratorPrivate::_k_deviceRemoved(const QString &udi)
{
    kDebug(603) << k_funcinfo << udi << endl;
    AudioDevice dev;
    foreach (const AudioDevice &listedDev, capturedevicelist) {
        if (listedDev.udi() == udi && listedDev.isAvailable()) {
            // listedDev is the same devices as was removed
            kDebug(603) << "removing from capturedevicelist: " << listedDev.cardName() << endl;
            dev = listedDev;
            capturedevicelist.removeAll(listedDev);
            break;
        }
    }
    foreach (const AudioDevice &listedDev, playbackdevicelist) {
        if (listedDev.udi() == udi && listedDev.isAvailable()) {
            // listedDev is the same devices as was removed
            kDebug(603) << "removing from playbackdevicelist: " << listedDev.cardName() << endl;
            dev = listedDev;
            playbackdevicelist.removeAll(listedDev);
            break;
        }
    }

    if (dev.isValid()) {
        kDebug(603) << "emit q.deviceUnplugged " << dev.cardName() << endl;
        emit q.deviceUnplugged(dev);
    }
}

/*
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
        //kDebug(603) << "'" << line << "'" << endl;
        if (line.startsWith('#')) {
            continue; //skip comment lines
        }
        if (line.contains('#')) { // truncate comments at the end of the line
            line = line.left(line.indexOf('#'));
            //kDebug(603) << "'" << line << "'" << endl;
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
                    if (type == QLatin1String("ctl")) {
                        QString deviceName = word.right(word.size() - index - 1);
                        if (deviceName.startsWith('!')) {
                            deviceName = deviceName.right(deviceName.size() - 1);
                        }
                        AudioDevice dev(deviceName, AudioDevice::Control);
                        if (dev.isValid()) {
                            devicelist << dev;
                        }
                    } else if (type == QLatin1String("pcm")) {
                        QString deviceName = word.right(word.size() - index - 1);
                        if (deviceName.startsWith('!')) {
                            deviceName = deviceName.right(deviceName.size() - 1);
                        }
                        AudioDevice dev(deviceName, AudioDevice::Pcm);
                        if (dev.isValid()) {
                            devicelist << dev;
                        }
                    }
                }
            }
        }
    }
}
*/

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
