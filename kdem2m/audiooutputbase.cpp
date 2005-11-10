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
#include "audiooutputbase.h"
#include "ifaces/audiooutputbase.h"
#include "factory.h"

namespace Kdem2m
{
class AudioOutputBase::Private
{
	public:
		Private()
		{ }

};

AudioOutputBase::AudioOutputBase( QObject* parent )
	: QObject( parent )
	, m_iface( 0 )
	, d( new Private() )
{
	slotCreateIface();
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

AudioOutputBase::AudioOutputBase( Ifaces::AudioOutputBase* iface, QObject* parent )
	: QObject( parent )
	, m_iface( iface )
	, d( new Private() )
{
	connect( Factory::self(), SIGNAL( deleteYourObjects() ), SLOT( slotDeleteIface() ) );
	connect( Factory::self(), SIGNAL( recreateObjects() ), SLOT( slotCreateIface() ) );
}

AudioOutputBase::~AudioOutputBase()
{
	slotDeleteIface();
	delete d;
	d = 0;
}

bool AudioOutputBase::aboutToDeleteIface()
{
	return true;
}

void AudioOutputBase::slotDeleteIface()
{
	if( aboutToDeleteIface() )
	{
		delete m_iface;
		m_iface = 0;
	}
}

void AudioOutputBase::slotCreateIface()
{
	if( !m_iface )
		m_iface = createIface();
	setupIface();
}

void AudioOutputBase::setupIface()
{	if( !m_iface )
		return;


	// set up attributes
}

Ifaces::AudioOutputBase* AudioOutputBase::iface()
{
	if( !m_iface )
		slotCreateIface();
	return m_iface;
}

} //namespace Kdem2m

#include "audiooutputbase.moc"

// vim: sw=4 ts=4 tw=80 noet
