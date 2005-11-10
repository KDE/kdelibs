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

namespace Kdem2m
{

class BackendCapabilities::Private
{
	public:
		const Ifaces::Backend* backend;
};

static ::KStaticDeleter<BackendCapabilities> sd;

BackendCapabilities* BackendCapabilities::m_self = 0;

BackendCapabilities* BackendCapabilities::self()
{
	if( !m_self )
		sd.setObject( m_self, m_self = new BackendCapabilities() );
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

QStringList BackendCapabilities::availableSoundcardCaptureTypes() const
{
	return d->backend ? d->backend->availableSoundcardCaptureTypes() : QStringList();
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
	emit capabilitesChanged();
}

} // namespace Kdem2m
#include "backendcapabilities.moc"
// vim: sw=4 ts=4 noet
