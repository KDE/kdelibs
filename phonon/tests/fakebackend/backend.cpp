/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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

#include "backend.h"

#include "audiodataoutput.h"
#include "audiooutput.h"
#include "effect.h"
#include "mediaobject.h"
#include "videodataoutput.h"
#include "videowidget.h"
#include "visualization.h"
#include "volumefadereffect.h"

#include <QtCore/QSet>
#include <QtCore/QVariant>

#ifdef PHONON_MAKE_QT_ONLY_BACKEND
#include <QtCore/QtPlugin>
Q_EXPORT_PLUGIN2(phonon_fake, Phonon::Fake::Backend);
#else
#include <kgenericfactory.h>
typedef KGenericFactory<Phonon::Fake::Backend, Phonon::Fake::Backend> FakeBackendFactory;
K_EXPORT_COMPONENT_FACTORY(phonon_fake, FakeBackendFactory("fakebackend"))
#endif

namespace Phonon
{
namespace Fake
{

Backend::Backend(QObject *parent, const QStringList &)
    : QObject(parent)
{
    setProperty("identifier",     QLatin1String("phonon_fake"));
    setProperty("backendName",    QLatin1String("Fake"));
    setProperty("backendComment", QLatin1String("Testing Backend"));
    setProperty("backendVersion", QLatin1String("0.1"));
    setProperty("backendIcon",    QLatin1String(""));
    setProperty("backendWebsite", QLatin1String("http://multimedia.kde.org/"));
}

Backend::~Backend()
{
}

QObject *Backend::createObject(BackendInterface::Class c, QObject *parent, const QList<QVariant> &args)
{
    switch (c) {
    case MediaObjectClass:
        return new MediaObject(parent);
    case VolumeFaderEffectClass:
        return new VolumeFaderEffect(parent);
    case AudioOutputClass:
        {
            AudioOutput *ao = new AudioOutput(parent);
            m_audioOutputs.append(ao);
            return ao;
        }
    case AudioDataOutputClass:
        return new AudioDataOutput(parent);
    case VisualizationClass:
        return new Visualization(parent);
    case VideoDataOutputClass:
        return new VideoDataOutput(parent);
    case EffectClass:
        return new Effect(args[0].toInt(), parent);
    case VideoWidgetClass:
        return new VideoWidget(qobject_cast<QWidget *>(parent));
    }
    return 0;
}

bool Backend::supportsVideo() const
{
    return true;
}

bool Backend::supportsOSD() const
{
    return false;
}

bool Backend::supportsFourcc(quint32 fourcc) const
{
    switch(fourcc)
    {
    case 0x00000000:
        return true;
    default:
        return false;
    }
}

bool Backend::supportsSubtitles() const
{
    return false;
}

QStringList Backend::availableMimeTypes() const
{
    if (m_supportedMimeTypes.isEmpty())
        const_cast<Backend *>(this)->m_supportedMimeTypes
            << QLatin1String("audio/x-vorbis+ogg")
            //<< QLatin1String("audio/mpeg")
            << QLatin1String("audio/x-wav")
            << QLatin1String("video/x-ogm+ogg");
    return m_supportedMimeTypes;
}

QSet<int> Backend::objectDescriptionIndexes(ObjectDescriptionType type) const
{
    QSet<int> set;
    switch(type)
    {
    case Phonon::AudioOutputDeviceType:
        set << 10000 << 10001
            << 10002 << 10003
            << 10004 << 10005
            << 10006 << 10007
            << 10008 << 10009;
        break;
/*    case Phonon::AudioCaptureDeviceType:
        set << 20000 << 20001;
        break;
    case Phonon::VideoOutputDeviceType:
        set << 40000 << 40001 << 40002 << 40003;
        break;
    case Phonon::VideoCaptureDeviceType:
        set << 30000 << 30001;
        break;
    case Phonon::VisualizationType:
    case Phonon::AudioCodecType:
    case Phonon::VideoCodecType:
    case Phonon::ContainerFormatType:
        break;*/
    case Phonon::EffectType:
        set << 0x7F000001;
        break;
    }
    return set;
}

QHash<QByteArray, QVariant> Backend::objectDescriptionProperties(ObjectDescriptionType type, int index) const
{
    QHash<QByteArray, QVariant> ret;
    switch (type) {
    case Phonon::AudioOutputDeviceType:
        switch (index) {
        case 10000:
            ret.insert("name", QLatin1String("internal Soundcard"));
            ret.insert("icon", QLatin1String("audio-card"));
            break;
        case 10001:
            ret.insert("name", QLatin1String("USB Soundcard"));
            ret.insert("icon", QLatin1String("audio-card-usb"));
            break;
        case 10002:
            ret.insert("name", QLatin1String("Firewire Soundcard"));
            ret.insert("icon", QLatin1String("audio-card-firewire"));
            break;
        case 10003:
            ret.insert("name", QLatin1String("Headset"));
            ret.insert("icon", QLatin1String("audio-headset"));
            break;
        case 10004:
            ret.insert("name", QLatin1String("USB Headset"));
            ret.insert("icon", QLatin1String("audio-headset-usb"));
            break;
        case 10005:
            ret.insert("name", QLatin1String("Bluetooth Headset"));
            ret.insert("icon", QLatin1String("audio-headset-bluetooth"));
            break;
        case 10006:
            ret.insert("name", QLatin1String("Jack Audio Connection Kit"));
            ret.insert("icon", QLatin1String("jackd"));
            break;
        case 10007:
            ret.insert("name", QLatin1String("aRts"));
            ret.insert("icon", QLatin1String("arts"));
            break;
        case 10008:
            ret.insert("name", QLatin1String("ESD"));
            ret.insert("icon", QLatin1String("esd"));
            break;
        case 10009:
            ret.insert("name", QLatin1String("Pulseaudio"));
            ret.insert("icon", QLatin1String("pulseaudio"));
            break;
        }
        break;
/*    case Phonon::AudioCaptureDeviceType:
        switch (index) {
        case 20000:
            ret.insert("name", QLatin1String("Soundcard"));
            ret.insert("description", QLatin1String("first description"));
            break;
        case 20001:
            ret.insert("name", QLatin1String("DV"));
            ret.insert("description", QLatin1String("second description"));
            break;
        }
        break;
    case Phonon::VideoOutputDeviceType:
        switch (index) {
        case 40000:
            ret.insert("name", QLatin1String("XVideo"));
            break;
        case 40001:
            ret.insert("name", QLatin1String("XShm"));
            break;
        case 40002:
            ret.insert("name", QLatin1String("X11"));
            break;
        case 40003:
            ret.insert("name", QLatin1String("SDL"));
            break;
        }
        break;
    case Phonon::VideoCaptureDeviceType:
        switch (index) {
        case 30000:
            ret.insert("name", QLatin1String("USB Webcam"));
            ret.insert("description", QLatin1String("first description"));
            break;
        case 30001:
            ret.insert("name", QLatin1String("DV"));
            ret.insert("description", QLatin1String("second description"));
            break;
        }
        break;
    case Phonon::VisualizationType:
        break;
    case Phonon::AudioCodecType:
        break;
    case Phonon::VideoCodecType:
        break;
    case Phonon::ContainerFormatType:
        break;*/
    case Phonon::EffectType:
        switch (index) {
        case 0x7F000001:
            ret.insert("name", QLatin1String("Delay"));
            ret.insert("description", QLatin1String("Simple delay effect with time, feedback and level controls."));
            break;
        }
        break;
    }
    return ret;
}

bool Backend::startConnectionChange(QSet<QObject *> objects)
{
    bool success = true;
    foreach (QObject *o, objects) {
        MediaObject *mo = qobject_cast<MediaObject *>(o);
        if (mo) {
            success &= mo->wait();
        }
    }
    return success;
}

bool Backend::connectNodes(QObject *source, QObject *sink)
{
    MediaObject *mo = qobject_cast<MediaObject *>(source);
    AudioNode *an = qobject_cast<AudioNode *>(sink);
    if (an && !an->hasInput()) {
        if (mo)
            mo->addAudioNode(an);
        else if (Effect *ae = qobject_cast<Effect *>(source))
            ae->setAudioSink(an);
        else
            return false;
        return true;
    }
    VideoNode *vn = qobject_cast<VideoNode *>(sink);
    if (vn && !vn->hasInput()) {
        if (mo)
            mo->addVideoNode(vn);
        else
            return false;
        return true;
    }
    return false;
}

bool Backend::disconnectNodes(QObject *source, QObject *sink)
{
    MediaObject *mo = qobject_cast<MediaObject *>(source);

    AudioNode *an = qobject_cast<AudioNode *>(sink);
    if (an) {
        if (mo)
            return mo->removeAudioNode(an);
        else if (Effect *ae = qobject_cast<Effect *>(source))
            return ae->setAudioSink(0);
        else
            return false;
    }
    VideoNode *vn = qobject_cast<VideoNode *>(sink);
    if (vn && mo) {
        return mo->removeVideoNode(vn);
    }
    return false;
}

bool Backend::endConnectionChange(QSet<QObject *> objects)
{
    bool success = true;
    foreach (QObject *o, objects) {
        MediaObject *mo = qobject_cast<MediaObject *>(o);
        if (mo) {
            success &= mo->done();
        }
    }
    return success;
}

void Backend::freeSoundcardDevices()
{
    foreach (QPointer<AudioOutput> ao, m_audioOutputs)
        if (ao)
            ao->closeDevice();
}

}}

#include "moc_backend.cpp"
