/*  This file is part of the KDE project
    Copyright (C) 2006-2008 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "devicelisting.h"

#include "../libkaudiodevicelist/audiodeviceenumerator.h"
#include "../libkaudiodevicelist/audiodevice.h"
#include <QtCore/QMutableMapIterator>
#include <QtCore/QTimerEvent>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <klocale.h>
#include <ksharedconfig.h>

namespace Phonon
{

QList<int> DeviceListing::objectDescriptionIndexes(Phonon::ObjectDescriptionType type)
{
    switch (type) {
    case Phonon::AudioOutputDeviceType:
        checkAudioOutputs();
        return m_sortedOutputIndexes.values();
    case Phonon::AudioCaptureDeviceType:
        checkAudioInputs();
        return m_sortedInputIndexes.values();
    default:
        Q_ASSERT(false);
        return QList<int>();
    }
}

QHash<QByteArray, QVariant> DeviceListing::objectDescriptionProperties(Phonon::ObjectDescriptionType type, int index)
{
    QHash<QByteArray, QVariant> ret;
    switch (type) {
    case Phonon::AudioOutputDeviceType:
        checkAudioOutputs();
        if (m_outputInfos.contains(index)) {
            ret = m_outputInfos.value(index);
            if (m_useOss) {
                const QVariant driver = ret.value("driver");
                if (driver == QLatin1String("oss")) {
                    ret["name"] = i18n("%1 (OSS)", ret.value("name").toString());
                } else if (driver == QLatin1String("alsa")) {
                    ret["name"] = i18n("%1 (ALSA)", ret.value("name").toString());
                }
            }
            return ret;
        }
        break;
    case Phonon::AudioCaptureDeviceType:
        checkAudioInputs();
        if (m_inputInfos.contains(index)) {
            ret = m_inputInfos.value(index);
            if (m_useOss) {
                const QVariant driver = ret.value("driver");
                if (driver == QLatin1String("oss")) {
                    ret["name"] = i18n("%1 (OSS)", ret.value("name").toString());
                } else if (driver == QLatin1String("alsa")) {
                    ret["name"] = i18n("%1 (ALSA)", ret.value("name").toString());
                }
            }
            return ret;
        }
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    ret.insert("name", QString());
    ret.insert("description", QString());
    ret.insert("available", false);
    ret.insert("initialPreference", 0);
    ret.insert("isAdvanced", false);
    return ret;
}

static QHash<QByteArray, QVariant> propertiesHashFor(const Phonon::AudioDevice &dev)
{
    QHash<QByteArray, QVariant> deviceData;
    QString mixerDevice;
    int initialPreference = dev.initialPreference();
    switch (dev.driver()) {
        case Solid::AudioInterface::Alsa:
            deviceData.insert("driver", "alsa");
            initialPreference += 100;
            // guess the associated mixer device id (we use the knowledge about how
            // libkaudiodevicelist constructs ALSA device ids
            foreach (QString id, dev.deviceIds()) {
                const int idx = id.indexOf(QLatin1String("CARD="));
                if (idx > 0) {
                    id = id.mid(idx + 5);
                    const int commaidx = id.indexOf(QLatin1Char(','));
                    if (commaidx > 0) {
                        id = id.left(commaidx);
                    }
                    deviceData.insert("mixerDeviceId", QLatin1String("hw:") + id);
                    break;
                }
                deviceData.insert("mixerDeviceId", id);
            }
            break;
        case Solid::AudioInterface::OpenSoundSystem:
            deviceData.insert("driver", "oss");
            // fall through
        case Solid::AudioInterface::UnknownAudioDriver:
            initialPreference += 50;
            if (!dev.deviceIds().isEmpty()) {
                deviceData.insert("mixerDeviceId", dev.deviceIds().first());
            }
            break;
    }
    // TODO add PulseAudio logic
    const QString description = dev.deviceIds().isEmpty() ?
        i18n("<html>This device is currently not available (either it is unplugged or the "
                "driver is not loaded).</html>") :
        i18n("<html>This will try the following devices and use the first that works: "
                "<ol><li>%1</li></ol></html>", dev.deviceIds().join("</li><li>"));
    deviceData.insert("name", dev.cardName());
    // TODO add OSS/ALSA postfix
    deviceData.insert("description", description);
    if (!dev.iconName().isEmpty()) {
        deviceData.insert("icon", dev.iconName());
    }
    deviceData.insert("available", dev.isAvailable());
    deviceData.insert("initialPreference", initialPreference);
    deviceData.insert("isAdvanced", dev.isAdvancedDevice());
    deviceData.insert("deviceIds", dev.deviceIds());
    return deviceData;
}

void DeviceListing::ossSettingChanged(bool useOss)
{
    if (useOss == m_useOss) {
        return;
    }
    m_useOss = useOss;
    if (useOss) {
        // add OSS devices
        QList<Phonon::AudioDevice> audioDevices = Phonon::AudioDeviceEnumerator::availablePlaybackDevices();
        if (!m_outputInfos.isEmpty()) {
            foreach (const Phonon::AudioDevice &dev, audioDevices) {
                if (dev.driver() == Solid::AudioInterface::OpenSoundSystem) {
                    m_outputInfos.insert(-dev.index(), propertiesHashFor(dev));
                    m_sortedOutputIndexes.insert(-m_outputInfos[-dev.index()].value("initialPreference").toInt(), -dev.index());
                }
            }
        }
    } else {
        // remove all OSS devices
        QMutableMapIterator<int, QHash<QByteArray, QVariant> > it(m_outputInfos);
        while (it.hasNext()) {
            it.next();
            if (it.value().value("driver") == QLatin1String("oss")) {
                const int initialPreference = it.value().value("initialPreference").toInt();
                m_sortedOutputIndexes.remove(-initialPreference, it.key());
                it.remove();
            }
        }
    }
    m_signalTimer.start(0, this);
}

DeviceListing::DeviceListing()
{
    KSharedConfigPtr config;
    config = KSharedConfig::openConfig("phonon_platform_kde");
    m_useOss = KConfigGroup(config, "Settings").readEntry("showOssDevices", false);

    connect(Phonon::AudioDeviceEnumerator::self(), SIGNAL(devicePlugged(const Phonon::AudioDevice &)),
            this, SLOT(devicePlugged(const Phonon::AudioDevice &)));
    connect(Phonon::AudioDeviceEnumerator::self(), SIGNAL(deviceUnplugged(const Phonon::AudioDevice &)),
            this, SLOT(deviceUnplugged(const Phonon::AudioDevice &)));
}

DeviceListing::~DeviceListing()
{
}

void DeviceListing::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_signalTimer.timerId()) {
        m_signalTimer.stop();
        kDebug(600) << "emitting objectDescriptionChanged for AudioOutputDeviceType and AudioCaptureDeviceType";
        emit objectDescriptionChanged(Phonon::AudioOutputDeviceType);
        emit objectDescriptionChanged(Phonon::AudioCaptureDeviceType);
    }
}

void DeviceListing::checkAudioOutputs()
{
    if (m_outputInfos.isEmpty()) {
        QList<Phonon::AudioDevice> playbackDevices = Phonon::AudioDeviceEnumerator::availablePlaybackDevices();
        foreach (const Phonon::AudioDevice &dev, playbackDevices) {
            if (!m_useOss && dev.driver() == Solid::AudioInterface::OpenSoundSystem) {
                continue;
            }
            m_outputInfos.insert(-dev.index(), propertiesHashFor(dev));
            m_sortedOutputIndexes.insert(-m_outputInfos[-dev.index()].value("initialPreference").toInt(), -dev.index());
        }
    }
}

void DeviceListing::checkAudioInputs()
{
    if (m_inputInfos.isEmpty()) {
        QList<Phonon::AudioDevice> captureDevices = Phonon::AudioDeviceEnumerator::availableCaptureDevices();
        foreach (const Phonon::AudioDevice &dev, captureDevices) {
            if (!m_useOss && dev.driver() == Solid::AudioInterface::OpenSoundSystem) {
                continue;
            }
            m_inputInfos.insert(-dev.index(), propertiesHashFor(dev));
            m_sortedInputIndexes.insert(-m_inputInfos[-dev.index()].value("initialPreference").toInt(), -dev.index());
        }
    }
}

void DeviceListing::devicePlugged(const Phonon::AudioDevice &dev)
{
    kDebug(600) << dev.cardName();
    if (dev.isPlaybackDevice()) {
        m_outputInfos.insert(-dev.index(), propertiesHashFor(dev));
        m_sortedOutputIndexes.insert(-m_outputInfos[-dev.index()].value("initialPreference").toInt(), -dev.index());
        m_signalTimer.start(0, this);
    }
    if (dev.isCaptureDevice()) {
        m_inputInfos.insert(-dev.index(), propertiesHashFor(dev));
        m_sortedInputIndexes.insert(-m_inputInfos[-dev.index()].value("initialPreference").toInt(), -dev.index());
        m_signalTimer.start(0, this);
    }
}

void DeviceListing::deviceUnplugged(const Phonon::AudioDevice &dev)
{
    kDebug(600) << dev.cardName();
    if (dev.isPlaybackDevice()) {
        QMap<int, QHash<QByteArray, QVariant> >::iterator it = m_outputInfos.find(-dev.index());
        if (it != m_outputInfos.end()) {
            const int initialPreference = it.value().value("initialPreference").toInt();
            m_sortedOutputIndexes.remove(-initialPreference, it.key());
            m_outputInfos.erase(it);
        }
        m_signalTimer.start(0, this);
    }
    if (dev.isCaptureDevice()) {
        QMap<int, QHash<QByteArray, QVariant> >::iterator it = m_inputInfos.find(-dev.index());
        if (it != m_inputInfos.end()) {
            const int initialPreference = it.value().value("initialPreference").toInt();
            m_sortedInputIndexes.remove(-initialPreference, it.key());
            m_inputInfos.erase(it);
        }
        m_signalTimer.start(0, this);
    }
}

} // namespace Phonon
