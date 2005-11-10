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
#include "audiofftoutput.h"
#include "ifaces/audiofftoutput.h"
#include "factory.h"

namespace Kdem2m
{
class AudioFftOutput::Private
{
	public:
		Private()
		{ }

		int width;
		int rate;
		int availableFts;
};

AudioFftOutput::AudioFftOutput( QObject* parent )
	: AudioOutputBase( createIface( false ), parent )
	, d( new Private() )
{
	setupIface();
}

AudioFftOutput::AudioFftOutput( Ifaces::AudioOutputBase* iface, QObject* parent )
	: AudioOutputBase( iface, parent )
	, m_iface( dynamic_cast<Ifaces::AudioFftOutput*>( iface ) )
	, d( new Private() )
{
}

AudioFftOutput::~AudioFftOutput()
{
	slotDeleteIface();
	delete d;
	d = 0;
}

void AudioFftOutput::readFt( QVector<int>& buffer )
{
	iface()->readFt( buffer );
}

int AudioFftOutput::width() const
{
	return m_iface ? m_iface->width() : d->width;
}

void AudioFftOutput::setWidth( int newWidth )
{
	if( m_iface )
		d->width = m_iface->setWidth( newWidth );
	else
		d->width = newWidth;
}

int AudioFftOutput::rate() const
{
	return m_iface ? m_iface->rate() : d->rate;
}

void AudioFftOutput::setRate( int newRate )
{
	if( m_iface )
		d->rate = m_iface->setRate( newRate );
	else
		d->rate = newRate;
}

int AudioFftOutput::availableFts() const
{
	return m_iface ? m_iface->availableFts() : d->availableFts;
}

bool AudioFftOutput::aboutToDeleteIface()
{
	if( m_iface )
	{
		d->width = m_iface->width();
		d->rate = m_iface->rate();
		d->availableFts = m_iface->availableFts();
	}
	return AudioOutputBase::aboutToDeleteIface();
}

Ifaces::AudioOutputBase* AudioFftOutput::createIface( bool initialized )
{
	if( initialized )
		delete m_iface;
  	m_iface = Factory::self()->createAudioFftOutput( 0 );
	return m_iface;
}

void AudioFftOutput::setupIface()
{	AudioOutputBase::setupIface();

	if( !m_iface )
		return;


	// set up attributes
	m_iface->setWidth( d->width );
	m_iface->setRate( d->rate );
}

Ifaces::AudioFftOutput* AudioFftOutput::iface()
{
	if( !m_iface )
		slotCreateIface();
	return m_iface;
}

} //namespace Kdem2m

#include "audiofftoutput.moc"

// vim: sw=4 ts=4 tw=80 noet
