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
#include "audiopath.h"
#include "ifaces/audiopath.h"
#include "factory.h"

#include "audioeffect.h"
#include "audiooutputbase.h"

namespace KDEM2M
{
class AudioPath::Private
{
	public:
		Private()
		{ }

};

AudioPath::AudioPath( QObject* parent )
	: QObject( parent )
	, m_iface( 0 )
	, d( new Private() )
{
	slotCreateIface();
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

AudioPath::AudioPath( Ifaces::AudioPath* iface, QObject* parent )
	: QObject( parent )
	, m_iface( iface )
	, d( new Private() )
{
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

AudioPath::~AudioPath()
{
	slotDeleteIface();
	delete d;
	d = 0;
}

bool AudioPath::insertEffect( AudioEffect* newEffect, AudioEffect* insertBefore )
{
	return iface()->insertEffect( newEffect->iface(), insertBefore->iface() );
}

bool AudioPath::addOutput( AudioOutputBase* audioOutput )
{
	return iface()->addOutput( audioOutput->iface() );
}

bool AudioPath::removeOutput( AudioOutputBase* audioOutput )
{
	return iface()->removeOutput( audioOutput->iface() );
}

bool AudioPath::aboutToDeleteIface()
{
	return true;
}

void AudioPath::slotDeleteIface()
{
	if( aboutToDeleteIface() )
	{
		delete m_iface;
		m_iface = 0;
	}
}

void AudioPath::slotCreateIface()
{
	if( !m_iface )
		m_iface = createIface();
	setupIface();
}

Ifaces::AudioPath* AudioPath::createIface()
{
  	delete m_iface;
  	m_iface = Factory::self()->createAudioPath( this );
	return m_iface;
}

void AudioPath::setupIface()
{	if( !m_iface )
		return;


	// set up attributes
}

Ifaces::AudioPath* AudioPath::iface()
{
	if( !m_iface )
		slotCreateIface();
	return m_iface;
}

} //namespace KDEM2M

#include "audiopath.moc"

// vim: sw=4 ts=4 tw=80 noet
