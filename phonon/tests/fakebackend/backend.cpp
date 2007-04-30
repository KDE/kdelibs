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
#include "mediaobject.h"
#include "audiopath.h"
#include "audioeffect.h"
#include "audiooutput.h"
#include "audiodataoutput.h"
#include "visualization.h"
#include "videopath.h"
#include "videoeffect.h"
#include "brightnesscontrol.h"

#include <kgenericfactory.h>
#include "volumefadereffect.h"
#include <QSet>
#include "videodataoutput.h"
#include <QVariant>
#include "videowidget.h"

typedef KGenericFactory<Phonon::Fake::Backend, Phonon::Fake::Backend> FakeBackendFactory;
K_EXPORT_COMPONENT_FACTORY(phonon_fake, FakeBackendFactory("fakebackend"))

namespace Phonon
{
namespace Fake
{

Backend::Backend(QObject *parent, const QStringList &)
    : QObject(parent)
{
}

Backend::~Backend()
{
}

QObject *Backend::createObject(BackendInterface::Class c, QObject *parent, const QList<QVariant> &args)
{
    switch (c) {
    case MediaObjectClass:
        return new MediaObject(parent);
    case AudioPathClass:
        return new AudioPath(parent);
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
    case VideoPathClass:
        return new VideoPath(parent);
    case BrightnessControlClass:
        return new BrightnessControl(parent);
    case VideoDataOutputClass:
        return new VideoDataOutput(parent);
    case DeinterlaceFilterClass:
        return 0;//new DeinterlaceFilter(parent);
    case AudioEffectClass:
        return new AudioEffect(args[0].toInt(), parent);
    case VideoEffectClass:
        return new VideoEffect(args[0].toInt(), parent);
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
        set << 10000 << 10001;
        break;
    case Phonon::AudioCaptureDeviceType:
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
        break;
    case Phonon::AudioEffectType:
        set << 0x7F000001;
        break;
    case Phonon::VideoEffectType:
        set << 0x7E000001;
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
            break;
        case 10001:
            ret.insert("name", QLatin1String("USB Headset"));
            break;
        }
        break;
    case Phonon::AudioCaptureDeviceType:
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
        break;
    case Phonon::AudioEffectType:
        switch (index) {
        case 0x7F000001:
            ret.insert("name", QLatin1String("Delay"));
            ret.insert("description", QLatin1String("Simple delay effect with time, feedback and level controls."));
            break;
        }
        break;
    case Phonon::VideoEffectType:
        switch (index) {
        case 0x7E000001:
            ret.insert("name", QLatin1String("VideoEffect1"));
            ret.insert("description", QLatin1String("Description 1"));
            break;
        }
        break;
    }
    return ret;
}

void Backend::freeSoundcardDevices()
{
    foreach (QPointer<AudioOutput> ao, m_audioOutputs)
        if (ao)
            ao->closeDevice();
}

}}

#include "backend.moc"

// vim: sw=4 ts=4
