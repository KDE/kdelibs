/*  This file is part of the KDE project
    Copyright (C) 2005 Matthias Kretz <kretz@kde.org>

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
#include "ifaces/backend.h"
#include "factory.h"
#include "audiosource.h"
#include "videosource.h"

static KStaticDeleter<Phonon::BackendCapabilities> sd;

namespace Phonon
{

class BackendCapabilities::Private
{
	public:
		const Ifaces::Backend* backend;
};

BackendCapabilities* BackendCapabilities::m_self = 0;

BackendCapabilities* BackendCapabilities::self()
{
	if( !m_self )
	{
		m_self = new BackendCapabilities();
		::sd.setObject( m_self, m_self );
	}
	return m_self;
}

BackendCapabilities::BackendCapabilities()
	: d( new Private() )
{
	d->backend = Factory::self()->backend();
	connect( Factory::self(), SIGNAL( backendChanged() ), SLOT( slotBackendChanged() ) );
}

BackendCapabilities::~BackendCapabilities()
{
}

bool BackendCapabilities::supportsVideo() const
{
	return d->backend ? d->backend->supportsVideo() : false;
}

bool BackendCapabilities::supportsOSD() const
{
	return d->backend ? d->backend->supportsOSD() : false;
}

bool BackendCapabilities::supportsSubtitles() const
{
	return d->backend ? d->backend->supportsSubtitles() : false;
}

KMimeType::List BackendCapabilities::knownMimeTypes() const
{
	return d->backend ? d->backend->knownMimeTypes() : KMimeType::List();
}

QList<AudioSource> BackendCapabilities::availableAudioSources() const
{
	if( d->backend )
	{
		QList<AudioSource> ret;
		for( int i = 1; i <= d->backend->audioSourceCount(); ++i )
			ret.append( AudioSource( i,
						d->backend->audioSourceName( i ),
						d->backend->audioSourceDescription( i ),
						d->backend->audioSourceVideoIndex( i ) ) );
		return ret;
	}
	return QList<AudioSource>();
}

QList<VideoSource> BackendCapabilities::availableVideoSources() const
{
	if( d->backend )
	{
		QList<VideoSource> ret;
		for( int i = 1; i <= d->backend->videoSourceCount(); ++i )
			ret.append( VideoSource( i,
						d->backend->videoSourceName( i ),
						d->backend->videoSourceDescription( i ),
						d->backend->videoSourceAudioIndex( i ) ) );
		return ret;
	}
	return QList<VideoSource>();
}

QStringList BackendCapabilities::availableAudioEffects() const
{
	return d->backend ? d->backend->availableAudioEffects() : QStringList();
}

QStringList BackendCapabilities::availableVideoEffects() const
{
	return d->backend ? d->backend->availableVideoEffects() : QStringList();
}

void BackendCapabilities::slotBackendChanged()
{
	d->backend = Factory::self()->backend();
	emit capabilitiesChanged();
}

} // namespace Phonon
#include "backendcapabilities.moc"
// vim: sw=4 ts=4 noet
