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
#include "soundcardcapture.h"
#include "ifaces/soundcardcapture.h"
#include "factory.h"

namespace Kdem2m
{
class SoundcardCapture::Private
{
	public:
		Private( const QString& _type )
			: type( _type )
		{ }

		QString type;
};

SoundcardCapture::SoundcardCapture( const QString& type, QObject* parent )
	: MediaProducer( createIface( false ), parent )
	, d( new Private( type ) )
{
	setupIface();
}

SoundcardCapture::SoundcardCapture( Ifaces::MediaProducer* iface, const QString& type, QObject* parent )
	: MediaProducer( iface, parent )
	, m_iface( dynamic_cast<Ifaces::SoundcardCapture*>( iface ) )
	, d( new Private( type ) )
{
}

SoundcardCapture::~SoundcardCapture()
{
	slotDeleteIface();
	delete d;
	d = 0;
}

const QString& SoundcardCapture::type() const
{
	return d->type;
}

bool SoundcardCapture::aboutToDeleteIface()
{
	return MediaProducer::aboutToDeleteIface();
}

Ifaces::MediaProducer* SoundcardCapture::createIface( bool initialized )
{
	if( initialized )
		delete m_iface;
  	m_iface = Factory::self()->createSoundcardCapture( 0 );
	return m_iface;
}

void SoundcardCapture::setupIface()
{	MediaProducer::setupIface();

	if( !m_iface )
		return;

	// set up attributes
	m_iface->setType( d->type );
}

Ifaces::SoundcardCapture* SoundcardCapture::iface()
{
	if( !m_iface )
		slotCreateIface();
	return m_iface;
}

} //namespace Kdem2m

#include "soundcardcapture.moc"

// vim: sw=4 ts=4 tw=80 noet
