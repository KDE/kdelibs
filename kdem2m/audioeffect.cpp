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
#include "audioeffect.h"
#include "ifaces/audioeffect.h"
#include "factory.h"

namespace KDEM2M
{
class AudioEffect::Private
{
	public:
		Private( const QString& _type )
			: type( _type )
		{ }

		QString type;
};

AudioEffect::AudioEffect( const QString& type, QObject* parent )
	: QObject( parent )
	, m_iface( 0 )
	, d( new Private( type ) )
{
	slotCreateIface();
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

AudioEffect::AudioEffect( Ifaces::AudioEffect* iface, const QString& type, QObject* parent )
	: QObject( parent )
	, m_iface( iface )
	, d( new Private( type ) )
{
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

AudioEffect::~AudioEffect()
{
	slotDeleteIface();
	delete d;
	d = 0;
}

QString AudioEffect::type() const
{
	return m_iface ? m_iface->type() : d->type;
}

bool AudioEffect::aboutToDeleteIface()
{
	if( m_iface )
		d->type = m_iface->type();
	return true;
}

void AudioEffect::slotDeleteIface()
{
	if( aboutToDeleteIface() )
	{
		delete m_iface;
		m_iface = 0;
	}
}

void AudioEffect::slotCreateIface()
{
	if( !m_iface )
		m_iface = createIface();
	setupIface();
}

Ifaces::AudioEffect* AudioEffect::createIface()
{
  	delete m_iface;
  	m_iface = Factory::self()->createAudioEffect( 0 );
	return m_iface;
}

void AudioEffect::setupIface()
{	if( !m_iface )
		return;

	// set up attributes
	m_iface->setType( d->type );
}

Ifaces::AudioEffect* AudioEffect::iface()
{
	if( !m_iface )
		slotCreateIface();
	return m_iface;
}

} //namespace KDEM2M

#include "audioeffect.moc"

// vim: sw=4 ts=4 tw=80 noet
