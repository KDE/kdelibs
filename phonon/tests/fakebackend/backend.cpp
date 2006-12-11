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

#include "backend.h"
#include "mediaobject.h"
#include "mediaqueue.h"
#include "avcapture.h"
#include "bytestream.h"
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

typedef KGenericFactory<Phonon::Fake::Backend, Phonon::Fake::Backend> FakeBackendFactory;
K_EXPORT_COMPONENT_FACTORY( phonon_fake, FakeBackendFactory( "fakebackend" ) )

namespace Phonon
{
namespace Fake
{

Backend::Backend( QObject* parent, const QStringList& )
	: QObject( parent )
{
}

Backend::~Backend()
{
}

QObject* Backend::createObject0(BackendInterface::Class0 c, QObject *parent)
{
    switch (c) {
        case MediaObjectClass:
            return new MediaObject(parent);
        case MediaQueueClass:
            return new MediaQueue(parent);
        case AvCaptureClass:
            return new AvCapture(parent);
        case ByteStreamClass:
            return new ByteStream(parent);
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
    }
    return 0;
}

QObject* Backend::createObject1(BackendInterface::Class1 c, QObject *parent, QVariant arg1)
{
    switch (c) {
        case AudioEffectClass:
            return new AudioEffect(arg1.toInt(), parent);
        case VideoEffectClass:
            return new VideoEffect(arg1.toInt(), parent);
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

bool Backend::supportsFourcc( quint32 fourcc ) const
{
	switch( fourcc )
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

QStringList Backend::knownMimeTypes() const
{
	if( m_supportedMimeTypes.isEmpty() )
		const_cast<Backend*>( this )->m_supportedMimeTypes
			<< QLatin1String( "audio/vorbis" )
			//<< QLatin1String( "audio/x-mp3" )
			<< QLatin1String( "audio/x-wav" )
			<< QLatin1String( "video/x-ogm" );
	return m_supportedMimeTypes;
}

QSet<int> Backend::objectDescriptionIndexes( ObjectDescriptionType type ) const
{
	QSet<int> set;
	switch( type )
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

QString Backend::objectDescriptionName( ObjectDescriptionType type, int index ) const
{
	switch( type )
	{
		case Phonon::AudioOutputDeviceType:
			switch( index )
			{
				case 10000:
					return "internal Soundcard";
				case 10001:
					return "USB Headset";
			}
			break;
		case Phonon::AudioCaptureDeviceType:
			switch( index )
			{
				case 20000:
					return "Soundcard";
				case 20001:
					return "DV";
			}
			break;
		case Phonon::VideoOutputDeviceType:
			switch( index )
			{
				case 40000:
					return "XVideo";
				case 40001:
					return "XShm";
				case 40002:
					return "X11";
				case 40003:
					return "SDL";
			}
			break;
		case Phonon::VideoCaptureDeviceType:
			switch( index )
			{
				case 30000:
					return "USB Webcam";
				case 30001:
					return "DV";
				default:
					return QString();
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
			switch( index )
			{
				case 0x7F000001:
					return "Delay";
			}
			break;
		case Phonon::VideoEffectType:
			switch( index )
			{
				case 0x7E000001:
					return "VideoEffect1";
			}
			break;
	}
	return QString();
}

QString Backend::objectDescriptionDescription( ObjectDescriptionType type, int index ) const
{
	switch( type )
	{
		case Phonon::AudioOutputDeviceType:
			break;
		case Phonon::AudioCaptureDeviceType:
			switch( index )
			{
				case 20000:
					return "first description";
				case 20001:
					return "second description";
			}
			break;
		case Phonon::VideoOutputDeviceType:
			break;
		case Phonon::VideoCaptureDeviceType:
			switch( index )
			{
				case 30000:
					return "first description";
				case 30001:
					return "second description";
				default:
					return QString();
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
			switch( index )
			{
				case 0x7F000001:
					return "Simple delay effect with time, feedback and level controls.";
			}
			break;
		case Phonon::VideoEffectType:
			switch( index )
			{
				case 0x7E000001:
					return "Description 1";
			}
			break;
	}
	return QString();
}

char const* Backend::uiLibrary() const
{
	return "phonon_fakeui";
}

void Backend::freeSoundcardDevices()
{
	foreach( QPointer<AudioOutput> ao, m_audioOutputs )
		if( ao )
			ao->closeDevice();
}

}}

#include "backend.moc"

// vim: sw=4 ts=4
