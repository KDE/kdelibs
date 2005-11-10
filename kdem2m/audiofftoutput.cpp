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

namespace KDEM2M
{
class AudioFFTOutput::Private
{
	public:
		Private()
		{ }

		int width;
		int rate;
		int availableFTs;
};

AudioFFTOutput::AudioFFTOutput( QObject* parent )
	: AudioOutputBase( createIface( false ), parent )
	, d( new Private() )
{
	setupIface();
}

AudioFFTOutput::AudioFFTOutput( Ifaces::AudioOutputBase* iface, QObject* parent )
	: AudioOutputBase( iface, parent )
	, m_iface( dynamic_cast<Ifaces::AudioFFTOutput*>( iface ) )
	, d( new Private() )
{
}

AudioFFTOutput::~AudioFFTOutput()
{
	slotDeleteIface();
	delete d;
	d = 0;
}

void AudioFFTOutput::readFT( QVector<int>& buffer )
{
	iface()->readFT( buffer );
}

int AudioFFTOutput::width() const
{
	return m_iface ? m_iface->width() : d->width;
}

void AudioFFTOutput::setWidth( int newWidth )
{
	if( m_iface )
		d->width = m_iface->setWidth( newWidth );
	else
		d->width = newWidth;
}

int AudioFFTOutput::rate() const
{
	return m_iface ? m_iface->rate() : d->rate;
}

void AudioFFTOutput::setRate( int newRate )
{
	if( m_iface )
		d->rate = m_iface->setRate( newRate );
	else
		d->rate = newRate;
}

int AudioFFTOutput::availableFTs() const
{
	return m_iface ? m_iface->availableFTs() : d->availableFTs;
}

bool AudioFFTOutput::aboutToDeleteIface()
{
	if( m_iface )
	{
		d->width = m_iface->width();
		d->rate = m_iface->rate();
		d->availableFTs = m_iface->availableFTs();
	}
	return AudioOutputBase::aboutToDeleteIface();
}

Ifaces::AudioOutputBase* AudioFFTOutput::createIface( bool initialized )
{
	if( initialized )
		delete m_iface;
  	m_iface = Factory::self()->createAudioFFTOutput( 0 );
	return m_iface;
}

void AudioFFTOutput::setupIface()
{	AudioOutputBase::setupIface();

	if( !m_iface )
		return;


	// set up attributes
	m_iface->setWidth( d->width );
	m_iface->setRate( d->rate );
}

Ifaces::AudioFFTOutput* AudioFFTOutput::iface()
{
	if( !m_iface )
		slotCreateIface();
	return m_iface;
}

} //namespace KDEM2M

#include "audiofftoutput.moc"

// vim: sw=4 ts=4 tw=80 noet
