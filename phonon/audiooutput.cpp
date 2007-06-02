/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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
#include "audiooutput.h"
#include "audiooutput_p.h"
#include "factory.h"
#include "objectdescription.h"
#include "audiooutputadaptor.h"
#include "globalconfig.h"
#include "audiooutputinterface.h"
#include "phononnamespace_p.h"

#include <cmath>

#define PHONON_CLASSNAME AudioOutput
#define PHONON_INTERFACENAME AudioOutputInterface

namespace Phonon
{
AudioOutput::AudioOutput(Phonon::Category category, QObject *parent)
    : AbstractAudioOutput(*new AudioOutputPrivate, parent)
{
    K_D(AudioOutput);
    d->category = category;

    // select hardware device according to the category
    d->outputDeviceIndex = GlobalConfig().audioOutputDeviceFor(d->category);

    d->createBackendObject();
    new AudioOutputAdaptor(this);
    for (int i = 0; !QDBusConnection::sessionBus().registerObject("/AudioOutputs/" + QString::number(i), this); ++i);

    connect(Factory::sender(), SIGNAL(availableAudioOutputDevicesChanged()), SLOT(_k_deviceListChanged()));
}

void AudioOutputPrivate::createBackendObject()
{
    if (m_backendObject)
        return;
    Q_Q(AudioOutput);
    m_backendObject = Factory::createAudioOutput(q);
    if (m_backendObject) {
        setupBackendObject();
    }
}

QString AudioOutput::name() const
{
    K_D(const AudioOutput);
    return d->name;
}

void AudioOutput::setName(const QString &newName)
{
    K_D(AudioOutput);
    d->name = newName;
}

void AudioOutput::setVolume(qreal volume)
{
    K_D(AudioOutput);
    d->volume = volume;
    if (k_ptr->backendObject() && !d->muted) {
        // using Stevens' power law loudness is proportional to (sound pressure)^0.67
        // sound pressure is proportional to voltage:
        // p² \prop P \prop V²
        // => if a factor for loudness of x is requested
        INTERFACE_CALL(setVolume(pow(volume, 1.4925373)));
    } else {
        emit volumeChanged(volume);
    }
}

qreal AudioOutput::volume() const
{
    K_D(const AudioOutput);
    if (d->muted || !d->m_backendObject) {
        return d->volume;
    }
    return pow(INTERFACE_CALL(volume()), 0.67);
}

#ifndef PHONON_LOG10OVER20
#define PHONON_LOG10OVER20
static const qreal log10over20 = 0.1151292546497022842; // ln(10) / 20
#endif // PHONON_LOG10OVER20

qreal AudioOutput::volumeDecibel() const
{
    K_D(const AudioOutput);
    if (d->muted || !d->m_backendObject) {
        return -log(d->volume) / log10over20;
    }
    return -0.67 * log(INTERFACE_CALL(volume())) / log10over20;
}

void AudioOutput::setVolumeDecibel(qreal newVolumeDecibel)
{
    setVolume(exp(-newVolumeDecibel * log10over20));
}

bool AudioOutput::isMuted() const
{
    K_D(const AudioOutput);
    return d->muted;
}

void AudioOutput::setMuted(bool mute)
{
    K_D(AudioOutput);
    if (d->muted != mute) {
        if (mute) {
            d->muted = mute;
            if (k_ptr->backendObject()) {
                INTERFACE_CALL(setVolume(0.0));
            }
        } else {
            if (k_ptr->backendObject()) {
                INTERFACE_CALL(setVolume(pow(d->volume, 1.4925373)));
            }
            d->muted = mute;
        }
        emit mutedChanged(mute);
    }
}

Category AudioOutput::category() const
{
    K_D(const AudioOutput);
    return d->category;
}

AudioOutputDevice AudioOutput::outputDevice() const
{
    K_D(const AudioOutput);
    int index;
    if (d->m_backendObject)
        index = INTERFACE_CALL(outputDevice());
    else
        index = d->outputDeviceIndex;
    return AudioOutputDevice::fromIndex(index);
}

bool AudioOutput::setOutputDevice(const AudioOutputDevice &newAudioOutputDevice)
{
    K_D(AudioOutput);
    if (!newAudioOutputDevice.isValid()) {
        d->outputDeviceOverridden = false;
        d->outputDeviceIndex = GlobalConfig().audioOutputDeviceFor(d->category);
    } else {
        d->outputDeviceOverridden = true;
        d->outputDeviceIndex = newAudioOutputDevice.index();
    }
    if (k_ptr->backendObject()) {
        return INTERFACE_CALL(setOutputDevice(d->outputDeviceIndex));
    }
    return true;
}

bool AudioOutputPrivate::aboutToDeleteBackendObject()
{
    if (m_backendObject) {
        volume = pINTERFACE_CALL(volume());
    }
    return AbstractAudioOutputPrivate::aboutToDeleteBackendObject();
}

void AudioOutputPrivate::setupBackendObject()
{
    Q_Q(AudioOutput);
    Q_ASSERT(m_backendObject);
    AbstractAudioOutputPrivate::setupBackendObject();

    QObject::connect(m_backendObject, SIGNAL(volumeChanged(qreal)), q, SLOT(_k_volumeChanged(qreal)));
    QObject::connect(m_backendObject, SIGNAL(audioDeviceFailed()), q, SLOT(_k_audioDeviceFailed()));

    // set up attributes
    pINTERFACE_CALL(setVolume(volume));

    // if the output device is not available and the device was not explicitly set
    if (!pINTERFACE_CALL(setOutputDevice(outputDeviceIndex)) && !outputDeviceOverridden) {
        // fall back in the preference list of output devices
        QList<int> deviceList = GlobalConfig().audioOutputDeviceListFor(category);
        if (deviceList.isEmpty()) {
            return;
        }
        if (outputDeviceIndex == deviceList.takeFirst()) { // removing the first device so that
            // if it's the same device as the one we tried we only try all the following
            foreach (int devIndex, deviceList) {
                if (pINTERFACE_CALL(setOutputDevice(devIndex))) {
                    handleAutomaticDeviceChange(devIndex, AudioOutputPrivate::FallbackChange);
                    break; // found one that works
                }
            }
        }
    }
}

void AudioOutputPrivate::_k_volumeChanged(qreal newVolume)
{
    if (!muted) {
        Q_Q(AudioOutput);
        emit q->volumeChanged(pow(newVolume, 0.67));
    }
}

void AudioOutputPrivate::_k_revertFallback()
{
    if (deviceBeforeFallback == -1) {
        return;
    }
    outputDeviceIndex = deviceBeforeFallback;
    pINTERFACE_CALL(setOutputDevice(outputDeviceIndex));
    Q_Q(AudioOutput);
    emit q->outputDeviceChanged(AudioOutputDevice::fromIndex(outputDeviceIndex));
}

void AudioOutputPrivate::_k_audioDeviceFailed()
{
    pDebug() << Q_FUNC_INFO;
    // outputDeviceIndex identifies a failing device
    // fall back in the preference list of output devices
    QList<int> deviceList = GlobalConfig().audioOutputDeviceListFor(category);
    foreach (int devIndex, deviceList) {
        // if it's the same device as the one that failed, ignore it
        if (outputDeviceIndex != devIndex) {
            if (pINTERFACE_CALL(setOutputDevice(devIndex))) {
                handleAutomaticDeviceChange(devIndex, FallbackChange);
                break; // found one that works
            }
        }
    }
}

void AudioOutputPrivate::_k_deviceListChanged()
{
    pDebug() << Q_FUNC_INFO;
    // let's see if there's a usable device higher in the preference list
    QList<int> deviceList = GlobalConfig().audioOutputDeviceListFor(category);
    foreach (int devIndex, deviceList) {
        pDebug() << devIndex;
        if (outputDeviceIndex == devIndex) {
            break; // we've reached the currently used device, nothing to change
        }
        if (pINTERFACE_CALL(setOutputDevice(devIndex))) {
            handleAutomaticDeviceChange(devIndex, HigherPreferenceChange);
            break; // found one with higher preference that works
        }
    }
}

void AudioOutputPrivate::handleAutomaticDeviceChange(int newIndex, DeviceChangeType type)
{
    Q_Q(AudioOutput);
    deviceBeforeFallback = outputDeviceIndex;
    outputDeviceIndex = newIndex;
    emit q->outputDeviceChanged(AudioOutputDevice::fromIndex(outputDeviceIndex));
    QString text;
    AudioOutputDevice device1 = AudioOutputDevice::fromIndex(deviceBeforeFallback);
    AudioOutputDevice device2 = AudioOutputDevice::fromIndex(outputDeviceIndex);
    switch (type) {
    case FallbackChange:
        text = AudioOutput::tr("<html>The audio playback device <b>%1</b> does not work.<br/>"
            "Falling back to <b>%2</b>.</html>").arg(device1.name()).arg(device2.name());
        break;
    case HigherPreferenceChange:
        text = AudioOutput::tr("<html>Switching to the audio playback device <b>%1</b><br/>"
                "which just became available and has higher preference.</html>").arg(device2.name());
        break;
    }
    Factory::notification("AudioDeviceFallback", text,
            QStringList(AudioOutput::tr("Revert back to device '%1'").arg(device1.name())),
            q, SLOT(_k_revertFallback()));
}

} //namespace Phonon

#include "audiooutput.moc"

#undef PHONON_CLASSNAME
#undef PHONON_INTERFACENAME
// vim: sw=4 ts=4 tw=100 et
