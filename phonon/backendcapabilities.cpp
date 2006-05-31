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

#include "backendcapabilities.h"
#include "factory.h"
#include "audiooutputdevice.h"
#include "videooutputdevice.h"
#include "audiocapturedevice.h"
#include "videocapturedevice.h"
#include "visualizationeffect.h"
#include "audioeffectdescription.h"
#include "videoeffectdescription.h"
#include "audiocodec.h"
#include "videocodec.h"
#include "containerformat.h"
#include <QList>
#include <QSet>
#include <QStringList>
#include <kservicetypetrader.h>
#include <kmimetype.h>
#include "phonondefs.h"

static KStaticDeleter<Phonon::BackendCapabilities> sd;

namespace Phonon
{

class BackendCapabilities::Private
{
};

BackendCapabilities* BackendCapabilities::m_self = 0;

BackendCapabilities* BackendCapabilities::self()
{
	if( !m_self )
		::sd.setObject( m_self, new BackendCapabilities() );
	return m_self;
}

BackendCapabilities::BackendCapabilities()
	: d( 0 ) //when changing this also uncomment the delete in the dtor
{
	m_self = this;
	connect( Factory::self(), SIGNAL( backendChanged() ), SLOT( slotBackendChanged() ) );
}

BackendCapabilities::~BackendCapabilities()
{
	// delete d;
	// d = 0;
}

#define SUPPORTS( foo ) \
bool BackendCapabilities::supports ## foo() \
{ \
	QObject* backendObject = Factory::self()->backend(); \
	if( !backendObject ) \
		return false; \
	bool ret; \
	pBACKEND_GET( bool, ret, "supports"#foo ); \
	return ret; \
}

SUPPORTS( Video )
SUPPORTS( OSD )
SUPPORTS( Subtitles )

QStringList BackendCapabilities::knownMimeTypes()
{
	QObject* backendObject = Factory::self()->backend( false );
	if( backendObject )
	{
		QStringList ret;
		pBACKEND_GET( QStringList, ret, "knownMimeTypes" );
		return ret;
	}
	else
	{
		const KService::List offers = KServiceTypeTrader::self()->query( "PhononBackend",
				"Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1" );
		if ( !offers.isEmpty() ) {
			QStringList mimeTypes = offers.first()->serviceTypes();
			mimeTypes.removeAll( "PhononBackend" );
			return mimeTypes;
		}
		return QStringList();
	}
}

bool BackendCapabilities::isMimeTypeKnown( QString mimeType )
{
	QObject* backendObject = Factory::self()->backend( false );
	if( backendObject )
	{
		QStringList ret;
		pBACKEND_GET( QStringList, ret, "knownMimeTypes" );
		return ret.contains( mimeType );
	}
	else
	{
		const KService::List offers = KServiceTypeTrader::self()->query( "PhononBackend",
				"Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1" );
		if ( !offers.isEmpty() )
			return offers.first()->hasMimeType( KMimeType::mimeType(mimeType).data() );
		return false;
	}
}

#define availableDevicesImpl( classname, indexesMethod ) \
QList<classname> BackendCapabilities::available ## classname ## s() \
{ \
	QObject* backendObject = Factory::self()->backend(); \
	QList<classname> ret; \
	if( backendObject ) \
	{ \
		QSet<int> deviceIndexes; \
		pBACKEND_GET( QSet<int>, deviceIndexes, #indexesMethod ); \
		foreach( int i, deviceIndexes ) \
			ret.append( classname::fromIndex( i ) ); \
	} \
	return ret; \
}
availableDevicesImpl( AudioOutputDevice, audioOutputDeviceIndexes )
availableDevicesImpl( AudioCaptureDevice, audioCaptureDeviceIndexes )
availableDevicesImpl( VideoOutputDevice, videoOutputDeviceIndexes )
availableDevicesImpl( VideoCaptureDevice, videoCaptureDeviceIndexes )
availableDevicesImpl( VisualizationEffect, visualizationIndexes )
availableDevicesImpl( AudioCodec, audioCodecIndexes )
availableDevicesImpl( VideoCodec, videoCodecIndexes )
availableDevicesImpl( ContainerFormat, containerFormatIndexes )

QList<AudioEffectDescription> BackendCapabilities::availableAudioEffects()
{
	QObject* backendObject = Factory::self()->backend();
	QList<AudioEffectDescription> ret;
	if( backendObject )
	{
		QSet<int> deviceIndexes;
		pBACKEND_GET( QSet<int>, deviceIndexes, "audioEffectIndexes" );
		foreach( int i, deviceIndexes )
			ret.append( AudioEffectDescription::fromIndex( i ) );
	}
	return ret;
}

QList<VideoEffectDescription> BackendCapabilities::availableVideoEffects()
{
	QObject* backendObject = Factory::self()->backend();
	QList<VideoEffectDescription> ret;
	if( backendObject )
	{
		QSet<int> deviceIndexes;
		pBACKEND_GET( QSet<int>, deviceIndexes, "videoEffectIndexes" );
		foreach( int i, deviceIndexes )
			ret.append( VideoEffectDescription::fromIndex( i ) );
	}
	return ret;
}

void BackendCapabilities::slotBackendChanged()
{
	emit capabilitiesChanged();
}

} // namespace Phonon
#include "backendcapabilities.moc"
// vim: sw=4 ts=4 noet
